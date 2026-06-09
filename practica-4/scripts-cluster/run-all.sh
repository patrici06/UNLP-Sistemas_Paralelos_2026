#!/bin/bash
#
# run-all.sh
# Script maestro para enviar TODOS los jobs SLURM al cluster.
#
# Uso: ./run-all.sh
# Salida: jobs.submitted (lista de JOBIDs)
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

mkdir -p logs-cluster

JOBS_FILE="logs-cluster/jobs.submitted"
echo "Enviando jobs al cluster..." > "$JOBS_FILE"
echo "Fecha: $(date)" >> "$JOBS_FILE"
echo "========================================" >> "$JOBS_FILE"

submit_job() {
    local script="$1"
    local jobid
    jobid=$(sbatch "$script" | awk '{print $4}')
    echo "$jobid  $script" >> "$JOBS_FILE"
    echo "Enviado: $script  -> JOBID=$jobid"
}

# Ejercicio 1
submit_job "ej1-mpi-simple.slurm"

# Ejercicio 2
submit_job "ej2-blocking.slurm"
submit_job "ej2-non-blocking.slurm"

# Ejercicio 3
submit_job "ej3-blocking-ring.slurm"
submit_job "ej3-non-blocking-ring.slurm"

# Ejercicio 4
submit_job "ej4-mpi-matmul.slurm"

# Ejercicio 5 (plantilla - comentado por defecto)
# submit_job "ej5-vector-stats.slurm"

# Ejercicio 6 (plantilla - comentado por defecto)
# submit_job "ej6-hybrid-matmul.slurm"

echo ""
echo "========================================"
echo "Todos los jobs han sido enviados."
echo "Lista guardada en: $JOBS_FILE"
echo ""
echo "Para verificar estado de los jobs:"
echo "  squeue -u \$USER"
echo ""
echo "Una vez finalizados, ejecute:"
echo "  ./collect-metrics.sh"
