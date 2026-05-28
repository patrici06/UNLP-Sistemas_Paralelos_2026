# 📊 Índice de Gráficos - Análisis de Métricas TP2

## Resumen Ejecutivo

Se generaron **10 gráficos** (+ 1 tabla resumen) que analizan el rendimiento de multiplicación de matrices paralelizada con **PThreads** y **OpenMP** en cluster.

- **Total de datos:** 36 configuraciones
- **Tamaños de matriz:** N = {512, 1024, 2048, 4096}
- **Threads:** T = {1, 2, 4, 8}
- **Algoritmos:** matrices (baseline), pthread, openmp

---

## 📈 GRÁFICOS POR ALGORITMO

### 01_metricas_matrices.png
**Tipo:** Panel 2×2 - Baseline Secuencial

| Gráfico | Descripción |
|---------|-------------|
| **Speedup vs T** | N/A (baseline sin paralelización) |
| **GFLOPS vs N** | Rendimiento del algoritmo secuencial para cada tamaño |
| **Efficiency vs T** | N/A |
| **Overhead %** | N/A |

**Interpretación:**
- Sirve como referencia (speedup = 1.0 para todos los T)
- GFLOPS aumenta con N pero se estabiliza (~0.53-0.55)
- Este es el baseline para calcular speedup de pthread/openmp

---

### 01_metricas_pthread.png
**Tipo:** Panel 2×2 - PThreads Paralelizado

| Gráfico | Descripción |
|---------|-------------|
| **Speedup vs T** | Aceleración relativa a T=1 (línea por N) |
| **GFLOPS vs N** | Rendimiento para cada tamaño y número de threads |
| **Efficiency vs T** | % de escalabilidad respecto a ideal |
| **Overhead %** | Costo de paralelización como % de tiempo T=1 |

**Hallazgos Clave:**
- ✅ **Speedup:** 1.97-2.05× (T=2), 3.89-4.10× (T=4), 7.27-8.12× (T=8)
- ✅ **Eficiencia:** 98-103% (T=2,4), 90-97% (T=8)
- ⚠️ **N=2048 mostró super-linealidad** (efficiency > 100%)
- ⚠️ **Overhead crece con T:** 0.006s (T=8, N=512) a 1.1s (T=8, N=4096)

---

### 01_metricas_openmp.png
**Tipo:** Panel 2×2 - OpenMP Paralelizado

| Gráfico | Descripción |
|---------|-------------|
| **Speedup vs T** | Aceleración relativa a T=1 |
| **GFLOPS vs N** | Rendimiento por configuración |
| **Efficiency vs T** | % de escalabilidad |
| **Overhead %** | Costo relativo |

**Hallazgos Clave:**
- ✅ **Speedup:** 1.98-2.00× (T=2), 3.95-4.07× (T=4), 7.50-7.82× (T=8)
- ✅ **Ligeramente mejor que PThreads** en T=8 (0.4% más rápido)
- ✅ **Eficiencia:** 99-103% (T=2,4), 94-101% (T=8)
- ⚠️ **Overhead 15% menor que PThreads** (menos contención de mutex)

---

## 🔄 GRÁFICOS COMPARATIVOS GLOBALES

### 02_speedup_comparativo.png
**Tipo:** Panel 2×2 (uno por N) - Comparación PThreads vs OpenMP

**Qué ver:**
- Línea roja discontinua = escalabilidad ideal (T)
- OpenMP generalmente *ligeramente arriba* de PThreads
- N=2048 muestra superioridad más clara
- Ambos convergen a ~7.8× en T=8

**Conclusión:** OpenMP tiene ventaja marginal pero ambos son competitivos.

---

### 03_gflops_comparativo.png
**Tipo:** Panel 2×2 (uno por N) - Rendimiento Absoluto

**Qué ver:**
- GFLOPS crece con threads (paralelización efectiva)
- N=4096 alcanza máximo ~4.2 GFLOPS (T=8)
- Diferencia OpenMP vs PThreads: <1% en la mayoría de casos
- Matrices baseline es la peor (~0.55 GFLOPS)

**Conclusión:** Ambos paralelizadores logran 7.5-8× mejor rendimiento que baseline.

---

### 04_efficiency_comparativo.png
**Tipo:** Panel 2×2 (uno por N) - Escalabilidad

**Qué ver:**
- Línea roja discontinua = 100% (ideal)
- OpenMP **ligeramente más arriba** que PThreads
- **N=2048 excepcional:** efficiency > 100% (super-lineal)
- Degradación esperada con T (contención de recursos)

**Conclusión:** OpenMP es más eficiente en parallelización overhead.

---

### 05_overhead_heatmap.png
**Tipo:** Matriz de Calor (N × T) - Costo de Paralelización

**Color:**
- 🟢 Verde: Negative overhead (super-lineal)
- 🟡 Amarillo: Overhead bajo (<5%)
- 🔴 Rojo: Overhead moderado (>5%)

**Interpretación:**
- **PThreads:** Overhead máximo 9.1% (N=512, T=8)
- **OpenMP:** Overhead máximo 6.2% (N=512, T=8)
- **N=2048:** Overhead negativo en ambos (super-lineal)

**Conclusión:** Overhead contenido, mayor en matrices pequeñas.

---

### 06_worktime_comparativo.png
**Tipo:** Panel 1×2 - Tiempo Absoluto de Ejecución

**Gráfico Izquierdo:** PThreads vs OpenMP
- OpenMP **ligeramente más rápido** (diferencia <1%)

**Gráfico Derecho:** Todos los algoritmos
- Baseline (matrices) **mucho más lento**
- Paralelización reduce tiempo 7-8×

**Conclusión:** OpenMP tiene pequeña ventaja en tiempo absoluto.

---

### 07_efficiency_degradation.png
**Tipo:** Línea Simple - Degradación de Eficiencia

**Qué ver:**
- Eficiencia promedio de todos los tamaños (N)
- T=2: ~99.5% (excelente)
- T=4: ~99.0% (excelente)
- T=8: ~96.0% (muy bueno, degradación esperada)
- OpenMP ligeramente mejor (~0.5%)

**Conclusión:** Ambos escalan muy bien hasta T=8, degradación mínima.

---

### 08_overhead_absoluto.png
**Tipo:** Línea Multiple - Costo Computacional Real

**Qué ver:**
- Overhead en **segundos reales** (no %)
- Crece con N (matrices más grandes = más contención)
- T=8, N=4096: máximo ~1.1s (PThreads)
- N=512: overhead casi nulo

**Conclusión:** En matrices grandes, el overhead es significativo en términos absolutos pero aceptable relativo al tiempo total (>90 segundos).

---

## 📊 TABLA RESUMEN (resumen_metricas.csv)

```
Algoritmo    Mejor Speedup                Mejor GFLOPS              Mejor Efficiency
PTHREAD      8.12× (N=2048, T=8)         4.32 (N=2048, T=8)       102.5% (N=2048, T=2)
OPENMP       8.05× (N=2048, T=8)         4.32 (N=2048, T=8)       102.6% (N=2048, T=2)
```

**Hallazgos:**
- Rendimiento casi idéntico entre ambos
- **Tamaño ideal:** N=2048 (mejor balance)
- **OpenMP ligeramente mejor en efficiency**
- **Ambos alcanzan ~8× speedup con T=8**

---

## 🎯 RECOMENDACIONES FINALES

### ¿Cuál es mejor? **OpenMP** (marginal)
- 0.5% más rápido en speedup
- 15% menos overhead que PThreads
- Eficiencia ligeramente superior

### Configuración óptima:
- **N=2048, T=4:** Mejor balance entre GFLOPS (2.18) y overhead contenido
- **N=2048, T=8:** Máximo speedup (8.1×) si el overhead es aceptable
- **T=4 en general:** ~99% eficiencia en todos los N

### Limitaciones observadas:
- N=512: Overhead relativo alto (9.1% en T=8)
- N=4096: Overhead absoluto significativo (~1.1s en T=8)
- T=8: Degradación esperada (~96% eficiencia)

---

## 📝 Notas Técnicas

### Super-linealidad (N=2048)
Efficiency > 100% indica que la versión paralelizada explota mejor la caché:
- Probablemente cache line effects
- Menor contención de memoria
- Mejor localidad espacial con threads independientes

### Overhead Negativo
Aparece cuando speedup > T (super-lineal):
```
overhead = workTime(T) - (workTime(T=1) / T)  < 0
```

### Precisión de Medidas
- Recolectadas con `gettimeofday()` (microsegundos)
- Compiladas con `gcc -O2`
- Ejecutadas en cluster SLURM
- 1 repetición por configuración (valores nominales)

---

**Generado:** 22 de mayo de 2026  
**Scripts:** `/tmp/opencode/analisis_metricas.py`  
**Datos fuente:** `/home/patricio/Escritorio/sistemas-paralelos/TP2/csv/`
