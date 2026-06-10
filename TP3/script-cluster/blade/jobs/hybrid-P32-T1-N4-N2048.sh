#!/bin/bash
#SBATCH -N 4
#SBATCH --exclusive
#SBATCH --tasks-per-node=8
#SBATCH --cpus-per-task=1
#SBATCH -o /nethome/spusuario21/TP3/script-cluster/blade/hybrid-P32-T1-N4/N2048/output_%j.txt
#SBATCH -e /nethome/spusuario21/TP3/script-cluster/blade/hybrid-P32-T1-N4/N2048/errors_%j.txt
export OMP_NUM_THREADS=1
mpirun --bind-to none /nethome/spusuario21/TP3/script-cluster/bin/hybrid 2048 32 1
