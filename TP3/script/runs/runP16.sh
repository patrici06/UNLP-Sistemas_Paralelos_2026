#!/bin/bash
#SBATCH -J MPI_P16
#SBATCH -N 2
#SBATCH --tasks-per-node=8
#SBATCH --exclusive
#SBATCH -o /home/patricio/Escritorio/sistemas-paralelos/TP3/script/runs/P16_%j.out
#SBATCH -e /home/patricio/Escritorio/sistemas-paralelos/TP3/script/runs/P16_%j.err

OUTPUT_FILE="/home/patricio/Escritorio/sistemas-paralelos/TP3/times.txt"

echo "=== BENCHMARK MPI PURO ===" > $OUTPUT_FILE
echo "Config: P=16 MPI" >> $OUTPUT_FILE
echo "Nodos: 2 | MPI por nodo: 8" >> $OUTPUT_FILE
echo "Fecha: $(date)" >> $OUTPUT_FILE
echo "----------------------------------------" >> $OUTPUT_FILE

echo "Iniciando tanda para P=16..."

for N in 2048 4096
do
    echo "Configuracion: P=16 MPI | Matriz N: $N" >> $OUTPUT_FILE
    mpirun -np 16 /home/patricio/Escritorio/sistemas-paralelos/TP3/matrices-mpi $N 16 >> $OUTPUT_FILE
    echo "----------------------------------------" >> $OUTPUT_FILE
done

echo "=== Pruebas P=16 finalizadas. Resultados en $OUTPUT_FILE ==="
