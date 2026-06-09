#!/bin/bash

BINARIO="bin/mpi-sec"

# --map-by socket:pe=1 asigna 1 core por proceso MPI
# --bind-to core fija cada proceso a un core sin migracion
mpirun --map-by socket:pe=1 --bind-to core ./$BINARIO $1 $2
