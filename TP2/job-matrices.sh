#!/bin/bash
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH --partition=Blade
#SBATCH --time=00:15:00

# ============================================================
# Job Script para CLUSTER - matrices.c (REFERENCIA SECUENCIAL)
# ============================================================
# Parámetros:
#   $1: N (tamaño de matriz)
#
# Ejecutado por: exec-cluster-matrices.sh
# Compilación: -O2 (única)
# ============================================================

N=$1

if [ -z "$N" ]; then
    echo "Uso: $0 N"
    echo "  N: tamaño de matriz"
    exit 1
fi

echo "=========================================="
echo "Compilando matrices.c..."
echo "=========================================="
TMP_BIN="./matrices-run-$$"
gcc -O2 -o "$TMP_BIN" ./matrices.c -lm || { echo "ERROR: Compilacion fallida"; exit 1; }
echo "✓ Compilación exitosa"
echo ""
echo "==========================================="
echo "Ejecutando matrices.c (REFERENCIA)"
echo "==========================================="
echo "N = $N"
echo "Compilación: -O2"
echo "Host: $(hostname)"
echo "Date: $(date '+%Y-%m-%d %H:%M:%S')"
echo "Threads disponibles: $(nproc)"
echo "==========================================="
echo ""

"$TMP_BIN" $N

echo ""
echo "=========================================="
echo "Ejecución completada"
echo "=========================================="
