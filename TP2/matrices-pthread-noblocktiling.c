#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
#include<math.h>
#include<string.h>
#include<stdbool.h>
#include<pthread.h>

// SIN BLOCK TILING - versión naive para comparación

// ========================================
// VARIABLES GLOBALES
// ========================================
pthread_mutex_t mutex_metrics = PTHREAD_MUTEX_INITIALIZER;
double g_maxA = -999999999, g_minA = 999999999, g_sumA = 0.0;
double g_maxB = -999999999, g_minB = 999999999, g_sumB = 0.0;
double factor_final = 0.0;

pthread_barrier_t barrier_sync;
static double ref_time_sequential = -1.0;

// ========================================
// ESTRUCTURAS
// ========================================
typedef struct {
    int id;
    int N;
    int T;
    int chunk;
    double *A;
    double *B;
    double *D;
    double *R;
} thread_args_t;

// ========================================
// DECLARACIONES
// ========================================
double dwalltime();
void *thread_worker(void *ptr);

// ========================================
// MAIN
// ========================================
int main(int argc, char*argv[]) {
    int i, n, t;
    double timetick, workTime;

    if ((argc != 3) || ((n = atoi(argv[1])) <= 0) || ((t = atoi(argv[2])) <= 0)) {
        printf("Uso: %s N T\n", argv[0]);
        printf("N: tamaño de matriz, T: cantidad de hilos\n");
        exit(1);
    }

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

    pthread_attr_t attr;
    pthread_t threads[t];
    thread_args_t thread_args[t];
    
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

    // =========================
    // VALIDACIÓN
    // =========================
    
    int nan_count = 0, inf_count = 0;

    for (i = 0; i < n*n; i++) {
        if (isnan(R[i])) nan_count++;
        if (isinf(R[i])) inf_count++;
    }

    double gflops = ((double)2*n*n*n)/(workTime*1e9);
    
    double speedup = 1.0;
    double efficiency = 100.0;
    
    if (t == 1) {
        ref_time_sequential = workTime;
    } else if (ref_time_sequential > 0) {
        speedup = ref_time_sequential / workTime;
        efficiency = (speedup / (double)t) * 100.0;
    }
    
    printf("RESULT;%d;%d;%lf;%lf;%lf;%lf\n", n, t, workTime, gflops, speedup, efficiency);
    printf("CONSTANTE_K;%lf\n", factor_final);

    if (nan_count == 0 && inf_count == 0)
        printf("VALIDATION;OK\n");
    else
        printf("VALIDATION;ERROR;NaN=%d;Inf=%d\n", nan_count, inf_count);

    free(A);
    free(B);
    free(D);
    free(R);
    
    pthread_mutex_destroy(&mutex_metrics);

    return 0;
}

// ========================================
// FUNCTION: thread_worker (SIN BLOCK TILING)
// Multiplicación naive elemento por elemento
// ========================================
void *thread_worker(void *ptr) {
    thread_args_t *p = (thread_args_t*)ptr;
    
    int id = (int)p->id;
    int N = (int)p->N;
    int T = (int)p->T;
    int chunk = (int)p->chunk;
    double *A = (double*)p->A;
    double *B = (double*)p->B;
    double *D = (double*)p->D;
    double *R = (double*)p->R;
    
    int start = id * chunk;
    int end = (id == T - 1) ? N : (id + 1) * chunk;

    // ========================================
    // ETAPA 0: Cálculo LOCAL de métricas
    // ========================================
    double l_maxA = -999999999, l_minA = 999999999, l_sumA = 0.0;
    double l_maxB = -999999999, l_minB = 999999999, l_sumB = 0.0;
    
    for (int i = start * N; i < end * N; i++) {
        if (A[i] > l_maxA) l_maxA = A[i];
        if (A[i] < l_minA) l_minA = A[i];
        l_sumA += A[i];
        
        if (B[i] > l_maxB) l_maxB = B[i];
        if (B[i] < l_minB) l_minB = B[i];
        l_sumB += B[i];
    }
    
    pthread_mutex_lock(&mutex_metrics);
    if (l_maxA > g_maxA) g_maxA = l_maxA;
    if (l_minA < g_minA) g_minA = l_minA;
    g_sumA += l_sumA;
    if (l_maxB > g_maxB) g_maxB = l_maxB;
    if (l_minB < g_minB) g_minB = l_minB;
    g_sumB += l_sumB;
    pthread_mutex_unlock(&mutex_metrics);
    
    pthread_barrier_wait(&barrier_sync);
    
    // Solo hilo 0 calcula la CONSTANTE
    if (id == 0) {
        double promA = g_sumA / (double)(N * N);
        double promB = g_sumB / (double)(N * N);
        factor_final = (g_maxA * g_maxB - g_minA * g_minB) / (promA * promB);
    }
    
    pthread_barrier_wait(&barrier_sync);
    
    // ========================================
    // ETAPA 1: Multiplicación NAIVE B x B^T -> D
    // Sin block tiling - elemento por elemento
    // ========================================
    for (int i = start; i < end; i++) {
        int in = i * N;
        for (int j = 0; j < N; j++) {
            int jn = j * N;
            double sum = 0.0;
            for (int k = 0; k < N; k++) {
                sum += B[in + k] * B[k + jn];
            }
            D[i + jn] += sum;
        }
    }
    
    pthread_barrier_wait(&barrier_sync);
    
    // ========================================
    // ETAPA 2: Multiplicación NAIVE A x D -> R
    // Sin block tiling - elemento por elemento
    // ========================================
    for (int i = start; i < end; i++) {
        int in = i * N;
        for (int j = 0; j < N; j++) {
            int jn = j * N;
            double sum = 0.0;
            for (int k = 0; k < N; k++) {
                sum += A[in + k] * D[jn + k];
            }
            R[in + j] += sum;
        }
    }
    
    pthread_barrier_wait(&barrier_sync);
    
    // ========================================
    // ETAPA 3: Aplicar factor_final
    // ========================================
    for (int i = start * N; i < end * N; i++) {
        R[i] *= factor_final;
    }
    
    pthread_exit(NULL);
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
