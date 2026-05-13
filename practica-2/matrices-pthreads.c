#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
#include<math.h>
#include<string.h>
#include<pthread.h>

#define BS 64

double dwalltime();
void blkmul(double *ablk, double *bblk, double *cblk, int n);
void matmulblks(double *a, double *b, double *c, int n); 
void matmulblks_parallel(double *a, double *b, double *c, int n, int num_threads);
void print_matrix(double *mat, int n, const char *name, int max_print);
void transpose_matrix(double *mat, double *mat_t, int n);
void transpose_block(double *mat, double *mat_t, int n);

//Definimos la estructura que pthread necesitara
typedef struct{
	double *matrizA; 
	double *matrizB; 
	double *resultado; 
	int n;
	int mi_id; 
	int num_threads;
} thread_args_t;

// Forward declaration de la función worker
void* worker_thread(void* arg);

// ========================
// FUNCIÓN WORKER PARA PTHREADS
// ========================
// Cada thread procesa sus filas de bloques de forma independiente
void* worker_thread(void* arg) {
    thread_args_t *args = (thread_args_t*) arg;
    int n = (int) args->n;
    int num_threads = (int) args->num_threads;
    int mi_id = args->mi_id;
    
    double *a = (double*) args->matrizA;
    double *b = args->matrizB;
    double *c = args->resultado;
    
    // Calcular rango de filas de bloques que este thread procesará
    int num_block_rows = n / BS;  // Total de filas de bloques
    int blocks_per_thread = num_block_rows / num_threads;
    int remainder = num_block_rows % num_threads;
    
    // Distribución: primeros 'remainder' threads obtienen un bloque extra
    int my_start_block, my_end_block;
    if (mi_id < remainder) {
        my_start_block = mi_id * (blocks_per_thread + 1);
        my_end_block = my_start_block + blocks_per_thread + 1;
    } else {
        my_start_block = remainder * (blocks_per_thread + 1) + (mi_id - remainder) * blocks_per_thread;
        my_end_block = my_start_block + blocks_per_thread;
    }
    
    // Convertir bloques a filas reales
    int i_start = my_start_block * BS;
    int i_end = my_end_block * BS;
    
    // Loop de multiplicación bloqueada (solo para mis filas)
    for (int i = i_start; i < i_end; i += BS) {
        int in = i * n;
        for (int k = 0; k < n; k += BS) {
            int kn = k * n;
            for (int j = 0; j < n; j += BS) {
                blkmul(&a[in + k], &b[kn + j], &c[in + j], n);
            }
        }
    }
    
    pthread_exit(NULL);
}

// ========================
// FUNCIÓN PRINCIPAL DE PARALELIZACIÓN
// ========================
// Crea num_threads threads y los sincroniza
void matmulblks_parallel(double *a, double *b, double *c, int n, int num_threads) {
    pthread_t threads[num_threads];
    thread_args_t args[num_threads];
    int i;
    
    // Crear threads
    for (i = 0; i < num_threads; i++) {
        args[i].matrizA = a;
        args[i].matrizB = b;
        args[i].resultado = c;
        args[i].n = n;
        args[i].mi_id = i;
        args[i].num_threads = num_threads;
        
        // Crear thread
        pthread_create(&threads[i], NULL, worker_thread, (void*)&args[i]);
    }
    
    // Sincronizar: esperar a que todos los threads terminen
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}


int main(int argc, char*argv[]) {
    double *a, *b, *bt, *d, *c, *r;
    int i, j, k, n, num_threads;
    int print_matrices, nan_count, inf_count;
    double MaxA, MinA, PromA, MaxB, MinB, PromB;
    double timetick, workTime;
    double constante;

    print_matrices = 0;
    num_threads = 4;  // Default

    if ((argc < 2) || ((n = atoi(argv[1])) <= 0) || ((n % BS) != 0)) {
        printf("\nError: N debe ser multiplo de BS=%d\nUsar: %s N [num_threads] [print_matrices(0|1)]\n", BS, argv[0]);
        exit(1);
    }
    
    if (argc >= 3) {
        num_threads = atoi(argv[2]);
        if (num_threads <= 0) {
            printf("Error: num_threads debe ser > 0\n");
            exit(1);
        }
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

    // a y b en row-major coherente con accesos posteriores
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            a[i*n + j] = rand() % 10 + 1;
            b[i*n + j] = rand() % 10 + 1;
        }
    }

    // d en row-major para consistencia con los cálculos
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            d[i*n + j] = 0.0;
        }
    }

    // c y r en row-major
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            c[i*n + j] = 0.0;
            r[i*n + j] = 0.0;
        }
    }

    // PRINT MATRICES (FUERA DE MEDICIÓN DE TIEMPO)
    if (print_matrices && n <= 8) {
        print_matrix(a, n, "MATRIZ A", n);
        print_matrix(b, n, "MATRIZ B", n);
    }
    
    
    // INICIA MEDICIÓN DE TIEMPO
    timetick = dwalltime();
    // TRANSPOSICIÓN DE B (dentro de medición de tiempo)
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

    // D = B^T * B usando algoritmo bloqueado paralelo
    matmulblks_parallel(bt, b, d, n, num_threads);
    
    // C = A * D usando algoritmo bloqueado paralelo
    matmulblks_parallel(a, d, c, n, num_threads);
    
    // r = constante * c (recorrido lineal)
    constante = ((MaxA * MaxB) - (MinA * MinB)) / (PromA * PromB);

    for (i = 0; i < n*n; i++) {
        r[i] = constante * c[i];
    }

    workTime = dwalltime() - timetick;
    // FIN MEDICIÓN DE TIEMPO

    // PRINT RESULTADOS (FUERA DE MEDICIÓN)
    if (print_matrices && n <= 8) {
        printf("\nCONSTANTE = (%.2f * %.2f - %.2f * %.2f) / (%.2f * %.2f) = %.6f\n",
               MaxA, MaxB, MinA, MinB, PromA, PromB, constante);
        print_matrix(d, n, "MATRIZ D = B^T * B", n);
        print_matrix(c, n, "MATRIZ C = A * D", n);
        printf("\n=== RESULTADO RESULTADO ===\n");
        print_matrix(r, n, "MATRIZ R", n);
    }

    // Validación de NaN e Inf
    nan_count = 0;
    inf_count = 0;
    for (i = 0; i < n*n; i++) {
        if (isnan(r[i])) nan_count++;
        if (isinf(r[i])) inf_count++;
    }

    printf("RESULT;%d;%lf;%lf\n", n, workTime, ((double)2*n*n*n)/(workTime*1e9));

    if (nan_count == 0 && inf_count == 0)
        printf("VALIDATION;OK\n");
    else
        printf("VALIDATION;ERROR;NaN=%d;Inf=%d\n", nan_count, inf_count);

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
//  FUNCIONES
// ========================
/* Multiply square matrices, blocked version (row-major) */
void matmulblks(double *a, double *b, double *c, int n)
{
  int i, j, k;    /* Índices de bloques */
  // c ya viene inicializada
  for (i = 0; i < n; i += BS)
  {
    int in = i * n;
    for (k = 0; k < n; k += BS)
    {
      int kn = k * n;
      for (j = 0; j < n; j += BS)
      {
        blkmul(&a[in + k], &b[kn + j], &c[in + j], n);
      }
    }
  }
}

/*****************************************************************/

/* Multiply (block)submatrices */
void blkmul(double *ablk, double *bblk, double *cblk, int n)
{
  int i, j, k;    /* Índices dentro del bloque */

  for (i = 0; i < BS; i++)
  {
    int in = i * n; 
    for (k = 0; k < BS; k++)
    {
      int kn = k * n;
      for (j = 0; j < BS; j++)
      {
        cblk[in + j] += ablk[in + k] * bblk[kn + j];
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


    
