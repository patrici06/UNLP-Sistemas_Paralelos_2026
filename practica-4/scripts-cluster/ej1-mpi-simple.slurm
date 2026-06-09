#!/bin/bash
#
# ej1-mpi-simple.slurm
# Script SLURM para Ejercicio 1: mpi-simple-solucion.c (comunicacion en anillo)
#
# Uso en cluster:   sbatch ej1-mpi-simple.slurm
# NUNCA ejecutar asi: ./ej1-mpi-simple.slurm
#

#SBATCH --job-name=ej1-mpi-simple
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH --tasks-per-node=4
#SBATCH -o logs-cluster/ej1-mpi-simple-%j.out
#SBATCH -e logs-cluster/ej1-mpi-simple-%j.err

set -e

# Cambiar al directorio del proyecto (ajustar si es necesario)
cd /home/patricio/Escritorio/sistemas-paralelos/practica-4

SRC_DIR="Fuentes"
BIN_DIR="bin"
LOG_DIR="scripts-cluster/logs-cluster"

mkdir -p "$BIN_DIR"
mkdir -p "$LOG_DIR"

# Compilar
SRC="$SRC_DIR/mpi-simple-solucion.c"
BIN="$BIN_DIR/mpi-simple-solucion"

echo "==> [Ej1] Compilando $SRC ..."
mpicc "$SRC" -o "$BIN"

echo ""
echo "==> [Ej1] Ejecutando mpi-simple-solucion con P={4,8,16} ..."
echo ""

PROCESOS=(4 8 16)

for P in "${PROCESOS[@]}"; do
    LOGFILE="$LOG_DIR/ej1-mpi-simple-P${P}.log"
    echo "--- Ejercicio 1 | P=$P ---" | tee "$LOGFILE"

    if [ "$P" -ge 16 ]; then
        echo "    (usando --oversubscribe para P=16)" | tee -a "$LOGFILE"
        mpirun --oversubscribe -np "$P" ./"$BIN" | tee -a "$LOGFILE"
    else
        mpirun -np "$P" ./"$BIN" | tee -a "$LOGFILE"
    fi

    echo "    Log guardado en: $LOGFILE"
    echo ""
done

echo "==> [Ej1] Finalizado. Revisar logs en: $LOG_DIR/"
