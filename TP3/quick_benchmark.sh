#!/bin/bash

echo "========== BENCHMARK: MPI vs Hybrid =========="
echo ""

# Test rápido: n=256, np=2, variando OpenMP threads

n=256
np=2

echo "n=$n, np=$np"
echo ""

# MPI Puro (baseline)
echo "--- MPI Puro ---"
time_mpi=$(mpirun -np $np ./matrices-mpi $n 2>&1 | grep "^RESULT" | cut -d';' -f3)
gflops_mpi=$(mpirun -np $np ./matrices-mpi $n 2>&1 | grep "^RESULT" | cut -d';' -f4)
echo "Time: ${time_mpi}s, GFLOPs: ${gflops_mpi}"

# Hybrid con OMP_NUM_THREADS=1
echo "--- Hybrid (OMP_NUM_THREADS=1) ---"
export OMP_NUM_THREADS=1
time_h1=$(mpirun -np $np ./matrices-hybrid $n 2>&1 | grep "^RESULT" | cut -d';' -f3)
gflops_h1=$(mpirun -np $np ./matrices-hybrid $n 2>&1 | grep "^RESULT" | cut -d';' -f4)
speedup_h1=$(echo "scale=3; $time_mpi / $time_h1" | bc)
echo "Time: ${time_h1}s, GFLOPs: ${gflops_h1}, Speedup: ${speedup_h1}x"

# Hybrid con OMP_NUM_THREADS=2
echo "--- Hybrid (OMP_NUM_THREADS=2) ---"
export OMP_NUM_THREADS=2
time_h2=$(mpirun -np $np ./matrices-hybrid $n 2>&1 | grep "^RESULT" | cut -d';' -f3)
gflops_h2=$(mpirun -np $np ./matrices-hybrid $n 2>&1 | grep "^RESULT" | cut -d';' -f4)
speedup_h2=$(echo "scale=3; $time_mpi / $time_h2" | bc)
echo "Time: ${time_h2}s, GFLOPs: ${gflops_h2}, Speedup: ${speedup_h2}x"

# Hybrid con OMP_NUM_THREADS=4
echo "--- Hybrid (OMP_NUM_THREADS=4) ---"
export OMP_NUM_THREADS=4
time_h4=$(mpirun -np $np ./matrices-hybrid $n 2>&1 | grep "^RESULT" | cut -d';' -f3)
gflops_h4=$(mpirun -np $np ./matrices-hybrid $n 2>&1 | grep "^RESULT" | cut -d';' -f4)
speedup_h4=$(echo "scale=3; $time_mpi / $time_h4" | bc)
echo "Time: ${time_h4}s, GFLOPs: ${gflops_h4}, Speedup: ${speedup_h4}x"

echo ""
echo "========== Resumen para n=256, np=2 =========="
echo "| Config                  | Time (s)  | GFLOPs  | Speedup |"
echo "|-------------------------|-----------|---------|---------|"
printf "| MPI Puro                | %9s | %7s | 1.000x  |\n" "$time_mpi" "$gflops_mpi"
printf "| Hybrid (omp=1)          | %9s | %7s | %7sx |\n" "$time_h1" "$gflops_h1" "$speedup_h1"
printf "| Hybrid (omp=2)          | %9s | %7s | %7sx |\n" "$time_h2" "$gflops_h2" "$speedup_h2"
printf "| Hybrid (omp=4)          | %9s | %7s | %7sx |\n" "$time_h4" "$gflops_h4" "$speedup_h4"
echo ""
