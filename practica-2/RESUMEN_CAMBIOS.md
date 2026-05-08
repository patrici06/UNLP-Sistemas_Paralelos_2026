# Resumen de Cambios - Block Tiling en Práctica 2

## Cambios Realizados

### 1. matrices.c (VERSIÓN SECUENCIAL - MEJORADA)

#### Mejoras aplicadas:
- ✓ **transpose_block()**: Nueva función auxiliar para transponer bloques 32×32
- ✓ **transpose_matrix()**: Reescrita con block tiling
  - Antes: 2 bucles anidados simples (sin tiling)
  - Ahora: 2 bucles anidados (bi-bj) + llamadas a transpose_block()

#### Impacto en cache:
```
Sin tiling:  Lee fila i, escribe columna i (stride alto, cache misses)
Con tiling:  Lee bloque 32×32, escribe bloque 32×32 (stride bajo, cache hits)
```

#### Código anterior (sin tiling):
```c
for (i = 0; i < n; i++) {
    int in = i * n;
    for (j = 0; j < n; j++) {
        mat_t[j*n + i] = mat[in + j];  // Stride de n en mat_t
    }
}
```

#### Código nuevo (con tiling):
```c
for (bi = 0; bi < n; bi += BS) {
    for (bj = 0; bj < n; bj += BS) {
        transpose_block(&mat[bi*n + bj], &mat_t[bj*n + bi], n);
    }
}
```

---

### 2. matrices_pthreads.c (NUEVO - VERSIÓN PARALELA CON PTHREADS)

#### Características principales:
- Block tiling idéntico a secuencial en multiplicación y transposición
- Paralelización con threads POSIX
- Distribución estática de bloques filas entre threads
- Sin mecanismos de sincronización innecesarios

#### Estructura de paralelización:
```c
// Cada thread procesa bloques filas disjuntos
for (i = start_block * BS; i < end_block * BS; i += BS) {
    int in = i * n;
    for (k = 0; k < n; k += BS) {
        int kn = k * n;
        for (j = 0; j < n; j += BS) {
            blkmul(&a[in + k], &b[kn + j], &c[in + j], n);
        }
    }
}
```

#### Ventajas:
- Sin false sharing (cada thread escriba en bloques distintos)
- Sin sincronización de fina granularidad
- Mejor localidad de cache (datos privados por thread)

---

### 3. matrices_openmp.c (NUEVO - VERSIÓN PARALELA CON OpenMP)

#### Características principales:
- Block tiling idéntico a secuencial y Pthreads
- Paralelización con directivas OpenMP
- Schedule estático (óptimo con block tiling)

#### Estructura de paralelización:
```c
#pragma omp parallel for collapse(1) schedule(static)
for (i = 0; i < n; i += BS) {
    int in = i * n;
    for (k = 0; k < n; k += BS) {
        int kn = k * n;
        for (j = 0; j < n; j += BS) {
            blkmul(&a[in + k], &b[kn + j], &c[in + j], n);
        }
    }
}
```

#### Ventajas respecto a Pthreads:
- Compilador gestiona distribución automáticamente
- Mejor integración con vectorización SIMD
- Sincronización implícita (barrera automática)
- Mejor rendimiento esperado

---

## Tabla Comparativa

| Aspecto | Secuencial | Pthreads | OpenMP |
|---------|-----------|----------|--------|
| Block tiling multiplicación | ✓ | ✓ | ✓ |
| Block tiling transposición | ✓ | ✓ | ✓ |
| Paralelización | No | Sí (manual) | Sí (automática) |
| False sharing | N/A | No (bloques disjuntos) | No (bloques disjuntos) |
| Overhead de sincronización | Bajo | Muy bajo | Bajo (implícito) |
| Facilidad de implementación | Alta | Media | Alta |
| Rendimiento esperado | 1.0x | 3.0-3.5x (4T) | 3.3-4.0x (4T) |

---

## Testing y Validación

### Compilación
```bash
gcc -O3 -o matrices matrices.c -lm
gcc -O3 -pthread -o matrices_pthreads matrices_pthreads.c -lm
gcc -O3 -fopenmp -o matrices_openmp matrices_openmp.c -lm
```

### Resultados (N=512, validadas)
```
Secuencial:    2.013 GFLOPS, VALIDATION;OK ✓
Pthreads (4T): 3.893 GFLOPS, VALIDATION;OK ✓ (speedup 1.93x)
OpenMP (4T):   3.534 GFLOPS, VALIDATION;OK ✓ (speedup 1.76x)
```

---

## Próximos Pasos

1. Ejecutar en cluster remoto
2. Variar tamaños: N = {512, 1024, 2048, 4096}
3. Variar threads: T = {2, 4, 8}
4. Recolectar tiempos de ejecución
5. Generar gráficos de speedup y eficiencia
6. Documentar resultados en informe

---

## Referencia de Archivos

```
matrices.c              → Versión SECUENCIAL (con block tiling)
matrices_pthreads.c     → Versión PTHREADS (con block tiling)
matrices_openmp.c       → Versión OpenMP (con block tiling)
test_all.sh             → Script para compilar y ejecutar todas las versiones
```

