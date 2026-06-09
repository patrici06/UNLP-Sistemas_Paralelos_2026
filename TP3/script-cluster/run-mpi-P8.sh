#!/bin/bash

BIN="bin"
mkdir -p ./$BIN

echo "Compilando binario MPI..."
mpicc ../matrices-mpi.c -o ./$BIN/mpi-sec -lm

echo "----------------------------------------"

P=8
NODES=1

for n in 512 1024 2048 4096; do
    DIR="mpi-P${P}-N${NODES}/N${n}"
    mkdir -p "$DIR"
    echo "MPI | P=${P} | N=${n} | nodos=${NODES} | → ${DIR}/"
    sbatch -N $NODES --ntasks-per-node=8 --exclusive \
        -o "$DIR/output_%j.txt" -e "$DIR/errors_%j.txt" \
        ./secuencialMpi.sh "$n" "$P"
    echo "----------------------------------------"
done
