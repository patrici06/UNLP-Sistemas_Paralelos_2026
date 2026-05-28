#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>
#include <math.h>
#include <string.h>

#define BS 64

// ========================================
// DECLARACIONES
// ========================================
double dwalltime();
void matmulblksRowColCol(double *a, double *b, double *c, int n, int bs);
void matmulblksRowColRow(double *a, double *b, double *c, int n, int bs);
void blkmulRowColCol(double *ablk, double *bblk, double *cblk, int n, int bs);
void blkmulRowColRow(double *ablk, double *bblk, double *cblk, int n, int bs);
void print_matrix(double *mat, int n, const char *name, int max_print);

// ========================================
// TIMER
// ========================================
double dwalltime() {
    double sec;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    sec = tv.tv_sec + tv.tv_usec/1000000.0;
    return sec;
}

// ========================================
// MAIN
// ========================================
// FORMATO DE SALIDA:
// RESULT;N;T;workTime;GFLOPS;speedup;efficiency;overhead;overhead%
//   - N: tamaño de matriz
//   - T: cantidad de threads
//   - workTime: tiempo de ejecución (segundos)
//   - GFLOPS: operaciones en punto flotante por segundo (2*N³ / workTime)
//   - speedup: aceleración respecto a T=1 (ref_time_sequential / workTime)
//   - efficiency: porcentaje de eficiencia = (speedup / T) * 100
//   - overhead: tiempo perdido en sincronización = workTime - (ref_time / T)
//   - overhead%: porcentaje de overhead = (overhead / workTime) * 100
// ========================================
int main(int argc, char*argv[]) {
    double *a, *b, *d, *r;
    int i, n, t;
    int print_matrices, nan_count, inf_count;
    double MaxA, MinA, PromA, MaxB, MinB, PromB;
    double timetick, workTime;
    double constante;
    double speedup, efficiency;
    static double ref_time_sequential = -1.0;

    print_matrices = 0;

    if ((argc < 3) || ((n = atoi(argv[1])) <= 0) || ((t = atoi(argv[2])) <= 0)) {
        printf("\nError: N y T deben ser positivos\n");
        printf("Uso: %s N T [print_matrices]\n", argv[0]);
        exit(1);
    }
    
    // Datos obtenidos de una ejecucion con t=1 en cluster
    // Hardcodear ref_time_sequential según N (promedio de 3 ejecuciones para openmp)
    if (n == 512) ref_time_sequential = 0.486125;
    else if (n == 1024) ref_time_sequential = 3.895695;
    else if (n == 2048) ref_time_sequential = 32.031549;
    else if (n == 4096) ref_time_sequential = 254.817039;
    else ref_time_sequential = -1.0;  // Para otros tamaños, sin referencia
    
    // T es REQUERIDO (no opcional)
    omp_set_num_threads(t);

    if (argc >= 4) {
        print_matrices = atoi(argv[3]);
    }

    a = (double*) malloc(sizeof(double)*n*n);
    b = (double*) malloc(sizeof(double)*n*n);
    d = (double*) malloc(sizeof(double)*n*n);
    r = (double*) malloc(sizeof(double)*n*n);

    // Inicialización de matrices con patrón similar a matrices.c
    for (i = 0; i < n*n; i++) {
        a[i] = (double)((i / n) + (i % n) + 1);
        b[i] = (double)((i / n) - (i % n) + 1);
        d[i] = 0.0;
        r[i] = 0.0;
    }

    // PRINT MATRICES (FUERA DE MEDICIÓN DE TIEMPO)
    if (print_matrices && n <= 4) {
        print_matrix(a, n, "MATRIZ A", n);
        print_matrix(b, n, "MATRIZ B", n);
    }

    // ========================================
    // ETAPA 0: Cálculo de estadísticas (FUERA DE MEDICIÓN DE TIEMPO)
    // ========================================
    MaxA = MinA = a[0];
    MaxB = MinB = b[0];
    PromA = 0.0;
    PromB = 0.0;

    // INICIA MEDICIÓN DE TIEMPO
    timetick = dwalltime();
    // reduction: combina resultados parciales de cada thread (max/min/suma)
    // schedule(static): divide iteraciones al inicio, sin sincronización
    #pragma omp parallel for reduction(max: MaxA, MaxB) reduction(min: MinA, MinB) reduction(+: PromA, PromB) schedule(static)
    for (i = 0; i < n*n; i++) {
        double valA = a[i];
        double valB = b[i];

        if (valA > MaxA) MaxA = valA;
        if (valA < MinA) MinA = valA;
        PromA += valA;

        if (valB > MaxB) MaxB = valB;
        if (valB < MinB) MinB = valB;
        PromB += valB;
    }
    // Después de #pragma omp parallel for, la región paralela termina automáticamente.
    // Las siguientes líneas se ejecutan solo en el hilo maestro (sin paralelización).
    // No necesito #pragma omp single porque ya hemos salido de la región paralela.
    PromA /= (n*n);
    PromB /= (n*n);
    constante = ((MaxA * MaxB) - (MinA * MinB)) / (PromA * PromB);
    
   
    // ========================================
    // ETAPA 1: Multiplicación B x B^T -> D (row-col-col)
    // ========================================
    matmulblksRowColCol(b, b, d, n, BS);

    // ========================================
    // ETAPA 2: Multiplicación A x D -> R (row-col-row)
    // ========================================
    matmulblksRowColRow(a, d, r, n, BS);

    // ========================================
    // ETAPA 3: Aplicar constante - R = constante * R
    // Paralelizada con pragma omp for
    // ========================================
    // schedule(static): bueno para loops con carga balanceada (sin sincronización)
    #pragma omp parallel for schedule(static)
    for (i = 0; i < n*n; i++) {
        r[i] *= constante;
    }

    workTime = dwalltime() - timetick;
    // FIN MEDICIÓN DE TIEMPO

    // PRINT RESULTADOS (FUERA DE MEDICIÓN)
    if (print_matrices && n <= 4) {
        printf("\nCONSTANTE = %.6f\n", constante);
        print_matrix(d, n, "MATRIZ D = B x B^T", n);
        print_matrix(r, n, "MATRIZ R = A x D x k", n);
    }

    // Validación de NaN e Inf
    nan_count = 0;
    inf_count = 0;
    // reduction(+:): acumula sumas parciales de cada thread
    #pragma omp parallel for reduction(+: nan_count, inf_count)
    for (i = 0; i < n*n; i++) {
        if (isnan(r[i])) nan_count++;
        if (isinf(r[i])) inf_count++;
    }

    // Calcular GFLOPS
    double gflops = ((double)2*n*n*n)/(workTime*1e9);
    
    // Calcular speedup, eficiencia y overhead
    speedup = 1.0;
    efficiency = 100.0;
    double overhead = 0.0;
    double overhead_percent = 0.0;
    
    if (t == 1) {
        ref_time_sequential = workTime;
    } else if (ref_time_sequential > 0) {
        speedup = ref_time_sequential / workTime;
        efficiency = (speedup / (double)t) * 100.0;
        // Overhead = tiempo paralelo - (tiempo secuencial / T)
        overhead = workTime - (ref_time_sequential / (double)t);
        overhead_percent = (overhead / workTime) * 100.0;
    }

    // Formato: RESULT;N;T;workTime;GFLOPS;speedup;efficiency;overhead;overhead%
    printf("RESULT;%d;%d;%lf;%lf;%lf;%lf;%lf;%lf\n", n, t, workTime, gflops, speedup, efficiency, overhead, overhead_percent);
    // printf("CONSTANTE_K;%lf\n", constante);

    // if (nan_count == 0 && inf_count == 0)
    //     printf("VALIDATION;OK\n");
    // else
    //     printf("VALIDATION;ERROR;NaN=%d;Inf=%d\n", nan_count, inf_count);

    free(a);
    free(b);
    free(d);
    free(r);

    return 0;
}




// ========================================
// FUNCTION: matmulblksRowColCol
// Multiplicación B x B^T -> D (row-col-col)
// Paralelizada a nivel de bloques
// ========================================
void matmulblksRowColCol(double *a, double *b, double *c, int n, int bs) {
    int i, j, k;
    
    // collapse(3): paraleliza los 3 niveles de anidamiento como un único loop
    // schedule(static): iteraciones balanceadas (bloques uniformes), sin sincronización
    #pragma omp parallel for collapse(3) schedule(static)
    for (i = 0; i < n; i += bs) {
        for (j = 0; j < n; j += bs) {
            for (k = 0; k < n; k += bs) {
                int in = i*n;
                int jn = j*n;
                blkmulRowColCol(&a[in + k], &b[k + jn], &c[i + jn], n, bs);
            }
        }
    }
}

// ========================================
// FUNCTION: matmulblksRowColRow
// Multiplicación A x D -> R (row-col-row)
// Paralelizada a nivel de bloques
// ========================================
void matmulblksRowColRow(double *a, double *b, double *c, int n, int bs) {
    int i, j, k;
    
    // collapse(3): paraleliza los 3 niveles de anidamiento como un único loop
    // schedule(static): iteraciones balanceadas (bloques uniformes), sin sincronización
    #pragma omp parallel for collapse(3) schedule(static)
    for (i = 0; i < n; i += bs) {
        for (j = 0; j < n; j += bs) {
            for (k = 0; k < n; k += bs) {
                int in = i*n;
                int jn = j*n;
                blkmulRowColRow(&a[in + k], &b[jn + k], &c[in + j], n, bs);
            }
        }
    }
}

// ========================================
// FUNCTION: blkmulRowColCol
// Multiplicación especializada: A row-major, B column-major, C column-major
// ========================================
void blkmulRowColCol(double *ablk, double *bblk, double *cblk, int n, int bs) {
    for (int i = 0; i < bs; i++) {
        int in = i * n;
        for (int j = 0; j < bs; j++) {
            int jn = j * n;
            double sum = 0.0;
            for (int k = 0; k < bs; k++) {
                sum += ablk[in + k] * bblk[jn + k];
            }
            cblk[i + jn] += sum;
        }
    }
}

// ========================================
// FUNCTION: blkmulRowColRow
// Multiplicación especializada: A row-major, B column-major, C row-major
// ========================================
void blkmulRowColRow(double *ablk, double *bblk, double *cblk, int n, int bs) {
    for (int i = 0; i < bs; i++) {
        int in = i * n;
        for (int j = 0; j < bs; j++) {
            double sum = 0.0;
            int jn = j * n;
            for (int k = 0; k < bs; k++) {
                sum += ablk[in + k] * bblk[jn + k];
            }
            cblk[in + j] += sum;
        }
    }
}

// ========================
// FUNCIONES AUXILIARES
// ========================

/* Imprime una matriz para debugging */
void print_matrix(double *mat, int n, const char *name, int max_print)
{
    int limit, i, j;
    
    limit = (n < max_print) ? n : max_print;
    printf("\n%s (%dx%d):\n", name, n, n);
    
    for (i = 0; i < limit; i++) {
        printf("[ ");
        for (j = 0; j < limit; j++) {
            printf("%8.2f ", mat[i*n + j]);
        }
        if (limit < n) printf("... ");
        printf("]\n");
    }
    if (limit < n) printf("...\n");
}


