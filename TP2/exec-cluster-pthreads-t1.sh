#!/bin/bash

# ============================================================
# Script para CLUSTER - matrices-pthread.c (T=1 SOLO)
# ============================================================
# Compilación: EN CADA NODO
# Parámetros:
#   - Tamaños: N = {512, 1024, 2048, 4096}
#   - Compilación: -O2 -pthread (en el nodo cluster)
#
# Estructura de salida:
#   salidas/pthread-t1/N512/outputs/exec_*.txt
#   salidas/pthread-t1/N1024/outputs/exec_*.txt
#   salidas/pthread-t1/N2048/outputs/exec_*.txt
#   salidas/pthread-t1/N4096/outputs/exec_*.txt
# ============================================================

set -e

matrix_sizes=(512 1024 2048 4096)
SALIDAS_DIR="./salidas"

echo "======================================"
echo "ENVÍO DE TRABAJOS AL CLUSTER"
echo "Compilación: EN CADA NODO"
echo "======================================"
echo ""

for n in "${matrix_sizes[@]}"; do
    EXEC_DIR="$SALIDAS_DIR/pthread-t1/N${n}"
    OUTPUT_DIR="$EXEC_DIR/outputs"
    mkdir -p "$OUTPUT_DIR"
    
    echo -n "  Enviando N=$n... "
    
    job_id=$(sbatch \
        -o "$OUTPUT_DIR/exec_%j.txt" \
        --job-name="pthread-t1-N${n}" \
        --time=00:15:00 \
        --partition=Blade \
        -N 1 --exclusive \
        ./job-pthread-t1.sh $n 2>&1 | awk '{print $NF}')
    
    echo "Job ID: $job_id"
done

echo ""
echo "======================================"
echo "Esperando jobs..."
echo "Verifica con: squeue -u \$USER"
echo "Resultados en: salidas/pthread-t1/N*/outputs/exec_*.txt"
echo "======================================"
