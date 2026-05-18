#!/bin/bash

parameters_values=(512 1024 2048 4096)
bs=${1:-32}  # Block size por defecto 32 si no se proporciona

# Crear directorio principal de reportes
mkdir -p reports

# Función para procesar salida y crear reportes
process_output() {
	local output="$1"
	local matrix_size="$2"
	local optimization_level="$3"
	local block_size="$4"
	local output_dir="$5"
	local report_file="$6"

	# Extraer líneas RESULT y VALIDATION
	local result_line=$(echo "$output" | grep "^RESULT;")
	local validation_line=$(echo "$output" | grep "^VALIDATION;")

	# Crear reporte
	{
		echo "=== REPORTE DE EJECUCIÓN ==="
		echo "Matriz Size: $matrix_size"
		echo "Optimization Level: -O$optimization_level"
		echo "Block Size: $block_size"
		echo "Fecha: $(date)"
		echo ""
		echo "=== SALIDA DEL PROGRAMA ==="
		echo "$result_line"
		echo "$validation_line"
		echo ""
		echo "=== SALIDA COMPLETA ==="
		echo "$output"
	} > "$report_file"
}

for p in "${parameters_values[@]}"; do
	dir="./${p}-BASE${bs}"
	mkdir -p "$dir/outputs"
	mkdir -p "$dir/errors"
	mkdir -p "$dir/reports"

	echo "======================================"
	echo "Procesando matriz de tamaño: $p BS=$bs"
	echo "======================================"

	for o in 1 2 3; do
		echo ""
		# Alterar el valor de BS en el archivo antes de compilar
		sed -i "s/#define BS [0-9]*/#define BS $bs/" ./re-matrices.c
		
		echo "Compilando con -O$o BS=$bs..."
		gcc -O${o} ./re-matrices.c -o ./re-entregaO${o}
		
		if [ $? -ne 0 ]; then
			echo "ERROR: Compilación fallida para -O$o"
			continue
		fi

		echo "Ejecutando con parametro=$p O=$o BS=$bs"
		
		# Ejecutar localmente y capturar salida
		output=$(./re-entregaO${o} $p 2>&1)
		exit_code=$?

		# Guardar salida completa en archivo
		output_file="$dir/outputs/output_local_O${o}_$(date +%s).txt"
		echo "$output" > "$output_file"

		# Procesar y crear reporte
		report_file="$dir/reports/report_O${o}_$(date +%s).txt"
		process_output "$output" "$p" "$o" "$bs" "$dir/outputs" "$report_file"

		echo "Salida guardada en: $output_file"
		echo "Reporte generado en: $report_file"
		echo ""
		echo "Resumen:"
		echo "$output" | grep "^RESULT;"
		echo "$output" | grep "^VALIDATION;"

	done

	echo ""
done

echo "======================================"
echo "Todas las ejecuciones completadas"
echo "======================================"
