#!/bin/bash
#
# run-blocking-ring.sh
# Script para compilar y ejecutar blocking-ring.c con los parametros
# especificados en la Practica 4, Ejercicio 3.
#
# Uso: ./run-blocking-ring.sh
# Salida: logs/blocking-ring-P<N>-N<SIZE>.log
#

set -e

SRC_DIR="Fuentes"
BIN_DIR="bin"
LOG_DIR="logs"

# Crear directorios necesarios
mkdir -p "$BIN_DIR"
mkdir -p "$LOG_DIR"

# Compilar
SRC="$SRC_DIR/blocking-ring.c"
BIN="$BIN_DIR/blocking-ring"

echo "==> Compilando $SRC ..."
mpicc "$SRC" -o "$BIN" -lm

# Parametros de prueba
PROCESOS=(2 4 8)
SIZES=(10000000 20000000)

echo ""
echo "==> Ejecutando pruebas de blocking-ring ..."
echo ""

for P in "${PROCESOS[@]}"; do
    for N in "${SIZES[@]}"; do
        LOGFILE="$LOG_DIR/blocking-ring-P${P}-N${N}.log"
        
        echo "--- P=$P, N=$N ---"
        
        # Para P=16 se requiere --oversubscribe (segun nota de la practica)
        if [ "$P" -ge 8 ]; then
            echo "    (usando --oversubscribe para P=8)"
            mpirun --oversubscribe -np "$P" ./"$BIN" "$N" | tee "$LOGFILE"
        else
            mpirun -np "$P" ./"$BIN" "$N" | tee "$LOGFILE"
        fi
        
        echo "    Resultado guardado en: $LOGFILE"
        echo ""
    done
done

echo "==> Pruebas de blocking-ring completadas."
echo "    Logs disponibles en: $LOG_DIR/"
