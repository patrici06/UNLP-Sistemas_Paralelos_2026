#!/bin/bash
# =============================================================================
# Generador de scripts SLURM individuales: MPI puro e Híbrido MPI+OpenMP
#
# Sigue la estructura del script de referencia provisto.
# Uso:  ./generar_run.sh
# =============================================================================
set -e

SCRIPT_DIR=$(dirname "$(realpath "$0")")
BASE_DIR=$(realpath "$SCRIPT_DIR/..")
BIN_DIR="$BASE_DIR"
RUNS_DIR="$SCRIPT_DIR/runs"
mkdir -p "$RUNS_DIR"

# =============================================================================
# CONFIGURACION (editar segun el cluster)
# =============================================================================
CORES_PER_NODE=64       # cores fisicos por nodo del cluster
SIZES=(2048 4096)       # tamanios de matriz a probar
TB=64                   # tamanio de bloque (hardcodeado en binarios)
OUTPUT_FILE="$BASE_DIR/times.txt" # archivo de salida con resultados (absoluto)

# ---------------------------------------------------------------------------
# Configuraciones MPI puro
#   Solo se especifica P (total de procesos MPI).
#   PPN fijo = 8 (como en blade.sh original)
# ---------------------------------------------------------------------------
MPI_CONFS=(
    8       # P=8   → N=1, 8 PPN
    16      # P=16  → N=2, 8 PPN
    32      # P=32  → N=4, 8 PPN
)

# ---------------------------------------------------------------------------
# Configuraciones Híbrido (MPI+OpenMP)
#   Formato: total_workers:threads
#   P_total = MPI_procs × OMP_threads
# ---------------------------------------------------------------------------
HYBRID_CONFS=(
    "16:8"     # P=16  →  2 MPI ×  8 OMP
    "32:8"     # P=32  →  4 MPI ×  8 OMP
    "64:8"     # P=64  →  8 MPI ×  8 OMP
)

# =============================================================================
# Funciones generadoras de scripts
# =============================================================================

# ---- Script MPI puro -------------------------------------------------------
generar_mpi() {
    local P=$1
    local PPN=8                            # fijo (como blade.sh)
    local NODES=$(( (P + PPN - 1) / PPN )) # ceil(P / PPN)
    local OUTFILE="$RUNS_DIR/runP${P}.sh"

    cat > "$OUTFILE" <<EOF
#!/bin/bash
#SBATCH -J MPI_P${P}
#SBATCH -N $NODES
#SBATCH --tasks-per-node=$PPN
#SBATCH --exclusive
#SBATCH -o $RUNS_DIR/P${P}_%j.out
#SBATCH -e $RUNS_DIR/P${P}_%j.err

OUTPUT_FILE="$OUTPUT_FILE"

echo "=== BENCHMARK MPI PURO ===" > \$OUTPUT_FILE
echo "Config: P=${P} MPI" >> \$OUTPUT_FILE
echo "Nodos: ${NODES} | MPI por nodo: ${PPN}" >> \$OUTPUT_FILE
echo "Fecha: \$(date)" >> \$OUTPUT_FILE
echo "----------------------------------------" >> \$OUTPUT_FILE

echo "Iniciando tanda para P=${P}..."

for N in ${SIZES[*]}
do
    echo "Configuracion: P=${P} MPI | Matriz N: \$N" >> \$OUTPUT_FILE
    mpirun -np ${P} ${BIN_DIR}/matrices-mpi \$N ${P} >> \$OUTPUT_FILE
    echo "----------------------------------------" >> \$OUTPUT_FILE
done

echo "=== Pruebas P=${P} finalizadas. Resultados en \$OUTPUT_FILE ==="
EOF

    chmod +x "$OUTFILE"
    echo "  [MPI]  $OUTFILE  (${P} MPI, ${NODES} nodos, ${PPN} PPN)"
}

# ---- Script Híbrido --------------------------------------------------------
generar_hybrid() {
    local TOTAL=$1     # total workers = MPI_procs × OMP_threads
    local THREADS=$2   # OpenMP threads por proceso
    local MPI_PROCS=$(( TOTAL / THREADS ))
    local MAX_PPN=$(( CORES_PER_NODE / THREADS ))

    if [ "$MPI_PROCS" -le "$MAX_PPN" ]; then
        local NODES=1
        local PPN=$MPI_PROCS
    else
        local NODES=$(( (MPI_PROCS + MAX_PPN - 1) / MAX_PPN ))
        local PPN=$MAX_PPN
    fi

    local OUTFILE="$RUNS_DIR/runP${TOTAL}T${THREADS}.sh"
    local USED_CORES=$(( PPN * THREADS ))

    cat > "$OUTFILE" <<EOF
#!/bin/bash
#SBATCH -J H_P${TOTAL}T${THREADS}
#SBATCH -N $NODES
#SBATCH --tasks-per-node=$PPN
#SBATCH --exclusive
#SBATCH -o $RUNS_DIR/P${TOTAL}T${THREADS}_%j.out
#SBATCH -e $RUNS_DIR/P${TOTAL}T${THREADS}_%j.err

OUTPUT_FILE="$OUTPUT_FILE"

echo "=== BENCHMARK HÍBRIDO MPI+OPENMP ===" > \$OUTPUT_FILE
echo "Config: P=${TOTAL} T=${THREADS} (${MPI_PROCS} MPI × ${THREADS} OMP)" >> \$OUTPUT_FILE
echo "Nodos: ${NODES} | MPI por nodo: ${PPN} | Cores usados/nodo: ${USED_CORES}/${CORES_PER_NODE}" >> \$OUTPUT_FILE
echo "Fecha: \$(date)" >> \$OUTPUT_FILE
echo "----------------------------------------" >> \$OUTPUT_FILE

echo "Iniciando tanda para P=${TOTAL}..."

export OMP_NUM_THREADS=${THREADS}
for N in ${SIZES[*]}
do
    echo "Configuracion: P=${TOTAL} | Matriz N: \$N" >> \$OUTPUT_FILE
    mpirun --bind-to none -np ${MPI_PROCS} ${BIN_DIR}/hibrido-pato \$N ${MPI_PROCS} >> \$OUTPUT_FILE
    echo "----------------------------------------" >> \$OUTPUT_FILE
done

echo "=== Pruebas P=${TOTAL} finalizadas. Resultados en \$OUTPUT_FILE ==="
EOF

    chmod +x "$OUTFILE"
    echo "  [HYB]  $OUTFILE  (${MPI_PROCS} MPI × ${THREADS} OMP = ${TOTAL} total, ${NODES} nodos, ${PPN} MPI/nodo)"
}

# =============================================================================
# Generacion
# =============================================================================
echo ""
echo "Generando scripts en: $RUNS_DIR"
echo "Cores por nodo: $CORES_PER_NODE"
echo "----------------------------------------"

for p in "${MPI_CONFS[@]}"; do
    generar_mpi "$p"
done

for cfg in "${HYBRID_CONFS[@]}"; do
    IFS=':' read -r total threads <<< "$cfg"
    if [ $((total % threads)) -ne 0 ]; then
        echo "  [SKIP] P=${total} T=${threads} (P no es multiplo de T)"
        continue
    fi
    generar_hybrid "$total" "$threads"
done

echo "----------------------------------------"
echo "Listo. Scripts generados en: $RUNS_DIR/"
echo ""

# =============================================================================
# INSTRUCTIVO
# =============================================================================
cat <<'INSTRUCTIVO'
╔══════════════════════════════════════════════════════════════════════════════╗
║                        INSTRUCTIVO DE USO                                   ║
╚══════════════════════════════════════════════════════════════════════════════╝

1. COMPILACION (siempre con -O2)
   ─────────────────────────────────────────────────────────────
   Los scripts buscan binarios en TP3/ (el directorio padre de script/).
   Para compilar todo:

       cd .. && ./blade.sh                    # antes: ./blade.sh
       # o directamente desde script/:
       ../blade.sh                            # opcion 7 (Compilar binarios)

   O manualmente desde TP3/:

       cd ..
       mpicc -O2 matrices-mpi.c   -o matrices-mpi   -lm
       mpicc -O2 hibrido-pato.c   -o hibrido-pato   -fopenmp -lm

2. GENERACION DE SCRIPTS
   ─────────────────────────────────────────────────────────────
   Todos los scripts .sh estan en script/. Ejecutar desde ahi:

       cd script
       ./generar_run.sh

   Editar CORES_PER_NODE, SIZES, MPI_CONFS y HYBRID_CONFS dentro
   de script/generar_run.sh segun el cluster.

   Los scripts generados quedan en script/runs/:

       runP8.sh      → MPI puro,    8 procesos,  1 nodo,  8 PPN
       runP16.sh     → MPI puro,   16 procesos,  2 nodos, 8 PPN
       runP32.sh     → MPI puro,   32 procesos,  4 nodos, 8 PPN
       runP16T8.sh   → Híbrido, 2 MPI x 8 OMP = 16,  1 nodo,  2 PPN
       runP32T8.sh   → Híbrido, 4 MPI x 8 OMP = 32,  1 nodo,  4 PPN
       runP64T8.sh   → Híbrido, 8 MPI x 8 OMP = 64,  1 nodo,  8 PPN

3. ENVIO A SLURM
   ─────────────────────────────────────────────────────────────
   Desde script/:

       sbatch runs/runP32.sh       # MPI puro
       sbatch runs/runP32T8.sh     # Híbrido

   Todos de una vez:

       for s in runs/runP*.sh; do sbatch $s; done

   Ver cola:

       squeue -u $USER

4. CASOS DEL SCRIPT ORIGINAL (blade.sh opciones 1-6)
   ─────────────────────────────────────────────────────────────
   blade.sh original      →  Script equivalente
   ─────────────────────────────────────────────────────────────
   Opcion 1: MPI P=8      →  sbatch runs/runP8.sh
   Opcion 2: MPI P=16     →  sbatch runs/runP16.sh
   Opcion 3: MPI P=32     →  sbatch runs/runP32.sh
   Opcion 4: Hib P=16     →  sbatch runs/runP16T8.sh
   Opcion 5: Hib P=32     →  sbatch runs/runP32T8.sh
   Opcion 6: Bench P=16,32 →  sbatch runs/runP16T8.sh && sbatch runs/runP32T8.sh

5. FORMATO DE SALIDA
   ─────────────────────────────────────────────────────────────
   Cada script genera times.txt en TP3/ (raiz del proyecto):

       === BENCHMARK MPI PURO ===
       Config: P=32 MPI
       Nodos: 4 | MPI por nodo: 8
       Fecha: ...
       ----------------------------------------
       Configuracion: P=32 MPI | Matriz N: 2048
       <resultado del binario>
       ----------------------------------------
       ...

   Para ver los resultados:

       ./mostrar_salidas.sh

6. BINDING
   ─────────────────────────────────────────────────────────────
   MPI puro:  no se especifica binding; OpenMPI usa el default
              (asigna procesos a cores segun politica del cluster).

   Hibrido:   --bind-to none  para que OpenMP gestione libremente
              los hilos dentro de los cores asignados por SLURM.

   Si el cluster requiere binding explicito para MPI puro,
   agregar --map-by node o --bind-to core segun corresponda.

7. PERSONALIZACION
   ─────────────────────────────────────────────────────────────
   - CORES_PER_NODE: cambiar si el cluster tiene distinta cantidad
     de cores por nodo (ej: 48, 128).
   - SIZES: agregar o quitar tamanios de matriz.
   - MPI_CONFS / HYBRID_CONFS: agregar nuevas combinaciones.
   - OUTPUT_FILE: nombre del archivo de resultados (absoluto).

   Despues de editar, volver a ejecutar ./generar_run.sh.

INSTRUCTIVO
