#!/bin/bash

# ============================================================
# Script: Procesar y generar reportes en formato CSV
# ============================================================
# Uso: ./generate-reports.sh [tipo] [formato]
#
# Parámetros:
#   tipo: 'matrices', 'pthreads', 'openmp', 'all' (default: all)
#   formato: 'csv', 'json', 'txt' (default: csv)
#
# Ejemplos:
#   ./generate-reports.sh                    # CSV de todos
#   ./generate-reports.sh pthreads csv       # CSV solo pthreads
#   ./generate-reports.sh openmp json        # JSON solo openmp
#   ./generate-reports.sh all txt            # TXT de todos
# ============================================================

TIPO=${1:-all}
FORMATO=${2:-csv}

SALIDAS_DIR="./salidas"
REPORTES_DIR="./reportes"
mkdir -p "$REPORTES_DIR"

# Color para salida
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}=========================================${NC}"
echo -e "${BLUE}Generando reportes en formato: $FORMATO${NC}"
echo -e "${BLUE}Tipo: $TIPO${NC}"
echo -e "${BLUE}=========================================${NC}\n"

# Función para procesar resultados
process_results() {
    local tipo=$1
    local formato=$2
    local output_file=""

    if [ "$formato" = "csv" ]; then
        output_file="$REPORTES_DIR/${tipo}_results.csv"
        
        # Encabezado CSV
        echo "TIPO,TAMAÑO,THREADS,OPTIMIZACIÓN,TIEMPO_SEG,CHECKSUM,VALIDACIÓN,ARCHIVO" > "$output_file"
        
        # Buscar y procesar resultados
        if [ "$tipo" = "matrices" ]; then
            find "$SALIDAS_DIR/matrices" -name "*.txt" -type f | while read file; do
                if grep -q "^RESULT;" "$file"; then
                    result_line=$(grep "^RESULT;" "$file")
                    validation=$(grep "^VALIDATION;" "$file" | cut -d';' -f2)
                    
                    n=$(echo "$result_line" | cut -d';' -f2)
                    time=$(echo "$result_line" | cut -d';' -f3)
                    checksum=$(echo "$result_line" | cut -d';' -f4)
                    
                    # Extraer O del path
                    o=$(echo "$file" | grep -o "/O[0-9]/" | tr -d '/')
                    
                    echo "matrices,$n,N/A,$o,$time,$checksum,$validation,$file" >> "$output_file"
                fi
            done
            
        elif [ "$tipo" = "pthreads" ]; then
            find "$SALIDAS_DIR/pthreads" -name "*.txt" -type f | while read file; do
                if grep -q "^RESULT;" "$file"; then
                    result_line=$(grep "^RESULT;" "$file")
                    validation=$(grep "^VALIDATION;" "$file" | cut -d';' -f2)
                    
                    n=$(echo "$result_line" | cut -d';' -f2)
                    time=$(echo "$result_line" | cut -d';' -f3)
                    checksum=$(echo "$result_line" | cut -d';' -f4)
                    
                    # Extraer T y O del path
                    t=$(echo "$file" | grep -o "/T[0-9]/" | tr -d '/')
                    o=$(echo "$file" | grep -o "/O[0-9]/" | tr -d '/')
                    
                    echo "pthreads,$n,$t,$o,$time,$checksum,$validation,$file" >> "$output_file"
                fi
            done
            
        elif [ "$tipo" = "openmp" ]; then
            find "$SALIDAS_DIR/openmp" -name "*.txt" -type f | while read file; do
                if grep -q "^RESULT;" "$file"; then
                    result_line=$(grep "^RESULT;" "$file")
                    validation=$(grep "^VALIDATION;" "$file" | cut -d';' -f2)
                    
                    n=$(echo "$result_line" | cut -d';' -f2)
                    time=$(echo "$result_line" | cut -d';' -f3)
                    checksum=$(echo "$result_line" | cut -d';' -f4)
                    
                    # Extraer T y O del path
                    t=$(echo "$file" | grep -o "/T[0-9]/" | tr -d '/')
                    o=$(echo "$file" | grep -o "/O[0-9]/" | tr -d '/')
                    
                    echo "openmp,$n,$t,$o,$time,$checksum,$validation,$file" >> "$output_file"
                fi
            done
        fi
        
        if [ -f "$output_file" ]; then
            echo -e "${GREEN}✓ Generado: $output_file${NC}"
            echo "  Líneas: $(wc -l < "$output_file")"
            echo ""
        fi

    elif [ "$formato" = "txt" ]; then
        output_file="$REPORTES_DIR/${tipo}_results.txt"
        
        {
            echo "=========================================="
            echo "Resultados: $tipo"
            echo "Fecha: $(date '+%Y-%m-%d %H:%M:%S')"
            echo "=========================================="
            echo ""
            
            if [ "$tipo" = "matrices" ]; then
                echo "RESULTADOS SECUENCIALES (matrices.c)"
                echo ""
                echo "N      O   Tiempo(s)  Checksum     Validación"
                echo "---    ---  ---------  --------     ----------"
                
                find "$SALIDAS_DIR/matrices" -name "*.txt" -type f | sort | while read file; do
                    if grep -q "^RESULT;" "$file"; then
                        result_line=$(grep "^RESULT;" "$file")
                        validation=$(grep "^VALIDATION;" "$file" | cut -d';' -f2)
                        
                        n=$(echo "$result_line" | cut -d';' -f2)
                        time=$(echo "$result_line" | cut -d';' -f3)
                        checksum=$(echo "$result_line" | cut -d';' -f4)
                        o=$(echo "$file" | grep -o "/O[0-9]/" | tr -d '/')
                        
                        printf "%-6s %-3s %9.6f  %-12s %s\n" "$n" "$o" "$time" "$checksum" "$validation"
                    fi
                done
                
            elif [ "$tipo" = "pthreads" ]; then
                echo "RESULTADOS PTHREADS"
                echo ""
                echo "N      T  O   Tiempo(s)  Checksum     Validación"
                echo "---    -- --- ---------  --------     ----------"
                
                find "$SALIDAS_DIR/pthreads" -name "*.txt" -type f | sort | while read file; do
                    if grep -q "^RESULT;" "$file"; then
                        result_line=$(grep "^RESULT;" "$file")
                        validation=$(grep "^VALIDATION;" "$file" | cut -d';' -f2)
                        
                        n=$(echo "$result_line" | cut -d';' -f2)
                        time=$(echo "$result_line" | cut -d';' -f3)
                        checksum=$(echo "$result_line" | cut -d';' -f4)
                        t=$(echo "$file" | grep -o "/T[0-9]/" | tr -d '/')
                        o=$(echo "$file" | grep -o "/O[0-9]/" | tr -d '/')
                        
                        printf "%-6s %-2s %-3s %9.6f  %-12s %s\n" "$n" "$t" "$o" "$time" "$checksum" "$validation"
                    fi
                done
                
            elif [ "$tipo" = "openmp" ]; then
                echo "RESULTADOS OpenMP"
                echo ""
                echo "N      T  O   Tiempo(s)  Checksum     Validación"
                echo "---    -- --- ---------  --------     ----------"
                
                find "$SALIDAS_DIR/openmp" -name "*.txt" -type f | sort | while read file; do
                    if grep -q "^RESULT;" "$file"; then
                        result_line=$(grep "^RESULT;" "$file")
                        validation=$(grep "^VALIDATION;" "$file" | cut -d';' -f2)
                        
                        n=$(echo "$result_line" | cut -d';' -f2)
                        time=$(echo "$result_line" | cut -d';' -f3)
                        checksum=$(echo "$result_line" | cut -d';' -f4)
                        t=$(echo "$file" | grep -o "/T[0-9]/" | tr -d '/')
                        o=$(echo "$file" | grep -o "/O[0-9]/" | tr -d '/')
                        
                        printf "%-6s %-2s %-3s %9.6f  %-12s %s\n" "$n" "$t" "$o" "$time" "$checksum" "$validation"
                    fi
                done
            fi
            
        } > "$output_file"
        
        if [ -f "$output_file" ]; then
            echo -e "${GREEN}✓ Generado: $output_file${NC}"
            echo ""
        fi

    elif [ "$formato" = "json" ]; then
        output_file="$REPORTES_DIR/${tipo}_results.json"
        
        {
            echo "{"
            echo "  \"tipo\": \"$tipo\","
            echo "  \"fecha\": \"$(date '+%Y-%m-%d %H:%M:%S')\","
            echo "  \"resultados\": ["
            
            first=true
            if [ "$tipo" = "matrices" ]; then
                find "$SALIDAS_DIR/matrices" -name "*.txt" -type f | sort | while read file; do
                    if grep -q "^RESULT;" "$file"; then
                        if [ "$first" = false ]; then echo ","; fi
                        
                        result_line=$(grep "^RESULT;" "$file")
                        validation=$(grep "^VALIDATION;" "$file" | cut -d';' -f2)
                        
                        n=$(echo "$result_line" | cut -d';' -f2)
                        time=$(echo "$result_line" | cut -d';' -f3)
                        checksum=$(echo "$result_line" | cut -d';' -f4)
                        o=$(echo "$file" | grep -o "/O[0-9]/" | tr -d '/')
                        
                        cat <<EOF
    {
      "tipo": "matrices",
      "matriz_size": $n,
      "optimization": "$o",
      "tiempo_segundos": $time,
      "checksum": $checksum,
      "validacion": "$validation",
      "archivo": "$file"
    }
EOF
                        first=false
                    fi
                done
            fi
            
            echo ""
            echo "  ]"
            echo "}"
            
        } > "$output_file"
        
        if [ -f "$output_file" ]; then
            echo -e "${GREEN}✓ Generado: $output_file${NC}"
            echo ""
        fi
    fi
}

# Procesar según tipo
if [ "$TIPO" = "all" ]; then
    process_results "matrices" "$FORMATO"
    process_results "pthreads" "$FORMATO"
    process_results "openmp" "$FORMATO"
else
    process_results "$TIPO" "$FORMATO"
fi

echo -e "${GREEN}Reportes generados en: $REPORTES_DIR/${NC}\n"
echo "Archivos generados:"
ls -lh "$REPORTES_DIR" | tail -n +2
