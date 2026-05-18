#!/bin/bash
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH --partition=Blade
#SBATCH --time=00:10:00

# ============================================================
# Job Script para CLUSTER - matrices-open-mp.c (OpenMP)
# ============================================================
# Parámetros:
#   $1: N (tamaño de matriz)
#   $2: O (nivel de optimización)
#   $3: T (cantidad de threads - VARIABLE DE ENTORNO)
#
# IMPORTANTE: Los threads se pasan por OMP_NUM_THREADS
# Este es el script que ejecuta en el nodo del cluster
#
# Ejecutado por: exec-cluster-openmp.sh
# ============================================================

N=$1
O=$2
T=$3

echo "=========================================="
echo "Ejecutando matrices-openmp"
echo "N = $N"
echo "Threads = $T (OMP_NUM_THREADS)"
echo "Optimization = -O$O"
echo "Host: $(hostname)"
echo "Date: $(date '+%Y-%m-%d %H:%M:%S')"
echo "=========================================="

# Establecer variable de entorno para OpenMP
export OMP_NUM_THREADS=$T

./matrices-openmp-O${O} $N

echo "=========================================="
echo "Ejecución completada"
echo "=========================================="
