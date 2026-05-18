#!/bin/bash
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH --partition=Blade
#SBATCH --time=00:10:00

echo "Running N=$1 O=$2 BS=$3"

./re-entregaO${2} $1
