#!/bin/bash

BIN="bin"
mkdir -p ./$BIN

echo "Compilando binario Hybrid..."
mpicc ../matrices-hybrid.c -o ./$BIN/hybrid -fopenmp -lm

echo "----------------------------------------"

# 8 cores total (1 nodo x 8 cores)
# T=4 → 2 tasks MPI (2x4=8 cores/nodo)
P_MPI=2
THREADS=4
NODES=1

for n in 512 1024 2048 4096; do
    DIR="hybrid-P8-T${THREADS}-N${NODES}/N${n}"
    mkdir -p "$DIR"
    echo "HYBRID | P_MPI=${P_MPI} | T=${THREADS} | cores=${NODES}x8=$((NODES*8)) | N=${n} | → ${DIR}/"
    sbatch -N $NODES --ntasks-per-node=$P_MPI --cpus-per-task=$THREADS --exclusive \
        -o "$DIR/output_%j.txt" -e "$DIR/errors_%j.txt" \
        ./secuenciaHybridoMP.sh "$n" "$P_MPI" "$THREADS"
    echo "----------------------------------------"
done
