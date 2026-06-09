#!/bin/bash
#
# run-mpi-matmul.sh
# Script para compilar y ejecutar mpi_matmul.c con los parametros
# especificados en la Practica 4, Ejercicio 4.
# Calcula Speedup y Eficiencia a partir del tiempo secuencial medido
# automaticamente para cada N.
#
# Uso: ./run-mpi-matmul.sh
# Salida: log-matrices/matriz-P<N>-N<SIZE>.log
#

set -e

SRC_DIR="Fuentes"
LOG_DIR="log-matrices"
BIN_DIR="bin"

mkdir -p "$LOG_DIR"
mkdir -p "$BIN_DIR"

# Compilar
SRC="$SRC_DIR/mpi_matmul.c"
BIN="$BIN_DIR/mpi_matmul"

echo "=> Compilando $SRC ..."
mpicc "$SRC" -o "$BIN" -lm

# Parametros
TIEMPO_SEC_LOCAL=0.679252
PROCESOS=(2 4)
SIZES=(512 1024 2048)

echo ""
echo "=> Ejecutando pruebas de mpi-matmul ..."
echo ""

# Array asociativo para guardar tiempos secuenciales por N
declare -A T_SEQ

# -----------------------------------------------
# 1) Obtener tiempos secuenciales (P=1) para cada N
# -----------------------------------------------
for N in "${SIZES[@]}"; do
    echo "--- Secuencial (P=1), N=$N ---"
    SEC_LOG="$LOG_DIR/matriz-P1-N${N}.log"

    mpirun -np 1 ./"$BIN" "$N" | tee "$SEC_LOG"

    # Extraer tiempo secuencial del log
    T_S=$(grep "Tiempo total=" "$SEC_LOG" | awk -F'Tiempo total=' '{print $2}' | awk '{print $1}')
    T_SEQ[$N]=$T_S

    echo "    Tiempo secuencial (N=$N): $T_S segundos"
    echo ""
done

echo "=========================================="
echo "  RESUMEN DE EJECUCIONES PARALELOS"
echo "=========================================="
echo ""

# -----------------------------------------------
# 2) Ejecutar casos paralelos y calcular metricas
# -----------------------------------------------
for P in "${PROCESOS[@]}"; do
    for N in "${SIZES[@]}"; do
        LOGFILE="$LOG_DIR/matriz-P${P}-N${N}.log"

        echo "--- P=$P, N=$N ---"

        mpirun -np "$P" ./"$BIN" "$N" | tee "$LOGFILE"

        # Extraer tiempo total del log
        T_P=$(grep "Tiempo total=" "$LOGFILE" | awk -F'Tiempo total=' '{print $2}' | awk '{print $1}')
        T_S=${T_SEQ[$N]}

        # Calcular speedup y eficiencia usando bc, formateando siempre 6 decimales
        SPEEDUP=$(echo "scale=6; $T_S / $T_P" | bc | awk '{printf "%0.6f\n", $0}')
        EFICIENCIA=$(echo "scale=6; $SPEEDUP / $P" | bc | awk '{printf "%0.6f\n", $0}')

        echo ""
        echo ">> Metricas para P=$P, N=$N:"
        echo "   Tiempo Secuencial (Ts): $T_S"
        echo "   Tiempo Paralelo   (Tp): $T_P"
        echo "   Speedup    (S=Ts/Tp): $SPEEDUP"
        echo "   Eficiencia (E=S/P)   : $EFICIENCIA"
        echo ""

        # Agregar metricas al final del log para referencia
        {
            echo ""
            echo "=== Metricas calculadas ==="
            echo "Procesos: $P"
            echo "Dimension N: $N"
            echo "Tiempo Secuencial: $T_S"
            echo "Tiempo Paralelo: $T_P"
            echo "Speedup: $SPEEDUP"
            echo "Eficiencia: $EFICIENCIA"
        } >> "$LOGFILE"

        echo "    Resultado guardado en: $LOGFILE"
        echo ""
    done
done

# -----------------------------------------------
# 3) Tabla resumen final
# -----------------------------------------------
echo "=========================================="
echo "  TABLA RESUMEN"
echo "=========================================="
printf "%-10s %-10s %-15s %-15s %-15s %-15s\n" "N" "P" "T_Seq" "T_Par" "Speedup" "Eficiencia"
for P in "${PROCESOS[@]}"; do
    for N in "${SIZES[@]}"; do
        LOGFILE="$LOG_DIR/matriz-P${P}-N${N}.log"
        T_S=${T_SEQ[$N]}
        T_P=$(grep "Tiempo total=" "$LOGFILE" | awk -F'Tiempo total=' '{print $2}' | awk '{print $1}')
        SPEEDUP=$(grep "^Speedup:" "$LOGFILE" | awk '{print $2}')
        EFICIENCIA=$(grep "^Eficiencia:" "$LOGFILE" | awk '{print $2}')
        printf "%-10s %-10s %-15s %-15s %-15s %-15s\n" "$N" "$P" "$T_S" "$T_P" "$SPEEDUP" "$EFICIENCIA"
    done
done

echo ""
echo "=> Pruebas de mpi-matmul completadas."
echo "    Logs disponibles en: $LOG_DIR/"
