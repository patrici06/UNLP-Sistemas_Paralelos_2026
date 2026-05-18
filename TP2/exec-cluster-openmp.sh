#!/bin/bash

# ============================================================
# Script para CLUSTER - matrices-open-pm.c (OpenMP)
# ============================================================
# Este script:
#   1. Compila matrices-open-pm.c (-O1, -O2, -O3)
#   2. Envía jobs al cluster mediante sbatch
#   3. Captura salidas en estructura ordenada
#
# Parámetros:
#   - Tamaños: N = {512, 1024, 2048, 4096}
#   - Threads: T = {2, 4, 8} (por VARIABLE DE ENTORNO en job)
#   - Optimización: O = {1, 2, 3}
#
# Estructura de salida (EN CLUSTER):
#   salidas/
#   ├─ openmp/
#   │  ├─ N512/T2/O1/{outputs,reports}
#   │  ├─ N512/T2/O2/{outputs,reports}
#   │  ├─ N512/T2/O3/{outputs,reports}
#   │  ├─ N512/T4/...
#   │  ├─ N512/T8/...
#   │  ├─ N1024/...
#   │  ├─ N2048/...
#   │  └─ N4096/...
# ============================================================

set -e

matrix_sizes=(512 1024 2048 4096)
optimization_levels=(1 2 3)
thread_counts=(2 4 8)

SALIDAS_DIR="./salidas"
TIPO_EXEC="openmp"
mkdir -p "$SALIDAS_DIR/$TIPO_EXEC"

echo "======================================"
echo "ENVÍO DE TRABAJOS AL CLUSTER - OpenMP"
echo "======================================"
echo ""

# Verificar que el archivo fuente existe
if [ ! -f "./matrices-open-pm.c" ]; then
	echo "❌ ERROR: No se encuentra matrices-open-pm.c"
	ls -la matrices*.c 2>/dev/null || echo "No hay archivos matrices*.c"
	exit 1
fi

echo "Compilando matrices-open-pm.c con -fopenmp..."
for o in "${optimization_levels[@]}"; do
	echo -n "  -O$o... "
	compile_log="/tmp/gcc_O${o}.log"
	if gcc -O${o} -fopenmp ./matrices-open-pm.c -o ./matrices-openmp-O${o} -lm >"$compile_log" 2>&1; then
		echo "✓"
	else
		echo "✗ ERROR"
		echo ""
		echo "=== Detalles del error ==="
		cat "$compile_log"
		echo "=========================="
		exit 1
	fi
done
echo ""

echo "Compilando matrices-open-pm.c con -fopenmp (en el frontend del cluster)..."
for o in "${optimization_levels[@]}"; do
	echo -n "  -O$o... "
	if gcc -O${o} -fopenmp ./matrices-open-pm.c -o ./matrices-openmp-O${o} -lm 2>&1; then
		echo "✓"
	else
		echo "✗ ERROR en compilación"
		echo "Verifica que matrices-open-pm.c existe y gcc tiene -fopenmp disponible"
		exit 1
	fi
done
echo ""

for n in "${matrix_sizes[@]}"; do
	echo "=================================================="
	echo "Enviando trabajos: N=$n (OpenMP)"
	echo "=================================================="

	for t in "${thread_counts[@]}"; do
		for o in "${optimization_levels[@]}"; do
			EXEC_DIR="$SALIDAS_DIR/$TIPO_EXEC/N${n}/T${t}/O${o}"
			OUTPUT_DIR="$EXEC_DIR/outputs"
			REPORT_DIR="$EXEC_DIR/reports"
			mkdir -p "$OUTPUT_DIR"
			mkdir -p "$REPORT_DIR"

			echo -n "  T$t-O$o... "

			# Enviar job a cluster
			job_id=$(sbatch \
				-o "$OUTPUT_DIR/openmp_%j.txt" \
				--job-name="omp-N${n}-T${t}-O${o}" \
				--time=00:10:00 \
				--partition=Blade \
				-N 1 --exclusive \
				./job-openmp.sh $n $o $t 2>&1 | awk '{print $NF}')

			echo "Job ID: $job_id"

		done
	done
	echo ""
done

echo "======================================"
echo "Todos los trabajos han sido enviados"
echo "Monitorea con: squeue -u \$USER"
echo "Salidas en: $SALIDAS_DIR/$TIPO_EXEC/N{512,1024,2048,4096}/T{2,4,8}/O{1,2,3}/outputs/"
echo "======================================"
