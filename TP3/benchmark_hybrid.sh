#!/bin/bash

# Benchmark: MPI Puro vs MPI+OpenMP (Hybrid)
# Nota: En máquina local, no esperamos grandes mejoras.
#       Las mejoras reales se verán en un cluster con múltiples nodos.

echo "=============================================="
echo "BENCHMARK: MPI Puro vs MPI+OpenMP Hybrid"
echo "=============================================="
echo ""
echo "Nota: Este benchmark es LOCAL. Las mejoras reales se verán en cluster."
echo ""

# Casos de test
SIZES=(128 256 512)
PROCS=(2 4)
OMP_THREADS=(1 2 4)

# Función para ejecutar un test
run_test() {
    local variant=$1   # "mpi" o "hybrid"
    local n=$2
    local np=$3
    local omp=$4
    
    if [ "$variant" = "mpi" ]; then
        result=$(mpirun -np $np /home/patricio/Escritorio/sistemas-paralelos/TP3/matrices-mpi $n 2>&1 | grep "^RESULT")
    else
        export OMP_NUM_THREADS=$omp
        result=$(mpirun -np $np /home/patricio/Escritorio/sistemas-paralelos/TP3/matrices-hybrid $n 2>&1 | grep "^RESULT")
    fi
    echo "$result"
}

# Crear tabla CSV para análisis
csv_file="/tmp/benchmark_results.csv"
echo "n,variant,np,omp_threads,time_sec,gflops,overhead_pct,speedup" > "$csv_file"

for n in "${SIZES[@]}"; do
    echo ""
    echo "========== n=$n =========="
    
    # Test MPI puro (baseline)
    echo ""
    echo "--- MPI Puro (baseline) ---"
    for np in "${PROCS[@]}"; do
        if [ $((n % np)) -ne 0 ]; then continue; fi
        result=$(run_test "mpi" $n $np 1)
        echo "np=$np: $result"
        
        # Guardar en CSV
        time=$(echo "$result" | cut -d';' -f3)
        gflops=$(echo "$result" | cut -d';' -f4)
        overhead=$(echo "$result" | cut -d';' -f5 | sed 's/%//')
        echo "$n,mpi,$np,1,$time,$gflops,$overhead,1.0" >> "$csv_file"
    done
    
    # Test Hybrid con diferentes threads
    echo ""
    echo "--- MPI+OpenMP Hybrid ---"
    for np in "${PROCS[@]}"; do
        if [ $((n % np)) -ne 0 ]; then continue; fi
        
        for omp in "${OMP_THREADS[@]}"; do
            result=$(run_test "hybrid" $n $np $omp)
            time=$(echo "$result" | cut -d';' -f3)
            gflops=$(echo "$result" | cut -d';' -f4)
            overhead=$(echo "$result" | cut -d';' -f5 | sed 's/%//')
            
            # Obtener tiempo MPI puro para calcular speedup
            mpi_time=$(mpirun -np $np /home/patricio/Escritorio/sistemas-paralelos/TP3/matrices-mpi $n 2>&1 | grep "^RESULT" | cut -d';' -f3)
            if [ ! -z "$mpi_time" ] && [ ! -z "$time" ]; then
                speedup=$(echo "scale=3; $mpi_time / $time" | bc)
            else
                speedup="N/A"
            fi
            
            echo "np=$np, omp=$omp: $result | Speedup: ${speedup}x"
            echo "$n,hybrid,$np,$omp,$time,$gflops,$overhead,$speedup" >> "$csv_file"
        done
    done
done

echo ""
echo "=============================================="
echo "✓ Benchmark completado"
echo "✓ Resultados guardados en: $csv_file"
echo "=============================================="

# Mostrar resumen
echo ""
echo "=== RESUMEN DE RESULTADOS ==="
echo ""
cat "$csv_file" | column -t -s','
