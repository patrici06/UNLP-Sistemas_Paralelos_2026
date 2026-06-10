#!/bin/bash
#SBATCH -N 4
#SBATCH --exclusive
#SBATCH --tasks-per-node=8
#SBATCH -o /nethome/spusuario21/TP3/script-cluster/blade/mpi-P32-N4/N4096/output_%j.txt
#SBATCH -e /nethome/spusuario21/TP3/script-cluster/blade/mpi-P32-N4/N4096/errors_%j.txt
mpirun /nethome/spusuario21/TP3/script-cluster/bin/mpi-sec 4096 32
