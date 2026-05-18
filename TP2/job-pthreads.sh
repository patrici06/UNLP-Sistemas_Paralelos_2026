#!/bin/bash
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH --partition=Blade
#SBATCH --time=00:10:00

# ============================================================
# Job Script para CLUSTER - matrices-pthreads.c
# ============================================================
# Parámetros:
#   $1: N (tamaño de matriz)
#   $2: O (nivel de optimización)
#   $3: T (cantidad de threads - PARÁMETRO)
#
# Ejecutado por: exec-cluster-pthreads.sh
# ============================================================

N=$1
O=$2
T=$3

echo "=========================================="
echo "Ejecutando matrices-pthreads"
echo "N = $N"
echo "Threads = $T (parámetro)"
echo "Optimization = -O$O"
echo "Host: $(hostname)"
echo "Date: $(date '+%Y-%m-%d %H:%M:%S')"
echo "=========================================="

./matrices-pthreads-O${O} $N $T

echo "=========================================="
echo "Ejecución completada"
echo "=========================================="
