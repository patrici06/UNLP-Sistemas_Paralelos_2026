# 📊 Contexto Técnico - Métricas de Rendimiento TP2

## 1. Descripción General del Proyecto

**Objetivo:** Evaluar rendimiento de multiplicación de matrices paralelizada con **PThreads** y **OpenMP** en cluster.

**Estructura de la Operación:**
```
R = k × C    donde   C = A × D   y   D = B × B^T
k = ((MaxA × MaxB) - (MinA × MinB)) / (PromA × PromB)
```

- A, B: matrices N×N inicializadas
- D: resultado de B × B^T (almacenado en column-major)
- C: resultado de A × D (almacenado en row-major)  
- R: resultado final k × C
- k: constante matemática derivada de estadísticas de A y B

**Complejidad computacional:** O(N³) operaciones en punto flotante (específicamente 2N³ FLOPs)

---

## 2. Estructura de Archivos CSV

### Ubicación
```
/home/patricio/Escritorio/sistemas-paralelos/TP2/
├── resultados_comparativo.csv    ← Todos los algoritmos combinados
├── resultados_matrices.csv       ← Baseline secuencial
├── resultados_pthread.csv        ← PThreads paralelo
└── resultados_openmp.csv         ← OpenMP paralelo
```

### Estructura de Filas (idéntica en todos)
```
N,T,Algorithm,workTime,GFLOPS,speedup,efficiency,overhead,overhead%
```

### Dimensiones
- **N (tamaño de matriz):** 512, 1024, 2048, 4096
- **T (threads):** 1, 2, 4, 8
- **Algoritmos:** matrices (secuencial), pthread, openmp
- **Total de datos:** 36 filas (4N × 4T × 2 paralelización + 4 baseline)

---

## 3. Definiciones de Métricas

### 3.1 workTime (segundos)
**Definición:** Tiempo de ejecución del algoritmo completo (D + C + k×C)

```
workTime = dwalltime_end - dwalltime_start
```

**Características:**
- Incluye overhead de paralelización (creación threads, barreras, mutex)
- Medido con `gettimeofday()` en cluster
- Precisión: microsegundos

**Rango observado:**
- N=512 T=1: ~0.48s
- N=4096 T=8: ~32.8s

---

### 3.2 GFLOPS (Giga-FLOPs por segundo)
**Definición:** Throughput de operaciones en punto flotante

```
GFLOPS = (2 × N³) / (workTime × 10⁹)
```

**Interpretación:**
- Mayor = mejor desempeño del algoritmo
- Escala con N y T (más trabajo, menos overhead relativo)
- Limitado por ancho de banda de memoria

**Rango observado:**
- N=512 T=1: ~0.55 GFLOPS (peor - matriz pequeña)
- N=4096 T=8 (openmp): ~4.19 GFLOPS (mejor - paralelización efectiva)

---

### 3.3 speedup
**Definición:** Aceleración relativa al tiempo secuencial (T=1)

```
speedup = workTime(T=1) / workTime(T)
```

**Características:**
- speedup = 1.0 → sin ganancia (no paralelización)
- speedup = T → escalabilidad **lineal perfecta**
- speedup < T → overhead de paralelización
- speedup > T → super-lineal (raro, puede ocurrir por cache effects)

**Valores por algoritmo:**

#### PThreads
- T=2: ~1.97-2.05× (excelente escalabilidad)
- T=4: ~3.89-4.10× (muy buena)
- T=8: ~7.27-8.12× (buena, acercándose a T)

#### OpenMP
- T=2: ~1.98-2.00× (excelente)
- T=4: ~3.95-4.07× (muy buena)
- T=8: ~7.50-7.82× (buena)

#### Matrices (secuencial)
- Todos T: speedup = 1.0 (por definición)

---

### 3.4 efficiency
**Definición:** Porcentaje de escalabilidad ideal

```
efficiency = (speedup / T) × 100%
```

**Interpretación:**
- 100% = escalabilidad perfecta (speedup = T)
- <100% = overhead de paralelización
- >100% = super-lineal (raro)

**Rango observado:**
- T=2: 98-103% (excelente, algunas super-lineales)
- T=4: 97-103% (muy buena)
- T=8: 90-97% (degradación esperada)

**Observación:** N=2048 muestra >100% en T=2,4 → super-linear due to cache optimization

---

### 3.5 overhead (segundos)
**Definición:** Costo computacional de la paralelización

```
overhead = workTime(T) - (workTime(T=1) / T)
```

**Interpretación:**
- overhead > 0 → paralelización tiene costo fijo
- overhead < 0 → super-lineal (speedup mejor que ideal)
- overhead ≈ 0 → sin costo significativo

**Causas:**
- Creación/destrucción de threads
- pthread_barrier_wait()
- pthread_mutex_lock()
- OpenMP pragma overhead

**Rango observado:**
- T=1: 0.0s (sin paralelización)
- T=2: 0-0.4s (bajo)
- T=4: 0-0.46s (bajo a moderado)
- T=8: 0-1.1s (moderado, máximo en N=4096)

---

### 3.6 overhead%
**Definición:** Overhead como porcentaje del tiempo de T=1

```
overhead% = (overhead / workTime(T=1)) × 100%
```

**Interpretación:**
- overhead% > 0 → costo relativo significativo
- overhead% < 0 → super-lineal
- overhead% ≈ 0 → costo despreciable

**Rango observado:**
- T=2: -2.5% a +1.5% (overhead muy pequeño)
- T=4: -2.5% a +2.8% (overhead contenido)
- T=8: -1.5% a +9.1% (más visible pero aceptable)

**Tendencia:** Increases con T (más threads = más contención)

---

## 4. Contexto Técnico de Implementación

### 4.1 PThreads (matrices-pthread.c)

**Sincronización:**
```c
pthread_barrier_t barrier;  // 3-stage coordination
pthread_mutex_t mutex;      // Metric aggregation
```

**Esquema:**
1. Threads ejecutan trabajo en paralelo
2. pthread_barrier_wait() al final de cada etapa
3. Thread 0 calcula métricas con mutex protection
4. Barreras previenen data races

**Compilación:** `gcc -O2 -pthread -o matrices-pthread matrices-pthread.c -lm`

**ref_time_sequential (hardcoded):**
```c
if (n == 512) ref_time_sequential = 0.486594;
else if (n == 1024) ref_time_sequential = 3.900768;
else if (n == 2048) ref_time_sequential = 32.309650;
else if (n == 4096) ref_time_sequential = 254.224642;
```

---

### 4.2 OpenMP (matrices-open-mp.c)

**Paralelización:**
```c
#pragma omp parallel for collapse(3) schedule(static)
```

**Características:**
- collapse(3): Paraleliza 3 loops anidados
- schedule(static): Distribución equitativa
- T parámetro obligatorio (no fallback a OMP_NUM_THREADS)

**Compilación:** `gcc -O2 -fopenmp -o matrices-open-mp matrices-open-mp.c -lm`

**ref_time_sequential (hardcoded):**
```c
if (n == 512) ref_time_sequential = 0.486125;
else if (n == 1024) ref_time_sequential = 3.895695;
else if (n == 2048) ref_time_sequential = 32.031549;
else if (n == 4096) ref_time_sequential = 254.817039;
```

---

### 4.3 Matrices (matrices.c)

**Algoritmo:** Secuencial, block-tiling con BS=64

**Características:**
- Base de comparación para speedup
- Sin threads, sin paralelización
- Optimización: Localidad de caché con bloques 64×64

**Compilación:** `gcc -O2 -o matrices matrices.c -lm`

**Salida:** No incluye speedup (solo baseline)

---

## 5. Infraestructura de Ejecución

### Cluster SLURM
- **Queue:** sbatch
- **Compilation:** En cada nodo compute (no frontend)
- **Binary naming:** Temporal con $$ (process ID) para evitar conflictos

### Job Scripts
```bash
TMP_BIN="./program-run-$$"
gcc [flags] -o "$TMP_BIN" source.c -lm
"$TMP_BIN" $N $T
rm -f "$TMP_BIN"
```

### Output Structure
```
salidas/
├── matrices-t1/N{512,1024,2048,4096}/outputs/exec_*.txt
├── pthread-t1/N{512,1024,2048,4096}/outputs/exec_*.txt
├── pthread/N{512,1024,2048,4096}-T{1,2,4,8}/outputs/exec_*.txt
├── openmp-t1/N{512,1024,2048,4096}/outputs/exec_*.txt
└── openmp/N{512,1024,2048,4096}-T{1,2,4,8}/outputs/exec_*.txt
```

---

## 6. Hallazgos Principales

### 6.1 Escalabilidad
- **Hasta T=4:** Excelente (efficiency > 98%)
- **T=8:** Aceptable (efficiency > 90%)
- **Limitación:** NUMA effects, contención de mutex, overhead de barreras

### 6.2 Comparación Algoritmos

| Métrica | PThread | OpenMP | Winner |
|---------|---------|--------|--------|
| Speedup T=8 (N=1024) | 7.79× | 7.82× | OpenMP (+0.4%) |
| GFLOPS T=8 (N=1024) | 4.29 | 4.31 | OpenMP (+0.5%) |
| Overhead T=8 (N=1024) | 0.013s | 0.011s | OpenMP (15% menos) |
| **Recomendación** | ✓ Competitivo | ✓ Ligeramente mejor | **OpenMP** |

### 6.3 Efecto de N
- **N pequeño (512):** Overhead relativo mayor
- **N medio (1024, 2048):** Equilibrio óptimo
- **N grande (4096):** GFLOPS máximo, overhead absoluto crece

### 6.4 Super-linealidad (N=2048)
- Eficiencia > 100% en T=2, 4
- Posible causa: Cache optimization para este tamaño
- Indica: Buena localidad espacial con paralelización

---

## 7. Cómo Interpretar los Datos

### Pregunta: ¿Qué configuración es mejor?
**Respuesta:** Depende de objetivo:
- **Máximo GFLOPS:** T=8, N=4096 (pero overhead > 1s)
- **Mejor eficiencia:** T=4, cualquier N (efficiency ~100%)
- **Balance:** T=4, N=2048 (GFLOPS alto + overhead moderado)

### Pregunta: ¿Por qué algunos overheads son negativos?
**Respuesta:** Super-lineal due to cache effects. El programa paralelo explota mejor la caché que lo predicho idealmente.

### Pregunta: ¿Cuál es el speedup máximo observable?
**Respuesta:** ~7.82× con T=8 (81% efficiency = muy bueno para 8 threads)

---

## 8. Scripts de Análisis Disponibles

```bash
# Generar CSV comparativo
python3 generar_csv_comparativo.py

# Generar CSVs por algoritmo
python3 generar_csv_por_algoritmo.py

# Ver datos formateados
column -t -s, resultados_comparativo.csv

# Filtrar por algoritmo
grep "pthread" resultados_comparativo.csv
grep "openmp" resultados_comparativo.csv

# Filtrar por T
grep ",8," resultados_comparativo.csv
```

---

## 9. Contexto para Análisis Futuro

### Preguntas Abiertas
1. ¿Cómo escala con T>8?
2. ¿Efecto de diferentes block sizes (BS)?
3. ¿Impacto de NUMA en cluster?
4. ¿Escalabilidad débil vs fuerte?

### Datos Relevantes para Reportes
- Speedup vs T (gráficos por N)
- GFLOPS vs N (gráficos por T)
- Efficiency vs T (comparación algoritmos)
- Overhead analysis (identificar cuellos de botella)

### Recomendaciones de Visualización
- Gráficos de línea: speedup/efficiency vs T
- Gráficos de barras: comparación GFLOPS por N
- Heatmaps: eficiencia (N × T)

---

## 10. Metadata de Recolección

- **Cluster:** SLURM-based (detalles en infrastructure)
- **Compilador:** gcc -O2
- **Fecha:** Mayo 2026
- **Referencia tiempo:** Hardcoded desde T=1 runs
- **Precisión:** Microsegundos (gettimeofday)
- **Repeticiones:** 1 por configuración (valores en CSVs)

---

**Última actualización:** 22 de mayo de 2026  
**Archivos relacionados:** ANALISIS_RESULTADOS.md, generar_csv_*.py
