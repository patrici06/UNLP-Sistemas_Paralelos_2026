#!/bin/bash

# Script de benchmarking: MPI puro vs MPI+OpenMP

echo "=== BENCHMARK: MPI Puro vs MPI+OpenMP ==="
echo ""

# Configuraciones a testear
SIZES=(128 256 512)
PROCS=(2 4)

for size in "${SIZES[@]}"; do
    echo "========================================="
    echo "n=$size"
    echo "========================================="
    
    for np in "${PROCS[@]}"; do
        # Validar que n sea divisible por np
        if [ $((size % np)) -ne 0 ]; then
            echo "n=$size no es divisible por np=$np, saltando..."
            continue
        fi
        
        echo ""
        echo "--- MPI Puro (np=$np) ---"
        result_mpi=$(mpirun -np $np /home/patricio/Escritorio/sistemas-paralelos/TP3/matrices-mpi $size 2>&1 | grep "^RESULT")
        echo "$result_mpi"
        
        echo "--- MPI+OpenMP (np=$np, OMP_NUM_THREADS=4) ---"
        export OMP_NUM_THREADS=4
        result_hybrid=$(mpirun -np $np /home/patricio/Escritorio/sistemas-paralelos/TP3/matrices-hybrid $size 2>&1 | grep "^RESULT")
        echo "$result_hybrid"
        
        # Extraer tiempos (formato: RESULT;n;time;gflops;overhead;constante)
        time_mpi=$(echo "$result_mpi" | cut -d';' -f3)
        time_hybrid=$(echo "$result_hybrid" | cut -d';' -f3)
        
        if [ ! -z "$time_mpi" ] && [ ! -z "$time_hybrid" ]; then
            speedup=$(echo "scale=3; $time_mpi / $time_hybrid" | bc)
            echo "Speedup: ${speedup}x (MPI=$time_mpi, Hybrid=$time_hybrid)"
        fi
    done
done

echo ""
echo "=== Benchmark Completado ==="
