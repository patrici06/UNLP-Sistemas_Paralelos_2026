#!/bin/bash
#
# ej4-mpi-matmul.slurm
# Script SLURM para Ejercicio 4 (mpi_matmul.c)
#
# Parametros segun practica:
#   N = {512, 1024, 2048}
#   Ejecutar usando todos los nucleos de 1 y 2 nodos.
#
#   Se asumen 4 cores por nodo (ajustar si el cluster tiene otra config).
#   - 1 nodo -> 4 procesos
#   - 2 nodos -> 8 procesos
#   Ademas se mide el tiempo secuencial (P=1) para calcular Speedup y Eficiencia.
#
# Uso en cluster:   sbatch ej4-mpi-matmul.slurm
#

#SBATCH --job-name=ej4-mpi-matmul
#SBATCH -N 2
#SBATCH --exclusive
#SBATCH --tasks-per-node=4
#SBATCH -o logs-cluster/ej4-mpi-matmul-%j.out
#SBATCH -e logs-cluster/ej4-mpi-matmul-%j.err

set -e

cd /home/patricio/Escritorio/sistemas-paralelos/practica-4

SRC_DIR="Fuentes"
BIN_DIR="bin"
LOG_DIR="scripts-cluster/logs-cluster"

mkdir -p "$BIN_DIR" "$LOG_DIR"

SRC="$SRC_DIR/mpi_matmul.c"
BIN="$BIN_DIR/mpi_matmul"

echo "==> [Ej4] Compilando $SRC ..."
mpicc "$SRC" -o "$BIN" -lm

echo ""
echo "==> [Ej4] Ejecutando mpi_matmul ..."
echo ""

SIZES=(512 1024 2048)
# Configuracion de nodos/procesos a probar
# Formato: "Nodos|TareasPorNodo"
CONFIGS=(
    "1|1"   # Secuencial (P=1)
    "1|4"   # 1 nodo, 4 procesos
    "2|4"   # 2 nodos, 4 proc/nodo = 8 procesos total
)

declare -A T_SEQ

# -----------------------------------------------
# 1) Mediciones secuenciales y paralelas
# -----------------------------------------------
for CFG in "${CONFIGS[@]}"; do
    NODES=$(echo "$CFG" | cut -d'|' -f1)
    TPN=$(echo "$CFG" | cut -d'|' -f2)
    P=$(( NODES * TPN ))

    for N in "${SIZES[@]}"; do
        LOGFILE="$LOG_DIR/ej4-mpi-matmul-N${N}-P${P}.log"
        echo "--- Ejercicio 4 | N=$N, P=$P (${NODES} nodo(s), ${TPN} proc/nodo) ---" | tee "$LOGFILE"

        # Si es secuencial, guardamos su tiempo para speedup posterior
        if [ "$P" -eq 1 ]; then
            mpirun -np 1 ./"$BIN" "$N" | tee -a "$LOGFILE"
            T_S=$(grep "Tiempo total=" "$LOGFILE" | head -1 | awk -F'Tiempo total=' '{print $2}' | awk '{print $1}')
            T_SEQ[$N]=$T_S
            echo "    Tiempo secuencial (N=$N): $T_S s" | tee -a "$LOGFILE"
        else
            mpirun -np "$P" ./"$BIN" "$N" | tee -a "$LOGFILE"
        fi

        echo "    Log guardado en: $LOGFILE"
        echo ""
    done
done

# -----------------------------------------------
# 2) Calcular Speedup y Eficiencia para casos paralelos
# -----------------------------------------------
echo ""
echo "=========================================="
echo "  [Ej4] METRICAS CALCULADAS"
echo "=========================================="
echo ""

for CFG in "${CONFIGS[@]}"; do
    NODES=$(echo "$CFG" | cut -d'|' -f1)
    TPN=$(echo "$CFG" | cut -d'|' -f2)
    P=$(( NODES * TPN ))

    [ "$P" -eq 1 ] && continue

    for N in "${SIZES[@]}"; do
        LOGFILE="$LOG_DIR/ej4-mpi-matmul-N${N}-P${P}.log"
        T_P=$(grep "Tiempo total=" "$LOGFILE" | head -1 | awk -F'Tiempo total=' '{print $2}' | awk '{print $1}')
        T_S=${T_SEQ[$N]}

        SPEEDUP=$(echo "scale=6; $T_S / $T_P" | bc | awk '{printf "%0.6f\n", $0}')
        EFICIENCIA=$(echo "scale=6; $SPEEDUP / $P" | bc | awk '{printf "%0.6f\n", $0}')

        echo "N=$N | P=$P | Ts=$T_S | Tp=$T_P | S=$SPEEDUP | E=$EFICIENCIA"

        # Agregar metricas al log
        {
            echo ""
            echo "=== Metricas calculadas ==="
            echo "N: $N"
            echo "Procesos: $P"
            echo "Nodos: $NODES"
            echo "TareasPorNodo: $TPN"
            echo "Tiempo Secuencial: $T_S"
            echo "Tiempo Paralelo: $T_P"
            echo "Speedup: $SPEEDUP"
            echo "Eficiencia: $EFICIENCIA"
        } >> "$LOGFILE"
    done
done

# -----------------------------------------------
# 3) Tabla resumen
# -----------------------------------------------
echo ""
echo "=========================================="
echo "  [Ej4] TABLA RESUMEN"
echo "=========================================="
printf "%-8s %-8s %-15s %-15s %-15s %-15s\n" "N" "P" "T_Seq" "T_Par" "Speedup" "Eficiencia"
for CFG in "${CONFIGS[@]}"; do
    NODES=$(echo "$CFG" | cut -d'|' -f1)
    TPN=$(echo "$CFG" | cut -d'|' -f2)
    P=$(( NODES * TPN ))
    [ "$P" -eq 1 ] && continue

    for N in "${SIZES[@]}"; do
        LOGFILE="$LOG_DIR/ej4-mpi-matmul-N${N}-P${P}.log"
        T_S=${T_SEQ[$N]}
        T_P=$(grep "Tiempo total=" "$LOGFILE" | head -1 | awk -F'Tiempo total=' '{print $2}' | awk '{print $1}')
        SPEEDUP=$(grep "^Speedup:" "$LOGFILE" | awk '{print $2}')
        EFICIENCIA=$(grep "^Eficiencia:" "$LOGFILE" | awk '{print $2}')
        printf "%-8s %-8s %-15s %-15s %-15s %-15s\n" "$N" "$P" "$T_S" "$T_P" "$SPEEDUP" "$EFICIENCIA"
    done
done

echo ""
echo "==> [Ej4] Finalizado. Logs en: $LOG_DIR/"
