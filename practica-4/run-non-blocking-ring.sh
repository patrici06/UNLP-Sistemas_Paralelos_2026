#!/bin/bash
#
# run-non-blocking-ring.sh
# Script para compilar y ejecutar non-blocking-ring.c con los parametros
# especificados en la Practica 4, Ejercicio 3.
#
# Uso: ./run-non-blocking-ring.sh
# Salida: logs/non-blocking-ring-P<N>-N<SIZE>.log
#

set -e

SRC_DIR="Fuentes"
BIN_DIR="bin"
LOG_DIR="logs"

# Crear directorios necesarios
mkdir -p "$BIN_DIR"
mkdir -p "$LOG_DIR"

# Compilar
SRC="$SRC_DIR/non-blocking-ring.c"
BIN="$BIN_DIR/non-blocking-ring"

echo "==> Compilando $SRC ..."
mpicc "$SRC" -o "$BIN" -lm

# Parametros de prueba (Practica 4, Ejercicio 3)
PROCESOS=(4 8 16)
SIZES=(10000000 20000000 40000000)

echo ""
echo "==> Ejecutando pruebas de non-blocking-ring ..."
echo ""

for P in "${PROCESOS[@]}"; do
    for N in "${SIZES[@]}"; do
        LOGFILE="$LOG_DIR/non-blocking-ring-P${P}-N${N}.log"
        
        echo "--- P=$P, N=$N ---"
        
        # --oversubscribe necesario cuando P supera los nucleos disponibles
        if [ "$P" -ge 8 ]; then
            echo "    (usando --oversubscribe)"
            mpirun --oversubscribe -np "$P" ./"$BIN" "$N" | tee "$LOGFILE"
        else
            mpirun -np "$P" ./"$BIN" "$N" | tee "$LOGFILE"
        fi
        
        echo "    Resultado guardado en: $LOGFILE"
        echo ""
    done
done

echo "==> Pruebas de non-blocking-ring completadas."
echo "    Logs disponibles en: $LOG_DIR/"
