# Análisis de eficiencia: `hibrido.c` vs código de referencia

## 1. Estructura general de ambos algoritmos

Ambos resuelven: **R = A × (B × B^T) × constante**

```
                      A (n×n)
                      ──────
   B (n×n) ──→  B × B^T  ──→  A × (B × B^T)  ──→  R × constante
                                                  ↘
                                            R = resultado final
```

**Misma matemática, misma cantidad de FLOPS** (2 × n³/numProcs por proceso).  
La diferencia está en **cómo se organizan los accesos a memoria y la sincronización**.

---

## 2. Diferencias clave

### 2.1 MPI Scatter/Bcast: ¿fuera o dentro del parallel?

#### `hibrido.c` (original) — DENTRO

```c
#pragma omp parallel
{
    #pragma omp master
    {
        MPI_Scatter(a, ...);   // solo el master comunica
        MPI_Bcast(b, ...);     // solo el master comunica
    }
    #pragma omp barrier        // otros threads esperan ociosos
    // ... resto del trabajo ...
}
```

```
TIEMPO:
Thread 0 (master)  ┌──MPI_Scatter──┬──MPI_Bcast──┬░░trabajo░░┐
Thread 1             ░░░░espera░░░░░░░░░espera░░░░│░░trabajo░░│
Thread 2             ░░░░espera░░░░░░░░░espera░░░░│░░trabajo░░│
Thread 3             ░░░░espera░░░░░░░░░espera░░░░│░░trabajo░░│
                     └─────────────────────────────┴───────────┘
                                        ↑
                              threads ociosos durante MPI
```

**Problema:** los hilos no-master se crean, se quedan esperando en barreras, y no hacen nada útil hasta que el master termina de comunicar.

#### `hibrido-pato.c` — FUERA

```c
MPI_Scatter(A_global, ...);   // sin hilos creados aún
MPI_Bcast(B, ...);            // sin hilos creados aún

#pragma omp parallel           // threads se crean DESPUÉS de la comunicación
{
    // ... todo el trabajo útil ...
}
```

```
TIEMPO:
Proceso       ┌┬MPI_Scatter┬┬MPI_Bcast┬┐
               │              │          │
               │ MPI sin      │          │
               │ overhead     │          │
               │ de threads   │          │
               └──────────────┴──────────┘

Luego:
Thread 0 (master)  ┌────────trabajo────────┐
Thread 1           │────────trabajo────────│
Thread 2           │────────trabajo────────│
Thread 3           │────────trabajo────────│
                    └──────────────────────┘
               ↑
         threads creados SOLO para trabajo útil
```

**Ventaja:** 0 overhead de threads ociosos durante comunicación colectiva.

---

### 2.2 Cómo se obtiene B × B^T: matmul vs transpuesta

#### `hibrido.c` — calcula D = B × B^T como matmul

```c
// D = B × B^T  (n³/numProcs FLOPS)
for (int ii = 0; ii < stripSize; ii += BS) {
    int i = stripStart + ii;
    int in = i * n;
    int local_in = ii * n;
    for (int j = 0; j < n; j += BS) {
        int jn = j * n;
        for (int k = 0; k < n; k += BS) {
            blkmulRowColRow(&b[in + k], &b[jn + k],
                            &partialD[local_in + j], n, BS);
        }
    }
}
// Luego Allgather(D)  — O(n²) comunicación
```

**Costo:** n³/numProcs FLOPS × 1 matmul antes del Allgather.

```
  B (n×n) ──→  matmul B × B^T  ──→  D (n×n)  ──→  Allgather(D)
                  ↑                      ↑
                O(n³)                O(n²) comunic.
```

#### `hibrido-pato.c` — transpone B localmente, Allgather, luego matmul

```c
// BT_local = B^T  (SOLO transpuesta local: O(n²/numProcs))
for (int i = 0; i < stripSize; i++) {
    int b_row = stripStart + i;
    for (int j = 0; j < n; j++)
        BT_local[i * n + j] = B[j * n + b_row];
}
// Allgather(BT_local → BT_global)  — O(n²) comunicación
MPI_Allgather(BT_local, ... , BT_global, ...);

// 1er matmul: T = A × B  — O(n³/numProcs)
// blkmulRowColRow(A_block, BT_block, T_block) → T = A × (BT)^T = A × B
blkmulRowColRow(&A_local[ii*n + k], &BT_global[j*n + k],
                &T_local[ii*n + j], n, BS);
```

**Costo:** solo O(n²/numProcs) de transpuesta local antes del Allgather, luego 1 matmul.

```
  B (n×n) ──→  transpose local  ──→  BT_local  ──→  Allgather → BT_global
                  ↑                        ↑
                O(n²)                  O(n²) comunic.
                  
  A (franja) ──→  matmul A × B (via BT_global) ──→  T (franja)
                     ↑
                   O(n³/numProcs)   ← idéntico al 2do paso del original
```

#### Comparación visual del flujo completo

```
ORIGINAL (hibrido.c):
  B ─→ matmul B×B^T ─→ Allgather D ─→ matmul A×D ─→ R
       O(n³)              O(n²)         O(n³)
       └───── 2 × O(n³) ──────┘

REFERENCIA (hibrido-pato.c):
  B ─→ transpose O(n²) ─→ Allgather BT ─→ matmul A×B ─→ matmul T×B ─→ R
                              O(n²)          O(n³)         O(n³)
                              └────── 2 × O(n³) ──────┘
```

**Ambos hacen 2 × O(n³/numProcs) matmuls.**  
Pero el reference reemplaza el primer matmul (B × B^T) por una **transpuesta O(n²)** — que es 3 órdenes de magnitud más barata para n grandes (n=1024: n³ ≈ 10⁹ vs n² ≈ 10⁶).

---

### 2.3 Inicialización de buffers: paralela vs secuencial

#### `hibrido.c` — calloc secuencial dentro de master

```c
#pragma omp master
partialD = (double *)calloc((size_t)stripSize * n, sizeof(double));
//            └── solo 1 thread hace el trabajo ──┘
```

```
Thread 0 (master)  ┌──────calloc (1 thread)──────┐
Thread 1             ░░░░░░░░espera░░░░░░░░░░░░░░░│
Thread 2             ░░░░░░░░espera░░░░░░░░░░░░░░░│
```

**Problema:** `calloc` zeroea la memoria secuencialmente. Para stripSize × n = 512×512/2 = 131072 doubles ≈ 1 MB, no es crítico. Para matrices grandes (n=4096: 32 MB por proceso), la diferencia es notable.

#### `hibrido-pato.c` — malloc + init paralelo

```c
double *T_local = (double *)malloc(stripSize * n * sizeof(double));
// ...
#pragma omp parallel for schedule(static)
for (int i = 0; i < stripSize * n; i++)
    T_local[i] = R_local[i] = 0.0;
```

```
Thread 0  ┌────inicializa 1/4 del buffer────┐
Thread 1  │────inicializa 1/4 del buffer────│
Thread 2  │────inicializa 1/4 del buffer────│
Thread 3  │────inicializa 1/4 del buffer────│
           └─────────────────────────────────┘
              ↑ TODOS los threads trabajan
```

**Ventaja:** H = 4 threads → inicialización ~4× más rápida.

---

### 2.4 Reducciones: sobre `parallel` vs sobre `for`

#### `hibrido.c` — reduction en `#pragma omp for`

```c
// Antes del parallel
double localMinA, localMaxA, localSumA;  // sin inicializar

#pragma omp parallel
{
    #pragma omp master { localMinA = localA[0]; ... }  // init manual
    #pragma omp barrier
    #pragma omp for reduction(min:localMinA, ...) ...   // reduction en for
}
```

**Problema:** 2 pasos extras (init manual + barrier) que no aportan valor.  
Además, usar `reduction` en `#pragma omp for` dentro de `#pragma omp parallel` puede causar errores de compilación en compiladores modernos (GCC 14+ rechaza reduction variables privadas en el outer context).

#### `hibrido-pato.c` — reduction en `#pragma omp parallel`

```c
double l_minA = DBL_MAX, l_maxA = -DBL_MAX, l_sumA = 0.0;

#pragma omp parallel reduction(min:l_minA,l_minB) \
                       reduction(max:l_maxA,l_maxB) \
                       reduction(+:l_sumA,l_sumB)
{
    #pragma omp for schedule(static)
    for (int i = 0; i < stripSize * n; i++) {
        if (A_local[i] < l_minA) l_minA = A_local[i];
        if (A_local[i] > l_maxA) l_maxA = A_local[i];
        l_sumA += A_local[i];
    }
    // ...
}
```

**Ventaja:** 
- OpenMP maneja la inicialización de las variables de reducción automáticamente (MIN → DBL_MAX, MAX → -DBL_MAX, SUM → 0)
- Sin barrier extra
- Sin init manual
- Código más limpio y portable

---

## 3. Resumen de diferencias

| Aspecto | `hibrido.c` | `hibrido-pato.c` | Impacto |
|---|---|---|---|
| MPI Scatter/Bcast | Dentro de parallel | Fuera de parallel | **Medio** — threads no se crean antes de tiempo |
| Primer paso de D | O(n³) matmul B×B^T | O(n²) transpose local | **Alto** — evita matmul completo |
| Init de buffers | `calloc` en master (1 thread) | `malloc` + parallel for (H threads) | **Alto** — init ~H× más rápida |
| Reducciones | `reduction` en `for` + init manual | `reduction` en `parallel` | **Bajo** — código más limpio, portable |
| Número de matmuls | 2 × O(n³) | 2 × O(n³) | **Igual** — misma aritmética |
| Allgather | 1 (D) | 1 (BT_global) | **Igual** — mismo volumen |
| Validación | OK | OK | — |
| Output | `RESULT;...` | `RESULT;...` | **Idéntico** |

## 4. Conclusión

La mayor ganancia está en **reemplazar un matmul O(n³) por una transpuesta O(n²)**.  
Aunque ambos flujos terminan haciendo 2 matmuls × n³/numProcs FLOPS, el reference reorganiza el cómputo para:

1. Construir B^T (casi gratis: O(n²))
2. Hacer el primer matmul A × B usando B^T (mismo costo que B × B^T pero produce T, un resultado intermedio reutilizable)
3. Hacer el segundo matmul T × B usando B directamente (mismo costo)

Mientras que el original:

1. Hace un matmul completo B × B^T para producir D
2. Comunicar D vía Allgather
3. Hace A × D

La diferencia es que el **primer matmul del reference (A × B) ya está contribuyendo al resultado final** (es parte de `A × B × B^T`), mientras que el **primer matmul del original (B × B^T) solo produce un intermediario (D)** que luego debe ser comunicado y re-multiplicado. El reference "fusiona" conceptualmente la transpuesta con el primer matmul, ahorrando O(n³) de trabajo.
