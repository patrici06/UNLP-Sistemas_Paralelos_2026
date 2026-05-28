#!/bin/bash

# ============================================================
# Script para CLUSTER - matrices.c (Referencia Secuencial)
# ============================================================
# Este script:
#   1. Envía trabajos para N={512, 1024, 2048, 4096}
#   2. Cada nodo compila matrices.c con -O2
#   3. T=1 (solo secuencial)
#
# Parámetros:
#   - Tamaños: N = {512, 1024, 2048, 4096}
#   - Thread: T = 1 (único, secuencial)
#   - Compilación: -O2 (EN EL NODO)
#
# Estructura de salida:
#   salidas/matrices/N512/outputs/
#   salidas/matrices/N1024/outputs/
#   salidas/matrices/N2048/outputs/
#   salidas/matrices/N4096/outputs/
#
# USO: ./exec-cluster-matrices.sh
# ============================================================

set -e

matrix_sizes=(512 1024 2048 4096)

SALIDAS_DIR="./salidas"

echo "======================================"
echo "ENVÍO DE TRABAJOS AL CLUSTER"
echo "(Compilación en cada nodo)"
echo "======================================"
echo ""

for n in "${matrix_sizes[@]}"; do
    EXEC_DIR="$SALIDAS_DIR/matrices/N${n}"
    OUTPUT_DIR="$EXEC_DIR/outputs"
    mkdir -p "$OUTPUT_DIR"
    
    echo -n "  Enviando N=$n (matrices.c)... "
    
    # Enviar job a cluster
    job_id=$(sbatch \
        -o "$OUTPUT_DIR/exec_%j.txt" \
        --job-name="matrices-N${n}" \
        --time=00:15:00 \
        --partition=Blade \
        -N 1 --exclusive \
        ./job-matrices.sh $n 2>&1 | awk '{print $NF}')
    
    echo "Job ID: $job_id"
done

echo ""
echo "======================================"
echo "Todos los trabajos han sido enviados"
echo "======================================"
echo "Monitorea con: squeue -u \$USER"
echo "Ver salidas en:"
echo "  salidas/matrices/N{512,1024,2048,4096}/outputs/exec_*.txt"
echo ""
echo "NOTA: matrices.c es la REFERENCIA SECUENCIAL"
echo "      Usad estos tiempos como baseline"
echo "======================================"
