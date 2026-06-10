#!/bin/bash
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH --tasks-per-node=8
#SBATCH -o /nethome/spusuario21/TP3/script-cluster/blade/mpi-P8-N1/N1024/output_%j.txt
#SBATCH -e /nethome/spusuario21/TP3/script-cluster/blade/mpi-P8-N1/N1024/errors_%j.txt
mpirun /nethome/spusuario21/TP3/script-cluster/bin/mpi-sec 1024 8
