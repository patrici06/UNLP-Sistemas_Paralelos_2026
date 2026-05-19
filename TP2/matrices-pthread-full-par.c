#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
#include<math.h>
#include<string.h>
#include<stdbool.h>
#include<pthread.h>


#define BS 64

double dwalltime();

typedef struct {
    int thread_id;
    int start_block_row;
    int end_block_row;
    int N;
    int T;
    int blocks_per_dim;
} thread_args_t;

typedef struct {
    int thread_id;
    int start_idx;
    int end_idx;
    int _pad1;  // Padding para alinear a 64 bytes y evitar false sharing
    double MaxA, MinA, PromA;
    double MaxB, MinB, PromB;
} stats_args_t;

typedef struct {
    int thread_id;
    int start_idx;
    int end_idx;
    double constante;
} scalar_args_t;

void *matmulblksRowColColParallel(void *ptr);
void *matmulblksRowColRowParallel(void *ptr);
void *calculateStatsParallel(void *ptr);
void *scalarMultiplyParallel(void *ptr);

//Mejora el performance y aprovecha optimizaciones del compilador
void blkmulRowColCol(double *ablk, double *bblk, double *cblk, int n, int bs);
void blkmulRowColRow(double *ablk, double *bblk, double *cblk, int n, int bs);

/* Print matrix with specified storage order */
void print_matrix(double *mat, int n, const char *name, int order, bool print);

double *a, *b, *d, *c, *r;

int main(int argc, char*argv[]) {
    int i, j, k, n, t;   
    double MaxA, MinA, PromA, MaxB, MinB, PromB;
    double timetick, workTime;

    if ((argc != 3) || ((n = atoi(argv[1])) <= 0 || (n % BS != 0)) || ((t = atoi(argv[2])) <= 0)) {
        printf("\nUsar: %s N - debe ser multiplo de %d\n", argv[0], BS);
        printf("\nUsar T: cantidad de hilos\n", argv[0]);
        exit(1);
    }
    pthread_attr_t attr;
    pthread_t threads[t];
    thread_args_t args[t];
    stats_args_t stats_args[t];
    scalar_args_t scalar_args[t];

    bool print = false;
    print = BS == 8;
    a = (double*) malloc(sizeof(double)*n*n);
    b = (double*) malloc(sizeof(double)*n*n);
    d = (double*) malloc(sizeof(double)*n*n);
    c = (double*) malloc(sizeof(double)*n*n);
    r = (double*) malloc(sizeof(double)*n*n);

    // Inicialización
    for (i = 0; i < n; i++) {
      for (j = 0; j < n; j++) {
        // Inicializamos A en row-major y B en row-major
        a[i*n + j] = (double)(i + j + 1);
        b[i*n + j] = (double)(i - j + 1);
       }
    }

    memset(d, 0, sizeof(double)*n*n);
    memset(c, 0, sizeof(double)*n*n);
    memset(r, 0, sizeof(double)*n*n);

    // TESTING: Print initial matrices
    if (print) {
        printf("\n========== TESTING: Matrices Iniciales ==========\n");
        print_matrix(a, n, "Matriz A (row-major)", 0, print);
        print_matrix(b, n, "Matriz B (row-major)", 0, print);
        printf("================================================\n\n");
    }

    timetick = dwalltime();

    //acceso paralelizado para calcular maximo, minimo y promedio de A y B
    PromA = PromB = 0.0;
    MaxA = MinA = a[0];
    MaxB = MinB = b[0];
    
    pthread_attr_init(&attr);
    
    // Distribución de elementos a threads para cálculo de stats
    int elements_per_thread = (n*n) / t;
    for (i = 0; i < t; i++) {
        stats_args[i].thread_id = i;
        stats_args[i].start_idx = i * elements_per_thread;
        stats_args[i].end_idx = (i == t - 1) ? (n*n) : (i + 1) * elements_per_thread;
        stats_args[i].MaxA = a[stats_args[i].start_idx];
        stats_args[i].MinA = a[stats_args[i].start_idx];
        stats_args[i].PromA = 0.0;
        stats_args[i].MaxB = b[stats_args[i].start_idx];
        stats_args[i].MinB = b[stats_args[i].start_idx];
        stats_args[i].PromB = 0.0;
        pthread_create(&threads[i], &attr, calculateStatsParallel, (void *) &stats_args[i]);
    }
    
    // Sincronizar y recopilar resultados
    for (i = 0; i < t; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Reducción secuencial de resultados parciales
    for (i = 0; i < t; i++) {
        if (stats_args[i].MaxA > MaxA) MaxA = stats_args[i].MaxA;
        if (stats_args[i].MinA < MinA) MinA = stats_args[i].MinA;
        PromA += stats_args[i].PromA;
        
        if (stats_args[i].MaxB > MaxB) MaxB = stats_args[i].MaxB;
        if (stats_args[i].MinB < MinB) MinB = stats_args[i].MinB;
        PromB += stats_args[i].PromB;
    }
    
    PromA /= (n*n);
    PromB /= (n*n);
    
    //row, col, col
    //primero B x BT para obtener D con column major order
    //luego A x D para obtener C con row major order 
    //inicializar el atributo de los threads
    
    // Distribución de filas de bloques a threads
    // blocks_per_dim: cantidad de bloques por dimensión (filas o columnas de bloques BS x BS)
    int blocks_per_dim = n / BS;
    int block_rows_per_thread = blocks_per_dim / t;
    
    //inicio los trabajos para primera multiplicación
    for (i = 0; i < t; i++) {
        args[i].thread_id = i;
        args[i].start_block_row = i * block_rows_per_thread;
        args[i].end_block_row = (i == t - 1) ? blocks_per_dim : (i + 1) * block_rows_per_thread;
        args[i].N = n;
        args[i].T = t;
        args[i].blocks_per_dim = blocks_per_dim;
        pthread_create(&threads[i], &attr, matmulblksRowColColParallel, (void *) &args[i]);
    }
    //sincronizo antes de continuar
    for (i = 0; i < t; i++) {
        pthread_join(threads[i], NULL);
    }

    //solucion especializada para el escenario row, col, row
    for (i = 0; i < t; i++) {
        args[i].thread_id = i;
        args[i].start_block_row = i * block_rows_per_thread;
        args[i].end_block_row = (i == t - 1) ? blocks_per_dim : (i + 1) * block_rows_per_thread;
        args[i].N = n;
        args[i].T = t;
        args[i].blocks_per_dim = blocks_per_dim;
        pthread_create(&threads[i], &attr, matmulblksRowColRowParallel, (void *) &args[i]);
    }
    //sincronizo antes de continuar
    for (i = 0; i < t; i++) {
        pthread_join(threads[i], NULL);
    }
    //recorrido lineal dado que es una constante matematica por una matriz
    double constante = ((MaxA * MaxB) - (MinA * MinB)) / (PromA * PromB);

    // Multiplicación escalar paralelizada
    for (i = 0; i < t; i++) {
        scalar_args[i].thread_id = i;
        scalar_args[i].start_idx = i * elements_per_thread;
        scalar_args[i].end_idx = (i == t - 1) ? (n*n) : (i + 1) * elements_per_thread;
        scalar_args[i].constante = constante;
        pthread_create(&threads[i], &attr, scalarMultiplyParallel, (void *) &scalar_args[i]);
    }
    //sincronizo antes de continuar
    for (i = 0; i < t; i++) {
        pthread_join(threads[i], NULL);
    }

    workTime = dwalltime() - timetick;

    // TESTING: Print intermediate results
    if (print) {
        printf("\n========== TESTING: D = B x B^T ==========\n");
        print_matrix(d, n, "Matriz D (column-major)", 2, print);
        printf("=========================================\n\n");
        printf("========== TESTING: C = A x D ==========\n");
        print_matrix(c, n, "Matriz C (row-major)", 0, print);
        printf("========================================\n\n");
        printf("\n========== TESTING: R = k * C ==========\n");
        printf("Constante k = %lf\n", constante);
        print_matrix(r, n, "Matriz R (row-major)", 0, print);
        printf("=======================================\n\n");
    }
    // =========================
    // VALIDACIÓN
    // =========================
    int nan_count = 0, inf_count = 0;

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
    free(d);
    free(c);
    free(r);

    return 0;
}


//implementacion de las funciones especializadas para cada escenario de orden de almacenamiento
void * matmulblksRowColColParallel(void *ptr) {
    // Implementación especializada para A row-major, B column-major, C column-major
    // Copiar argumentos localmente para evitar race condition
    thread_args_t args_local = *((thread_args_t *) ptr);
    
    int start_block_row = args_local.start_block_row;
    int end_block_row = args_local.end_block_row;
    int n = args_local.N;
    int bs = BS;
    int blocks_per_dim = args_local.blocks_per_dim;
    
    for (int i_block = start_block_row; i_block < end_block_row; i_block++) {
        int block_i = i_block * bs;
        // Orden i, k, j optimiza reutilización de caché: B[i,k] se carga y reutiliza para todos los j
        for (int k = 0; k < n; k += bs) {
            int kn = k * n;
            for (int j_block = 0; j_block < blocks_per_dim; j_block++) {
                int block_j = j_block * bs;
                int in = block_i * n;
                blkmulRowColCol(&b[in + k], &b[kn + block_j*n], &d[block_i + block_j*n], n, bs);
            }
        }
    }
    pthread_exit(NULL);
}

void * matmulblksRowColRowParallel(void *ptr) {
    // Implementación especializada para A row-major, B col-major, C row-major
    // Copiar argumentos localmente para evitar race condition
    thread_args_t args_local = *((thread_args_t *) ptr);
    
    int start_block_row = args_local.start_block_row;
    int end_block_row = args_local.end_block_row;
    int n = args_local.N;
    int bs = BS;
    int blocks_per_dim = args_local.blocks_per_dim;
    
    for (int i_block = start_block_row; i_block < end_block_row; i_block++) {
        int block_i = i_block * bs;
        // Orden i, k, j optimiza reutilización de caché: A[i,k] se carga y reutiliza para todos los j
        for (int k = 0; k < n; k += bs) {
            int kn = k * n;
            for (int j_block = 0; j_block < blocks_per_dim; j_block++) {
                int block_j = j_block * bs;
                int in = block_i * n;
                blkmulRowColRow(&a[in + k], &d[kn + block_j], &c[in + block_j], n, bs);
            }
        }
    }
    pthread_exit(NULL);
}

void * calculateStatsParallel(void *ptr) {
    // Cálculo paralelo de máximos, mínimos y promedios
    // Cada thread procesa su rango de elementos y mantiene resultados locales
    stats_args_t *args = (stats_args_t *) ptr;
    int start_idx = args->start_idx;
    int end_idx = args->end_idx;
    
    // Inicializar con el primer elemento del rango
    double MaxA_local = a[start_idx];
    double MinA_local = a[start_idx];
    double PromA_local = 0.0;
    double MaxB_local = b[start_idx];
    double MinB_local = b[start_idx];
    double PromB_local = 0.0;
    
    // Recorrer el rango asignado
    for (int i = start_idx; i < end_idx; i++) {
        double valA = a[i];
        double valB = b[i];
        
        if (valA > MaxA_local) MaxA_local = valA;
        if (valA < MinA_local) MinA_local = valA;
        PromA_local += valA;
        
        if (valB > MaxB_local) MaxB_local = valB;
        if (valB < MinB_local) MinB_local = valB;
        PromB_local += valB;
    }
    
    // Guardar resultados locales en la estructura
    args->MaxA = MaxA_local;
    args->MinA = MinA_local;
    args->PromA = PromA_local;
    args->MaxB = MaxB_local;
    args->MinB = MinB_local;
    args->PromB = PromB_local;
    
    pthread_exit(NULL);
}

void * scalarMultiplyParallel(void *ptr) {
    // Multiplicación escalar paralelizada: r[i] = constante * c[i]
    scalar_args_t *args = (scalar_args_t *) ptr;
    int start_idx = args->start_idx;
    int end_idx = args->end_idx;
    double constante = args->constante;
    
    for (int i = start_idx; i < end_idx; i++) {
        r[i] = constante * c[i];
    }
    
    pthread_exit(NULL);
}

void blkmulRowColCol(double *ablk, double *bblk, double *cblk, int n, int bs) {
    // Implementación especializada para A row-major, B column-major, C column-major
    for (int i = 0; i < bs; i++) {
        int in = i*n;
        for (int k = 0; k < bs; k++) {        
            int kn = k*n;
            double sum = 0.0;
                for (int j = 0; j < bs; j++) {
            
                    sum += ablk[in + k] * bblk[kn + j];
                }
                cblk[i + kn] += sum;
            }
    }
}

void blkmulRowColRow(double *ablk, double *bblk, double *cblk, int n, int bs) {
    // Implementación especializada para A row-major, B column-major, C row-major
    for (int i = 0; i < bs; i++) {
        int in = i*n;
        for (int k = 0; k < bs; k++) {
            double sum = 0.0;
            int kn = k*n;  
            for (int j = 0; j < bs; j++) {
                sum += ablk[in + k] * bblk[kn + j];
            }
            cblk[in + k] += sum;
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
/* Print matrix with specified storage order */
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
