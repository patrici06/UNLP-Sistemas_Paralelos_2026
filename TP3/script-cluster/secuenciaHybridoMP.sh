#!/bin/bash

BINARIO="bin/hybrid"

export OMP_NUM_THREADS=$3

# --map-by socket:pe=OMP_NUM_THREADS asigna pe=hilos por proceso MPI
# --bind-to core fija los hilos a cores individuales sin migracion
mpirun --map-by socket:pe=$OMP_NUM_THREADS --bind-to core ./$BINARIO $1 $2 $3
