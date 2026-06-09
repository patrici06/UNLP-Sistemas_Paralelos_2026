#!/bin/bash
#
# ej3-non-blocking-ring.slurm
# Script SLURM para Ejercicio 3 (non-blocking-ring.c)
#
# Parametros segun practica:
#   P = {4, 8, 16}
#   N = {10000000, 20000000, 40000000}
#
# Nota: Para P=16 agregar --overcommit y --oversubscribe.
#
# Uso en cluster:   sbatch ej3-non-blocking-ring.slurm
#

#SBATCH --job-name=ej3-non-blocking-ring
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH --overcommit
#SBATCH --tasks-per-node=4
#SBATCH -o logs-cluster/ej3-non-blocking-ring-%j.out
#SBATCH -e logs-cluster/ej3-non-blocking-ring-%j.err

set -e

cd /home/patricio/Escritorio/sistemas-paralelos/practica-4

SRC_DIR="Fuentes"
BIN_DIR="bin"
LOG_DIR="scripts-cluster/logs-cluster"

mkdir -p "$BIN_DIR" "$LOG_DIR"

SRC="$SRC_DIR/non-blocking-ring.c"
BIN="$BIN_DIR/non-blocking-ring"

echo "==> [Ej3-non-blocking-ring] Compilando $SRC ..."
mpicc "$SRC" -o "$BIN" -lm

echo ""
echo "==> [Ej3-non-blocking-ring] Ejecutando pruebas ..."
echo ""

PROCESOS=(4 8 16)
SIZES=(10000000 20000000 40000000)

for P in "${PROCESOS[@]}"; do
    for N in "${SIZES[@]}"; do
        LOGFILE="$LOG_DIR/ej3-non-blocking-ring-P${P}-N${N}.log"
        echo "--- Ejercicio 3 (non-blocking-ring) | P=$P, N=$N ---" | tee "$LOGFILE"

        if [ "$P" -ge 16 ]; then
            echo "    (usando --oversubscribe)" | tee -a "$LOGFILE"
            mpirun --oversubscribe -np "$P" ./"$BIN" "$N" | tee -a "$LOGFILE"
        else
            mpirun -np "$P" ./"$BIN" "$N" | tee -a "$LOGFILE"
        fi

        echo "    Log guardado en: $LOGFILE"
        echo ""
    done
done

echo "==> [Ej3-non-blocking-ring] Finalizado."
