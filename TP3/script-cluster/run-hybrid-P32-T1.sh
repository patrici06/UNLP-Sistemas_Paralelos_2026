#!/bin/bash

BIN="bin"
mkdir -p ./$BIN

echo "Compilando binario Hybrid..."
mpicc ../matrices-hybrid.c -o ./$BIN/hybrid -fopenmp -lm

echo "----------------------------------------"

# 32 cores total (4 nodos x 8 cores)
# T=1 → 8 tasks MPI por nodo (8x1=8 cores/nodo) → MPI puro
P_MPI=32
THREADS=1
NODES=4

for n in 512 1024 2048 4096; do
    DIR="hybrid-P32-T${THREADS}-N${NODES}/N${n}"
    mkdir -p "$DIR"
    echo "HYBRID | P_MPI=${P_MPI} | T=${THREADS} | cores=${NODES}x8=$((NODES*8)) | N=${n} | → ${DIR}/"
    sbatch -N $NODES --ntasks-per-node=$((P_MPI/NODES)) --cpus-per-task=$THREADS --exclusive \
        -o "$DIR/output_%j.txt" -e "$DIR/errors_%j.txt" \
        ./secuenciaHybridoMP.sh "$n" "$P_MPI" "$THREADS"
    echo "----------------------------------------"
done
