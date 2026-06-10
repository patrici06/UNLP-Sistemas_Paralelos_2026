#!/bin/bash
#SBATCH -N 2
#SBATCH --exclusive
#SBATCH --tasks-per-node=8
#SBATCH --cpus-per-task=1
#SBATCH -o /nethome/spusuario21/TP3/script-cluster/blade/hybrid-P16-T1-N2/N1024/output_%j.txt
#SBATCH -e /nethome/spusuario21/TP3/script-cluster/blade/hybrid-P16-T1-N2/N1024/errors_%j.txt
export OMP_NUM_THREADS=1
mpirun --bind-to none /nethome/spusuario21/TP3/script-cluster/bin/hybrid 1024 16 1
