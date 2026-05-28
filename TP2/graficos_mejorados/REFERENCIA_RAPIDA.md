# ⚡ REFERENCIA RÁPIDA - Gráficos Mejorados

## 📍 Ubicación
```
/home/patricio/Escritorio/sistemas-paralelos/TP2/graficos_mejorados/
```

## 📊 Los 9 Gráficos (en orden)

| # | Archivo | Qué Muestra | Busca Esto |
|---|---------|------------|-----------|
| 1 | `01_SPEEDUP.png` | Aceleración vs threads | Líneas que suben hacia T |
| 2 | `02_GFLOPS.png` | Rendimiento absoluto | Diferencia baseline vs paralelo |
| 3 | `03_EFFICIENCY.png` | Escalabilidad % | Dónde se degrada (baja de 100%) |
| 4 | `04_OVERHEAD_ABSOLUTO.png` | Costo real (seg) | OpenMP abajo = mejor |
| 5 | `05_OVERHEAD_RELATIVO.png` | Costo % | Negativo = super-lineal (bueno) |
| 6 | `06_COMPARATIVA_T8.png` | PThreads vs OpenMP | Barras: cuál está más arriba |
| 7 | `07_HEATMAP_SPEEDUP.png` | Matriz N×T | Rojo (arriba-derecha) = mejor |
| 8 | `08_HEATMAP_EFFICIENCY.png` | Matriz de eficiencia | Verde = ideal, rojo = peor |
| 9 | `09_TABLA_RESUMEN.png` | Resumen ganadores | Decisiones rápidas |

## 🎨 Colores (siempre iguales)
- 🔴 **PThreads** = Rojo vibrante
- 🟦 **OpenMP** = Turquesa
- ⬜ **Baseline** = Gris

## ⚡ Referencia Velocista

### Para responder "¿Es buena la escalabilidad?"
→ Mira `03_EFFICIENCY.png`: ¿está cerca de 100%? ✅ = Buena

### Para responder "¿Cuál es más rápido?"
→ Mira `06_COMPARATIVA_T8.png`: ¿Barras OpenMP más altas? ✅ = OpenMP mejor

### Para responder "¿Hay overhead?"
→ Mira `05_OVERHEAD_RELATIVO.png`: ¿Valores negativos? ✅ = Super-lineal (bueno)

### Para responder "¿Qué tamaño es mejor?"
→ Mira `07_HEATMAP_SPEEDUP.png`: ¿Dónde está más rojo?

## 📈 Patrones a Reconocer

### Línea que sube desde izquierda a derecha
✅ **BUENO**: Mayor threads = mejor rendimiento/speedup

### Línea horizontal en ~100% (eficiencia)
✅ **EXCELENTE**: Escalabilidad perfecta

### Línea con puntos rojos arriba de otras
✅ **GANADOR**: Ese algoritmo es mejor

### Valores negativos en overhead
🌟 **BONUS**: Super-linealidad, mejor que ideal

## 🎯 Recomendación

**USA OPENMP** (marginal pero consistentemente mejor)
- 23% menos overhead
- 0.4% más rápido
- Mejor eficiencia

**MEJOR CONFIGURACIÓN**: N=2048, T=8

## 📚 Documentación Completa
Ver: `LEEME.md` en el mismo directorio

---

**Fecha:** 22 mayo 2026 | **Resolución:** 300 DPI | **Estado:** Listo para usar
