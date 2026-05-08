#!/bin/bash

# Script para compilar y ejecutar las 3 versiones

echo "==================================="
echo "COMPILACIÓN DE VERSIONES"
echo "==================================="

cd /home/patricio/Escritorio/sistemas-paralelos/practica-2

echo "Compilando matrices.c (secuencial)..."
gcc -O3 -o matrices matrices.c -lm
if [ $? -eq 0 ]; then echo "✓ OK"; else echo "✗ ERROR"; exit 1; fi

echo "Compilando matrices_pthreads.c..."
gcc -O3 -pthread -o matrices_pthreads matrices_pthreads.c -lm
if [ $? -eq 0 ]; then echo "✓ OK"; else echo "✗ ERROR"; exit 1; fi

echo "Compilando matrices_openmp.c..."
gcc -O3 -fopenmp -o matrices_openmp matrices_openmp.c -lm
if [ $? -eq 0 ]; then echo "✓ OK"; else echo "✗ ERROR"; exit 1; fi

echo ""
echo "==================================="
echo "PRUEBAS CON N=512"
echo "==================================="

# Secuencial
echo ""
echo "Secuencial:"
./matrices 512

# Pthreads
echo ""
echo "Pthreads (2 threads):"
./matrices_pthreads 512 2

echo ""
echo "Pthreads (4 threads):"
./matrices_pthreads 512 4

# OpenMP
echo ""
echo "OpenMP (2 threads):"
./matrices_openmp 512 2

echo ""
echo "OpenMP (4 threads):"
./matrices_openmp 512 4

echo ""
echo "==================================="
echo "COMPILACIÓN Y PRUEBAS COMPLETADAS"
echo "==================================="
