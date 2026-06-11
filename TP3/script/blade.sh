#!/bin/bash
set -e

SCRIPT_DIR=$(dirname "$(realpath "$0")")
BASE_DIR=$(realpath "$SCRIPT_DIR/..")

# =============================================================================
# Orquestador interactivo: Blade (Cluster Multicore)
# Los binarios se compilan y buscan en BASE_DIR (TP3/).
# =============================================================================

function enviar_mpi() {
    local P=$1
    local N=$2
    local TPN=$3
    for SIZE in 512 1024 2048 4096; do
        OUTDIR="$BASE_DIR/mpi-P${P}-N${N}/N${SIZE}"
        mkdir -p "$OUTDIR"
        JOBFILE="$BASE_DIR/jobs/mpi-P${P}-N${N}-N${SIZE}.sh"
        cat > "$JOBFILE" <<EOF
#!/bin/bash
#SBATCH -N $N
#SBATCH --exclusive
#SBATCH --tasks-per-node=$TPN
#SBATCH -o $OUTDIR/output_%j.txt
#SBATCH -e $OUTDIR/errors_%j.txt
mpirun $BASE_DIR/matrices-mpi $SIZE $P
EOF
        chmod +x "$JOBFILE"
        echo "  [Enviando] $JOBFILE (size=$SIZE)"
        sbatch "$JOBFILE"
    done
}

function enviar_mpi_p1() {
    local P=1
    local N=1
    local TPN=1
    for SIZE in 512 1024 2048 4096; do
        OUTDIR="$BASE_DIR/mpi-P${P}-N${N}/N${SIZE}"
        mkdir -p "$OUTDIR"
        JOBFILE="$BASE_DIR/jobs/mpi-P${P}-N${N}-N${SIZE}.sh"
        cat > "$JOBFILE" <<EOF
#!/bin/bash
#SBATCH -N $N
#SBATCH --exclusive
#SBATCH --tasks-per-node=$TPN
#SBATCH -o $OUTDIR/output_%j.txt
#SBATCH -e $OUTDIR/errors_%j.txt
mpirun -np 1 $BASE_DIR/matrices-mpi $SIZE 1
EOF
        chmod +x "$JOBFILE"
        echo "  [Enviando] $JOBFILE (size=$SIZE)"
        sbatch "$JOBFILE"
    done
}

function enviar_hybrid_p1t1() {
    local P=1
    local N=1
    local TPN=1
    local T=1
    for SIZE in 512 1024 2048 4096; do
        OUTDIR="$BASE_DIR/hybrid-P${P}-T${T}-N${N}/N${SIZE}"
        mkdir -p "$OUTDIR"
        JOBFILE="$BASE_DIR/jobs/hybrid-P${P}-T${T}-N${N}-N${SIZE}.sh"
        cat > "$JOBFILE" <<EOF
#!/bin/bash
#SBATCH -N $N
#SBATCH --exclusive
#SBATCH --tasks-per-node=$TPN
#SBATCH --cpus-per-task=$T
#SBATCH -o $OUTDIR/output_%j.txt
#SBATCH -e $OUTDIR/errors_%j.txt
export OMP_NUM_THREADS=$T
mpirun --bind-to none $BASE_DIR/hibrido $SIZE 1
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
        OUTDIR="$BASE_DIR/hybrid-P${P}-T${T}-N${N}/N${SIZE}"
        mkdir -p "$OUTDIR"
        JOBFILE="$BASE_DIR/jobs/hybrid-P${P}-T${T}-N${N}-N${SIZE}.sh"
        cat > "$JOBFILE" <<EOF
#!/bin/bash
#SBATCH -N $N
#SBATCH --exclusive
#SBATCH --tasks-per-node=$TPN
#SBATCH --cpus-per-task=$T
#SBATCH -o $OUTDIR/output_%j.txt
#SBATCH -e $OUTDIR/errors_%j.txt
export OMP_NUM_THREADS=$T
mpirun --bind-to none $BASE_DIR/hibrido $SIZE $P
EOF
        chmod +x "$JOBFILE"
        echo "  [Enviando] $JOBFILE (size=$SIZE)"
        sbatch "$JOBFILE"
    done
}

function enviar_benchmark_hybrid() {
    for CONFIG in "16:2:1:8" "32:4:1:8"; do
        IFS=':' read -r P N TPN T <<< "$CONFIG"
        for SIZE in 2048 4096; do
            OUTDIR="$BASE_DIR/hybrid-bench-P${P}-T${T}-N${N}/N${SIZE}"
            mkdir -p "$OUTDIR"
            JOBFILE="$BASE_DIR/jobs/hybrid-bench-P${P}-T${T}-N${N}-N${SIZE}.sh"
            cat > "$JOBFILE" <<EOF
#!/bin/bash
#SBATCH -J Hibrid_P${P}_N${SIZE}
#SBATCH -N $N
#SBATCH --exclusive
#SBATCH --tasks-per-node=$TPN
#SBATCH --cpus-per-task=$T
#SBATCH -o $OUTDIR/output_%j.txt
#SBATCH -e $OUTDIR/errors_%j.txt
export OMP_NUM_THREADS=$T
mpirun --bind-to none $BASE_DIR/hibrido $SIZE $P
EOF
            chmod +x "$JOBFILE"
            echo "  [Enviando] $JOBFILE (P=${P}=${N}MPIx${T}OMP, size=$SIZE)"
            sbatch "$JOBFILE"
        done
    done
}

function compilar() {
    echo "Compilando binarios en $BASE_DIR (siempre -O2)..."
    mpicc -O2 "$BASE_DIR/matrices-mpi.c" -o "$BASE_DIR/matrices-mpi" -lm
    mpicc -O2 "$BASE_DIR/hibrido.c" -o "$BASE_DIR/hibrido" -fopenmp -lm
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
    echo "  4) Hibrido P=16 (N=2, 8 hilos/nodo, 1 proceso/nodo = 2 MPI x 8 OMP)"
    echo "  5) Hibrido P=32 (N=4, 8 hilos/nodo, 1 proceso/nodo = 4 MPI x 8 OMP)"
    echo "  6) Benchmark Hibrido P=16,32 (N=2048,4096)"
    echo "  7) MPI P=1 (N=1, 1 tarea/nodo, 1 proceso MPI)"
    echo "  8) Hibrido P=1 T=1 (N=1, 1 tarea/nodo, 1 MPI x 1 OMP)"
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
            echo "=== Escenario: Hibrido P=16 (2 MPI x 8 OMP), N=2 ==="
            enviar_hybrid 2 2 1 8
            ;;
        5)
            echo "=== Escenario: Hibrido P=32 (4 MPI x 8 OMP), N=4 ==="
            enviar_hybrid 4 4 1 8
            ;;
        6)
            echo "=== Benchmark Hibrido P=16,32 ==="
            enviar_benchmark_hybrid
            ;;
        7)
            echo "=== Escenario: MPI P=1 (N=1, 1 tarea/nodo) ==="
            enviar_mpi_p1
            ;;
        8)
            echo "=== Escenario: Hibrido P=1 T=1 (N=1, 1 tarea/nodo, 1 MPI x 1 OMP) ==="
            enviar_hybrid_p1t1
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
