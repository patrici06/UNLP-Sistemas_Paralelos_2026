#!/bin/bash
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH --partition=XeonPHI
#SBATCH --tasks-per-node=16
#SBATCH -o /nethome/spusuario21/TP3/script-cluster/xeonphi/mpi-P16-N1/N1024/output_%j.txt
#SBATCH -e /nethome/spusuario21/TP3/script-cluster/xeonphi/mpi-P16-N1/N1024/errors_%j.txt
source $ONEAPI_PATH/setvars.sh > /dev/null 2>&1
mkdir -p "/nethome/spusuario21/TP3/script-cluster/bin"
mpiicc -o "/nethome/spusuario21/TP3/script-cluster/bin/mpi-xeonphi" "/nethome/spusuario21/TP3/matrices-mpi.c"
mpirun "/nethome/spusuario21/TP3/script-cluster/bin/mpi-xeonphi" 1024 16
