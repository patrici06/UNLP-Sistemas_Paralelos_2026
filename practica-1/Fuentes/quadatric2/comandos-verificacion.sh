#!/bin/bash
# Script de verificación para tu análisis de double vs float

echo "=== ANÁLISIS DE ARQUITECTURA Y VECTORIZACIÓN ==="
echo ""

# Paso 1: Información de CPU
echo "### 1. FLAGS DE CPU (tu máquina local)"
echo "Ejecuta esto en tu máquina:"
echo "cat /proc/cpuinfo | grep flags | head -1"
echo ""
echo "Ejecuta esto en el cluster:"
echo "ssh usuario@cluster 'cat /proc/cpuinfo | grep flags | head -1'"
echo ""
echo "BUSCA: sse2, sse4_1, sse4_2, avx, avx2, avx512"
echo ""
echo "---"
echo ""

# Paso 2: Compilador flags
echo "### 2. FLAGS NATIVOS DEL COMPILADOR (en ambas máquinas)"
echo "gcc --version"
echo "gcc -march=native -Q --help=target 2>/dev/null | grep -E '(sse|avx|fma)' | head -20"
echo ""
echo "---"
echo ""

# Paso 3: Compilar sin vectorización
echo "### 3. COMPILAR SIN VECTORIZACIÓN (en ambas máquinas)"
echo "cd /home/patricio/Escritorio/sistemas-paralelos/practica-1/Fuentes/"
echo "gcc -O2 -fno-tree-vectorize quadatric2.c -o quadatric2_no_vec -lm"
echo "echo 'SIN VECTORIZACIÓN:'; ./quadatric2_no_vec"
echo ""
echo "---"
echo ""

# Paso 4: Compilar CON vectorización
echo "### 4. COMPILAR CON VECTORIZACIÓN (en ambas máquinas)"
echo "gcc -O3 -ftree-vectorize -march=native quadatric2.c -o quadatric2_vec -lm"
echo "echo 'CON VECTORIZACIÓN:'; ./quadatric2_vec"
echo ""
echo "---"
echo ""

# Paso 5: Inspeccionar ensamblador
echo "### 5. VER INSTRUCCIONES SIMD GENERADAS (en ambas máquinas)"
echo "gcc -O3 -march=native -S quadatric2.c"
echo ""
echo "Busca instrucciones SIMD:"
echo "grep -E '(paddd|padds|addps|addpd|mulps|mulpd|movaps|movapd)' quadatric2.s | head -20"
echo ""
echo "SIGNIFICADO:"
echo "  - addps/mulps = Operaciones float vectorizadas (4x)"
echo "  - addpd/mulpd = Operaciones double vectorizadas (2x)"
echo "  - movaps/movapd = Movimientos vectorizados"
echo "  - Si aparecen muy pocas = No hay vectorización"
echo ""
echo "---"
echo ""

# Paso 6: Perfilar con perf
echo "### 6. MEDIR CACHE MISSES (requiere perf, en ambas máquinas)"
echo ""
echo "En tu máquina local:"
echo "perf stat -e cache-misses,cache-references,instructions,cycles -r 3 ./quadatric2_vec"
echo ""
echo "En el cluster (si tienes acceso):"
echo "ssh usuario@cluster 'cd /path/to/quadatric2 && perf stat -e cache-misses,cache-references,instructions,cycles -r 3 ./quadatric2_vec'"
echo ""
echo "COMPARAR:"
echo "  - Cache-misses ratio"
echo "  - IPC (instructions per cycle)"
echo "  - Si float en cluster tiene mejor ratio → confirma SIMD"
echo ""
echo "---"
echo ""

# Paso 7: Script de comparación automática
echo "### 7. SCRIPT DE COMPARACIÓN (guarda esto como compare.sh)"
cat > /tmp/compare_script.sh << 'EOF'
#!/bin/bash
# Script para compilar y comparar automáticamente

WORKDIR="/home/patricio/Escritorio/sistemas-paralelos/practica-1/Fuentes"
cd $WORKDIR

echo "=== COMPILANDO VERSIONES ==="

# Sin vectorización
echo "Compilando SIN vectorización..."
gcc -O2 -fno-tree-vectorize quadatric2.c -o quadatric2_no_vec -lm 2>&1 | grep -v "warning" || true

# Con vectorización
echo "Compilando CON vectorización..."
gcc -O3 -ftree-vectorize -march=native quadatric2.c -o quadatric2_vec -lm 2>&1 | grep -v "warning" || true

# Flags de compilación agresivos
echo "Compilando CON FLAGS AGRESIVOS..."
gcc -O3 -march=native -ffast-math quadatric2.c -o quadatric2_aggressive -lm 2>&1 | grep -v "warning" || true

echo ""
echo "=== RESULTADOS ==="
echo ""

echo "1. SIN VECTORIZACIÓN:"
./quadatric2_no_vec

echo ""
echo "2. CON VECTORIZACIÓN (-O3 -march=native):"
./quadatric2_vec

echo ""
echo "3. CON FLAGS AGRESIVOS (-ffast-math):"
./quadatric2_aggressive

echo ""
echo "=== ANÁLISIS ==="
echo "Si en cluster TIMES=200:"
echo "  - Sin vec: Double ≈ Float"
echo "  - Con vec: Float ~ 2x mejor que Double"
echo "→ Entonces la causa es SIMD"
EOF

chmod +x /tmp/compare_script.sh
cat /tmp/compare_script.sh
echo ""
echo "Ejecuta: bash /tmp/compare_script.sh"
echo ""

echo "=== FIN DE INSTRUCCIONES ==="
