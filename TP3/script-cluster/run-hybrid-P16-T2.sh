#!/bin/bash

BIN="bin"
mkdir -p ./$BIN

echo "Compilando binario Hybrid..."
mpicc ../matrices-hybrid.c -o ./$BIN/hybrid -fopenmp -lm

echo "----------------------------------------"

# 16 cores total (2 nodos x 8 cores)
# T=2 → 4 tasks MPI por nodo (4x2=8 cores/nodo)
P_MPI=8
THREADS=2
NODES=2

for n in 512 1024 2048 4096; do
    DIR="hybrid-P16-T${THREADS}-N${NODES}/N${n}"
    mkdir -p "$DIR"
    echo "HYBRID | P_MPI=${P_MPI} | T=${THREADS} | cores=${NODES}x8=$((NODES*8)) | N=${n} | → ${DIR}/"
    sbatch -N $NODES --ntasks-per-node=$((P_MPI/NODES)) --cpus-per-task=$THREADS --exclusive \
        -o "$DIR/output_%j.txt" -e "$DIR/errors_%j.txt" \
        ./secuenciaHybridoMP.sh "$n" "$P_MPI" "$THREADS"
    echo "----------------------------------------"
done
