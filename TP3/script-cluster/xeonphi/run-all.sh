#!/bin/bash
set -e

SCRIPT_DIR=$(dirname "$(realpath "$0")")
cd "$SCRIPT_DIR" || exit 1

BASE_DIR=$(realpath "$SCRIPT_DIR")
BIN_DIR=$(realpath "$SCRIPT_DIR/../bin")
SRC_DIR=$(realpath "$SCRIPT_DIR/../..")

# =============================================================================
# Orquestador interactivo: Intel XeonPHI (Manycore)
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
#SBATCH --partition=XeonPHI
#SBATCH --tasks-per-node=$TPN
#SBATCH -o $BASE_DIR/$OUTDIR/output_%j.txt
#SBATCH -e $BASE_DIR/$OUTDIR/errors_%j.txt
source \$ONEAPI_PATH/setvars.sh > /dev/null 2>&1
mkdir -p "$BIN_DIR"
mpiicc -o "$BIN_DIR/mpi-xeonphi" "$SRC_DIR/matrices-mpi.c"
mpirun "$BIN_DIR/mpi-xeonphi" $SIZE $P
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
#SBATCH --partition=XeonPHI
#SBATCH --tasks-per-node=$TPN
#SBATCH --cpus-per-task=$T
#SBATCH -o $BASE_DIR/$OUTDIR/output_%j.txt
#SBATCH -e $BASE_DIR/$OUTDIR/errors_%j.txt
source \$ONEAPI_PATH/setvars.sh > /dev/null 2>&1
mkdir -p "$BIN_DIR"
mpiicc -fopenmp -o "$BIN_DIR/hybrid-xeonphi" "$SRC_DIR/matrices-hybrid.c"
export OMP_NUM_THREADS=$T
mpirun --bind-to none "$BIN_DIR/hybrid-xeonphi" $SIZE $P $T
EOF
        chmod +x "$JOBFILE"
        echo "  [Enviando] $JOBFILE (size=$SIZE)"
        sbatch "$JOBFILE"
    done
}

mkdir -p "$BASE_DIR/jobs"

while true; do
    echo ""
    echo "=============================================="
    echo "  Orquestador: XeonPHI (Intel Manycore)"
    echo "=============================================="
    echo ""
    echo "  1) MPI  P=16  (N=1, 16 tasks/nodo)"
    echo "  2) MPI  P=32  (N=1, 32 tasks/nodo)"
    echo "  3) Hibrido P=16, T=4 (N=1, 16 tasks/nodo)"
    echo "  4) Hibrido P=32, T=2 (N=1, 32 tasks/nodo)"
    echo "  5) TODOS los escenarios MPI (1,2)"
    echo "  6) TODOS los escenarios Hibrido (3,4)"
    echo "  7) TODOS los escenarios"
    echo "  0) Salir"
    echo ""
    read -rp "Seleccione una opcion: " opcion
    echo ""

    case "$opcion" in
        1)
            echo "=== Escenario: MPI P=16, N=1 ==="
            enviar_mpi 16 1 16
            ;;
        2)
            echo "=== Escenario: MPI P=32, N=1 ==="
            enviar_mpi 32 1 32
            ;;
        3)
            echo "=== Escenario: Hibrido P=16, T=4, N=1 ==="
            enviar_hybrid 16 1 16 4
            ;;
        4)
            echo "=== Escenario: Hibrido P=32, T=2, N=1 ==="
            enviar_hybrid 32 1 32 2
            ;;
        5)
            echo "=== Escenarios MPI (1,2) ==="
            enviar_mpi 16 1 16
            enviar_mpi 32 1 32
            ;;
        6)
            echo "=== Escenarios Hibrido (3,4) ==="
            enviar_hybrid 16 1 16 4
            enviar_hybrid 32 1 32 2
            ;;
        7)
            echo "=== TODOS los escenarios ==="
            enviar_mpi 16 1 16
            enviar_mpi 32 1 32
            enviar_hybrid 16 1 16 4
            enviar_hybrid 32 1 32 2
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
