#!/bin/bash

# ============================================================
# Script: Comparar resultados entre tipos (matrices, pthreads, openmp)
# ============================================================
# Genera análisis comparativo y cálculo de speedup
#
# Uso: ./compare-results.sh [N] [O]
#
# Parámetros:
#   N: Tamaño de matriz (512, 1024, 2048, 4096) - opcional
#   O: Nivel de optimización (1, 2, 3) - opcional
#
# Ejemplos:
#   ./compare-results.sh              # Comparar todo
#   ./compare-results.sh 1024         # Comparar N=1024 todos los O
#   ./compare-results.sh 1024 3       # Comparar N=1024, O=3
# ============================================================

N=${1}
O=${2}

SALIDAS_DIR="./salidas"
REPORTES_DIR="./reportes"
mkdir -p "$REPORTES_DIR"

# Colores
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${BLUE}=========================================${NC}"
echo -e "${BLUE}Comparación de Resultados${NC}"
if [ -n "$N" ]; then
    echo -e "${BLUE}Matriz: N=$N${NC}"
fi
if [ -n "$O" ]; then
    echo -e "${BLUE}Optimización: -O$O${NC}"
fi
echo -e "${BLUE}=========================================${NC}\n"

# Función para extraer tiempo
get_time() {
    local file=$1
    if [ -f "$file" ] && grep -q "^RESULT;" "$file"; then
        grep "^RESULT;" "$file" | cut -d';' -f3
    else
        echo ""
    fi
}

# Función para calcular speedup
calculate_speedup() {
    local t_seq=$1
    local t_par=$2
    
    if [ -n "$t_seq" ] && [ -n "$t_par" ]; then
        echo "scale=2; $t_seq / $t_par" | bc
    else
        echo "N/A"
    fi
}

# Función para comparar N y O específicos
compare_specific() {
    local n=$1
    local o=$2
    
    echo -e "${YELLOW}═══════════════════════════════════════${NC}"
    echo -e "${YELLOW}N=$n, O=$o${NC}"
    echo -e "${YELLOW}═══════════════════════════════════════${NC}\n"
    
    # Obtener tiempo secuencial
    seq_file=$(find "$SALIDAS_DIR/matrices/N${n}/O${o}" -name "*.txt" -type f | head -1)
    t_seq=$(get_time "$seq_file")
    
    if [ -z "$t_seq" ]; then
        echo "⚠ No hay datos para N=$n, O=$o"
        return
    fi
    
    echo -e "SECUENCIAL (baseline):  ${GREEN}${t_seq}s${NC}\n"
    
    # Comparar con pthreads
    echo -e "${BLUE}PTHREADS vs SECUENCIAL:${NC}"
    echo "T    Tiempo(s)   Speedup"
    echo "---  ---------   -------"
    
    for t in 2 4 8; do
        pth_file=$(find "$SALIDAS_DIR/pthreads/N${n}/T${t}/O${o}" -name "*.txt" -type f | head -1 2>/dev/null)
        t_pth=$(get_time "$pth_file")
        
        if [ -n "$t_pth" ]; then
            speedup=$(calculate_speedup "$t_seq" "$t_pth")
            printf "%-3s %9.6f   %6s\n" "T$t" "$t_pth" "$speedup"
        else
            printf "%-3s %9s   %6s\n" "T$t" "-" "-"
        fi
    done
    
    echo ""
    echo -e "${BLUE}OpenMP vs SECUENCIAL:${NC}"
    echo "T    Tiempo(s)   Speedup"
    echo "---  ---------   -------"
    
    for t in 2 4 8; do
        omp_file=$(find "$SALIDAS_DIR/openmp/N${n}/T${t}/O${o}" -name "*.txt" -type f | head -1 2>/dev/null)
        t_omp=$(get_time "$omp_file")
        
        if [ -n "$t_omp" ]; then
            speedup=$(calculate_speedup "$t_seq" "$t_omp")
            printf "%-3s %9.6f   %6s\n" "T$t" "$t_omp" "$speedup"
        else
            printf "%-3s %9s   %6s\n" "T$t" "-" "-"
        fi
    done
    
    echo ""
    echo -e "${BLUE}PTHREADS vs OpenMP (mismo N, O, T):${NC}"
    echo "T    Pthreads    OpenMP    Diferencia"
    echo "---  ---------   ---------  ---------"
    
    for t in 2 4 8; do
        pth_file=$(find "$SALIDAS_DIR/pthreads/N${n}/T${t}/O${o}" -name "*.txt" -type f | head -1 2>/dev/null)
        omp_file=$(find "$SALIDAS_DIR/openmp/N${n}/T${t}/O${o}" -name "*.txt" -type f | head -1 2>/dev/null)
        
        t_pth=$(get_time "$pth_file")
        t_omp=$(get_time "$omp_file")
        
        if [ -n "$t_pth" ] && [ -n "$t_omp" ]; then
            diff=$(echo "scale=6; $t_pth - $t_omp" | bc)
            printf "%-3s %9.6f   %9.6f   %9.6f\n" "T$t" "$t_pth" "$t_omp" "$diff"
        else
            printf "%-3s %9s   %9s   %9s\n" "T$t" "-" "-" "-"
        fi
    done
    
    echo ""
}

# Procesar comparaciones
if [ -n "$N" ] && [ -n "$O" ]; then
    # Comparación específica N y O
    compare_specific "$N" "$O"
    
elif [ -n "$N" ]; then
    # Comparación para N específico, todos los O
    for o in 1 2 3; do
        compare_specific "$N" "$o"
    done
    
else
    # Comparación general para todos N y O
    for n in 512 1024 2048 4096; do
        for o in 1 2 3; do
            compare_specific "$n" "$o"
        done
    done
fi

echo -e "${GREEN}✓ Análisis completado${NC}\n"
