#!/bin/bash
#
# run-ej4.sh
# Envia el job SLURM del Ejercicio 4 al cluster.
#
# Uso: ./run-ej4.sh
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

JOBS_FILE="jobs.submitted"
echo "Enviando Ejercicio 4 al cluster..." > "$JOBS_FILE"
echo "Fecha: $(date)" >> "$JOBS_FILE"
echo "========================================" >> "$JOBS_FILE"

jobid=$(sbatch ej4-mpi-matmul.slurm | awk '{print $4}')
echo "$jobid  ej4-mpi-matmul.slurm" >> "$JOBS_FILE"

echo ""
echo "========================================"
echo "Job enviado exitosamente."
echo "JOBID: $jobid"
echo "Script: ej4-mpi-matmul.slurm"
echo "========================================"
echo ""
echo "Para verificar estado:"
echo "  squeue -u \$USER"
echo ""
echo "Una vez finalizado, ejecute:"
echo "  ./collect-metrics-ej4.sh"
