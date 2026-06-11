#!/bin/bash
#SBATCH -J H_P64T8
#SBATCH -N 1
#SBATCH --tasks-per-node=8
#SBATCH --exclusive
#SBATCH -o /home/patricio/Escritorio/sistemas-paralelos/TP3/script/runs/P64T8_%j.out
#SBATCH -e /home/patricio/Escritorio/sistemas-paralelos/TP3/script/runs/P64T8_%j.err

OUTPUT_FILE="/home/patricio/Escritorio/sistemas-paralelos/TP3/times.txt"

echo "=== BENCHMARK HÍBRIDO MPI+OPENMP ===" > $OUTPUT_FILE
echo "Config: P=64 T=8 (8 MPI × 8 OMP)" >> $OUTPUT_FILE
echo "Nodos: 1 | MPI por nodo: 8 | Cores usados/nodo: 64/64" >> $OUTPUT_FILE
echo "Fecha: $(date)" >> $OUTPUT_FILE
echo "----------------------------------------" >> $OUTPUT_FILE

echo "Iniciando tanda para P=64..."

export OMP_NUM_THREADS=8
for N in 2048 4096
do
    echo "Configuracion: P=64 | Matriz N: $N" >> $OUTPUT_FILE
    mpirun --bind-to none -np 8 /home/patricio/Escritorio/sistemas-paralelos/TP3/hibrido-pato $N 8 >> $OUTPUT_FILE
    echo "----------------------------------------" >> $OUTPUT_FILE
done

echo "=== Pruebas P=64 finalizadas. Resultados en $OUTPUT_FILE ==="
