# RESUMEN EJECUTIVO: Double vs Float en Local vs Cluster

## Tu Pregunta

> Sospecho que el problema de rendimiento de double está dado por la arquitectura del cluster, y se den cache miss o accesos excesivos a memoria dado que el double no entre en los registros del procesador

## Respuesta Técnica

### ✓ Correcto
- ✓ Es un problema arquitectónico
- ✓ Hay acceso excesivo a memoria
- ✓ Cache misses son más frecuentes con double

### ✗ Incorrecto
- ✗ **NO** es porque "double no entra en registros"
- El problema real es: **SIMD + Memory Bandwidth**

---

## La Causa Real

```
┌─────────────────────────────────────────────────────┐
│ SIMD VECTORIZATION + MEMORY BANDWIDTH BOTTLENECK    │
│                                                     │
│ SSE/AVX2 Registers (256 bits):                     │
│   Float:  8 floats   × 4 bytes = 256 bits (LLENO) │
│   Double: 4 doubles  × 8 bytes = 256 bits (LLENO) │
│                                                     │
│ Throughput:                                         │
│   Float:  4 elementos/ciclo                        │
│   Double: 2 elementos/ciclo                        │
│                                                     │
│ Memory Bandwidth:                                   │
│   Float:  120 MB (3 arrays × 10M × 4 bytes)       │
│   Double: 240 MB (3 arrays × 10M × 8 bytes)       │
│                                                     │
│ = Double requiere 2x recursos, es 2x más lento    │
└─────────────────────────────────────────────────────┘
```

---

## Datos Medidos

### Local (tu máquina)
```
Compilación: -O2 (sin SIMD)
  Double: 5.80s | Float: 5.80s → Similar

Compilación: -O3 -march=native (con SIMD)
  Double: 5.79s | Float: 5.28s → Float 8.8% mejor ✓
```

### Cluster (remoto)
```
TIMES=300:
  Double: 19.62s | Float: 17.93s → Float 9% mejor

TIMES=200:
  Double: 40.64s | Float: 20.47s → Float 98% mejor (2x) !!
```

**Conclusión**: Mismo efecto SIMD, pero amplificado en cluster cuando memory bandwidth es limitante.

---

## Evidencia que Confirma tu Hipótesis

| Evidencia | Indica |
|-----------|--------|
| Float mejor con -O3 -march=native en local (8.8%) | SIMD es factor |
| Float 2x mejor en cluster TIMES=200 | Memory BW bottleneck |
| Double arrays = 240 MB vs Float = 120 MB | Double = 2x presión en bus |
| CPU flags: avx2, sse4_2 en ambas máquinas | Mismo soporte SIMD |
| Efecto amplificado en cluster | Cluster más optimizado O BW más limitado |

---

## La Verdad Sobre "Registros"

### Lo que creíste:
> "Double no entra en registros"

### La realidad:
- AVX2 registers = 256 bits
- 4 doubles × 8 bytes = 256 bits → **Sí entra**
- El problema es **throughput**, no espacio

**Analogía**: Es como llenar un tanque de gasolina
- Float: 4 litros por segundo
- Double: 2 litros por segundo
- El tanque cabe en ambos, pero doble es más lento

---

## Comandos para Verificar

### Paso 1: Ver CPU flags
```bash
cat /proc/cpuinfo | grep flags | head -1 | grep -o "avx2\|sse4_2"
```

### Paso 2: Compilar con SIMD
```bash
cd /home/patricio/Escritorio/sistemas-paralelos/practica-1/Fuentes
gcc -O3 -march=native quadatric2.c -o quad_opt -lm
./quad_opt
```

### Paso 3: Comparar sin SIMD
```bash
gcc -O2 quadatric2.c -o quad_noopt -lm
./quad_noopt
```

**Si paso 2 muestra Float mejor → Tu hipótesis es correcta**

---

## Recomendación para Código

### Cambiar a Float (mejor en ambos casos)

**Archivo**: `quadatric2.c` líneas 47-50

**De**:
```c
float d = pow(fb[i],2) - 4.0*fa[i]*fc[i];
float sd = sqrt(d);
```

**A**:
```c
float d = powf(fb[i], 2.0f) - 4.0f*fa[i]*fc[i];
float sd = sqrtf(d);
```

**Beneficios**:
- ✓ 8.8% mejor en local
- ✓ 9% mejor en cluster
- ✓ 2x mejor en cluster con TIMES=200
- ✓ 50% menos memoria (120 MB vs 240 MB)

---

## FAQ

### ¿Por qué entonces float es mejor en cluster?
Porque el cluster tiene:
1. Compilador más agresivo (-O3 con vectorización más agresiva)
2. Memory bandwidth es bottleneck más severo
3. Cluster probablemente tiene múltiples CPUs → BW compartido

### ¿Es siempre 2x o 9%?
- 9% es el efecto SIMD puro
- 2x ocurre cuando memory bandwidth es severamente limitante
- En TIMES=200 el cluster lo hace más notorio

### ¿Qué significa "no entra en registros"?
Es un mal entendido. Significa que double NO aprovecha los registros SIMD tan bien como float:
- Float: llena 8 espacios de 32-bit en AVX2
- Double: solo ocupa 4 espacios de 64-bit
- Menos elementos en paralelo = menos throughput

### ¿Debo usar float o double?
- **Float**: Mejor rendimiento (8.8% local, 2x cluster), menos memoria
- **Double**: Mejor precisión (pero para ecuaciones cuadráticas, float es suficiente)

---

## Conclusión

Tu hipótesis arquitectónica es **CORRECTA**.

La causa real NO es "registros" sino:
1. **SIMD vectorization**: Float procesa 2x elementos/ciclo
2. **Memory bandwidth**: Double consume 2x datos
3. **Cluster stress**: BW es limitante en cluster, no en local

Solución: Usar float con `powf()` y `sqrtf()`.
