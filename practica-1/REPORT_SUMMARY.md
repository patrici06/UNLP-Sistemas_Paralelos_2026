# Reporte de Análisis de Benchmarks de Multiplicación de Matrices

## Resumen Ejecutivo

Se ha analizado completa los benchmarks de multiplicación de matrices compilados con diferentes opciones de optimización en arquitecturas de 32 bits (BS32) y 64 bits (BS64).

### Datos Generales
- **Total de benchmarks analizados:** 32
- **Tamaños de entrada:** 512, 1024, 2048, 4096
- **Métodos de compilación:** 8 (4 en BS32, 4 en BS64)
- **Período de análisis:** Abril 2025

---

## Resultados Principales

### BS32 (32 bits)
| Compilación | Tiempo Promedio | GFLOPS Promedio | Observaciones |
|------------|-----------------|-----------------|---------------|
| **03** (baseline) | 82.04s | 0.4746 | Baseline para comparación |
| **03-fopenmp** | 81.90s | 0.4771 | -0.2% mejora mínima |
| **03-funroll** | 64.17s | 0.6096 | **+22% más rápido** ⭐ |
| **03-march-native** | 82.00s | 0.4768 | -0.3% impacto insignificante |

### BS64 (64 bits)
| Compilación | Tiempo Promedio | GFLOPS Promedio | Observaciones |
|------------|-----------------|-----------------|---------------|
| **03** (baseline) | 79.38s | 0.4944 | Baseline para comparación |
| **03-fopemp** | 79.60s | 0.5065 | +1.2% mejora marginal |
| **03-fullroll** | 61.44s | 0.6393 | **+23% más rápido** ⭐ |
| **03-march-native** | 78.70s | 0.5054 | +2.2% mejora moderada |

---

## Análisis Detallado

### Optimizaciones Más Efectivas

#### 1. **Loop Unrolling (funroll/fullroll)** - EXCELENTE ✅
- **BS32 (funroll):** Reduce tiempo en ~22% (82.04s → 64.17s)
- **BS64 (fullroll):** Reduce tiempo en ~23% (79.38s → 61.44s)
- **GFLOPS mejorado:** Aumenta ~28-29%
- **Conclusión:** Esta es la optimización más impactante. El despliegue de bucles reduce instrucciones de salto y mejora el paralelismo a nivel de instrucción.

#### 2. **OpenMP (-fopenmp/-fopemp)** - POBRE ❌
- **BS32:** -0.2% (sin mejora)
- **BS64:** +1.2% (mejora insignificante)
- **Conclusión:** En esta versión compilada aparentemente sin paralelismo multihilo en tiempo de ejecución, el overhead de OpenMP no justifica la compilación.

#### 3. **Native Architecture (-march-native)** - DÉBIL ⚠️
- **BS32:** -0.3% (sin mejora)
- **BS64:** +2.2% (mejora marginal)
- **Conclusión:** El impacto es mínimo en ambas arquitecturas. La ganancia en BS64 sugiere que algunas instrucciones específicas del procesador ayudan ligeramente.

---

## Análisis de Escalabilidad

### Escalado con Tamaño de Entrada

Ambas arquitecturas muestran un patrón consistente:
- **n=512:** ~0.5s (pequeño dataset, overhead significativo)
- **n=1024:** ~4.5s (tiempo cuadrático aparente)
- **n=2048:** ~34-36s (crecimiento consistente)
- **n=4096:** ~280-290s (mantenimiento de patrón)

**Observación:** El escalado es más cercano a O(n³) de lo esperado en multiplicación de matrices.

---

## Recomendaciones

1. **Usar BS64 + fullroll** para máximo rendimiento
   - Logra 0.6393 GFLOPS (mejor que BS32 + funroll: 0.6096)
   - Reducción de tiempo de ~23% vs baseline

2. **Evitar OpenMP** en esta configuración
   - No proporciona beneficio sin paralelismo multihilo activo
   - Añade overhead de compilación innecesario

3. **Considerar march-native en BS64**
   - Proporciona mejora marginal (+2.2%)
   - Poco costo de compilación adicional

4. **Investigar paralelización multihilo**
   - Las versiones OpenMP actuales no generan mejora
   - Podrían haber errores de compilación o bandera faltante
   - La paralelización correcta podría proporcionar speedups significativos

---

## Archivos Generados

### Reportes de Datos
- `benchmark_report.csv` - Datos detallados de todos los benchmarks
- `benchmark_summary.csv` - Estadísticas agregadas (media, std, min, max)
- `benchmark_report.html` - Reporte HTML interactivo

### Visualizaciones
- `graph_1_time_comparison.png` - Tiempo de ejecución vs tamaño de entrada
- `graph_2_gflops_comparison.png` - GFLOPS alcanzados vs tamaño de entrada
- `graph_3_boxplot_times.png` - Distribución de tiempos por método
- `graph_4_heatmap_times.png` - Mapa de calor de tiempos promedio
- `graph_5_speedup_comparison.png` - Speedup relativo al baseline

---

## Conclusión

Las optimizaciones de **loop unrolling** (funroll/fullroll) son significativamente más efectivas que `-march-native` u `-fopenmp` en esta implementación. Se logra una **mejora del 22-23%** en tiempo de ejecución usando estas optimizaciones, pasando de ~80s a ~64s para la configuración de 4096x4096.

Para casos de uso donde se requiera máximo rendimiento, **se recomienda BS64 con fullroll**, logrando un pico de 0.6393 GFLOPS.

---

*Reporte generado: 10 de Abril de 2025*
*Directorio de análisis: /home/patricio/Escritorio/sistemas-paralelos/practica-1/cluster*
