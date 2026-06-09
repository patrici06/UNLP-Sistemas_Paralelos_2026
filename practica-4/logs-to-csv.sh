#!/bin/bash
#
# logs-to-csv.sh
# Genera un CSV consolidado a partir de todos los logs generados
# por los scripts de ejecucion del proyecto.
#
# Uso: ./logs-to-csv.sh [archivo_salida.csv]
# Salida por defecto: resultados.csv
#

set -e

OUTPUT="${1:-resultados.csv}"

echo "Generando CSV consolidado: $OUTPUT"

# Cabecera del CSV
echo "algoritmo,procesos,N,tiempo_total,tiempo_comunicacion,speedup,eficiencia" > "$OUTPUT"

# -----------------------------------------------
# Procesar logs de mpi-matmul
# -----------------------------------------------
for LOG in log-matrices/matriz-*.log; do
    [ -e "$LOG" ] || continue

    BASENAME=$(basename "$LOG")
    # Formato esperado: matriz-P<N>-N<SIZE>.log
    P=$(echo "$BASENAME" | sed -E 's/matriz-P([0-9]+)-N[0-9]+\.log/\1/')
    N=$(echo "$BASENAME" | sed -E 's/matriz-P[0-9]+-N([0-9]+)\.log/\1/')

    # Extraer tiempos de la salida estandar del programa
    T_TOTAL=$(grep "Tiempo total=" "$LOG" | head -1 | awk -F'Tiempo total=' '{print $2}' | awk '{print $1}')
    T_COMM=$(grep "Tiempo comunicacion=" "$LOG" | head -1 | awk -F'Tiempo comunicacion=' '{print $2}' | awk '{print $1}')

    # Extraer speedup y eficiencia si fueron agregados por el script run-mpi-matmul.sh
    SPEEDUP=$(grep "^Speedup:" "$LOG" | awk '{print $2}')
    EFICIENCIA=$(grep "^Eficiencia:" "$LOG" | awk '{print $2}')

    # Manejar valores vacios para CSV
    T_TOTAL=${T_TOTAL:-""}
    T_COMM=${T_COMM:-""}
    SPEEDUP=${SPEEDUP:-""}
    EFICIENCIA=${EFICIENCIA:-""}

    echo "mpi-matmul,$P,$N,$T_TOTAL,$T_COMM,$SPEEDUP,$EFICIENCIA" >> "$OUTPUT"
done

# -----------------------------------------------
# Procesar logs de blocking-ring
# -----------------------------------------------
for LOG in logs/blocking-ring-*.log; do
    [ -e "$LOG" ] || continue

    BASENAME=$(basename "$LOG")
    P=$(echo "$BASENAME" | sed -E 's/blocking-ring-P([0-9]+)-N[0-9]+\.log/\1/')
    N=$(echo "$BASENAME" | sed -E 's/blocking-ring-P[0-9]+-N([0-9]+)\.log/\1/')

    T_COMM=$(grep "Tiempo de comunicacion :" "$LOG" | sed -E 's/.*Tiempo de comunicacion\s*:\s*([0-9.]+)\s*seconds.*/\1/')
    T_COMM=${T_COMM:-""}

    echo "blocking-ring,$P,$N,,$T_COMM,," >> "$OUTPUT"
done

# -----------------------------------------------
# Procesar logs de non-blocking-ring
# -----------------------------------------------
for LOG in logs/non-blocking-ring-*.log; do
    [ -e "$LOG" ] || continue

    BASENAME=$(basename "$LOG")
    P=$(echo "$BASENAME" | sed -E 's/non-blocking-ring-P([0-9]+)-N[0-9]+\.log/\1/')
    N=$(echo "$BASENAME" | sed -E 's/non-blocking-ring-P[0-9]+-N([0-9]+)\.log/\1/')

    T_COMM=$(grep "Tiempo de comunicacion :" "$LOG" | sed -E 's/.*Tiempo de comunicacion\s*:\s*([0-9.]+)\s*seconds.*/\1/')
    T_COMM=${T_COMM:-""}

    echo "non-blocking-ring,$P,$N,,$T_COMM,," >> "$OUTPUT"
done

echo ""
echo "CSV generado exitosamente: $OUTPUT"
echo "Total de filas: $(wc -l < "$OUTPUT") (incluyendo cabecera)"
echo ""
