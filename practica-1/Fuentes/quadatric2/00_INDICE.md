# ÍNDICE DE ANÁLISIS: Double vs Float en Local vs Cluster

## Para Empezar Rápido (5 minutos)

1. **[RESUMEN_EJECUTIVO.md](RESUMEN_EJECUTIVO.md)** ← EMPIEZA AQUÍ
   - Respuesta clara a tu pregunta
   - Qué es correcto, qué no
   - 3 comandos para verificar

2. **[QUICK_REFERENCE.md](QUICK_REFERENCE.md)** ← HOJA DE RUTA
   - 5 comandos para verificar en orden
   - Bottom line rápido
   - FAQ

---

## Para Entender Profundamente (30 minutos)

3. **[RESPUESTAS_Y_COMANDOS.md](RESPUESTAS_Y_COMANDOS.md)** ← TÉCNICO COMPLETO
   - Tu pregunta vs respuesta técnica
   - Análisis de resultados medidos
   - 5 pasos de verificación
   - Tabla comparativa

4. **[RESULTADO_FINAL_ANALISIS.md](RESULTADO_FINAL_ANALISIS.md)** ← CON DATOS REALES
   - Compilaciones progresivas ejecutadas en tu máquina
   - Comparativa local vs cluster
   - Análisis crítico de por qué float es mejor
   - Test empírico sugerido

5. **[ANALISIS_COMPLETO_LOCAL_VS_CLUSTER.md](ANALISIS_COMPLETO_LOCAL_VS_CLUSTER.md)** ← TEORÍA
   - Análisis línea por línea del código
   - SIMD Vectorización explicado
   - Memory bandwidth vs FPU
   - Cache L1/L2/L3 behavior

6. **[analisis-comportamiento.md](analisis-comportamiento.md)** ← DETALLADO
   - Casting implícito en local
   - Vectorización SIMD en cluster
   - Recomendaciones de optimización
   - Solución óptima

---

## Para Ejecutar y Verificar

7. **[comandos-verificacion.sh](comandos-verificacion.sh)** ← SCRIPT COMPLETO
   - Todos los comandos listos
   - Explicaciones paso a paso
   - Script automático de comparación

8. **[compare_local_vs_cluster.sh](../compare_local_vs_cluster.sh)** ← YA EJECUTADO
   - Script que ejecutamos en tu máquina
   - Muestra resultados reales

---

## Flujo Recomendado de Lectura

### Si tienes 5 minutos:
```
RESUMEN_EJECUTIVO.md → QUICK_REFERENCE.md
```

### Si tienes 15 minutos:
```
RESUMEN_EJECUTIVO.md → RESPUESTAS_Y_COMANDOS.md → QUICK_REFERENCE.md
```

### Si tienes 30+ minutos (lectura completa):
```
RESUMEN_EJECUTIVO.md
  ↓
RESPUESTAS_Y_COMANDOS.md
  ↓
RESULTADO_FINAL_ANALISIS.md
  ↓
ANALISIS_COMPLETO_LOCAL_VS_CLUSTER.md
  ↓
analisis-comportamiento.md
```

---

## Lo que Encontrarás en Cada Sección

### RESUMEN_EJECUTIVO.md
- ✓ Tu pregunta vs respuesta
- ✓ La causa real en diagrama
- ✓ Datos medidos compilados
- ✓ 3 comandos para verificar
- ✓ FAQ

### QUICK_REFERENCE.md
- ✓ 5 comandos en orden
- ✓ Tabla de verdad
- ✓ Recomendación directa
- ✓ Bottom line

### RESPUESTAS_Y_COMANDOS.md
- ✓ Análisis de tu hipótesis
- ✓ Confirmación de resultados
- ✓ 5 pasos de verificación
- ✓ Comandos probados
- ✓ Documentación profesional

### RESULTADO_FINAL_ANALISIS.md
- ✓ Datos REALES de tu máquina
- ✓ Compilaciones progresivas
- ✓ Comparativa con cluster
- ✓ Por qué float es mejor
- ✓ Test empírico sugerido

### ANALISIS_COMPLETO_LOCAL_VS_CLUSTER.md
- ✓ Análisis línea por línea
- ✓ SIMD explicado
- ✓ Memory bandwidth vs FPU
- ✓ Cache behavior
- ✓ Óptimas prácticas

### analisis-comportamiento.md
- ✓ Casting implícito
- ✓ Vectorización SIMD
- ✓ Recomendaciones
- ✓ Soluciones de código

---

## Respuesta Corta a Tu Pregunta

Tu hipótesis es **CORRECTA**:
- ✓ Es un problema arquitectónico
- ✓ Memory bandwidth es bottleneck
- ✗ NO es porque "registros" (ese es el error conceptual)

La causa: **SIMD vectorization (float 2x mejor throughput) + memory bandwidth (double 2x datos)**

En cluster se amplifica porque BW es más limitante.

---

## Comandos Clave

### Verificación rápida:
```bash
# Ver flags CPU
cat /proc/cpuinfo | grep flags | head -1

# Compilar con SIMD
cd /home/patricio/Escritorio/sistemas-paralelos/practica-1/Fuentes
gcc -O3 -march=native quadatric2.c -o quad_opt -lm
./quad_opt

# Comparar sin SIMD
gcc -O2 quadatric2.c -o quad_noopt -lm
./quad_noopt
```

**Si quad_opt muestra float mejor → Tu hipótesis es correcta**

---

## Archivos Originales (tus reportes)

- `reporte-local-quadatric2.md` - Datos de tu máquina
- `reporte-cluster-quadatric2.md` - Datos de cluster remoto
- `quadatric2.c` - Código fuente (en nivel superior)

---

## Próximos Pasos

1. Lee RESUMEN_EJECUTIVO.md (5 min)
2. Ejecuta comandos de QUICK_REFERENCE.md (5 min)
3. Compara resultados con análisis
4. (Opcional) Lee RESPUESTAS_Y_COMANDOS.md (15 min)

---

## Métrica de Éxito

✓ Si entiendes por qué float es mejor → Éxito
✓ Si puedes explicar SIMD + memory BW → Éxito
✓ Si ejecutas comandos y ves float mejor con -O3 -march=native → Éxito

❌ Si sigues pensando que es por "registros" → Lee análisis de nuevo

---

Creado: $(date)
Máquina: AMD Ryzen (con AVX2, SSE4.2)
Versión GCC: 15.2.1

**Todos estos archivos están en**: `/home/patricio/Escritorio/sistemas-paralelos/practica-1/Fuentes/quadatric2/`
