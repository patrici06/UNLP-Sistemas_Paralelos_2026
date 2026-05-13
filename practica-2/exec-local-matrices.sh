#!/bin/bash

# ============================================================
# Script de ejecución LOCAL para matrices.c (SECUENCIAL)
# ============================================================
# Estructura de almacenamiento:
#   salidas/
#   ├─ matrices/
#   │  ├─ N512/
#   │  │  ├─ O1/outputs/
#   │  │  ├─ O1/reports/
#   │  │  ├─ O2/outputs/
#   │  │  ├─ O2/reports/
#   │  │  ├─ O3/outputs/
#   │  │  └─ O3/reports/
#   │  ├─ N1024/
#   │  ├─ N2048/
#   │  └─ N4096/
#
# Permite discriminar fácilmente:
#   - Tamaño de matriz: N{512,1024,2048,4096}
#   - Nivel de optimización: O{1,2,3}
#   - Tipo de ejecución: local vs cluster
#
# Salida de programa: RESULT;size;time;checksum
# ============================================================

set -e

matrix_sizes=(512 1024 2048 4096)
optimization_levels=(1 2 3)

# Crear directorio raíz de salidas
SALIDAS_DIR="./salidas"
TIPO_EXEC="matrices"  # secuencial
mkdir -p "$SALIDAS_DIR/$TIPO_EXEC"

echo "======================================"
echo "SCRIPT DE EJECUCIÓN LOCAL - SECUENCIAL"
echo "======================================"
echo "Directorio de salidas: $SALIDAS_DIR/$TIPO_EXEC"
echo ""

# Función para procesar salida y crear reportes
process_output() {
	local output="$1"
	local matrix_size="$2"
	local optimization_level="$3"
	local output_dir="$4"
	local report_file="$5"

	# Extraer líneas RESULT y VALIDATION
	local result_line=$(echo "$output" | grep "^RESULT;" || echo "RESULT;ERROR")
	local validation_line=$(echo "$output" | grep "^VALIDATION;" || echo "VALIDATION;ERROR")

	# Crear reporte
	{
		echo "=== REPORTE DE EJECUCIÓN ==="
		echo "Tipo: SECUENCIAL (matrices.c)"
		echo "Matriz Size: $matrix_size"
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

# Compilar ejecutables para cada nivel de optimización
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

# Procesar cada tamaño de matriz
for n in "${matrix_sizes[@]}"; do
	echo "=================================================="
	echo "Procesando: N=$n (SECUENCIAL)"
	echo "=================================================="

	for o in "${optimization_levels[@]}"; do
		# Crear estructura de directorios
		EXEC_DIR="$SALIDAS_DIR/$TIPO_EXEC/N${n}/O${o}"
		OUTPUT_DIR="$EXEC_DIR/outputs"
		REPORT_DIR="$EXEC_DIR/reports"
		mkdir -p "$OUTPUT_DIR"
		mkdir -p "$REPORT_DIR"

		echo -n "  O$o... "

		# Ejecutar y capturar salida
		timestamp=$(date +%s%N)
		output=$(./matrices-O${o} $n 2>&1)
		exit_code=$?

		# Guardar salida completa
		output_file="$OUTPUT_DIR/matrices_${timestamp}.txt"
		echo "$output" > "$output_file"

		# Procesar y crear reporte
		report_file="$REPORT_DIR/matrices_${timestamp}.txt"
		process_output "$output" "$n" "$o" "$OUTPUT_DIR" "$report_file"

		# Mostrar resumen
		echo "$output" | grep "^RESULT;" | awk -F';' '{printf "T=%.6fs\n", $3}' || echo "ERROR"

	done
	echo ""
done

echo "======================================"
echo "Ejecuciones completadas"
echo "Estructura de salidas:"
echo "  $SALIDAS_DIR/$TIPO_EXEC/N{512,1024,2048,4096}/O{1,2,3}/{outputs,reports}"
echo "======================================"
