#!/bin/bash
#SBATCH -J H_P1T1
#SBATCH -N 1
#SBATCH --tasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --exclusive
#SBATCH -o /home/patricio/Escritorio/sistemas-paralelos/TP3/script/runs/P1T1_%j.out
#SBATCH -e /home/patricio/Escritorio/sistemas-paralelos/TP3/script/runs/P1T1_%j.err

SCRIPT_DIR=$(dirname "$(realpath "$0")")
BASE_DIR=$(realpath "$SCRIPT_DIR/../..")
OUTPUT_FILE="$BASE_DIR/times.txt"

echo "=== BENCHMARK HÍBRIDO MPI+OPENMP (P=1 T=1) ===" > $OUTPUT_FILE
echo "Config: P=1 T=1 (1 MPI × 1 OMP)" >> $OUTPUT_FILE
echo "Nodos: 1 | MPI por nodo: 1 | Cores usados/nodo: 1" >> $OUTPUT_FILE
echo "Fecha: $(date)" >> $OUTPUT_FILE
echo "----------------------------------------" >> $OUTPUT_FILE

echo "Iniciando tanda para P=1..."

export OMP_NUM_THREADS=1
for N in 512 1024 2048 4096
do
    echo "Configuracion: P=1 | Matriz N: $N" >> $OUTPUT_FILE
    mpirun --bind-to none -np 1 $BASE_DIR/hibrido $N 1 >> $OUTPUT_FILE
    echo "----------------------------------------" >> $OUTPUT_FILE
done

echo "=== Pruebas P=1 finalizadas. Resultados en $OUTPUT_FILE ==="
