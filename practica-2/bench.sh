#!/bin/bash
echo "╔════════════════════════════════════════════════════════════════╗"
echo "║    ANÁLISIS DE RENDIMIENTO - MATRIZ PRODUCT (O3 optimized)    ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""

for N in 512 1024 2048 4096; do
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "Matriz: ${N}x${N} ($(echo "scale=1; ($N * $N) / 1000000" | bc)M elementos)"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    
    for T in 1 2 4 8; do
        echo -n "T=$T threads: "
        ./matrixprod $N $T 2>/dev/null | grep "Tiempo paralelo"
    done
    echo ""
done
