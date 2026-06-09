#!/bin/bash
#
# ej2-non-blocking.slurm
# Script SLURM para Ejercicio 2 (non-blocking master-worker)
#
# Uso en cluster:   sbatch ej2-non-blocking.slurm
#

#SBATCH --job-name=ej2-non-blocking
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH --tasks-per-node=4
#SBATCH -o logs-cluster/ej2-non-blocking-%j.out
#SBATCH -e logs-cluster/ej2-non-blocking-%j.err

set -e

cd /home/patricio/Escritorio/sistemas-paralelos/practica-4

SRC_DIR="Fuentes"
BIN_DIR="bin"
LOG_DIR="scripts-cluster/logs-cluster"

mkdir -p "$BIN_DIR" "$LOG_DIR"

SRC="$SRC_DIR/non-blocking.c"
BIN="$BIN_DIR/non-blocking"

echo "==> [Ej2-non-blocking] Compilando $SRC ..."
mpicc "$SRC" -o "$BIN"

echo ""
echo "==> [Ej2-non-blocking] Ejecutando con P={4,8,16} ..."
echo ""

PROCESOS=(4 8 16)

for P in "${PROCESOS[@]}"; do
    LOGFILE="$LOG_DIR/ej2-non-blocking-P${P}.log"
    echo "--- Ejercicio 2 (non-blocking) | P=$P ---" | tee "$LOGFILE"

    if [ "$P" -ge 16 ]; then
        echo "    (usando --oversubscribe)" | tee -a "$LOGFILE"
        mpirun --oversubscribe -np "$P" ./"$BIN" | tee -a "$LOGFILE"
    else
        mpirun -np "$P" ./"$BIN" | tee -a "$LOGFILE"
    fi

    echo "    Log guardado en: $LOGFILE"
    echo ""
done

echo "==> [Ej2-non-blocking] Finalizado."
