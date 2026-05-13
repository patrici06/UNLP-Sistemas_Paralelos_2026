#!/bin/bash

# ============================================================
# Script para CLUSTER - matrices.c (SECUENCIAL)
# ============================================================
# Este script:
#   1. Compila matrices.c (-O1, -O2, -O3)
#   2. Envía jobs al cluster mediante sbatch
#   3. Captura salidas en estructura ordenada
#
# Parámetros:
#   - Tamaños: N = {512, 1024, 2048, 4096}
#   - Optimización: O = {1, 2, 3}
#
# Estructura de salida (EN MOTO):
#   salidas/
#   ├─ matrices/
#   │  ├─ N512/O1/{outputs,reports}
#   │  ├─ N512/O2/{outputs,reports}
#   │  ├─ N512/O3/{outputs,reports}
#   │  ├─ N1024/...
#   │  ├─ N2048/...
#   │  └─ N4096/...
# ============================================================

set -e

matrix_sizes=(512 1024 2048 4096)
optimization_levels=(1 2 3)

SALIDAS_DIR="./salidas"
TIPO_EXEC="matrices"
mkdir -p "$SALIDAS_DIR/$TIPO_EXEC"

echo "======================================"
echo "ENVÍO DE TRABAJOS AL CLUSTER - SECUENCIAL"
echo "======================================"
echo ""

echo "Compilando matrices.c..."
for o in "${optimization_levels[@]}"; do
	echo -n "  -O$o... "
	if gcc -O${o} ./matrices.c -o ./matrices-O${o} -lm 2>/dev/null; then
		echo "✓"
	else
		echo "✗ ERROR"
		exit 1
	fi
done
echo ""

for n in "${matrix_sizes[@]}"; do
	echo "=================================================="
	echo "Enviando trabajos: N=$n (SECUENCIAL)"
	echo "=================================================="

	for o in "${optimization_levels[@]}"; do
		EXEC_DIR="$SALIDAS_DIR/$TIPO_EXEC/N${n}/O${o}"
		OUTPUT_DIR="$EXEC_DIR/outputs"
		REPORT_DIR="$EXEC_DIR/reports"
		mkdir -p "$OUTPUT_DIR"
		mkdir -p "$REPORT_DIR"

		echo -n "  O$o... "

		# Enviar job a cluster
		job_id=$(sbatch \
			-o "$OUTPUT_DIR/matrices_%j.txt" \
			--job-name="seq-N${n}-O${o}" \
			--time=00:10:00 \
			--partition=Blade \
			-N 1 --exclusive \
			./job-matrices.sh $n $o 2>&1 | awk '{print $NF}')

		echo "Job ID: $job_id"

	done
	echo ""
done

echo "======================================"
echo "Todos los trabajos han sido enviados"
echo "Monitorea con: squeue -u \$USER"
echo "Salidas en: $SALIDAS_DIR/$TIPO_EXEC/N{512,1024,2048,4096}/O{1,2,3}/outputs/"
echo "======================================"
