# Guía para Ejecutar en Cluster Remoto

## Información de Arquitectura (Cluster)

Según el informe anterior, el cluster tiene:
- **CPU**: AMD Ryzen 5 9600X (basado en Zen 5)
- **Núcleos**: 6 (12 hilos con SMT)
- **SIMD**: SSE4.2 (registros 128 bits, menos óptimo que AVX2)
- **Cache L1**: Típicamente 32KB por core
- **Cache L2**: Típicamente 512KB por core

## Compilación en Cluster

```bash
# SSH al cluster
ssh user@cluster_host

# Navegar al directorio
cd practica-2

# Compilar todas las versiones
gcc -O3 -o matrices matrices.c -lm
gcc -O3 -pthread -o matrices_pthreads matrices_pthreads.c -lm
gcc -O3 -fopenmp -o matrices_openmp matrices_openmp.c -lm
```

## Ejecución de Pruebas

### Plan de Pruebas Recomendado

```bash
#!/bin/bash
# Script: run_tests.sh

# Tamaños a probar
SIZES=(512 1024 2048 4096)

# Número de threads
THREADS=(2 4 8)

# Secuencial (línea base)
echo "=== SECUENCIAL ==="
for N in "${SIZES[@]}"; do
    echo "N=$N"
    time ./matrices $N
done

# Pthreads
echo "=== PTHREADS ==="
for N in "${SIZES[@]}"; do
    for T in "${THREADS[@]}"; do
        if [ $T -le 6 ]; then  # Max 6 cores
            echo "N=$N T=$T"
            time ./matrices_pthreads $N $T
        fi
    done
done

# OpenMP
echo "=== OpenMP ==="
export OMP_NUM_THREADS=1
for N in "${SIZES[@]}"; do
    for T in "${THREADS[@]}"; do
        if [ $T -le 6 ]; then  # Max 6 cores
            echo "N=$N T=$T"
            export OMP_NUM_THREADS=$T
            time ./matrices_openmp $N $T
        fi
    done
done
```

## Recolección de Datos

Los programas generan salida en formato CSV:
```
RESULT;N;T;time;GFLOPS
VALIDATION;OK|ERROR
```

### Script de Recolección

```bash
#!/bin/bash
# Script: collect_results.sh

LOG_FILE="results_$(date +%Y%m%d_%H%M%S).csv"

echo "N,Program,Threads,Time,GFLOPS" > $LOG_FILE

# Secuencial
for N in 512 1024 2048 4096; do
    ./matrices $N | grep "^RESULT" | awk -v prog="secuencial" -v t=1 -F';' \
        '{print $2","prog","t","$3","$4}' >> $LOG_FILE
done

# Pthreads
for N in 512 1024 2048 4096; do
    for T in 2 4 8; do
        ./matrices_pthreads $N $T | grep "^RESULT" | awk -v prog="pthreads" -F';' \
            '{print $2","prog","$3","$4","$5}' >> $LOG_FILE
    done
done

# OpenMP
export OMP_NUM_THREADS=1
for N in 512 1024 2048 4096; do
    for T in 2 4 8; do
        export OMP_NUM_THREADS=$T
        ./matrices_openmp $N $T | grep "^RESULT" | awk -v prog="openmp" -F';' \
            '{print $2","prog","$3","$4","$5}' >> $LOG_FILE
    done
done

echo "Resultados guardados en: $LOG_FILE"
```

## Esperado en Cluster

Basado en arquitectura SSE4.2 (menos optimizada que AVX2):

### Throughput Esperado (GFLOPS)

```
N=512:
  Secuencial:  ~1.5 GFLOPS (baseline)
  Pthreads(4): ~4.5 GFLOPS (speedup ~3.0x)
  OpenMP(4):   ~5.0 GFLOPS (speedup ~3.3x)

N=1024:
  Secuencial:  ~1.4 GFLOPS
  Pthreads(4): ~4.0 GFLOPS (speedup ~2.9x)
  OpenMP(4):   ~4.8 GFLOPS (speedup ~3.4x)

N=2048:
  Secuencial:  ~1.3 GFLOPS
  Pthreads(4): ~3.8 GFLOPS (speedup ~2.9x)
  OpenMP(4):   ~4.5 GFLOPS (speedup ~3.5x)

N=4096:
  Secuencial:  ~1.2 GFLOPS
  Pthreads(4): ~3.5 GFLOPS (speedup ~2.9x)
  OpenMP(4):   ~4.2 GFLOPS (speedup ~3.5x)
```

### Razones de Diferencia Local vs Cluster

1. **SIMD más estrecho** (SSE4.2 vs AVX2)
   - SSE: 128 bits = 4 floats, 2 doubles
   - AVX2: 256 bits = 8 floats, 4 doubles

2. **Overhead de coherencia de cache**
   - Local: coherencia muy rápida (SMT compartido)
   - Cluster: coherencia más lenta (cores separados)

3. **Ancho de banda de memoria**
   - Local: más rápido (arquitectura diferente)
   - Cluster: puede ser limitante para N=4096

## Optimizaciones Adicionales (si es necesario)

Si el rendimiento es insuficiente:

1. **Aumentar BS** (pero con cuidado):
   - Actual: BS=32 (óptimo para L1)
   - Posible: BS=48 (si L1 es mayor)
   - Riesgo: cache misses aumentan

2. **Desabilitar SMT**:
   ```bash
   echo 0 > /sys/devices/system/cpu/smt/control
   ```
   (requiere acceso root)

3. **Fijar CPU affinity** (Pthreads):
   ```c
   // En worker_thread:
   cpu_set_t cpuset;
   CPU_ZERO(&cpuset);
   CPU_SET(thread_id % num_cores, &cpuset);
   pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
   ```

4. **OpenMP affinity**:
   ```bash
   export OMP_PROC_BIND=spread
   export OMP_PLACES=cores
   ```

## Validación en Cluster

Importante: Verificar que TODAS las ejecuciones produzcan "VALIDATION;OK"

```bash
# Script de validación
for prog in ./matrices ./matrices_pthreads ./matrices_openmp; do
    for N in 512 1024 2048 4096; do
        echo -n "$prog N=$N: "
        if [ "$prog" = "./matrices" ]; then
            RESULT=$($prog $N 2>&1 | grep VALIDATION)
        else
            RESULT=$($prog $N 4 2>&1 | grep VALIDATION)
        fi
        echo $RESULT
    done
done
```

## Recomendaciones Finales

1. **Probar primero en local** para asegurar corrección
2. **Usar SSH multiplexing** para conexiones rápidas
3. **Ejecutar en batch** (redireccionar a archivo)
4. **Tener tiempos de ejecución listos** antes de entregar

