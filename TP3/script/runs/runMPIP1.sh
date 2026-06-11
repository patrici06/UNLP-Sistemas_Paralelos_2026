#!/bin/bash
#SBATCH -J MPI_P1
#SBATCH -N 1
#SBATCH --tasks-per-node=1
#SBATCH --exclusive
#SBATCH -o /home/patricio/Escritorio/sistemas-paralelos/TP3/script/runs/P1_%j.out
#SBATCH -e /home/patricio/Escritorio/sistemas-paralelos/TP3/script/runs/P1_%j.err

SCRIPT_DIR=$(dirname "$(realpath "$0")")
BASE_DIR=$(realpath "$SCRIPT_DIR/../..")
OUTPUT_FILE="$BASE_DIR/times.txt"

echo "=== BENCHMARK MPI PURO (P=1) ===" > $OUTPUT_FILE
echo "Config: P=1 MPI" >> $OUTPUT_FILE
echo "Nodos: 1 | MPI por nodo: 1" >> $OUTPUT_FILE
echo "Fecha: $(date)" >> $OUTPUT_FILE
echo "----------------------------------------" >> $OUTPUT_FILE

echo "Iniciando tanda para P=1..."

for N in 512 1024 2048 4096
do
    echo "Configuracion: P=1 MPI | Matriz N: $N" >> $OUTPUT_FILE
    mpirun -np 1 $BASE_DIR/matrices-mpi $N 1 >> $OUTPUT_FILE
    echo "----------------------------------------" >> $OUTPUT_FILE
done

echo "=== Pruebas P=1 finalizadas. Resultados en $OUTPUT_FILE ==="
