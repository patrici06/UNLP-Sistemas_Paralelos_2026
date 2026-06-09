#!/bin/bash
#
# ej6-hybrid-matmul.slurm
# Script SLURM para Ejercicio 6 (MPI + OpenMP multiplicacion de matrices)
#
# Parametros segun practica:
#   N = {512, 1024, 2048}
#   Ejecutar en 1 y 2 nodos, 1 proceso MPI por nodo, varios hilos OpenMP por nodo.
#
# IMPORTANTE: Antes de usar este script debe existir el archivo fuente
#   Fuentes/hybrid_matmul.c (implementacion propia del ejercicio 6)
#
# Uso en cluster:   sbatch ej6-hybrid-matmul.slurm
#

#SBATCH --job-name=ej6-hybrid-matmul
#SBATCH -N 2
#SBATCH --exclusive
#SBATCH --tasks-per-node=1
#SBATCH -o logs-cluster/ej6-hybrid-matmul-%j.out
#SBATCH -e logs-cluster/ej6-hybrid-matmul-%j.err

set -e

cd /home/patricio/Escritorio/sistemas-paralelos/practica-4

SRC_DIR="Fuentes"
BIN_DIR="bin"
LOG_DIR="scripts-cluster/logs-cluster"

mkdir -p "$BIN_DIR" "$LOG_DIR"

# Compilar con soporte OpenMP (descomentar cuando exista el fuente)
# SRC="$SRC_DIR/hybrid_matmul.c"
# BIN="$BIN_DIR/hybrid_matmul"
# echo "==> [Ej6] Compilando $SRC con OpenMP ..."
# mpicc "$SRC" -o "$BIN" -fopenmp -lm

# echo ""
# echo "==> [Ej6] Ejecutando hybrid-matmul ..."
# echo ""

# SIZES=(512 1024 2048)
# NODES_LIST=(1 2)
# THREADS_LIST=(2 4)

# for NODES in "${NODES_LIST[@]}"; do
#     for T in "${THREADS_LIST[@]}"; do
#         export OMP_NUM_THREADS=$T
#         LOGFILE="$LOG_DIR/ej6-hybrid-matmul-N${NODES}-T${T}-Nsize${N}.log"
#         echo "--- Ejercicio 6 | Nodos=$NODES, Hilos=$T, N=$N ---" | tee "$LOGFILE"
#
#         mpirun --bind-to none -np "$NODES" ./"$BIN" "$N" | tee -a "$LOGFILE"
#
#         echo "    Log guardado en: $LOGFILE"
#         echo ""
#     done
# done

# echo "==> [Ej6] Finalizado."

echo "ATENCION: Este script es una plantilla."
echo "Debes implementar Fuentes/hybrid_matmul.c y descomentar las lineas de compilacion/ejecucion."
