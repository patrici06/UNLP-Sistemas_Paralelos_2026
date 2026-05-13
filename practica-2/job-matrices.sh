#!/bin/bash
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH --partition=Blade
#SBATCH --time=00:10:00

# ============================================================
# Job Script para CLUSTER - matrices.c (SECUENCIAL)
# ============================================================
# Parámetros:
#   $1: N (tamaño de matriz)
#   $2: O (nivel de optimización)
#
# Ejecutado por: exec-cluster-matrices.sh
# ============================================================

N=$1
O=$2

echo "=========================================="
echo "Ejecutando matrices (SECUENCIAL)"
echo "N = $N"
echo "Optimization = -O$O"
echo "Host: $(hostname)"
echo "Date: $(date '+%Y-%m-%d %H:%M:%S')"
echo "=========================================="

./matrices-O${O} $N

echo "=========================================="
echo "Ejecución completada"
echo "=========================================="
