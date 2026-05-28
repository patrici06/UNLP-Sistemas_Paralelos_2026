#!/bin/bash
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH --partition=Blade
#SBATCH --time=00:15:00

# ============================================================
# Job Script para CLUSTER - matrices-pthread.c
# ============================================================
# Parámetros:
#   $1: N (tamaño de matriz)
#   $2: T (cantidad de threads)
#
# Ejecutado por: exec-cluster-pthread.sh
# Compilación: -O2 (única)
# ============================================================

N=$1
T=$2

if [ -z "$N" ] || [ -z "$T" ]; then
    echo "Uso: $0 N T"
    echo "  N: tamaño de matriz"
    echo "  T: cantidad de threads"
    exit 1
fi

echo "=========================================="
echo "Compilando matrices-pthread.c..."
echo "=========================================="
TMP_BIN="./matrices-pthread-run-$$"
gcc -O2 -pthread -o "$TMP_BIN" ./matrices-pthread.c -lm || { echo "ERROR: Compilacion fallida"; exit 1; }
echo "✓ Compilación exitosa"
echo ""
echo "==========================================="
echo "Ejecutando matrices-pthread"
echo "==========================================="
echo "N = $N"
echo "T = $T"
echo "Compilación: -O2 -pthread"
echo "Host: $(hostname)"
echo "Date: $(date '+%Y-%m-%d %H:%M:%S')"
echo "Threads disponibles: $(nproc)"
echo "==========================================="
echo ""

"$TMP_BIN" $N $T

echo ""
echo "=========================================="
echo "Ejecución completada"
echo "=========================================="
