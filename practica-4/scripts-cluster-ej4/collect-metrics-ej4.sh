#!/bin/bash
#
# collect-metrics-ej4.sh
# Extrae metricas de los logs planos y genera tablas + CSV.
# Lee directamente los tiempos del output del programa C (Tiempo total= / Tiempo comunicacion=).
# NO depende del bloque "=== Metricas calculadas ===" del script .slurm.
#
# Uso: ./collect-metrics-ej4.sh
#

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "========================================"
echo "Recolectando metricas de Ejercicio 4"
echo "========================================"
echo ""

# ============================================
# Funciones auxiliares robustas
# ============================================

extract_time() {
    local log="$1"
    local key="$2"
    grep "$key=" "$log" | head -1 | sed -E "s/.*$key=([0-9.eE+-]+).*/\1/"
}

# ============================================
# Paso 1: Leer tiempos secuenciales (P=1) por N
# ============================================
declare -A T_SEQ_MAP

for LOG in ej4-N*-P1.log; do
    [ -e "$LOG" ] || continue
    N=$(echo "$LOG" | sed -E 's/ej4-N([0-9]+)-P1\.log/\1/')
    T_S=$(extract_time "$LOG" "Tiempo total")
    if [ -n "$T_S" ]; then
        T_SEQ_MAP[$N]=$T_S
        echo "[OK] Secuencial leido N=$N -> Ts=$T_S s"
    fi
done

echo ""

# ============================================
# CSV y tabla principal
# ============================================
CSV="resultados-ej4.csv"
echo "ejercicio,algoritmo,nodos,procesos,N,tiempo_total,tiempo_comunicacion,tiempo_secencial,speedup,eficiencia" > "$CSV"

TABLE="tabla-ej4-mpi-matmul.md"
echo "# Ejercicio 4 - MPI Matrix Multiplication" > "$TABLE"
echo "" >> "$TABLE"
echo "| N    | P | T_Seq (s) | T_Par (s) | T_Comm (s) | Speedup | Eficiencia |" >> "$TABLE"
echo "|------|---|-----------|-----------|------------|---------|------------|" >> "$TABLE"

# ============================================
# Paso 2: Procesar todos los logs (P=1 y paralelos)
# ============================================
for LOG in ej4-N*-P*.log; do
    [ -e "$LOG" ] || continue
    BASENAME=$(basename "$LOG")
    N=$(echo "$BASENAME" | sed -E 's/ej4-N([0-9]+)-P[0-9]+\.log/\1/')
    P=$(echo "$BASENAME" | sed -E 's/ej4-N[0-9]+-P([0-9]+)\.log/\1/')

    T_TOTAL=$(extract_time "$LOG" "Tiempo total")
    T_COMM=$(extract_time "$LOG" "Tiempo comunicacion")

    T_TOTAL=${T_TOTAL:-"N/A"}
    T_COMM=${T_COMM:-"N/A"}

    # Tiempo secuencial para este N
    T_S=${T_SEQ_MAP[$N]:-"N/A"}

    # Calcular Speedup y Eficiencia (solo si es paralelo y tenemos datos)
    SPEEDUP="N/A"
    EFIC="N/A"
    NODOS="N/A"

    if [ "$P" -eq 1 ]; then
        NODOS=1
    elif [ "$P" -eq 4 ]; then
        NODOS=1
        if [ "$T_TOTAL" != "N/A" ] && [ "$T_S" != "N/A" ]; then
            SPEEDUP=$(awk -v ts="$T_S" -v tp="$T_TOTAL" 'BEGIN {printf "%.6f", ts/tp}')
            EFIC=$(awk -v s="$SPEEDUP" -v p="$P" 'BEGIN {printf "%.6f", s/p}')
        fi
    elif [ "$P" -eq 8 ]; then
        NODOS=2
        if [ "$T_TOTAL" != "N/A" ] && [ "$T_S" != "N/A" ]; then
            SPEEDUP=$(awk -v ts="$T_S" -v tp="$T_TOTAL" 'BEGIN {printf "%.6f", ts/tp}')
            EFIC=$(awk -v s="$SPEEDUP" -v p="$P" 'BEGIN {printf "%.6f", s/p}')
        fi
    fi

    echo "| $N | $P | $T_S | $T_TOTAL | $T_COMM | $SPEEDUP | $EFIC |" >> "$TABLE"
    echo "ej4,mpi-matmul,${NODOS},${P},${N},${T_TOTAL},${T_COMM},${T_S},${SPEEDUP},${EFIC}" >> "$CSV"
done

echo "[OK] Tabla generada: $TABLE"

# ============================================
# Tabla comparativa por N (una tabla por cada N)
# ============================================
TABLE_COMP="tabla-ej4-comparativa-por-N.md"
echo "# Ejercicio 4 - Comparativa por N (Speedup y Eficiencia)" > "$TABLE_COMP"
echo "" >> "$TABLE_COMP"

for N in 512 1024 2048; do
    echo "## N = $N" >> "$TABLE_COMP"
    echo "" >> "$TABLE_COMP"
    echo "| P | T_Seq (s) | T_Par (s) | Speedup | Eficiencia |" >> "$TABLE_COMP"
    echo "|---|-----------|-----------|---------|------------|" >> "$TABLE_COMP"

    for LOG in ej4-N${N}-P*.log; do
        [ -e "$LOG" ] || continue
        BASENAME=$(basename "$LOG")
        P=$(echo "$BASENAME" | sed -E 's/ej4-N[0-9]+-P([0-9]+)\.log/\1/')
        [ "$P" -eq 1 ] && continue

        T_TOTAL=$(extract_time "$LOG" "Tiempo total")
        T_S=${T_SEQ_MAP[$N]:-"N/A"}

        if [ "$T_TOTAL" != "N/A" ] && [ "$T_S" != "N/A" ]; then
            SPEEDUP=$(awk -v ts="$T_S" -v tp="$T_TOTAL" 'BEGIN {printf "%.6f", ts/tp}')
            EFIC=$(awk -v s="$SPEEDUP" -v p="$P" 'BEGIN {printf "%.6f", s/p}')
        else
            SPEEDUP="N/A"
            EFIC="N/A"
        fi

        echo "| $P | $T_S | $T_TOTAL | $SPEEDUP | $EFIC |" >> "$TABLE_COMP"
    done
    echo "" >> "$TABLE_COMP"
done

echo "[OK] Tabla comparativa generada: $TABLE_COMP"

# ============================================
# Resumen final
# ============================================
echo ""
echo "========================================"
echo "Recoleccion de Ejercicio 4 completada."
echo "========================================"
echo ""
echo "Archivos generados en el directorio actual:"
ls -1 "$CSV" "$TABLE" "$TABLE_COMP" 2>/dev/null || true
echo ""
echo "CSV: $CSV"
total=$(($(wc -l < "$CSV") - 1))
echo "Total filas (sin cabecera): $total"
