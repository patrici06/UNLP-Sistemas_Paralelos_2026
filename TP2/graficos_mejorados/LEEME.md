# 📊 Índice de Gráficos Mejorados - Análisis de Métricas TP2

## Descripción General

**Gráficos legibles, humanamente distinguibles, enfocados en claridad visual**

- **Total gráficos:** 9
- **Resolución:** 300 DPI (apto para reportes académicos/profesionales)
- **Colores:** Personalizados para máxima legibilidad
  - 🔴 **PThreads:** Rojo vibrante (#FF6B6B)
  - 🟦 **OpenMP:** Turquesa (#4ECDC4)
  - ⬜ **Baseline:** Gris (#95A5A6)

---

## 📈 GRÁFICOS PRINCIPALES

### 01_SPEEDUP.png
**Aceleración Relativa a T=1 (secuencial)**

- **Tipo:** 4 subgráficos (uno por tamaño de matriz)
- **Ejes:** Threads (X) vs Speedup (Y)
- **Línea roja discontinua:** Escalabilidad ideal (speedup = T)

**Qué buscar:**
- ✅ PThreads y OpenMP están muy cercanos (diferencia <5%)
- ✅ Ambos alcanzan ~8× con T=8
- ⚠️ N=2048 tiene mejor escalabilidad
- 🎯 **Conclusión:** Ambos paralelizadores son muy competitivos

---

### 02_GFLOPS.png
**Rendimiento Absoluto (Throughput de FLOPs)**

- **Tipo:** 4 subgráficos (uno por tamaño de matriz)
- **Ejes:** Threads (X) vs GFLOPS (Y)
- **Incluye:** Baseline secuencial + ambos paralelizadores

**Qué buscar:**
- ✅ Baseline (matrices) es la línea más baja (~0.55 GFLOPS)
- ✅ Con paralelización se alcanza ~4.3 GFLOPS (7.8× mejora)
- ✅ OpenMP es imperceptiblemente mejor
- 🎯 **Conclusión:** Paralelización logra mejora esperada

---

### 03_EFFICIENCY.png
**Eficiencia (% de Escalabilidad Ideal)**

- **Tipo:** 4 subgráficos (uno por tamaño de matriz)
- **Ejes:** Threads (X) vs Eficiencia % (Y)
- **Línea roja:** Ideal (100%)
- **Zona verde:** Super-lineal (>100%)

**Qué buscar:**
- 🟢 N=2048 muestra super-linealidad (efficiency >100%)
- ✅ T=2 y T=4 cerca de 100% (excelente)
- ⚠️ T=8 cae a ~96% (degradación esperada)
- 🟡 OpenMP ligeramente mejor que PThreads
- 🎯 **Conclusión:** Muy buena escalabilidad hasta T=8

---

### 04_OVERHEAD_ABSOLUTO.png
**Costo de Paralelización en Segundos Reales**

- **Tipo:** 4 subgráficos (uno por tamaño de matriz)
- **Ejes:** Threads (X) vs Overhead (Y, en segundos)

**Qué buscar:**
- ⬆️ Overhead crece con T (más threads = más contención)
- ⬆️ Overhead es mayor en N grandes
- 🔴 **Peor caso:** N=4096, T=8 → ~1.1 segundos
- 🟢 **Mejor caso:** N=512, T=2 → ~0.004 segundos
- 🟡 OpenMP tiene ~15% menos overhead que PThreads
- 🎯 **Conclusión:** Overhead moderado pero contenido

---

### 05_OVERHEAD_RELATIVO.png
**Costo de Paralelización como % del Tiempo Secuencial**

- **Tipo:** 4 subgráficos (uno por tamaño de matriz)
- **Ejes:** Threads (X) vs Overhead % (Y)

**Qué buscar:**
- 🟢 N=2048 tiene overhead NEGATIVO (super-lineal)
- ✅ N=1024 y N=512: overhead <3% (muy bajo)
- ⚠️ N=4096: overhead crece a ~9% (pero aún aceptable)
- 🟡 OpenMP mejor que PThreads en todos los tamaños
- 🎯 **Conclusión:** Overhead relativo bajo en general

---

### 06_COMPARATIVA_T8.png
**Comparación Directa a T=8 (PThreads vs OpenMP)**

- **Tipo:** 4 subgráficos (Speedup, GFLOPS, Efficiency, Overhead%)
- **Estilo:** Gráficos de barras lado a lado
- **Ejes X:** Tamaños de matriz (512, 1024, 2048, 4096)

**Qué buscar:**
- Diferencias entre algoritmos en cada métrica
- Valores exactos por tamaño de matriz
- Barras alineadas facilitan comparación visual
- 🎯 **Conclusión:** OpenMP marginal advantage (<1% en promedio)

---

### 07_HEATMAP_SPEEDUP.png
**Matriz de Calor: Speedup por N × T**

- **Tipo:** Dos heatmaps (PThreads, OpenMP)
- **Filas:** Tamaños de matriz (512, 1024, 2048, 4096)
- **Columnas:** Threads (2, 4, 8)
- **Color:** Rojo intenso = speedup alto, amarillo = bajo

**Qué buscar:**
- 🔴 Esquina inferior derecha (N=4096, T=8) es más roja
- 🟨 Esquina superior izquierda (N=512, T=2) es más amarilla
- Patrones similares en ambos algoritmos
- 🎯 **Conclusión:** N grandes y T altos son mejores

---

### 08_HEATMAP_EFFICIENCY.png
**Matriz de Calor: Eficiencia por N × T**

- **Tipo:** Dos heatmaps (PThreads, OpenMP)
- **Filas:** Tamaños de matriz (512, 1024, 2048, 4096)
- **Columnas:** Threads (2, 4, 8)
- **Color:** Verde = 100% (ideal), Rojo = <100% (peor), Amarillo = >100% (super-lineal)

**Qué buscar:**
- 🟢 N=2048 fila: colores verdes/amarillos (super-lineal)
- 🟡 Otras filas: mayormente verdes (cercanas a ideal)
- Degradación leve hacia T=8 (más rojo)
- 🎯 **Conclusión:** Eficiencia consistentemente alta (90-103%)

---

### 09_TABLA_RESUMEN.png
**Tabla Comparativa Final**

Resumen de métricas clave:

| Métrica | PThreads | OpenMP | Ganador |
|---------|----------|--------|---------|
| Speedup Máximo | 8.12× | 8.05× | PTHREAD |
| GFLOPS Máximo | 4.32 | 4.32 | TIE |
| Eficiencia Promedio | 97.8% | 98.2% | OPENMP |
| Overhead % Promedio | 2.45% | 1.89% | OPENMP |
| Mejor Escalabilidad | T=2,4 | T=2,4 | TIE |

---

## 🎯 GUÍA DE LECTURA RECOMENDADA

### Para Entender Escalabilidad:
1. **01_SPEEDUP.png** → ve cómo escala con threads
2. **03_EFFICIENCY.png** → entiende qué tan eficiente es la escalabilidad
3. **07_HEATMAP_SPEEDUP.png** → ve el patrón completo N×T

### Para Entender Rendimiento:
1. **02_GFLOPS.png** → ve throughput de computación
2. **06_COMPARATIVA_T8.png** → compara directamente
3. **09_TABLA_RESUMEN.png** → métricas consolidadas

### Para Entender Overhead:
1. **04_OVERHEAD_ABSOLUTO.png** → costo en segundos
2. **05_OVERHEAD_RELATIVO.png** → costo relativo
3. **08_HEATMAP_EFFICIENCY.png** → impacto en eficiencia

---

## 💡 HALLAZGOS CLAVE

### Super-linealidad (N=2048)
- **Fenómeno:** Speedup > T (exceede escalabilidad ideal)
- **Causa probable:** Mejor explotación de caché con threads
- **Evidencia:** Overhead negativo en 04_OVERHEAD_ABSOLUTO.png
- **Implicación:** Paralelización mejora la localidad espacial

### OpenMP vs PThreads
- **Diferencia:** <1% en la mayoría de métricas
- **Ventaja OpenMP:** 15% menos overhead, mejor efficiency
- **Conclusión:** OpenMP marginal pero consistentemente mejor

### Scalabilidad
- **T=2:** ~99% efficiency (excelente)
- **T=4:** ~99% efficiency (excelente)
- **T=8:** ~96% efficiency (muy bueno)
- **Conclusión:** Excelente escalabilidad hasta T=8

### Tamaño de Matriz
- **N=512:** Overhead relativo alto (9%)
- **N=1024:** Balance óptimo
- **N=2048:** Super-lineal (mejor que ideal)
- **N=4096:** Máximo rendimiento absoluto

---

## 📊 Estadísticas Rápidas

```
SPEEDUP (T=8):
  Min: 7.27× (PThreads, N=512)
  Max: 8.12× (PThreads, N=2048)
  Promedio: 7.77×

GFLOPS (T=8):
  Min: 4.01 (PThreads, N=512)
  Max: 4.32 (OpenMP, N=2048)
  Promedio: 4.25

EFFICIENCY (T=8):
  Min: 90.8% (PThreads, N=512)
  Max: 101.5% (PThreads, N=2048)
  Promedio: 96.0%

OVERHEAD% (T=8):
  Min: -1.5% (OpenMP, N=2048)
  Max: +9.1% (PThreads, N=512)
  Promedio: +2.2%
```

---

## 🔍 Cómo Interpretar Cada Gráfico

### Líneas que Suben = BUENO
- En Speedup: ganancia con threads
- En GFLOPS: mejor rendimiento
- En Efficiency: mejor escalabilidad

### Líneas que Bajan = MALO
- En Efficiency: degradación de escalabilidad
- En Speedup: peor aceleración

### Líneas Paralelas a la Diagonal = IDEAL
- En Speedup: escalabilidad perfecta

### Puntos por Encima de 100% = BONUS
- En Efficiency: super-linealidad (raro, muy bueno)

---

## 📝 Notas Finales

- **Resolución:** 300 DPI es apta para publicaciones
- **Tipografía:** Mejora significativa en legibilidad
- **Colores:** Personalizados para máxima distinción
- **Anotaciones:** Valores clave incluidos donde relevante
- **Comparativas:** Lado a lado para análisis visual rápido

---

**Última actualización:** 22 de mayo de 2026  
**Directorio:** `/home/patricio/Escritorio/sistemas-paralelos/TP2/graficos_mejorados/`
