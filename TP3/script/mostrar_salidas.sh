#!/bin/bash
# =============================================================================
# Muestra en pantalla todas las salidas generadas por los scripts de
# benchmark (MPI e Híbrido), indicando claramente el origen y los datos.
#
# Busca resultados en:
#   - times.txt  (TP3/, generado por runs/runP*.sh)
#   - runs/*.out / runs/*.err  (SLURM stdout/stderr)
#   - hybrid-P*/*/output_*.txt  (salidas de blade.sh)
#   - mpi-P*/*/output_*.txt     (salidas de blade.sh)
#
# Uso:  ./mostrar_salidas.sh
# =============================================================================
SCRIPT_DIR=$(dirname "$(realpath "$0")")
BASE_DIR=$(realpath "$SCRIPT_DIR/..")

# =============================================================================
# 1) times.txt  —  resultados estructurados (RESULT;...)
# =============================================================================
TIMES="$BASE_DIR/times.txt"
echo "========================================================================"
echo "  times.txt  (en $BASE_DIR)"
echo "========================================================================"
if [ -f "$TIMES" ]; then
    mode=""
    cur_pt=""  # ej: "16:8"  (P_total : threads)
    cur_n=""   # ultimo N visto en Configuracion
    while IFS= read -r line; do
        case "$line" in
            ===*BENCHMARK*===)
                # Detectar modo: si contiene HÍBRIDO/HIBRIDO es hybrid, sino mpi
                if [[ "$line" == *HÍBRIDO* ]] || [[ "$line" == *HIBRIDO* ]]; then
                    mode="hybrid"
                else
                    mode="mpi"
                fi
                cur_pt=""
                echo ""
                echo "────────────────────────────────────────────────────────"
                echo "  $line"
                echo "────────────────────────────────────────────────────────"
                ;;
            Config:*)
                echo "  $line"
                # Extraer T si es hibrido: "Config: P=16 T=8 (2 MPI × 8 OMP)"
                if [[ "$line" =~ T=([0-9]+) ]]; then
                    cur_pt="${BASH_REMATCH[1]}"
                fi
                ;;
            Nodos:*|Fecha:*)
                echo "  $line"
                ;;
            "Configuracion:"*)
                echo ""
                echo "  --- $line ---"
                ;;
            RESULT\;*)
                IFS=';' read -ra F <<< "$line"
                n="${F[1]}"
                procs="${F[2]}"
                if [ "${#F[@]}" -eq 9 ]; then
                    # matrices-hybrid.c original: n;procs;t;time;gflops;comm;speedup;eff
                    t="${F[3]}"
                    time="${F[4]}"
                    gflops="${F[5]}"
                    comm="${F[6]}"
                    speedup="${F[7]}"
                    eff="${F[8]}"
                    echo "    N=$n  |  MPI=$procs  OMP=$t  |  time=${time}s  GFLOPS=$gflops  comm=${comm}  speedup=$speedup  eff=$eff"
                elif [ "${#F[@]}" -eq 8 ]; then
                    time="${F[3]}"
                    gflops="${F[4]}"
                    comm="${F[5]}"
                    speedup="${F[6]}"
                    eff="${F[7]}"
                    if [ "$mode" = "hybrid" ] && [ -n "$cur_pt" ]; then
                        t="$cur_pt"
                        echo "    N=$n  |  MPI=$procs  OMP=$t  |  time=${time}s  GFLOPS=$gflops  comm=${comm}  speedup=$speedup  eff=$eff"
                    else
                        echo "    N=$n  |  MPI=$procs  |  time=${time}s  GFLOPS=$gflops  comm=${comm}  speedup=$speedup  eff=$eff"
                    fi
                else
                    echo "  $line"
                fi
                ;;
            ----------*)
                ;;
            "")
                ;;
            *)
                echo "  $line"
                ;;
        esac
    done < "$TIMES"
    echo ""
else
    echo "  (no existe — no se han corrido benchmarks)"
    echo ""
fi

# =============================================================================
# 2) runs/*.out  —  stdout de SLURM (scripts nuevos en script/runs/)
# =============================================================================
RUNS_DIR="$SCRIPT_DIR/runs"
if [ -d "$RUNS_DIR" ]; then
    found_any=0
    for salida in "$RUNS_DIR"/*.out; do
        [ -f "$salida" ] || continue
        found_any=1
        filename=$(basename "$salida")
        if [[ "$filename" =~ ^P([0-9]+)T([0-9]+)_[0-9]+\.out$ ]]; then
            p=${BASH_REMATCH[1]}
            t=${BASH_REMATCH[2]}
            origen="Hibrido  |  P=${p} (MPI=$((p/t)) × OMP=${t})"
        elif [[ "$filename" =~ ^P([0-9]+)_[0-9]+\.out$ ]]; then
            p=${BASH_REMATCH[1]}
            origen="MPI puro  |  P=${p}"
        else
            origen="$filename"
        fi
        echo "========================================================================"
        echo "  SLURM stdout: $filename  ($origen)"
        echo "========================================================================"
        cat "$salida"
        echo ""
    done

    # runs/*.err  —  stderr de SLURM (solo no vacios)
    for salida in "$RUNS_DIR"/*.err; do
        [ -f "$salida" ] || continue
        if [ ! -s "$salida" ]; then
            rm -f "$salida"
            continue
        fi
        found_any=1
        filename=$(basename "$salida")
        echo "========================================================================"
        echo "  ⚠ SLURM stderr: $filename"
        echo "========================================================================"
        cat "$salida"
        echo ""
    done

    if [ "$found_any" -eq 0 ]; then
        # No hay .out ni .err con contenido
        :
    fi
fi

# =============================================================================
# 3) hybrid-P* / mpi-P*  —  salidas de blade.sh (output_*.txt)
# =============================================================================
for dirpat in "hybrid-P*" "mpi-P*"; do
    for outdir in "$BASE_DIR"/$dirpat; do
        [ -d "$outdir" ] || continue
        dirname_base=$(basename "$outdir")
        echo "========================================================================"
        echo "  $dirname_base/"
        echo "========================================================================"
        for output_file in "$outdir"/*/output_*.txt; do
            [ -f "$output_file" ] || continue
            subdir=$(basename "$(dirname "$output_file")")
            filename=$(basename "$output_file")
            echo "--- $subdir/$filename ---"
            while IFS= read -r rline; do
                case "$rline" in
                    RESULT\;*)
                        IFS=';' read -ra F <<< "$rline"
                        n="${F[1]}"
                        procs="${F[2]}"
                        if [ "${#F[@]}" -eq 9 ]; then
                            t="${F[3]}"
                            time="${F[4]}"
                            gflops="${F[5]}"
                            comm="${F[6]}"
                            speedup="${F[7]}"
                            eff="${F[8]}"
                            echo "    N=$n  |  MPI=$procs  OMP=$t  |  time=${time}s  GFLOPS=$gflops  comm=${comm}  speedup=$speedup  eff=$eff"
                        elif [ "${#F[@]}" -eq 8 ]; then
                            time="${F[3]}"
                            gflops="${F[4]}"
                            comm="${F[5]}"
                            speedup="${F[6]}"
                            eff="${F[7]}"
                            echo "    N=$n  |  MPI=$procs  |  time=${time}s  GFLOPS=$gflops  comm=${comm}  speedup=$speedup  eff=$eff"
                        else
                            echo "    $rline"
                        fi
                        ;;
                    *)
                        echo "    $rline"
                        ;;
                esac
            done < "$output_file"
            echo ""
        done
    done
done
