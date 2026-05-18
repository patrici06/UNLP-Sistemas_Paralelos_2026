#!/bin/bash

# ============================================================
# Script de ejecución LOCAL para matrices-open-mp.c (OpenMP)
# ============================================================
# Estructura de almacenamiento:
#   salidas/
#   ├─ openmp/
#   │  ├─ N512/
#   │  │  ├─ T2/
#   │  │  │  ├─ O1/outputs/
#   │  │  │  ├─ O1/reports/
#   │  │  │  ├─ O2/outputs/
#   │  │  │  ├─ O2/reports/
#   │  │  │  ├─ O3/outputs/
#   │  │  │  └─ O3/reports/
#   │  │  ├─ T4/...
#   │  │  ├─ T8/...
#   │  ├─ N1024/...
#   │  ├─ N2048/...
#   │  └─ N4096/...
#
# Estructura discriminable:
#   - Tamaño: N{512,1024,2048,4096}
#   - Threads: T{2,4,8} (por VARIABLE DE ENTORNO)
#   - Optimización: O{1,2,3}
#   - Ejecución: local vs cluster
#
# IMPORTANTE: Los threads se pasan por VARIABLE DE ENTORNO (OMP_NUM_THREADS)
# Se establece antes de compilar en los scripts de cluster
# ============================================================

set -e

matrix_sizes=(512 1024 2048 4096)
optimization_levels=(1 2 3)
thread_counts=(2 4 8)

# Crear directorio raíz
SALIDAS_DIR="./salidas"
TIPO_EXEC="openmp"
mkdir -p "$SALIDAS_DIR/$TIPO_EXEC"

echo "======================================"
echo "SCRIPT DE EJECUCIÓN LOCAL - OpenMP"
echo "======================================"
echo "Directorio de salidas: $SALIDAS_DIR/$TIPO_EXEC"
echo "Threads: ${thread_counts[@]} (por OMP_NUM_THREADS)"
echo ""

process_output() {
	local output="$1"
	local matrix_size="$2"
	local optimization_level="$3"
	local thread_count="$4"
	local output_dir="$5"
	local report_file="$6"

	local result_line=$(echo "$output" | grep "^RESULT;" || echo "RESULT;ERROR")
	local validation_line=$(echo "$output" | grep "^VALIDATION;" || echo "VALIDATION;ERROR")

	{
		echo "=== REPORTE DE EJECUCIÓN ==="
		echo "Tipo: OpenMP"
		echo "Matriz Size: $matrix_size"
		echo "Threads: $thread_count (OMP_NUM_THREADS)"
		echo "Optimization: -O$optimization_level"
		echo "Fecha: $(date '+%Y-%m-%d %H:%M:%S')"
		echo "Host: $(hostname)"
		echo ""
		echo "=== RESULTADO ==="
		echo "$result_line"
		echo "$validation_line"
		echo ""
		echo "=== SALIDA COMPLETA ==="
		echo "$output"
	} > "$report_file"
}

echo "Compilando matrices-open-mp.c con -fopenmp..."
for o in "${optimization_levels[@]}"; do
	echo -n "  -O$o... "
	if gcc -O${o} -fopenmp ./matrices-open-pm.c -o ./matrices-openmp-O${o} -lm 2>/dev/null; then
		echo "✓"
	else
		echo "✗ ERROR"
		exit 1
	fi
done
echo ""

for n in "${matrix_sizes[@]}"; do
	echo "=================================================="
	echo "Procesando: N=$n (OpenMP)"
	echo "=================================================="

	for t in "${thread_counts[@]}"; do
		for o in "${optimization_levels[@]}"; do
			# Estructura: salidas/openmp/N512/T2/O1/{outputs,reports}
			EXEC_DIR="$SALIDAS_DIR/$TIPO_EXEC/N${n}/T${t}/O${o}"
			OUTPUT_DIR="$EXEC_DIR/outputs"
			REPORT_DIR="$EXEC_DIR/reports"
			mkdir -p "$OUTPUT_DIR"
			mkdir -p "$REPORT_DIR"

			echo -n "  T$t-O$o... "

			# Ejecutar con threads por VARIABLE DE ENTORNO
			timestamp=$(date +%s%N)
			export OMP_NUM_THREADS=$t
			output=$(./matrices-openmp-O${o} $n 2>&1)
			exit_code=$?

			# Guardar salida
			output_file="$OUTPUT_DIR/openmp_${timestamp}.txt"
			echo "$output" > "$output_file"

			# Crear reporte
			report_file="$REPORT_DIR/openmp_${timestamp}.txt"
			process_output "$output" "$n" "$o" "$t" "$OUTPUT_DIR" "$report_file"

			# Resumen
			echo "$output" | grep "^RESULT;" | awk -F';' '{printf "T=%.6fs\n", $3}' || echo "ERROR"

		done
	done
	echo ""
done

# Limpiar variable de entorno
unset OMP_NUM_THREADS

echo "======================================"
echo "Ejecuciones completadas"
echo "Estructura de salidas:"
echo "  $SALIDAS_DIR/$TIPO_EXEC/N{512,1024,2048,4096}/T{2,4,8}/O{1,2,3}/{outputs,reports}"
echo "======================================"
