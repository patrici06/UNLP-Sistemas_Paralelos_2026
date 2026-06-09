#!/bin/bash
#
# ej2-blocking.slurm
# Script SLURM para Ejercicio 2 (blocking master-worker)
#
# Uso en cluster:   sbatch ej2-blocking.slurm
#

#SBATCH --job-name=ej2-blocking
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH --tasks-per-node=4
#SBATCH -o logs-cluster/ej2-blocking-%j.out
#SBATCH -e logs-cluster/ej2-blocking-%j.err

set -e

cd /home/patricio/Escritorio/sistemas-paralelos/practica-4

SRC_DIR="Fuentes"
BIN_DIR="bin"
LOG_DIR="scripts-cluster/logs-cluster"

mkdir -p "$BIN_DIR" "$LOG_DIR"

# Compilar
SRC="$SRC_DIR/blocking.c"
BIN="$BIN_DIR/blocking"

echo "==> [Ej2-blocking] Compilando $SRC ..."
mpicc "$SRC" -o "$BIN"

echo ""
echo "==> [Ej2-blocking] Ejecutando con P={4,8,16} ..."
echo ""

PROCESOS=(4 8 16)

for P in "${PROCESOS[@]}"; do
    LOGFILE="$LOG_DIR/ej2-blocking-P${P}.log"
    echo "--- Ejercicio 2 (blocking) | P=$P ---" | tee "$LOGFILE"

    if [ "$P" -ge 16 ]; then
        echo "    (usando --oversubscribe)" | tee -a "$LOGFILE"
        mpirun --oversubscribe -np "$P" ./"$BIN" | tee -a "$LOGFILE"
    else
        mpirun -np "$P" ./"$BIN" | tee -a "$LOGFILE"
    fi

    echo "    Log guardado en: $LOGFILE"
    echo ""
done

echo "==> [Ej2-blocking] Finalizado."
