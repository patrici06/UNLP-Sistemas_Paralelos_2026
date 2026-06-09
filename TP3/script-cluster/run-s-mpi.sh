#!/bin/bash

BINARIOS="bin"
ORIGEN="files"
SALIDAS="secuencial-mpi"
ERRORES="err-sec-mpi"
BINARIO="mpi-sec"

mkdir -p ./$BINARIOS ./$ORIGEN ./$SALIDAS ./$ERRORES

echo "Compilando binario..."
mpicc ../matrices-mpi.c -o ./$BINARIOS/$BINARIO

echo "----------------------------------------"

PROCS=1

for i in 512 1024 2048 4046; do
	echo "Enviando trabajo secuencial MPI con carga: ${i}, procesos: ${PROCS}"
	if sbatch -N 1 --ntasks-per-node=1 --exclusive \
		-o "$SALIDAS/output_%j.txt" -e "$ERRORES/errors_%j.txt" \
		./secuencialMpi.sh "$i" "$PROCS"; then
		echo "Trabajo con carga ${i} enviado exitosamente"
	else
		echo "Error: falló el envío del trabajo con carga ${i}"
	fi
	echo "----------------------------------------"
done
