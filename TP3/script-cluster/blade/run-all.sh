#!/bin/bash
set -e

SCRIPT_DIR=$(dirname "$(realpath "$0")")
cd "$SCRIPT_DIR" || exit 1

BASE_DIR=$(realpath "$SCRIPT_DIR")
BIN_DIR=$(realpath "$SCRIPT_DIR/../bin")

# =============================================================================
# Orquestador interactivo: Blade (Cluster Multicore)
# Usa paths absolutos para evitar depender del directorio de trabajo de SLURM.
# =============================================================================

function enviar_mpi() {
    local P=$1
    local N=$2
    local TPN=$3
    for SIZE in 512 1024 2048 4096; do
        OUTDIR="mpi-P${P}-N${N}/N${SIZE}"
        mkdir -p "$BASE_DIR/$OUTDIR"
        JOBFILE="$BASE_DIR/jobs/mpi-P${P}-N${N}-N${SIZE}.sh"
        cat > "$JOBFILE" <<EOF
#!/bin/bash
#SBATCH -N $N
#SBATCH --exclusive
#SBATCH --tasks-per-node=$TPN
#SBATCH -o $BASE_DIR/$OUTDIR/output_%j.txt
#SBATCH -e $BASE_DIR/$OUTDIR/errors_%j.txt
mpirun $BIN_DIR/mpi-sec $SIZE $P
EOF
        chmod +x "$JOBFILE"
        echo "  [Enviando] $JOBFILE (size=$SIZE)"
        sbatch "$JOBFILE"
    done
}

function enviar_hybrid() {
    local P=$1
    local N=$2
    local TPN=$3
    local T=$4
    for SIZE in 512 1024 2048 4096; do
        OUTDIR="hybrid-P${P}-T${T}-N${N}/N${SIZE}"
        mkdir -p "$BASE_DIR/$OUTDIR"
        JOBFILE="$BASE_DIR/jobs/hybrid-P${P}-T${T}-N${N}-N${SIZE}.sh"
        cat > "$JOBFILE" <<EOF
#!/bin/bash
#SBATCH -N $N
#SBATCH --exclusive
#SBATCH --tasks-per-node=$TPN
#SBATCH --cpus-per-task=$T
#SBATCH -o $BASE_DIR/$OUTDIR/output_%j.txt
#SBATCH -e $BASE_DIR/$OUTDIR/errors_%j.txt
export OMP_NUM_THREADS=$T
mpirun --bind-to none $BIN_DIR/hybrid $SIZE $P $T
EOF
        chmod +x "$JOBFILE"
        echo "  [Enviando] $JOBFILE (size=$SIZE)"
        sbatch "$JOBFILE"
    done
}

function compilar() {
    echo "Compilando binarios para Blade (frontend)..."
    mkdir -p "$BIN_DIR"
    mpicc "$BASE_DIR/../../matrices-mpi.c" -o "$BIN_DIR/mpi-sec" -lm
    mpicc "$BASE_DIR/../../matrices-hybrid.c" -o "$BIN_DIR/hybrid" -fopenmp -lm
    echo "Compilacion OK."
}

mkdir -p "$BASE_DIR/jobs"

while true; do
    echo ""
    echo "=============================================="
    echo "  Orquestador: Blade (Cluster Multicore)"
    echo "=============================================="
    echo ""
    echo "  1) MPI  P=8   (N=1, 8 tasks/nodo)"
    echo "  2) MPI  P=16  (N=2, 8 tasks/nodo)"
    echo "  3) MPI  P=32  (N=4, 8 tasks/nodo)"
    echo "  4) Hibrido P=16, T=1 (N=2, 8 tasks/nodo)"
    echo "  5) Hibrido P=32, T=1 (N=4, 8 tasks/nodo)"
    echo "  6) TODOS los escenarios MPI (1,2,3)"
    echo "  7) TODOS los escenarios Hibrido (4,5)"
    echo "  8) TODOS los escenarios"
    echo "  9) Compilar binarios"
    echo "  0) Salir"
    echo ""
    read -rp "Seleccione una opcion: " opcion
    echo ""

    case "$opcion" in
        1)
            echo "=== Escenario: MPI P=8, N=1 ==="
            enviar_mpi 8 1 8
            ;;
        2)
            echo "=== Escenario: MPI P=16, N=2 ==="
            enviar_mpi 16 2 8
            ;;
        3)
            echo "=== Escenario: MPI P=32, N=4 ==="
            enviar_mpi 32 4 8
            ;;
        4)
            echo "=== Escenario: Hibrido P=16, T=1, N=2 ==="
            enviar_hybrid 16 2 8 1
            ;;
        5)
            echo "=== Escenario: Hibrido P=32, T=1, N=4 ==="
            enviar_hybrid 32 4 8 1
            ;;
        6)
            echo "=== Escenarios MPI (1,2,3) ==="
            enviar_mpi 8 1 8
            enviar_mpi 16 2 8
            enviar_mpi 32 4 8
            ;;
        7)
            echo "=== Escenarios Hibrido (4,5) ==="
            enviar_hybrid 16 2 8 1
            enviar_hybrid 32 4 8 1
            ;;
        8)
            echo "=== TODOS los escenarios ==="
            enviar_mpi 8 1 8
            enviar_mpi 16 2 8
            enviar_mpi 32 4 8
            enviar_hybrid 16 2 8 1
            enviar_hybrid 32 4 8 1
            ;;
        9)
            compilar
            ;;
        0)
            echo "Saliendo."
            exit 0
            ;;
        *)
            echo "Opcion invalida."
            ;;
    esac
    echo ""
    echo "Escenario(s) enviado(s). Use 'squeue' para verificar."
    read -rp "Presione Enter para continuar..."
done
