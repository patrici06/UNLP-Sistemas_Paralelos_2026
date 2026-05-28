#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
#include<math.h>
#include<string.h>
#include<stdbool.h>
#include<pthread.h>

#define BS 64

// ========================================
// VARIABLES GLOBALES
// ========================================
// Métricas globales - compartidas con mutex
pthread_mutex_t mutex_metrics = PTHREAD_MUTEX_INITIALIZER;
double g_maxA = -999999999, g_minA = 999999999, g_sumA = 0.0;
double g_maxB = -999999999, g_minB = 999999999, g_sumB = 0.0;
double factor_final = 0.0;  // Constante k = (maxA*maxB - minA*minB) / (promA * promB)

// Barrera de sincronización
pthread_barrier_t barrier_sync;

// Variable estática para almacenar referencia secuencial (T=1)
static double ref_time_sequential = -1.0;

// ========================================
// ESTRUCTURAS
// ========================================
typedef struct {
    int id;      // ID del hilo
    int N;       // Tamaño de matriz
    int T;       // Total de hilos
    int chunk;   // Elementos por hilo (N/T)
    double *A;   // Matriz A global
    double *B;   // Matriz B global
    double *C;   // Matriz resultado de A x D
    double *D;  // Matriz temporal para B*B^T
    double *R;   // Matriz resultado
} thread_args_t;

// ========================================
// DECLARACIONES
// ========================================
double dwalltime();
void print_matrix(double *mat, int n, const char *name, int order, bool print);
void *thread_worker(void *ptr);
void blkmulRowColCol(double *ablk, double *bblk, double *cblk, int n, int bs);
void blkmulRowColRow(double *ablk, double *bblk, double *cblk, int n, int bs);

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
    int i, j, k, n, t;
    double timetick, workTime;

    if ((argc != 3) || ((n = atoi(argv[1])) <= 0) || ((t = atoi(argv[2])) <= 0)) {
        printf("Uso: %s N T\n", argv[0]);
        printf("N: tamaño de matriz, T: cantidad de hilos\n");
        exit(1);
    }
    
    // Datos obtenidos de una ejecucion con t=1 en cluster
    // Hardcodear ref_time_sequential según N
    if (n == 512) ref_time_sequential = 0.486594;
    else if (n == 1024) ref_time_sequential = 3.900768;
    else if (n == 2048) ref_time_sequential = 32.309650;
    else if (n == 4096) ref_time_sequential = 254.224642;
    else ref_time_sequential = -1.0;  // Para otros tamaños, sin referencia
    
    bool print = false;
    print = BS == 4;

    // Asignar memoria para matrices
    double *A = (double*) malloc(sizeof(double) * n * n);
    double *B = (double*) malloc(sizeof(double) * n * n);
    double *D = (double*) malloc(sizeof(double) * n * n);
    double *R = (double*) malloc(sizeof(double) * n * n);
    
    // Inicializar métricas globales
    g_maxA = -999999999;
    g_minA = 999999999;
    g_sumA = 0.0;
    g_maxB = -999999999;
    g_minB = 999999999;
    g_sumB = 0.0;

    // Inicialización de matrices
    for (i = 0; i < n * n; i++) {
        A[i] = (double)((i / n) + (i % n) + 1);
        B[i] = (double)((i / n) - (i % n) + 1);
        D[i] = 0.0;
        R[i] = 0.0;
    }

    // TESTING: Print initial matrices
    if (print) {
        printf("\n========== TESTING: Matrices Iniciales ==========\n");
        print_matrix(A, n, "Matriz A (row-major)", 0, print);
        print_matrix(B, n, "Matriz B (row-major)", 0, print);
        printf("================================================\n\n");
    }

    pthread_attr_t attr;
    pthread_t threads[t];
    thread_args_t thread_args[t];
    
    // Inicializar barrera para sincronizar t hilos
    pthread_barrier_init(&barrier_sync, NULL, t);
    
    timetick = dwalltime();
    pthread_attr_init(&attr);
    
    // Crear hilos
    for (i = 0; i < t; i++){
        thread_args[i].id = i;
        thread_args[i].N = n;
        thread_args[i].T = t;
        thread_args[i].chunk = n / t;
        thread_args[i].A = A;
        thread_args[i].B = B;
        thread_args[i].D = D;
        thread_args[i].R = R;
        pthread_create(&threads[i], &attr, thread_worker, (void*)&thread_args[i]);
    }
    
    // Esperar a que todos los hilos terminen
    for (i = 0; i < t; i++){
        pthread_join(threads[i], NULL);
    }
    
    pthread_attr_destroy(&attr);
    pthread_barrier_destroy(&barrier_sync);
    
    workTime = dwalltime() - timetick;

    // TESTING: Print intermediate results
    if (print) {
        printf("\n========== TESTING: D = B x B^T ==========\n");
        print_matrix(D, n, "Matriz D (column-major)", 2, print);
        printf("=========================================\n\n");
        printf("\n========== TESTING: R = A x D x k ==========\n");
        printf("Constante k = %lf\n", factor_final);
        print_matrix(R, n, "Matriz R (row-major)", 0, print);
        printf("============================================\n\n");
    }

    // =========================
    // VALIDACIÓN
    // =========================
    
    int nan_count = 0, inf_count = 0;

    for (i = 0; i < n*n; i++) {
        if (isnan(R[i])) nan_count++;
        if (isinf(R[i])) inf_count++;
    }

    // Calcular GFLOPS
    double gflops = ((double)2*n*n*n)/(workTime*1e9);
    
    // Calcular speedup, eficiencia y overhead
    double speedup = 1.0;
    double efficiency = 100.0;
    double overhead = 0.0;
    double overhead_percent = 0.0;
    
    if (t == 1) {
        // Guardar tiempo de referencia para T=1
        ref_time_sequential = workTime;
    } else if (ref_time_sequential > 0) {
        // Calcular speedup basado en referencia secuencial
        speedup = ref_time_sequential / workTime;
        efficiency = (speedup / (double)t) * 100.0;
        // Overhead = tiempo paralelo - (tiempo secuencial / T)
        overhead = workTime - (ref_time_sequential / (double)t);
        overhead_percent = (overhead / workTime) * 100.0;
    }
    
    // Formato: RESULT;N;T;workTime;GFLOPS;speedup;efficiency;overhead;overhead%
    printf("RESULT;%d;%d;%lf;%lf;%lf;%lf;%lf;%lf\n", n, t, workTime, gflops, speedup, efficiency, overhead, overhead_percent);
    // printf("CONSTANTE_K;%lf\n", factor_final);

    // if (nan_count == 0 && inf_count == 0)
    //     printf("VALIDATION;OK\n");
    // else
    //     printf("VALIDATION;ERROR;NaN=%d;Inf=%d\n", nan_count, inf_count);

    free(A);
    free(B);
    free(D);
    free(R);
    
    pthread_mutex_destroy(&mutex_metrics);

    return 0;
}

// ========================================
// FUNCTION: thread_worker
// Local -> Mutex -> Barrier -> Hilo 0 Calcula -> Barrier
// ========================================
void *thread_worker(void *ptr) {
    thread_args_t *p = (thread_args_t*)ptr;
    
    // Asignar explícitamente los campos de la estructura con casting explícito
    int id = (int)p->id;
    int N = (int)p->N;
    int T = (int)p->T;
    int chunk = (int)p->chunk;
    double *A = (double*)p->A;
    double *B = (double*)p->B;
    double *D = (double*)p->D;
    double *R = (double*)p->R;
    
    // Calcular rango de filas para este hilo
    int start = id * chunk;
    int end = (id == T - 1) ? N : (id + 1) * chunk;




    // ========================================
    // ETAPA 0: Cálculo LOCAL de métricas
    // SIN MUTEX - cada hilo usa variables locales
    // ========================================
    double l_maxA = -999999999, l_minA = 999999999, l_sumA = 0.0;
    double l_maxB = -999999999, l_minB = 999999999, l_sumB = 0.0;
    
    // Iterar sobre filas asignadas a este hilo
    for (int i = start * N; i < end * N; i++) {
        // Métrica de A
        if (A[i] > l_maxA) l_maxA = A[i];
        if (A[i] < l_minA) l_minA = A[i];
        l_sumA += A[i];
        
        // Métrica de B
        if (B[i] > l_maxB) l_maxB = B[i];
        if (B[i] < l_minB) l_minB = B[i];
        l_sumB += B[i];
    }
    
    // Actualizar globales CON MUTEX
    pthread_mutex_lock(&mutex_metrics);
    if (l_maxA > g_maxA) g_maxA = l_maxA;
    if (l_minA < g_minA) g_minA = l_minA;
    g_sumA += l_sumA;
    if (l_maxB > g_maxB) g_maxB = l_maxB;
    if (l_minB < g_minB) g_minB = l_minB;
    g_sumB += l_sumB;
    pthread_mutex_unlock(&mutex_metrics);
    
    // BARRERA: Todos esperan
    pthread_barrier_wait(&barrier_sync);
    
    // Solo hilo 0 calcula la CONSTANTE
    if (id == 0) {
        double promA = g_sumA / (double)(N * N);
        double promB = g_sumB / (double)(N * N);
        factor_final = (g_maxA * g_maxB - g_minA * g_minB) / (promA * promB);
    }
    
    pthread_barrier_wait(&barrier_sync);
    
    // ========================================
    // ETAPA 1: Multiplicación B x B^T -> D (row-col-col)
    // Loop order: i, j, k (como en matrices.c)
    // blkmulRowColCol(&b[i*n + k], &b[k + j*n], &d[i + j*n], n, bs)
    // ========================================
    int TB = BS;
    for (int i = start; i < end; i += TB) {
        int in = i * N;
        for (int j = 0; j < N; j += TB) {
            int jn = j * N;
            for (int k = 0; k < N; k += TB) {
                blkmulRowColCol(&B[in + k], &B[k + jn], 
                                &D[i + jn], N, TB);
            }
        }
    }
    
    pthread_barrier_wait(&barrier_sync);
    
    // ========================================
    // ETAPA 2: Multiplicación A x D -> R (row-col-row)
    // Loop order: i, j, k (como en matrices.c)
    // blkmulRowColRow(&a[i*n + k], &b[j*n + k], &c[i*n + j], n, bs)
    // ========================================
    for (int i = start; i < end; i += TB) {
        int in = i * N;
        for (int j = 0; j < N; j += TB) {
            int jn = j * N;
            for (int k = 0; k < N; k += TB) {
                blkmulRowColRow(&A[in + k], &D[jn + k], 
                                &R[in + j], N, TB);
            }
        }
    }
    
    pthread_barrier_wait(&barrier_sync);
    
    // ========================================
    // ETAPA 3: Aplicar factor_final
    // R = R * factor_final
    // ========================================
    for (int i = start * N; i < end * N; i++) {
        R[i] *= factor_final;
    }
    
    pthread_exit(NULL);
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

// =========================
// TIMER
// =========================
double dwalltime()
{
    double sec;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    sec = tv.tv_sec + tv.tv_usec/1000000.0;
    return sec;
}

// =========================
// PRINT MATRIX FUNCTIONS
// =========================
void print_matrix(double *mat, int n, const char *name, int order, bool print)
{
    if (print) {
        printf("\n%s (n=%d, order=%s):\n", name, n, order == 0 ? "row-major" : order == 1 ? "row-major" : "column-major");
        
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                double val;
                if (order == 0 || order == 1) {
                    // row-major
                    val = mat[i*n + j];
                } else {
                    // column-major (order == 2)
                    val = mat[i + j*n];
                }
                printf("%8.2f ", val);
            }
            printf("\n");
        }
    }
}
