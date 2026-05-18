#!/bin/bash

parameters_values=(512 1024 2048 4096)
bs=${1:-32}  # Block size por defecto 32 si no se proporciona

for p in "${parameters_values[@]}"; do
	dir="./${p}-BASE${bs}"
	mkdir -p "$dir/outputs"
	mkdir -p "$dir/errors"

	for o in 1 2 3; do
		# Alterar el valor de BS en el archivo antes de compilar
		sed -i "s/#define BS [0-9]*/#define BS $bs/" ./re-matrices.c
		
		gcc -O${o} ./re-matrices.c -o ./re-entregaO${o}
		sleep 2

		echo "Enviando job con parametro=$p O=$o BS=$bs"

		# Enviar job y esperar a que termine con --wait
		sbatch \
			-o "$dir/outputs/output_O${o}_%j.txt" \
			-e "$dir/errors/error_O${o}_%j.txt" \
			./script-1.sh $p $o $bs
		
		echo "Job completado para parametro=$p O=$o BS=$bs"
	done
done
