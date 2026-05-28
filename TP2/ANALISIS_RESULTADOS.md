# 📊 Análisis Comparativo de Resultados

## Estructura del CSV

**Archivo:** `resultados_comparativo.csv`

```
N,T,Algorithm,workTime,GFLOPS,speedup,efficiency,overhead,overhead%
512,1,matrices,0.487664,0.550452,N/A,N/A,N/A,N/A
512,1,pthread,0.485223,0.553221,1.000000,100.000000,0.000000,0.000000
512,1,openmp,0.484865,0.553629,1.000000,100.000000,0.000000,0.000000
512,2,pthread,0.247083,1.086418,1.969355,98.467742,0.003786,1.532258
512,2,openmp,0.243589,1.102002,1.995678,99.783888,0.000526,0.216112
...
```

### Columnas:
- **N**: Tamaño de matriz (512, 1024, 2048, 4096)
- **T**: Número de threads (1, 2, 4, 8)
- **Algorithm**: pthread, openmp, matrices
- **workTime**: Tiempo de ejecución en segundos
- **GFLOPS**: Operaciones en punto flotante por segundo (miles de millones)
- **speedup**: Aceleración vs T=1 (solo para pthread y openmp)
- **efficiency**: Eficiencia = (speedup/T)*100 %
- **overhead**: Tiempo de overhead en segundos
- **overhead%**: Porcentaje de overhead

---

## Cómo Comparar

### 1️⃣ Por Tamaño de Matriz (N)

**Para N=512:**
```
512,1,matrices,0.487664,0.550452,N/A,N/A,N/A,N/A
512,1,pthread,0.485223,0.553221,1.000000,100.000000,0.000000,0.000000
512,1,openmp,0.484865,0.553629,1.000000,100.000000,0.000000,0.000000
512,2,pthread,0.247083,1.086418,1.969355,98.467742,0.003786,1.532258
512,2,openmp,0.243589,1.102002,1.995678,99.783888,0.000526,0.216112
512,4,pthread,0.125100,2.145765,3.889636,97.240902,0.003452,2.759098
512,4,openmp,0.124302,2.159540,3.910833,97.770836,0.002771,2.229164
512,8,pthread,0.066920,4.011286,7.271275,90.890932,0.006096,9.109068
512,8,openmp,0.064816,4.141500,7.500077,93.750967,0.004050,6.249033
```

✅ **Conclusiones:**
- **OpenMP más rápido** en N=512 (0.243s vs 0.247s en T=2)
- **OpenMP mejor speedup** (7.50x vs 7.27x en T=8)
- OpenMP overhead menor en N pequeño

---

### 2️⃣ Por Número de Threads (T)

**Para N=1024, T=2:**
```
1024,2,pthread,1.953608,1.099240,1.996699,99.834970,0.003224,0.165030
1024,2,openmp,1.965096,1.092814,1.982445,99.122257,0.017248,0.877743
```

✅ **Conclusiones:**
- **Speedup casi lineal** (1.99x con 2 threads)
- **PThread ligeramente mejor** (1.953s vs 1.965s)
- Overhead muy bajo (<2%)

**Para N=1024, T=8:**
```
1024,8,pthread,0.500572,4.290060,7.792622,97.407772,0.012976,2.592228
1024,8,openmp,0.498380,4.308929,7.816717,97.708963,0.011418,2.291037
```

✅ **Conclusiones:**
- **OpenMP mejor con T=8** (7.816x vs 7.792x)
- Eficiencia sigue siendo excelente (~97%)
- Overhead crece con T (normal)

---

### 3️⃣ Escalabilidad por Algoritmo

#### **PThread - Escalabilidad:**

| N    | T=1 | T=2  | T=4  | T=8  |
|------|-----|------|------|------|
| 512  | 1.0 | 1.97 | 3.89 | 7.27 |
| 1024 | 1.0 | 1.99 | 3.96 | 7.79 |
| 2048 | 1.0 | 2.05 | 4.10 | 8.12 |
| 4096 | 1.0 | 1.99 | 3.97 | 7.73 |

✅ Muy consistente. Speedup ~2.0 para T=2, ~4.0 para T=4, ~7.7-8.1 para T=8

#### **OpenMP - Escalabilidad:**

| N    | T=1 | T=2  | T=4  | T=8  |
|------|-----|------|------|------|
| 512  | 1.0 | 2.00 | 3.91 | 7.50 |
| 1024 | 1.0 | 1.98 | 3.95 | 7.82 |
| 2048 | 1.0 | 2.05 | 4.07 | 8.05 |
| 4096 | 1.0 | 2.00 | 3.98 | 7.76 |

✅ Muy similar a PThread. Ligeramente mejor en N grande (4096)

---

## Comandos Útiles para Análisis

### Ver solo speedup de pthread:
```bash
grep "^1024" resultados_comparativo.csv | grep "pthread" | cut -d, -f1,2,3,6
```

### Ver solo datos de T=8:
```bash
grep ",8," resultados_comparativo.csv
```

### Filtrar por algoritmo:
```bash
grep "pthread" resultados_comparativo.csv  # Solo pthread
grep "openmp" resultados_comparativo.csv   # Solo openmp
grep "matrices" resultados_comparativo.csv # Solo baseline
```

### Crear CSV solo de speedup:
```bash
(echo "N,T,pthread_speedup,openmp_speedup"; \
grep ",pthread," resultados_comparativo.csv | cut -d, -f1,2,6 | paste -d, - <(grep ",openmp," resultados_comparativo.csv | cut -d, -f6)) \
> speedup_comparison.csv
```

### Estadísticas por algoritmo:
```bash
echo "=== PThread ===" && grep "pthread" resultados_comparativo.csv | cut -d, -f6 | tail -n +1 | awk '{print $1}' | sort -n | tail -5 | head -1
echo "=== OpenMP ===" && grep "openmp" resultados_comparativo.csv | cut -d, -f6 | tail -n +1 | awk '{print $1}' | sort -n | tail -5 | head -1
```

---

## Visualización en Gráficos (gnuplot/Python)

### Speedup vs Threads (Python con matplotlib):
```python
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv('resultados_comparativo.csv')

# Filtrar por N
for n in [512, 1024, 2048, 4096]:
    subset = df[df['N'] == n]
    
    pthread = subset[subset['Algorithm'] == 'pthread'].sort_values('T')
    openmp = subset[subset['Algorithm'] == 'openmp'].sort_values('T')
    
    plt.figure(figsize=(8, 6))
    plt.plot(pthread['T'], pthread['speedup'], 'o-', label='PThread', linewidth=2)
    plt.plot(openmp['T'], openmp['speedup'], 's-', label='OpenMP', linewidth=2)
    plt.plot([1, 2, 4, 8], [1, 2, 4, 8], 'k--', label='Ideal', linewidth=1)
    
    plt.xlabel('Threads')
    plt.ylabel('Speedup')
    plt.title(f'Speedup vs Threads (N={n})')
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.savefig(f'speedup_n{n}.png', dpi=150)
```

### GFLOPS vs Threads (similar):
```python
# Cambiar 'speedup' por 'GFLOPS' en el código anterior
```

---

## Hallazgos Principales

### ✅ **Lo Bueno:**
1. **Escalabilidad excelente**: Ambos algoritmos escalan casi linealmente hasta T=4
2. **Overhead mínimo**: Apenas 1-3% para T=2, menos de 10% incluso en T=8
3. **OpenMP y PThread similares**: Diferencias <5% en la mayoría de casos
4. **GFLOPS creciente**: Aumenta con threads (esperado)

### ⚠️ **Lo Interesante:**
1. **Overhead negativo en N=2048**: Efficiency > 100% indica super-linear speedup
   - Posible: Better cache locality, fewer cache misses at larger N
2. **T=8 menos eficiente**: Overhead% sube a 3-9% (normal, comunicación entre threads)
3. **N=512**: Smallest matrix, highest relative overhead

### 🔍 **Comparación Final:**

| Métrica | Winner | Diferencia |
|---------|--------|-----------|
| Speedup T=8 | OpenMP (7.82 vs 7.79) | +0.4% |
| GFLOPS T=8 | OpenMP (4.31 vs 4.29) | +0.5% |
| Overhead T=8 | OpenMP | -15% (menor) |
| **Recomendación** | **OpenMP** | Ligeramente mejor |

---

## Exportar a Otros Formatos

### JSON:
```bash
python3 << 'EOF'
import csv, json
with open('resultados_comparativo.csv') as f:
    data = json.dumps(list(csv.DictReader(f)), indent=2)
with open('resultados_comparativo.json', 'w') as f:
    f.write(data)
EOF
```

### Excel/LibreOffice Calc:
```bash
# Directamente: Abrir resultados_comparativo.csv en Calc
libreoffice --calc resultados_comparativo.csv
```

---

## Conclusión

📌 **El CSV está listo para:**
- ✅ Análisis estadístico
- ✅ Gráficos comparativos
- ✅ Exportación a herramientas de visualización
- ✅ Escritura de informe final
