#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
#include<math.h>
#include<string.h>
#include<pthread.h>

#define BS 32

typedef struct {
    double *a, *b, *bt, *d, *c, *r;
    int n;
    int thread_id;
    int num_threads;
    double MaxA, MinA, PromA, MaxB, MinB, PromB;
    double constante;
} thread_args_t;

double dwalltime();
void blkmul(double *ablk, double *bblk, double *cblk, int n);
void matmulblks(double *a, double *b, double *c, int n, int thread_id, int num_threads);
void print_matrix(double *mat, int n, const char *name, int max_print);
void transpose_matrix(double *mat, double *mat_t, int n);
void transpose_block(double *mat, double *mat_t, int n);
void* worker_thread(void* args);

int main(int argc, char*argv[]) {
    double *a, *b, *bt, *d, *c, *r;
    int i, j, k, n, num_threads;
    int print_matrices, nan_count, inf_count;
    double MaxA, MinA, PromA, MaxB, MinB, PromB;
    double timetick, workTime;
    double constante;
    pthread_t *threads;
    thread_args_t *thread_args;

    print_matrices = 0;
    num_threads = 2;

    if ((argc < 2) || ((n = atoi(argv[1])) <= 0) || ((n % BS) != 0)) {
        printf("\nError: N debe ser multiplo de BS=%d\nUsar: %s N [num_threads(default=2)] [print_matrices(0|1)]\n", BS, argv[0]);
        exit(1);
    }
    
    if (argc >= 3) {
        num_threads = atoi(argv[2]);
    }
    
    if (argc >= 4) {
        print_matrices = atoi(argv[3]);
    }

    a = (double*) malloc(sizeof(double)*n*n);
    b = (double*) malloc(sizeof(double)*n*n);
    bt = (double*) malloc(sizeof(double)*n*n);
    d = (double*) malloc(sizeof(double)*n*n);
    c = (double*) malloc(sizeof(double)*n*n);
    r = (double*) malloc(sizeof(double)*n*n);

    // a y b en row-major
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            a[i*n + j] = rand() % 10 + 1;
            b[i*n + j] = rand() % 10 + 1;
        }
    }

    // d, c y r inicializadas
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            d[i*n + j] = 0.0;
            c[i*n + j] = 0.0;
            r[i*n + j] = 0.0;
        }
    }

    if (print_matrices && n <= 8) {
        print_matrix(a, n, "MATRIZ A", n);
        print_matrix(b, n, "MATRIZ B", n);
    }
    
    // INICIA MEDICIÓN DE TIEMPO
    timetick = dwalltime();
    
    // TRANSPOSICIÓN DE B (secuencial, bajo overhead relativo)
    transpose_matrix(b, bt, n);
    
    //acceso secuencial para calcular maximo, minimo y promedio de A y B
    PromA = PromB = 0.0;
    MaxA = MinA = a[0];
    MaxB = MinB = b[0];

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

    PromA /= (n*n);
    PromB /= (n*n);

    // Crear threads para multiplicaciones matriciales
    threads = (pthread_t*) malloc(num_threads * sizeof(pthread_t));
    thread_args = (thread_args_t*) malloc(num_threads * sizeof(thread_args_t));

    // D = B^T * B (paralelo con block tiling)
    for (i = 0; i < num_threads; i++) {
        thread_args[i].a = bt;
        thread_args[i].b = b;
        thread_args[i].c = d;
        thread_args[i].n = n;
        thread_args[i].thread_id = i;
        thread_args[i].num_threads = num_threads;
        pthread_create(&threads[i], NULL, worker_thread, &thread_args[i]);
    }
    
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // C = A * D (paralelo con block tiling)
    for (i = 0; i < num_threads; i++) {
        thread_args[i].a = a;
        thread_args[i].b = d;
        thread_args[i].c = c;
        thread_args[i].n = n;
        thread_args[i].thread_id = i;
        thread_args[i].num_threads = num_threads;
        pthread_create(&threads[i], NULL, worker_thread, &thread_args[i]);
    }
    
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // r = constante * c (secuencial, es linear scan)
    constante = ((MaxA * MaxB) - (MinA * MinB)) / (PromA * PromB);

    for (i = 0; i < n*n; i++) {
        r[i] = constante * c[i];
    }

    workTime = dwalltime() - timetick;
    // FIN MEDICIÓN DE TIEMPO

    if (print_matrices && n <= 8) {
        printf("\nCONSTANTE = (%.2f * %.2f - %.2f * %.2f) / (%.2f * %.2f) = %.6f\n",
               MaxA, MaxB, MinA, MinB, PromA, PromB, constante);
        print_matrix(d, n, "MATRIZ D = B^T * B", n);
        print_matrix(c, n, "MATRIZ C = A * D", n);
        printf("\n=== RESULTADO ===\n");
        print_matrix(r, n, "MATRIZ R", n);
    }

    // Validación de NaN e Inf
    nan_count = 0;
    inf_count = 0;
    for (i = 0; i < n*n; i++) {
        if (isnan(r[i])) nan_count++;
        if (isinf(r[i])) inf_count++;
    }

    printf("RESULT;%d;%d;%lf;%lf\n", n, num_threads, workTime, ((double)2*n*n*n)/(workTime*1e9));

    if (nan_count == 0 && inf_count == 0)
        printf("VALIDATION;OK\n");
    else
        printf("VALIDATION;ERROR;NaN=%d;Inf=%d\n", nan_count, inf_count);

    free(threads);
    free(thread_args);
    free(a);
    free(b);
    free(bt);
    free(d);
    free(c);
    free(r);

    return 0;
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

// ========================
// WORKER THREAD
// ========================
void* worker_thread(void* args)
{
    thread_args_t *targs = (thread_args_t*) args;
    matmulblks(targs->a, targs->b, targs->c, targs->n, targs->thread_id, targs->num_threads);
    return NULL;
}

// ========================
//  FUNCIONES
// ========================
/* Multiply square matrices, blocked version (row-major) - PARALELO */
void matmulblks(double *a, double *b, double *c, int n, int thread_id, int num_threads)
{
    int i, j, k;
    int blocks_per_thread = (n / BS) / num_threads;
    int start_block = thread_id * blocks_per_thread;
    int end_block = (thread_id == num_threads - 1) ? (n / BS) : (start_block + blocks_per_thread);
    
    for (i = start_block * BS; i < end_block * BS; i += BS) {
        int in = i * n;
        for (k = 0; k < n; k += BS) {
            int kn = k * n;
            for (j = 0; j < n; j += BS) {
                blkmul(&a[in + k], &b[kn + j], &c[in + j], n);
            }
        }
    }
}

/* Multiply (block)submatrices */
void blkmul(double *ablk, double *bblk, double *cblk, int n)
{
    int i, j, k;

    for (i = 0; i < BS; i++) {
        int in = i * n;
        for (k = 0; k < BS; k++) {
            int kn = k * n;
            double a_ik = ablk[in + k];
            for (j = 0; j < BS; j++) {
                cblk[in + j] += a_ik * bblk[kn + j];
            }
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

/* Transpone un bloque de tamaño BS x BS */
void transpose_block(double *mat, double *mat_t, int n)
{
    int i, j;
    
    for (i = 0; i < BS; i++) {
        int in = i * n;
        for (j = 0; j < BS; j++) {
            mat_t[j*n + i] = mat[in + j];
        }
    }
}

/* Transpone una matriz de n x n con block tiling para mejor cache locality */
void transpose_matrix(double *mat, double *mat_t, int n)
{
    int bi, bj;
    
    for (bi = 0; bi < n; bi += BS) {
        for (bj = 0; bj < n; bj += BS) {
            transpose_block(&mat[bi*n + bj], &mat_t[bj*n + bi], n);
        }
    }
}
