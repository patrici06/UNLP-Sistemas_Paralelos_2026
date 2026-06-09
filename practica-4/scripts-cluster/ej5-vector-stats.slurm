#!/bin/bash
#
# ej5-vector-stats.slurm
# Script SLURM para Ejercicio 5 (vector max, min, promedio)
#
# Parametros segun practica:
#   P = {2, 4, 8, 16}
#   N = {10000000, 50000000, 100000000} (ajustar a conveniencia)
#
# IMPORTANTE: Antes de usar este script debe existir el archivo fuente
#   Fuentes/vector-stats.c  (implementacion propia del ejercicio 5)
#
# Uso en cluster:   sbatch ej5-vector-stats.slurm
#

#SBATCH --job-name=ej5-vector-stats
#SBATCH -N 2
#SBATCH --exclusive
#SBATCH --overcommit
#SBATCH --tasks-per-node=4
#SBATCH -o logs-cluster/ej5-vector-stats-%j.out
#SBATCH -e logs-cluster/ej5-vector-stats-%j.err

set -e

cd /home/patricio/Escritorio/sistemas-paralelos/practica-4

SRC_DIR="Fuentes"
BIN_DIR="bin"
LOG_DIR="scripts-cluster/logs-cluster"

mkdir -p "$BIN_DIR" "$LOG_DIR"

# Compilar (descomentar cuando exista el fuente)
# SRC="$SRC_DIR/vector-stats.c"
# BIN="$BIN_DIR/vector-stats"
# echo "==> [Ej5] Compilando $SRC ..."
# mpicc "$SRC" -o "$BIN" -lm

# echo ""
# echo "==> [Ej5] Ejecutando vector-stats ..."
# echo ""

# PROCESOS=(2 4 8 16)
# SIZES=(10000000 50000000 100000000)

# declare -A T_SEQ

# for P in "${PROCESOS[@]}"; do
#     for N in "${SIZES[@]}"; do
#         LOGFILE="$LOG_DIR/ej5-vector-stats-P${P}-N${N}.log"
#         echo "--- Ejercicio 5 | P=$P, N=$N ---" | tee "$LOGFILE"
#
#         if [ "$P" -eq 1 ]; then
#             mpirun -np 1 ./"$BIN" "$N" | tee -a "$LOGFILE"
#             T_S=$(grep "Tiempo total=" "$LOGFILE" | head -1 | awk -F'Tiempo total=' '{print $2}' | awk '{print $1}')
#             T_SEQ[$N]=$T_S
#         else
#             if [ "$P" -ge 16 ]; then
#                 mpirun --oversubscribe -np "$P" ./"$BIN" "$N" | tee -a "$LOGFILE"
#             else
#                 mpirun -np "$P" ./"$BIN" "$N" | tee -a "$LOGFILE"
#             fi
#         fi
#
#         echo "    Log guardado en: $LOGFILE"
#         echo ""
#     done
# done

# echo "==> [Ej5] Finalizado."

echo "ATENCION: Este script es una plantilla."
echo "Debes implementar Fuentes/vector-stats.c y descomentar las lineas de compilacion/ejecucion."
