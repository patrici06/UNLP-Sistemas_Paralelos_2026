#!/bin/bash

BIN="bin"
mkdir -p ./$BIN

echo "Compilando binario Hybrid..."
mpicc ../matrices-hybrid.c -o ./$BIN/hybrid -fopenmp -lm

echo "----------------------------------------"

P_MPI=16
THREADS=2
NODES=4

for n in 512 1024 2048 4096; do
    DIR="hybrid-P${P_MPI}-T${THREADS}-N${NODES}/N${n}"
    mkdir -p "$DIR"
    echo "HYBRID | P_MPI=${P_MPI} | T=${THREADS} | nodos=${NODES} | N=${n} | → ${DIR}/"
    sbatch -N $NODES --ntasks-per-node=$((P_MPI/NODES)) --cpus-per-task=$THREADS --exclusive \
        -o "$DIR/output_%j.txt" -e "$DIR/errors_%j.txt" \
        ./secuenciaHybridoMP.sh "$n" "$P_MPI" "$THREADS"
    echo "----------------------------------------"
done
