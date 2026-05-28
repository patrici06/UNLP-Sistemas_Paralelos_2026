# Instrucciones de Ejecución - Pruebas Cluster

## Resumen

Este conjunto de scripts permite ejecutar pruebas de rendimiento en el cluster con:
- **Matrices**: N = {512, 1024, 2048, 4096}
- **Threads**: T = {1, 2, 4, 8}
- **Implementaciones**: pthread y openmp
- **Compilación**: -O2 (único nivel)

## Flujo de Ejecución

### 1. Compilación Local
Los scripts compilan automáticamente con `-O2`:
```bash
./exec-cluster-test.sh
```

**Nota:** La compilación ocurre en la máquina local antes de enviar a cluster.

### 2. Envío de Jobs
El script `exec-cluster-test.sh`:
1. Compila `matrices-pthread` y `matrices-open-mp` con `-O2`
2. Para cada (N, T, programa), envía un job con `sbatch`
3. Crea estructura de directorios: `salidas/N{n}-T{t}/{pthread,openmp}/outputs/`

### 3. Estructura de Salida

```
salidas/
├─ N512-T1/
│  ├─ pthread/outputs/exec_JOBID.txt
│  └─ openmp/outputs/exec_JOBID.txt
├─ N512-T2/
│  ├─ pthread/outputs/
│  └─ openmp/outputs/
├─ N512-T4/
├─ N512-T8/
├─ N1024-T1/
├─ N1024-T2/
...
└─ N4096-T8/
```

## Formato de Salida

Cada ejecución imprime:
```
RESULT;N;T;workTime;GFLOPS;speedup;efficiency;overhead;overhead%
CONSTANTE_K;valor
VALIDATION;OK/ERROR
```

### Explicación de campos RESULT:

| Campo | Descripción | Notas |
|-------|-------------|-------|
| N | Tamaño de matriz | 512, 1024, 2048, 4096 |
| T | Cantidad de threads | 1, 2, 4, 8 |
| workTime | Tiempo de ejecución (segundos) | Solo multiplicaciones, estadísticas fuera |
| GFLOPS | Operaciones flotantes/segundo | (2*N³) / (workTime * 1e9) |
| speedup | Aceleración vs T=1 | ref_time_sequential / workTime |
| efficiency | Eficiencia de paralelismo (%) | (speedup / T) * 100 |
| overhead | Tiempo de overhead (segundos) | workTime - (ref_time_sequential / T) |
| overhead% | Porcentaje de overhead | (overhead / workTime) * 100 |

### Ejemplo de lectura:
```
RESULT;4096;1;0.5234;31.255;1.000000;100.000000;0.000000;0.000000
RESULT;4096;2;0.2891;56.712;1.809;90.450;0.0184;6.36
RESULT;4096;4;0.1623;100.654;3.223;80.575;0.0389;23.95
RESULT;4096;8;0.0954;171.346;5.483;68.538;0.0612;64.08
```

**Interpretación:**
- T=1: 0.5234s (referencia)
- T=2: 0.2891s → speedup 1.809 → 90.45% eficiente → 6.36% overhead
- T=4: 0.1623s → speedup 3.223 → 80.57% eficiente → 23.95% overhead
- T=8: 0.0954s → speedup 5.483 → 68.53% eficiente → 64.08% overhead

## Cálculos Importantes

### Referencia Secuencial (T=1)
**IMPORTANTE:** El primer dato de T=1 es tu referencia. Guárdalo:
```bash
# Extraer ref_time_sequential de T=1
cat salidas/N4096-T1/pthread/outputs/exec_*.txt | grep "^RESULT"
# Resultará: RESULT;4096;1;0.5234;...
# ref_time_sequential = 0.5234
```

### Cálculo de Speedup (Para T > 1)
```
speedup = ref_time_sequential / workTime_paralelo
speedup = 0.5234 / 0.2891 = 1.809
```

### Cálculo de Eficiencia
```
efficiency = (speedup / T) * 100
efficiency = (1.809 / 2) * 100 = 90.45%
```

### Cálculo de Overhead
```
overhead = workTime - (ref_time_sequential / T)
overhead = 0.2891 - (0.5234 / 2) = 0.2891 - 0.2617 = 0.0274
overhead% = (0.0274 / 0.2891) * 100 = 9.47%
```

## Monitoreo

### Ver jobs en ejecución:
```bash
squeue -u $USER
```

### Ver job específico:
```bash
squeue -j JOBID
```

### Ver output en tiempo real:
```bash
tail -f salidas/N4096-T8/pthread/outputs/exec_JOBID.txt
```

## Extracción de Resultados

### Script para extraer todos los resultados:
```bash
# Listar todas las ejecuciones
find salidas -name "exec_*.txt" -exec grep "^RESULT" {} +

# Agrupar por tamaño
for n in 512 1024 2048 4096; do
    echo "=== N=$n ==="
    find salidas/N${n}-T* -name "exec_*.txt" -exec grep "^RESULT" {} +
done
```

### Script para generar reporte:
```bash
cat > analizar_resultados.sh << 'EOF'
#!/bin/bash
echo "N;T;Program;workTime;GFLOPS;speedup;efficiency;overhead;overhead%"
for dir in salidas/N*-T*/*/outputs/exec_*.txt; do
    grep "^RESULT" "$dir" | awk -F';' '{
        n = match(FILENAME, /N([0-9]+)-T([0-9]+)\/([a-z]+)\//, m)
        printf "%s;%s;%s;%s;%s;%s;%s;%s;%s\n", m[1], m[2], m[3], $4, $5, $6, $7, $8, $9
    }'
done
EOF
chmod +x analizar_resultados.sh
./analizar_resultados.sh
```

## Validación

Cada ejecución debe mostrar:
```
VALIDATION;OK
```

Si ves `VALIDATION;ERROR`, revisa los logs para encontrar NaN o Inf.

## Troubleshooting

### Error: "gcc: command not found"
Asegúrate de estar en un nodo con GCC disponible. En el cluster, usa módulos:
```bash
module load gcc
./exec-cluster-test.sh
```

### Jobs quedan en cola
Usa prioridad en sbatch. Edita `exec-cluster-test.sh`:
```bash
sbatch \
    -o "$OUTPUT_DIR/exec_%j.txt" \
    --job-name="test-N${n}-T${t}-${prog}" \
    --time=00:15:00 \
    --partition=Blade \
    -N 1 --exclusive \
    --priority=HIGH \  # Agregar esta línea
    ./job-test.sh $n $t $prog 2>&1 | awk '{print $NF}'
```

### Timeout de jobs
Si N=4096 con T=1 toma más de 15 minutos, aumenta `--time`:
```bash
--time=00:30:00  # 30 minutos
```

## Notas Importantes

1. **T=1 es referencia obligatoria**: Todos los cálculos de speedup y eficiencia dependen de este.
2. **Ejecutar en orden**: Recomendamos hacerlo secuencialmente por tamaño (primero N=512, luego 1024, etc).
3. **Los datos -O2 son finales**: No cambiar el nivel de compilación.
4. **Guardar logs**: Todos los archivos en `salidas/` son tu data final.

## Comando Rápido

Para ejecutar TODO automáticamente:
```bash
./exec-cluster-test.sh
# Monitorea con:
watch -n 5 'squeue -u $USER | tail -10'
# Cuando terminen todos, analiza:
find salidas -name "exec_*.txt" -exec grep "^RESULT" {} +
```
