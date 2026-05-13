# Análisis: Diferencia de Rendimiento entre Local y Cluster

## RESUMEN EJECUTIVO

Tu hipótesis es **acertada en dirección pero incompleta en la causa**. El problema NO es que double no entre en registros, sino que tiene comportamientos diferentes de cache miss por la **arquitectura SIMD/vectorización** del cluster.

---

## COMPARATIVA DE DATOS

### Local (tu máquina)
```
TIMES=10:   Double: 5.68s  | Float: 6.08s   → Double MEJOR (5% faster)
TIMES=100:  Double: 52.43s | Float: 47.38s  → Float MEJOR (10% faster)
TIMES=200:  Double: 81.43s | Float: 102.98s → Double MEJOR (20% faster)
TIMES=300:  Double: 128.31s| Float: 140.28s → Double MEJOR (9% faster)
```

### Cluster (remoto)
```
TIMES=10:   Double: 0.655s | Float: 0.598s  → Float MEJOR (9% faster)
TIMES=100:  Double: 6.646s | Float: 5.979s  → Float MEJOR (11% faster)
TIMES=200:  Double: 40.64s | Float: 20.47s  → Float MEJOR (2x más RÁPIDO!)
TIMES=300:  Double: 19.62s | Float: 17.93s  → Float MEJOR (9% faster)
```

---

## ANÁLISIS DE LAS CAUSAS RAÍZ

### 1. **Casting Implícito en Local (lo que viste)**

En la rama FLOAT (línea 47):
```c
float d = pow(fb[i],2) - 4.0*fa[i]*fc[i];
//         ^^^^^ pow(float, 2) sin conversión explícita
```

El compilador está haciendo:
- `pow(fb[i], 2)` → implícitamente `pow((double)fb[i], 2)` → resultado `double`
- Conversión back a `float` tiene overhead

**Local**: La penalización de casting se absorbe bien. Posiblemente:
- CPU con buen support para FPU x87 dual stack
- Cache L1/L2 suficientemente grande
- No hay vectorización agresiva

---

### 2. **Vectorización SIMD en Cluster (LA VERDADERA CAUSA)**

El cluster probablemente tiene CPU con **SSE/AVE/AVX** que vectoriza el código automáticamente.

**Por qué float es mejor con SIMD:**
- SSE puede procesar **4 floats simultáneamente** en 128 bits
- Con doubles solo procesa **2 doubles simultáneamente**
- El throughput de operaciones es **el doble con floats**

```
Cluster con vectorización automática:
- Float: 4 elementos/ciclo en SSE (128 bits)
- Double: 2 elementos/ciclo en SSE (128 bits)
→ Float tiene 2x mejor throughput
```

**Esto explica:**
- Times=200: Float **2x más rápido** (máximo beneficio SIMD)
- La ganancia se ve menos en Times=10 porque el overhead es dominante

---

### 3. **Local sin vectorización (o con limitaciones)**

Posiblemente tu máquina:
- No tiene AVX activado
- Compilador no vectoriza por bandera de compilación
- Usa FPU x87 (legacy) donde float/double son similares

---

## RECOMENDACIONES PARA VERIFICAR

### Comando 1: Ver flags de CPU del Cluster
```bash
# En el cluster (remoto):
ssh usuario@cluster "cat /proc/cpuinfo | grep flags | head -1"
```

**Qué buscar:**
- `sse2`, `sse4_1`, `sse4_2` → Vectorización con 2-4x elementos
- `avx`, `avx2`, `avx512` → Vectorización aún más agresiva

---

### Comando 2: Ver qué flags usa tu compilador LOCAL
```bash
# En tu máquina:
gcc --version
gcc -march=native -Q --help=target | grep -E "(sse|avx|fma)"
```

**Esperado:**
- Si ves muchos flags → Vectorización habilitada → Debería favorecer float
- Si ves pocos flags → Explicaría por qué local tiene double mejor

---

### Comando 3: Compilar el código CON y SIN vectorización

**Sin vectorización:**
```bash
gcc -O2 -fno-tree-vectorize quadatric2.c -o quadatric2_no_vec -lm
./quadatric2_no_vec
```

**Con vectorización (fuerza auto-vectorización):**
```bash
gcc -O3 -ftree-vectorize -march=native quadatric2.c -o quadatric2_vec -lm
./quadatric2_vec
```

**Resultado esperado:**
- Sin vectorización: double y float similar
- Con vectorización: float mucho más rápido que double

---

### Comando 4: Ver el código ensamblador generado
```bash
gcc -O3 -march=native -S quadatric2.c -o quadatric2.s
grep -A 20 "for (j=0; j<TIMES" quadatric2.s | grep -E "(movsd|movss|paddd|addpd|addps)"
```

**Esto te mostrará:**
- `addps` (add packed single) = Float vectorizado
- `addpd` (add packed double) = Double vectorizado
- `movss`/`movsd` = Operaciones scalar (sin vectorización)

---

### Comando 5: Perfilar con `perf` para cache misses
```bash
# En local:
perf stat -e cache-misses,cache-references ./quadatric2_vec

# En cluster (requiere acceso):
ssh usuario@cluster "perf stat -e cache-misses,cache-references ./quadatric2_vec"
```

**Resultado esperado:**
- **Cluster con float:** Menos cache misses (mejor localidad SIMD)
- **Cluster con double:** Más cache misses (menos elementos por línea de cache)

---

## CONCLUSIÓN Y RECOMENDACIÓN

✅ **Tu hipótesis es correcta en principio:** Diferencias arquitectónicas entre máquinas

❌ **Pero la causa NO es "double no entra en registros"**

✅ **La causa REAL es SIMD/Vectorización:**
- Float: 4x mejor throughput en SSE
- Double: 2x mejor throughput en SSE
- En operaciones CPU-bound masivas, esto es crítico

### Acción recomendada:
1. **Primero**: Ejecuta los Comandos 2 y 3 en ambas máquinas
2. **Luego**: Ejecuta Comando 5 para verificar cache behavior
3. **Finalmente**: Optimiza eligiendo float para cluster, pero DOCUMENTA que es por SIMD, no por cache

---

## SOLUCIÓN ÓPTIMA

Si quieres código que sea rápido en ambas arquitecturas:

```c
// Usa float explícitamente
#ifdef __SSE2__
    // SIMD path (cluster): usa float, mejor vectorización
    float d = powf(fb[i], 2.0f) - 4.0f*fa[i]*fc[i];
    float sd = sqrtf(d);
#else
    // Legacy path (máquinas sin SIMD)
    double d = pow((double)fb[i], 2.0) - 4.0*fa[i]*fc[i];
    double sd = sqrt(d);
#endif
```

**Pero lo ideal es:** Siempre usar `powf()` y `sqrtf()` explícitamente para float.
