#!/bin/bash
#SBATCH -J H_P16T8
#SBATCH -N 1
#SBATCH --tasks-per-node=2
#SBATCH --exclusive
#SBATCH -o /home/patricio/Escritorio/sistemas-paralelos/TP3/script/runs/P16T8_%j.out
#SBATCH -e /home/patricio/Escritorio/sistemas-paralelos/TP3/script/runs/P16T8_%j.err

OUTPUT_FILE="/home/patricio/Escritorio/sistemas-paralelos/TP3/times.txt"

echo "=== BENCHMARK HÍBRIDO MPI+OPENMP ===" > $OUTPUT_FILE
echo "Config: P=16 T=8 (2 MPI × 8 OMP)" >> $OUTPUT_FILE
echo "Nodos: 1 | MPI por nodo: 2 | Cores usados/nodo: 16/64" >> $OUTPUT_FILE
echo "Fecha: $(date)" >> $OUTPUT_FILE
echo "----------------------------------------" >> $OUTPUT_FILE

echo "Iniciando tanda para P=16..."

export OMP_NUM_THREADS=8
for N in 2048 4096
do
    echo "Configuracion: P=16 | Matriz N: $N" >> $OUTPUT_FILE
    mpirun --bind-to none -np 2 /home/patricio/Escritorio/sistemas-paralelos/TP3/hibrido-pato $N 2 >> $OUTPUT_FILE
    echo "----------------------------------------" >> $OUTPUT_FILE
done

echo "=== Pruebas P=16 finalizadas. Resultados en $OUTPUT_FILE ==="
