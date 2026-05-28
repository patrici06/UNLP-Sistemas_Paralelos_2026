#!/bin/bash
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH --partition=Blade
#SBATCH --time=00:15:00

# ============================================================
# Job Script para CLUSTER - matrices-open-mp.c (T=1 SOLO)
# ============================================================
# Parámetros:
#   $1: N (tamaño de matriz)
#
# Ejecutado por: exec-cluster-openmp-t1.sh
# Compilación: -O2 (única)
# ============================================================

N=$1

if [ -z "$N" ]; then
    echo "Uso: $0 N"
    echo "  N: tamaño de matriz"
    exit 1
fi

echo "=========================================="
echo "Compilando matrices-open-mp.c..."
echo "=========================================="
TMP_BIN="./matrices-open-mp-run-$$"
gcc -O2 -fopenmp -o "$TMP_BIN" ./matrices-open-mp.c -lm || { echo "ERROR: Compilacion fallida"; exit 1; }
echo "✓ Compilación exitosa"
echo ""
echo "==========================================="
echo "Ejecutando matrices-open-mp (T=1 - REFERENCIA)"
echo "==========================================="
echo "N = $N"
echo "Compilación: -O2 -fopenmp"
echo "Host: $(hostname)"
echo "Date: $(date '+%Y-%m-%d %H:%M:%S')"
echo "Threads disponibles: $(nproc)"
echo "==========================================="
echo ""

"$TMP_BIN" $N 1

echo ""
echo "=========================================="
echo "Ejecución completada"
echo "=========================================="
