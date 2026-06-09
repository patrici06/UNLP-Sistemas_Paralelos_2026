#!/bin/bash

BIN="bin"
mkdir -p ./$BIN

echo "Compilando binario Hybrid..."
mpicc ../matrices-hybrid.c -o ./$BIN/hybrid -fopenmp -lm

echo "----------------------------------------"

# 4 nodos, 1 proceso por nodo, 4 hilos por proceso
P_MPI=4
THREADS=4
NODES=4

for n in 512 1024 2048 4096; do
    DIR="hybrid-P4-T${THREADS}-N${NODES}/N${n}"
    mkdir -p "$DIR"
    echo "HYBRID | P_MPI=${P_MPI} | T=${THREADS} | nodos=${NODES} | 1 proceso/nodo × 4 hilos | N=${n} | → ${DIR}/"
    sbatch -N $NODES --ntasks-per-node=1 --cpus-per-task=$THREADS --exclusive \
        -o "$DIR/output_%j.txt" -e "$DIR/errors_%j.txt" \
        ./secuenciaHybridoMP.sh "$n" "$P_MPI" "$THREADS"
    echo "----------------------------------------"
done
