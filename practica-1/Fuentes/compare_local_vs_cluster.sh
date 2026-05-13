#!/bin/bash
# Script automático de comparación: sin vec vs con vec

WORKDIR="/home/patricio/Escritorio/sistemas-paralelos/practica-1/Fuentes"
cd "$WORKDIR"

echo "=== VERIFICACIÓN: CAUSA DE DIFERENCIA LOCAL VS CLUSTER ==="
echo ""
echo "Hipótesis: SIMD (Single Instruction Multiple Data)"
echo "  - Float: procesa 4 elementos/ciclo (SSE)"
echo "  - Double: procesa 2 elementos/ciclo (SSE)"
echo ""
echo "---"
echo ""

# Paso 0: Info del sistema
echo "### INFORMACIÓN DEL SISTEMA ###"
echo "CPU flags:"
cat /proc/cpuinfo | grep flags | head -1
echo ""
echo "Compilador:"
gcc --version | head -1
echo ""
echo "---"
echo ""

# Paso 1: Sin vectorización
echo "### 1. COMPILANDO SIN VECTORIZACIÓN ###"
gcc -O2 -fno-tree-vectorize quadatric2.c -o quadatric2_no_vec -lm
if [ $? -eq 0 ]; then
    echo "✓ Compilación exitosa"
    echo ""
    echo "Ejecutando (sin vectorización)..."
    ./quadatric2_no_vec
    echo ""
else
    echo "✗ Error en compilación"
fi

echo "---"
echo ""

# Paso 2: Con vectorización
echo "### 2. COMPILANDO CON VECTORIZACIÓN (-O3 -march=native) ###"
gcc -O3 -ftree-vectorize -march=native quadatric2.c -o quadatric2_vec -lm
if [ $? -eq 0 ]; then
    echo "✓ Compilación exitosa"
    echo ""
    echo "Ejecutando (con vectorización)..."
    ./quadatric2_vec
    echo ""
else
    echo "✗ Error en compilación"
fi

echo "---"
echo ""

# Paso 3: Generar ensamblador
echo "### 3. ANALIZANDO INSTRUCCIONES SIMD GENERADAS ###"
gcc -O3 -march=native -S quadatric2.c
echo ""
echo "Instrucciones SIMD detectadas:"
echo ""
echo "Float operations (debería tener muchas):"
grep -c "addps\|mulps\|movaps" quadatric2.s || echo "  (ninguna detectada)"
echo ""
echo "Double operations (debería tener pocas o ninguna si está bien optimizado):"
grep -c "addpd\|mulpd\|movapd" quadatric2.s || echo "  (ninguna detectada)"
echo ""

echo "---"
echo ""
echo "### RESUMEN ###"
echo ""
echo "Si en el resultado CON VECTORIZACIÓN ves:"
echo "  - Float significativamente más rápido que Double"
echo "  - Especialmente en TIMES=200 (que es donde viste 2x)"
echo ""
echo "→ CONFIRMADO: El cluster usa SIMD"
echo "→ Float es 2x mejor porque procesa 4 elementos vs 2 elementos"
echo ""
echo "Si ves similar:"
echo "  - Double y Float similares en ambas compilaciones"
echo ""
echo "→ Entonces NO es SIMD (sería otro factor)"
echo ""

