#!/bin/bash

BINARIOS="bin"
ORIGEN="files"
SALIDAS="secuencial-hybrid"
ERRORES="err-sec-hybrid"
BINARIO="hybrid"

mkdir -p ./$BINARIOS ./$ORIGEN ./$SALIDAS ./$ERRORES

echo "Compilando binario..."
mpicc ../matrices-hybrid.c -o ./$BINARIOS/$BINARIO -fopenmp -lm

echo "----------------------------------------"

PROCS=1
THREADS=1

for i in 512 1024 2048 4046; do
	echo "Enviando trabajo secuencial hybrid con carga: ${i}, procesos: ${PROCS}, hilos: ${THREADS}"
	if sbatch -N 1 --ntasks-per-node=1 --cpus-per-task=$THREADS --exclusive \
		-o "$SALIDAS/output_%j.txt" -e "$ERRORES/errors_%j.txt" \
		./secuenciaHybridoMP.sh "$i" "$PROCS" "$THREADS"; then
		echo "Trabajo con carga ${i} enviado exitosamente"
	else
		echo "Error: falló el envío del trabajo con carga ${i}"
	fi
	echo "----------------------------------------"
done
