#!/bin/bash
#SBATCH -J MPI_P32
#SBATCH -N 4
#SBATCH --tasks-per-node=8
#SBATCH --exclusive
#SBATCH -o /home/patricio/Escritorio/sistemas-paralelos/TP3/script/runs/P32_%j.out
#SBATCH -e /home/patricio/Escritorio/sistemas-paralelos/TP3/script/runs/P32_%j.err

OUTPUT_FILE="/home/patricio/Escritorio/sistemas-paralelos/TP3/times.txt"

echo "=== BENCHMARK MPI PURO ===" > $OUTPUT_FILE
echo "Config: P=32 MPI" >> $OUTPUT_FILE
echo "Nodos: 4 | MPI por nodo: 8" >> $OUTPUT_FILE
echo "Fecha: $(date)" >> $OUTPUT_FILE
echo "----------------------------------------" >> $OUTPUT_FILE

echo "Iniciando tanda para P=32..."

for N in 2048 4096
do
    echo "Configuracion: P=32 MPI | Matriz N: $N" >> $OUTPUT_FILE
    mpirun -np 32 /home/patricio/Escritorio/sistemas-paralelos/TP3/matrices-mpi $N 32 >> $OUTPUT_FILE
    echo "----------------------------------------" >> $OUTPUT_FILE
done

echo "=== Pruebas P=32 finalizadas. Resultados en $OUTPUT_FILE ==="
