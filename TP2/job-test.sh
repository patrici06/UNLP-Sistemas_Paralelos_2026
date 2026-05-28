#!/bin/bash
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH --partition=Blade
#SBATCH --time=00:30:00

# ============================================================
# Job Script para CLUSTER - Pruebas completas
# ============================================================
# Parámetros:
#   $1: N (tamaño de matriz)
#   $2: T (cantidad de threads)
#   $3: PROGRAM (pthread o openmp)
#
# Ejecutado por: exec-cluster-test.sh
# Compila solo -O2
# ============================================================

N=$1
T=$2
PROGRAM=$3

if [ -z "$N" ] || [ -z "$T" ] || [ -z "$PROGRAM" ]; then
    echo "Uso: $0 N T PROGRAM"
    echo "  N: tamaño matriz (512, 1024, 2048, 4096)"
    echo "  T: threads (1, 2, 4, 8)"
    echo "  PROGRAM: pthread o openmp"
    exit 1
fi

echo "=========================================="
echo "Ejecutando matrices-$PROGRAM (N=$N, T=$T)"
echo "=========================================="
echo "Host: $(hostname)"
echo "Date: $(date '+%Y-%m-%d %H:%M:%S')"
echo "Threads disponibles: $(nproc)"
echo "=========================================="
echo ""

# Determinar ejecutable según el programa
if [ "$PROGRAM" == "pthread" ]; then
    EXEC="./matrices-pthread"
else
    EXEC="./matrices-open-mp"
fi

# Ejecutar el programa
$EXEC $N $T 0

echo ""
echo "Ejecución completada"
