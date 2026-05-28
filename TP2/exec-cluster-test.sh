#!/bin/bash

# ============================================================
# Script para CLUSTER - Pruebas Completas (pthread + openmp)
# ============================================================
# Este script:
#   1. Compila matrices-pthread y matrices-open-mp con -O2
#   2. Ejecuta T=1 primero (para obtener ref_time_sequential)
#   3. Ejecuta T={2,4,8} para medir paralelismo
#   4. Estructura de salida ordenada por N y T
#
# Parámetros:
#   - Tamaños: N = {512, 1024, 2048, 4096}
#   - Threads: T = {1, 2, 4, 8}
#   - Compilación: -O2 (único nivel)
#
# Estructura de salida:
#   salidas/
#   ├─ N512-T1/
#   │  ├─ pthread/outputs/
#   │  └─ openmp/outputs/
#   ├─ N512-T2/
#   │  ├─ pthread/outputs/
#   │  └─ openmp/outputs/
#   ├─ N512-T4/
#   ├─ N512-T8/
#   ├─ N1024-T1/
#   ├─ N1024-T2/
#   ... y así para 2048, 4096
#
# USO: ./exec-cluster-test.sh
# ============================================================

set -e

matrix_sizes=(512 1024 2048 4096)
thread_counts=(1 2 4 8)
programs=("pthread" "openmp")

SALIDAS_DIR="./salidas"
COMPILE_FLAGS="-O2"

echo "======================================"
echo "COMPILACIÓN - CLUSTER TEST"
echo "======================================"
echo "Compilando con: $COMPILE_FLAGS"
echo ""

# Compilar matrices-pthread
echo -n "Compilando matrices-pthread... "
if gcc $COMPILE_FLAGS -pthread ./matrices-pthread.c -o ./matrices-pthread -lm 2>/dev/null; then
    echo "✓"
else
    echo "✗ ERROR"
    exit 1
fi

# Compilar matrices-open-mp
echo -n "Compilando matrices-open-mp... "
if gcc $COMPILE_FLAGS -fopenmp ./matrices-open-mp.c -o ./matrices-open-mp -lm 2>/dev/null; then
    echo "✓"
else
    echo "✗ ERROR"
    exit 1
fi

echo ""
echo "======================================"
echo "ENVÍO DE TRABAJOS AL CLUSTER"
echo "======================================"
echo ""

for n in "${matrix_sizes[@]}"; do
    for t in "${thread_counts[@]}"; do
        echo "=================================================="
        echo "Configuración: N=$n, T=$t"
        echo "=================================================="
        
        for prog in "${programs[@]}"; do
            EXEC_DIR="$SALIDAS_DIR/N${n}-T${t}/$prog"
            OUTPUT_DIR="$EXEC_DIR/outputs"
            mkdir -p "$OUTPUT_DIR"
            
            echo -n "  Enviando $prog... "
            
            # Enviar job a cluster
            job_id=$(sbatch \
                -o "$OUTPUT_DIR/exec_%j.txt" \
                --job-name="test-N${n}-T${t}-${prog}" \
                --time=00:15:00 \
                --partition=Blade \
                -N 1 --exclusive \
                ./job-test.sh $n $t $prog 2>&1 | awk '{print $NF}')
            
            echo "Job ID: $job_id"
            
        done
        echo ""
    done
done

echo "======================================"
echo "Todos los trabajos han sido enviados"
echo "======================================"
echo "Monitorea con: squeue -u \$USER"
echo "Ver salidas en:"
echo "  - pthread: $SALIDAS_DIR/N{512,1024,2048,4096}-T{1,2,4,8}/pthread/outputs/"
echo "  - openmp:  $SALIDAS_DIR/N{512,1024,2048,4096}-T{1,2,4,8}/openmp/outputs/"
echo ""
echo "IMPORTANTE - Lectura de resultados:"
echo "  Primero ejecuta con T=1 para obtener ref_time_sequential"
echo "  Luego interpreta T={2,4,8} usando ese tiempo de referencia"
echo "======================================"
