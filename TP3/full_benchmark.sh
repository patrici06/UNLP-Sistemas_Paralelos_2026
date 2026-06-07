#!/bin/bash

echo "========================================================"
echo "BENCHMARK COMPLETO: MPI Puro vs MPI+OpenMP Hybrid"
echo "========================================================"
echo ""

# Casos de test
SIZES=(128 256 512)
PROCS=(2 4)

# Archivo de salida
out_file="/tmp/benchmark_full.txt"
> "$out_file"

for n in "${SIZES[@]}"; do
    for np in "${PROCS[@]}"; do
        # Validar divisibilidad
        if [ $((n % np)) -ne 0 ]; then
            continue
        fi
        
        echo "" | tee -a "$out_file"
        echo "========== n=$n, np=$np ==========" | tee -a "$out_file"
        
        # MPI Puro
        echo "MPI Puro:" | tee -a "$out_file"
        result_mpi=$(mpirun -np $np ./matrices-mpi $n 2>&1 | grep "^RESULT")
        echo "$result_mpi" | tee -a "$out_file"
        time_mpi=$(echo "$result_mpi" | cut -d';' -f3)
        
        # Hybrid OMP=2
        echo "Hybrid (OMP=2):" | tee -a "$out_file"
        export OMP_NUM_THREADS=2
        result_h2=$(mpirun -np $np ./matrices-hybrid $n 2>&1 | grep "^RESULT")
        echo "$result_h2" | tee -a "$out_file"
        time_h2=$(echo "$result_h2" | cut -d';' -f3)
        speedup=$(echo "scale=3; $time_mpi / $time_h2" | bc)
        echo "Speedup: ${speedup}x" | tee -a "$out_file"
        
        # Hybrid OMP=4
        echo "Hybrid (OMP=4):" | tee -a "$out_file"
        export OMP_NUM_THREADS=4
        result_h4=$(mpirun -np $np ./matrices-hybrid $n 2>&1 | grep "^RESULT")
        echo "$result_h4" | tee -a "$out_file"
        time_h4=$(echo "$result_h4" | cut -d';' -f3)
        speedup=$(echo "scale=3; $time_mpi / $time_h4" | bc)
        echo "Speedup: ${speedup}x" | tee -a "$out_file"
    done
done

echo "" | tee -a "$out_file"
echo "========================================================"
echo "✓ Benchmark completado. Resultados en: $out_file"
echo "========================================================"

cat "$out_file"
