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
    int start_block;
    int end_block;
    int N;
    int T;
    int num_blocks;
} thread_args_t;

void *matmulblksRowColColParallel(void *ptr);
void *matmulblksRowColRowParallel(void *ptr);

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
    
    //row, col, col
    //primero B x BT para obtener D con column major order
    //luego A x D para obtener C con row major order 
    //inicializar el atributo de los threads
    pthread_attr_init(&attr);
    
    // Asegura que n sea múltiplo de BS para distribución uniforme de bloques
    int num_blocks = (n / BS) * (n / BS);
    
    //inicio los trabajos para primera multiplicación
    for (i = 0; i < t; i++) {
        args[i].thread_id = i;
        args[i].start_block = i * (num_blocks / t);
        args[i].end_block = (i == t - 1) ? num_blocks : (i + 1) * (num_blocks / t);
        args[i].N = n;
        args[i].T = t;
        args[i].num_blocks = num_blocks;
        pthread_create(&threads[i], &attr, matmulblksRowColColParallel, (void *) &args[i]);
    }
    //sincronizo antes de continuar
    for (i = 0; i < t; i++) {
        pthread_join(threads[i], NULL);
    }

    //solucion especializada para el escenario row, col, row
    for (i = 0; i < t; i++) {
        args[i].thread_id = i;
        args[i].start_block = i * (num_blocks / t);
        args[i].end_block = (i == t - 1) ? num_blocks : (i + 1) * (num_blocks / t);
        args[i].N = n;
        args[i].T = t;
        args[i].num_blocks = num_blocks;
        pthread_create(&threads[i], &attr, matmulblksRowColRowParallel, (void *) &args[i]);
    }
    //sincronizo antes de continuar
    for (i = 0; i < t; i++) {
        pthread_join(threads[i], NULL);
    }
    //recorrido lineal dado que es una constante matematica por una matriz
    double constante = ((MaxA * MaxB) - (MinA * MinB)) / (PromA * PromB);

    for (i = 0; i < n*n; i++) {
        r[i] = constante * c[i];
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
    // C[i + j*n] += A[i*k] * B[k + j*n]
    // Copiar argumentos localmente para evitar race condition
    thread_args_t args_local = *((thread_args_t *) ptr);
    
    int start_block = args_local.start_block;
    int end_block = args_local.end_block;
    int n = args_local.N;
    int bs = BS;
    int num_blocks_per_dim = n / bs;
    
    for (int block_id = start_block; block_id < end_block; block_id++) {
        int block_i = (block_id / num_blocks_per_dim) * bs;
        int block_j = (block_id % num_blocks_per_dim) * bs;
        
        for (int k = 0; k < n; k += bs) {
            int in = block_i * n;
            int kn = k * n;
            
            blkmulRowColCol(&b[in + k], &b[kn + block_j*n], &d[block_i + block_j*n], n, bs);
        }
    }
    pthread_exit(NULL);
}
void * matmulblksRowColRowParallel(void *ptr) {
    // Implementación especializada para A row-major, B col-major, C row-major
    // C[i*n + j] += A[i*n + k] * D[k*n + j]
    // Copiar argumentos localmente para evitar race condition
    thread_args_t args_local = *((thread_args_t *) ptr);
    
    int start_block = args_local.start_block;
    int end_block = args_local.end_block;
    int n = args_local.N;
    int bs = BS;
    int num_blocks_per_dim = n / bs;
    
    for (int block_id = start_block; block_id < end_block; block_id++) {
        int block_i = (block_id / num_blocks_per_dim) * bs;
        int block_j = (block_id % num_blocks_per_dim) * bs;
        
        for (int k = 0; k < n; k += bs) {
            int in = block_i * n;
            int kn = k * n;
            
            blkmulRowColRow(&a[in + k], &d[kn + block_j], &c[in + block_j], n, bs);
        }
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