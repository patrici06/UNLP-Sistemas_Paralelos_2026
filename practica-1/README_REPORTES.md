# Sistema de Generación de Reportes de Benchmarks

Este directorio contiene scripts para generar reportes y visualizaciones de los benchmarks de multiplicación de matrices.

## 📋 Descripción General

Los benchmarks provienen de dos directorios principales:
- **bs32/** - Compilaciones de 32 bits
- **bs64/** - Compilaciones de 64 bits

Cada directorio contiene subdirectorios `resultados-*` con los resultados de diferentes configuraciones de compilación:
- `03` - Compilación base (-O3)
- `03-fopenmp` / `03-fopemp` - Con soporte OpenMP
- `03-funroll` / `03-fullroll` - Con optimización de loop unrolling
- `03-march-native` - Con optimización para arquitectura nativa

## 🚀 Uso Rápido

### Generar todos los reportes:

```bash
cd /home/patricio/Escritorio/sistemas-paralelos/practica-1
python3 run_all_reports.py
```

Este comando genera automáticamente:
- Reportes CSV detallados
- Gráficos comparativos en PNG
- Reporte HTML interactivo

## 📁 Archivos de Salida

Todos los reportes se generan en:
```
/home/patricio/Escritorio/sistemas-paralelos/practica-1/cluster/
```

### Reportes de Datos (CSV)

| Archivo | Descripción |
|---------|-------------|
| `benchmark_report.csv` | Datos completos de todos los benchmarks (32 registros) |
| `benchmark_summary.csv` | Estadísticas agregadas (media, desv.est, min, max) por compilación y tamaño |

**Estructura del CSV principal:**
```csv
Bitness,Compilation,Entry_Size,Time_Output,GFLOPS
bs32,03,512,0.572261,0.469079
bs32,03,1024,4.523536,0.474736
...
```

### Visualizaciones (PNG)

| Archivo | Descripción |
|---------|-------------|
| `graph_1_time_comparison.png` | Comparativa de tiempos de ejecución por tamaño de entrada |
| `graph_2_gflops_comparison.png` | Comparativa de GFLOPS alcanzados |
| `graph_3_boxplot_times.png` | Distribución de tiempos por método de compilación |
| `graph_4_heatmap_times.png` | Mapa de calor de tiempos promedio |
| `graph_5_speedup_comparison.png` | Speedup relativo al baseline |

### Otros Reportes

| Archivo | Descripción |
|---------|-------------|
| `benchmark_report.html` | Reporte HTML interactivo con estadísticas visuales |
| `REPORT_SUMMARY.md` | Análisis detallado en formato Markdown |

## 📊 Análisis de Resultados

### Resumen Ejecutivo de Resultados:

#### BS32 (32 bits)
- **Mejor compilación:** `03-funroll` (64.17s promedio)
- **Mejora vs baseline:** +22% (vs 82.04s de -O3)
- **GFLOPS máximo:** 0.6096

#### BS64 (64 bits)
- **Mejor compilación:** `03-fullroll` (61.44s promedio)
- **Mejora vs baseline:** +23% (vs 79.38s de -O3)
- **GFLOPS máximo:** 0.6393 ⭐ **Mejor rendimiento general**

### Hallazgos Clave

1. **Loop Unrolling es lo más efectivo**
   - Reduce tiempos ~22-23%
   - Aumenta GFLOPS ~28-29%

2. **OpenMP tiene impacto mínimo**
   - Mejora <2% en estos benchmarks
   - Probablemente sin paralelismo multihilo activo

3. **Native Architecture proporciona mejora marginal**
   - -0.3% en BS32
   - +2.2% en BS64

4. **Escalado consistente**
   - Comportamiento O(n³) esperado
   - Proporcional al tamaño de entrada

## 🛠️ Scripts Disponibles

### `run_all_reports.py` (Recomendado)
Script maestro que ejecuta todos los generadores en orden.

**Uso:**
```bash
python3 run_all_reports.py
```

**Ventajas:**
- Ejecuta todos los scripts automáticamente
- Muestra progreso consolidado
- Maneja errores elegantemente

### `generate_reports.py`
Genera reportes CSV y gráficos PNG usando matplotlib y pandas.

**Uso directo:**
```bash
python3 generate_reports.py
```

**Genera:**
- benchmark_report.csv
- benchmark_summary.csv
- 5 gráficos PNG

**Requisitos:**
- pandas
- matplotlib
- seaborn
- numpy

### `generate_html_report.py`
Genera un reporte HTML interactivo con resumen ejecutivo.

**Uso directo:**
```bash
python3 generate_html_report.py
```

**Genera:**
- benchmark_report.html

**Requisitos:**
- pandas

## 📈 Interpretación de Gráficos

### Graph 1 & 2: Gráficos de Línea
- **Eje X:** Tamaño de entrada (escala logarítmica)
- **Eje Y:** Tiempo de ejecución o GFLOPS
- **Líneas:** Una por cada método de compilación
- **Interpretación:** Mostrar escalado y comparación directa

### Graph 3: Box Plot
- **Caja:** 25-75 percentil
- **Línea media:** Mediana
- **Puntos:** Outliers
- **Interpretación:** Variabilidad y distribución por compilación

### Graph 4: Heatmap
- **Color:** Intensidad del tiempo (rojo = más lento)
- **Filas:** Método de compilación
- **Columnas:** Tamaño de entrada
- **Interpretación:** Identificar patrones de rendimiento

### Graph 5: Speedup
- **Eje Y:** Ratio vs baseline (1.0 = igual)
- **>1.0:** Más rápido que baseline
- **<1.0:** Más lento que baseline
- **Interpretación:** Ganancia relativa de cada optimización

## 🔍 Análisis Manual

Si deseas analizar los datos manualmente:

```python
import pandas as pd

# Cargar datos
df = pd.read_csv('cluster/benchmark_report.csv')

# Filtrar por compilación
bs64_fullroll = df[(df['Bitness'] == 'bs64') & (df['Compilation'] == '03-fullroll')]

# Estadísticas
print(df.groupby(['Bitness', 'Compilation'])['Time_Output'].mean())
print(df.groupby(['Bitness', 'Compilation'])['GFLOPS'].mean())
```

## 💡 Recomendaciones de Uso

1. **Para análisis rápido:** Abrir `REPORT_SUMMARY.md`
2. **Para visualización interactiva:** Abrir `benchmark_report.html` en navegador
3. **Para análisis detallado:** Usar los CSVs con Excel, pandas o herramienta de análisis
4. **Para presentaciones:** Usar los gráficos PNG

## ⚙️ Personalización

Para modificar los gráficos, edita `generate_reports.py`:
- Cambia colores en la sección de estilos
- Ajusta tamaños de figura
- Añade o elimina tipos de gráficos
- Modifica métricas de agregación

## 📋 Requerimientos de Sistema

```
Python 3.7+
pandas >= 1.0
matplotlib >= 3.0
seaborn >= 0.10
numpy >= 1.18
```

## 🐛 Solución de Problemas

### Error: "No module named 'pandas'"
```bash
pip3 install pandas matplotlib seaborn
```

### Error: "No data found"
- Verificar que existen directorios `bs32/` y `bs64/`
- Verificar que existen subdirectorios `resultados-*`
- Verificar que existen archivos `output_*.txt` en los subdirectorios

### Gráficos vacíos
- Los datos podrían no estar siendo parseados correctamente
- Verificar formato de archivos .txt (deben iniciar con RESULT;)

## 📝 Licencia

Scripts de análisis generados para práctica educativa.

---

*Última actualización: Abril 10, 2025*
*Directorio base: /home/patricio/Escritorio/sistemas-paralelos/practica-1/*
