#!/bin/bash

# ============================================================
# Script para CLUSTER - matrices-open-mp.c (INDEPENDIENTE)
# ============================================================
# Este script:
#   1. Envía trabajos para N={512, 1024, 2048, 4096} con T={1, 2, 4, 8}
#   2. Cada nodo compila matrices-open-mp.c
#   3. Estructura de salida clara por N y T
#
# Parámetros:
#   - Tamaños: N = {512, 1024, 2048, 4096}
#   - Threads: T = {1, 2, 4, 8}
#   - Compilación: -O2 (EN EL NODO)
#
# Estructura de salida:
#   salidas/openmp/N512-T1/outputs/
#   salidas/openmp/N512-T2/outputs/
#   salidas/openmp/N512-T4/outputs/
#   salidas/openmp/N512-T8/outputs/
#   salidas/openmp/N1024-T1/outputs/
#   ... y así para 2048, 4096
#
# USO: ./exec-cluster-openmp.sh
# ============================================================

set -e

matrix_sizes=(512 1024 2048 4096)
thread_counts=(1 2 4 8)

SALIDAS_DIR="./salidas"

echo "======================================"
echo "ENVÍO DE TRABAJOS AL CLUSTER"
echo "(Compilación en cada nodo)"
echo "======================================"
echo ""
echo "ENVÍO DE TRABAJOS AL CLUSTER - OpenMP"
echo "======================================"
echo ""

for n in "${matrix_sizes[@]}"; do
    echo "=================================================="
    echo "Configuración: N=$n"
    echo "=================================================="
    
    for t in "${thread_counts[@]}"; do
        EXEC_DIR="$SALIDAS_DIR/openmp/N${n}-T${t}"
        OUTPUT_DIR="$EXEC_DIR/outputs"
        mkdir -p "$OUTPUT_DIR"
        
        echo -n "  T=$t... "
        
        # Enviar job a cluster
        job_id=$(sbatch \
            -o "$OUTPUT_DIR/exec_%j.txt" \
            --job-name="openmp-N${n}-T${t}" \
            --time=00:15:00 \
            --partition=Blade \
            -N 1 --exclusive \
            ./job-openmp.sh $n $t 2>&1 | awk '{print $NF}')
        
        echo "Job ID: $job_id"
    done
    echo ""
done

echo "======================================"
echo "Todos los trabajos han sido enviados"
echo "======================================"
echo "Monitorea con: squeue -u \$USER"
echo "Ver salidas en:"
echo "  salidas/openmp/N{512,1024,2048,4096}-T{1,2,4,8}/outputs/exec_*.txt"
echo ""
echo "IMPORTANTE: Ejecuta con T=1 primero para obtener referencia"
echo "======================================"
