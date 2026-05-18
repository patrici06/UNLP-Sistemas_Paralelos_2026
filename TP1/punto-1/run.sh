#!/bin/bash
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH --partition=Blade
#SBATCH --time=00:05:00

FILE=$1
#PARAM=$2
TIME=$2

./${FILE} > output${TIME}.txt 2> errors${TIME}.txt
