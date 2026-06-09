#!/bin/bash
#
# collect-metrics.sh
# Extrae metricas de los logs generados por los scripts SLURM y genera
# tablas individuales por ejercicio + un CSV consolidado.
#
# Uso: ./collect-metrics.sh
# Salida: tablas-cluster/
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

LOG_DIR="logs-cluster"
OUT_DIR="tablas-cluster"

mkdir -p "$OUT_DIR"

echo "========================================"
echo "Recolectando metricas de: $LOG_DIR/"
echo "========================================"
echo ""

# ============================================
# CSV Consolidado
# ============================================
CSV="$OUT_DIR/resultados-consolidados.csv"
echo "ejercicio,algoritmo,nodos,procesos,N,tiempo_total,tiempo_comunicacion,tiempo_secencial,speedup,eficiencia" > "$CSV"

# ============================================
# Funciones auxiliares
# ============================================

extract_comm_time() {
    local log="$1"
    grep "Tiempo de comunicacion" "$log" | sed -E 's/.*Tiempo de comunicacion\s*:\s*([0-9.]+)\s*seconds.*/\1/' | head -1
}

extract_total_time() {
    local log="$1"
    grep "Tiempo total=" "$log" | head -1 | awk -F'Tiempo total=' '{print $2}' | awk '{print $1}'
}

extract_comm_time_matmul() {
    local log="$1"
    grep "Tiempo comunicacion=" "$log" | head -1 | awk -F'Tiempo comunicacion=' '{print $2}' | awk '{print $1}'
}

extract_metric() {
    local log="$1"
    local key="$2"
    grep "^$key:" "$log" | awk '{print $2}' | head -1
}

# ============================================
# Ejercicio 3: Blocking Ring
# ============================================
TABLE_EJ3B="$OUT_DIR/tabla-ej3-blocking-ring.md"
echo "# Ejercicio 3 - Blocking Ring" > "$TABLE_EJ3B"
echo "" >> "$TABLE_EJ3B"
echo "| P | N         | T_comunicacion (s) |" >> "$TABLE_EJ3B"
echo "|---|-----------|-------------------|" >> "$TABLE_EJ3B"

for LOG in "$LOG_DIR"/ej3-blocking-ring-P*-N*.log; do
    [ -e "$LOG" ] || continue
    BASENAME=$(basename "$LOG")
    P=$(echo "$BASENAME" | sed -E 's/ej3-blocking-ring-P([0-9]+)-N[0-9]+\.log/\1/')
    N=$(echo "$BASENAME" | sed -E 's/ej3-blocking-ring-P[0-9]+-N([0-9]+)\.log/\1/')
    T_COMM=$(extract_comm_time "$LOG")
    T_COMM=${T_COMM:-"N/A"}
    echo "| $P | $N | $T_COMM |" >> "$TABLE_EJ3B"
    echo "ej3,blocking-ring,,${P},${N},,${T_COMM},,,," >> "$CSV"
done

echo "[OK] Tabla generada: $TABLE_EJ3B"

# ============================================
# Ejercicio 3: Non-Blocking Ring
# ============================================
TABLE_EJ3NB="$OUT_DIR/tabla-ej3-non-blocking-ring.md"
echo "# Ejercicio 3 - Non-Blocking Ring" > "$TABLE_EJ3NB"
echo "" >> "$TABLE_EJ3NB"
echo "| P | N         | T_comunicacion (s) |" >> "$TABLE_EJ3NB"
echo "|---|-----------|-------------------|" >> "$TABLE_EJ3NB"

for LOG in "$LOG_DIR"/ej3-non-blocking-ring-P*-N*.log; do
    [ -e "$LOG" ] || continue
    BASENAME=$(basename "$LOG")
    P=$(echo "$BASENAME" | sed -E 's/ej3-non-blocking-ring-P([0-9]+)-N[0-9]+\.log/\1/')
    N=$(echo "$BASENAME" | sed -E 's/ej3-non-blocking-ring-P[0-9]+-N([0-9]+)\.log/\1/')
    T_COMM=$(extract_comm_time "$LOG")
    T_COMM=${T_COMM:-"N/A"}
    echo "| $P | $N | $T_COMM |" >> "$TABLE_EJ3NB"
    echo "ej3,non-blocking-ring,,${P},${N},,${T_COMM},,,," >> "$CSV"
done

echo "[OK] Tabla generada: $TABLE_EJ3NB"

# ============================================
# Ejercicio 4: MPI Matmul
# ============================================
TABLE_EJ4="$OUT_DIR/tabla-ej4-mpi-matmul.md"
echo "# Ejercicio 4 - MPI Matrix Multiplication" > "$TABLE_EJ4"
echo "" >> "$TABLE_EJ4"
echo "| N    | P | T_Seq (s) | T_Par (s) | T_Comm (s) | Speedup | Eficiencia |" >> "$TABLE_EJ4"
echo "|------|---|-----------|-----------|------------|---------|------------|" >> "$TABLE_EJ4"

for LOG in "$LOG_DIR"/ej4-mpi-matmul-N*-P*.log; do
    [ -e "$LOG" ] || continue
    BASENAME=$(basename "$LOG")
    N=$(echo "$BASENAME" | sed -E 's/ej4-mpi-matmul-N([0-9]+)-P[0-9]+\.log/\1/')
    P=$(echo "$BASENAME" | sed -E 's/ej4-mpi-matmul-N[0-9]+-P([0-9]+)\.log/\1/')

    T_SEQ=$(extract_metric "$LOG" "Tiempo Secuencial")
    T_PAR=$(extract_metric "$LOG" "Tiempo Paralelo")
    T_COMM=$(extract_comm_time_matmul "$LOG")
    SPEEDUP=$(extract_metric "$LOG" "Speedup")
    EFIC=$(extract_metric "$LOG" "Eficiencia")
    NODOS=$(extract_metric "$LOG" "Nodos")

    T_SEQ=${T_SEQ:-"N/A"}
    T_PAR=${T_PAR:-"N/A"}
    T_COMM=${T_COMM:-"N/A"}
    SPEEDUP=${SPEEDUP:-"N/A"}
    EFIC=${EFIC:-"N/A"}
    NODOS=${NODOS:-"N/A"}

    echo "| $N | $P | $T_SEQ | $T_PAR | $T_COMM | $SPEEDUP | $EFIC |" >> "$TABLE_EJ4"
    echo "ej4,mpi-matmul,${NODOS},${P},${N},${T_PAR},${T_COMM},${T_SEQ},${SPEEDUP},${EFIC}" >> "$CSV"
done

echo "[OK] Tabla generada: $TABLE_EJ4"

# ============================================
# Ejercicios 1 y 2: no tienen metricas numericas estandarizadas,
# pero podemos listar los logs generados.
# ============================================
TABLE_EJ1="$OUT_DIR/tabla-ej1-mpi-simple.md"
echo "# Ejercicio 1 - MPI Simple (anillo)" > "$TABLE_EJ1"
echo "" >> "$TABLE_EJ1"
echo "| P | Archivo de log |" >> "$TABLE_EJ1"
echo "|---|----------------|" >> "$TABLE_EJ1"

for LOG in "$LOG_DIR"/ej1-mpi-simple-P*.log; do
    [ -e "$LOG" ] || continue
    BASENAME=$(basename "$LOG")
    P=$(echo "$BASENAME" | sed -E 's/ej1-mpi-simple-P([0-9]+)\.log/\1/')
    echo "| $P | $BASENAME |" >> "$TABLE_EJ1"
done

echo "[OK] Tabla generada: $TABLE_EJ1"

TABLE_EJ2="$OUT_DIR/tabla-ej2-comparativa.md"
echo "# Ejercicio 2 - Comparativa Blocking vs Non-Blocking" > "$TABLE_EJ2"
echo "" >> "$TABLE_EJ2"
echo "| Tipo | P | Archivo de log |" >> "$TABLE_EJ2"
echo "|------|---|----------------|" >> "$TABLE_EJ2"

for LOG in "$LOG_DIR"/ej2-blocking-P*.log; do
    [ -e "$LOG" ] || continue
    BASENAME=$(basename "$LOG")
    P=$(echo "$BASENAME" | sed -E 's/ej2-blocking-P([0-9]+)\.log/\1/')
    echo "| blocking | $P | $BASENAME |" >> "$TABLE_EJ2"
done

for LOG in "$LOG_DIR"/ej2-non-blocking-P*.log; do
    [ -e "$LOG" ] || continue
    BASENAME=$(basename "$LOG")
    P=$(echo "$BASENAME" | sed -E 's/ej2-non-blocking-P([0-9]+)\.log/\1/')
    echo "| non-blocking | $P | $BASENAME |" >> "$TABLE_EJ2"
done

echo "[OK] Tabla generada: $TABLE_EJ2"

# ============================================
# Resumen final
# ============================================
echo ""
echo "========================================"
echo "Recoleccion completada."
echo "========================================"
echo ""
echo "Archivos generados en: $OUT_DIR/"
ls -1 "$OUT_DIR/"
echo ""
echo "CSV consolidado: $CSV"
echo "Total filas (sin cabecera): $(($(wc -l < "$CSV") - 1))"
