#!/bin/bash

echo "==============================================="
echo "VALIDACIÓN: matrices-mpi vs matrices-open-mp"
echo "==============================================="
echo ""

# Test con n=64
n=64

echo "Ejecutando matrices-open-mp.c (referencia)..."
./matrices-open-mp $n 1 > /tmp/openmp_output.txt 2>&1
openmp_result=$(grep "^RESULT" /tmp/openmp_output.txt)
echo "OpenMP: $openmp_result"
echo ""

echo "Ejecutando matrices-mpi.c (MPI)..."
mpirun -np 1 ./matrices-mpi $n > /tmp/mpi_output.txt 2>&1
mpi_result=$(grep "^RESULT" /tmp/mpi_output.txt)
echo "MPI:    $mpi_result"
echo ""

echo "Validación en ambos:"
grep "VALIDATION" /tmp/openmp_output.txt
grep "VALIDATION" /tmp/mpi_output.txt
echo ""

# Extraer tiempos
openmp_time=$(echo $openmp_result | cut -d';' -f3)
mpi_time=$(echo $mpi_result | cut -d';' -f2)

echo "Tiempo OpenMP: $openmp_time"
echo "Tiempo MPI:    $mpi_time"
echo ""

if [ ! -z "$openmp_time" ] && [ ! -z "$mpi_time" ]; then
    ratio=$(echo "scale=2; $mpi_time / $openmp_time" | bc)
    echo "Ratio MPI/OpenMP: $ratio x"
fi
