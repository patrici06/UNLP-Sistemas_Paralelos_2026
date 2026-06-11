#!/bin/bash
# =============================================================================
# Genera CSVs estructurados a partir de times.txt
#
# Lee times.txt (generado por runs/runP*.sh) y produce:
#   data/resultados.csv       (consolidado)
#   data/mpi_resultados.csv   (solo MPI puro)
#   data/hybrid_resultados.csv (solo Híbrido)
#   data/tp2_resultados.csv   (plantilla para TP2, si no existe)
#
# Columnas CSV:
#   N,tipo,procs_mpi,omp_threads,total_workers,time_s,gflops,comm_pct,speedup,eff
# =============================================================================
SCRIPT_DIR=$(dirname "$(realpath "$0")")
BASE_DIR=$(realpath "$SCRIPT_DIR/..")
TIMES="$BASE_DIR/times.txt"
DATA_DIR="$BASE_DIR/data"

mkdir -p "$DATA_DIR"

# =============================================================================
# Cabeceras CSV
# =============================================================================
HEADER="N,tipo,procs_mpi,omp_threads,total_workers,time_s,gflops,comm_pct,speedup,eff"

# Inicializar archivos con cabecera
echo "$HEADER" > "$DATA_DIR/resultados.csv"
echo "$HEADER" > "$DATA_DIR/mpi_resultados.csv"
echo "$HEADER" > "$DATA_DIR/hybrid_resultados.csv"

# =============================================================================
# Parsea times.txt línea por línea
# =============================================================================
if [ ! -f "$TIMES" ]; then
    echo "  [ERROR] No existe $TIMES"
    echo "  Ejecuta primero los benchmarks (sbatch runs/runP*.sh)"
    exit 1
fi

mode=""
hybrid_threads=""
line_count=0

while IFS= read -r line; do
    # Detectar modo
    case "$line" in
        ===*BENCHMARK*===)
            if [[ "$line" == *HÍBRIDO* ]] || [[ "$line" == *HIBRIDO* ]]; then
                mode="hybrid"
            else
                mode="mpi"
            fi
            hybrid_threads=""
            ;;
        Config:*)
            if [[ "$line" =~ T=([0-9]+) ]]; then
                hybrid_threads="${BASH_REMATCH[1]}"
            fi
            if [[ "$mode" == "mpi" ]]; then
                if [[ "$line" =~ P=([0-9]+) ]]; then
                    hybrid_threads="1"
                fi
            fi
            ;;
        RESULT\;*)
            # RESULT;N;procs;[time;gflops;comm%;speedup;eff] (8 campos)
            #    o    ;N;procs;t;time;gflops;comm%;speedup;eff (9 campos, hybrid original)
            IFS=';' read -ra F <<< "$line"
            n="${F[1]}"
            procs="${F[2]}"

            if [ "${#F[@]}" -eq 9 ]; then
                t="${F[3]}"
                time_s="${F[4]}"
                gflops="${F[5]}"
                comm_pct="${F[6]}"
                speedup="${F[7]}"
                eff="${F[8]}"
            elif [ "${#F[@]}" -eq 8 ]; then
                time_s="${F[3]}"
                gflops="${F[4]}"
                comm_pct="${F[5]}"
                speedup="${F[6]}"
                eff="${F[7]}"
                t="${hybrid_threads:-1}"
            else
                continue
            fi

            # comm_pct y eff vienen con % — lo normalizamos
            comm_pct="${comm_pct/\%/}"
            eff="${eff/\%/}"

            # Calcular total_workers
            if [ "$mode" = "hybrid" ]; then
                total_workers=$(( procs * t ))
            else
                total_workers="$procs"
                t="1"
            fi

            csv_line="${n},${mode},${procs},${t},${total_workers},${time_s},${gflops},${comm_pct},${speedup},${eff}"

            echo "$csv_line" >> "$DATA_DIR/resultados.csv"
            if [ "$mode" = "mpi" ]; then
                echo "$csv_line" >> "$DATA_DIR/mpi_resultados.csv"
            else
                echo "$csv_line" >> "$DATA_DIR/hybrid_resultados.csv"
            fi
            line_count=$((line_count + 1))
            ;;
    esac
done < "$TIMES"

echo "  [OK] $line_count líneas procesadas"
echo "  [OUT] $(realpath "$DATA_DIR/resultados.csv")"
echo "  [OUT] $(realpath "$DATA_DIR/mpi_resultados.csv")"
echo "  [OUT] $(realpath "$DATA_DIR/hybrid_resultados.csv")"

# =============================================================================
# Crear plantilla para TP2 si no existe
# =============================================================================
TP2_TEMPLATE="$DATA_DIR/tp2_resultados.csv"
if [ ! -f "$TP2_TEMPLATE" ]; then
    cat > "$TP2_TEMPLATE" << 'EOF'
N,tipo,procs,time_s,gflops,speedup,eff
2048,Pthreads,8,,,,
4096,Pthreads,8,,,,
EOF
    echo "  [TPL] $TP2_TEMPLATE (plantilla — completar con datos de TP2)"
fi
