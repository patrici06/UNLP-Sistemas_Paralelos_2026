#include<stdio.h>
#include<stdlib.h>
#include<float.h>
#include<pthread.h>
#include<sys/time.h>
#include<sched.h>
#define BS 32 // Tamaño de bloque para optimización de cache (opcional)
double * A, *B, *C, *B_T;  // B_T = B transpuesta

// Estructura para parámetros locales de cada thread
typedef struct {
    int thread_id;
    int start_row;
    int end_row;
    int N;
    int T;
} thread_args_t;

void * matrixProd(void * ptr) ;

int main (int argc, char * argv[]){
    int i, j, N, T;
    struct timeval start, end;
    double timetick, timeticksseq; 

    //control de parametros 
    if (argc != 3) {
        printf("Uso: %s N T\n  N: tamaño de las matrices\n", argv[0]);
        printf("  T: número de hilos\n");
        exit(1);
    }
    N = atoi(argv[1]);
    T = atoi(argv[2]);
    pthread_attr_t attr;
    pthread_t threads[T];
    thread_args_t args[T];  // Array de estructuras con parámetros locales
    //reservar memoria para las matrices
    A = (double *) malloc(N * N * sizeof(double));
    B = (double *) malloc(N * N * sizeof(double));
    B_T = (double *) malloc(N * N * sizeof(double));  // B transpuesta
    C = (double *) malloc(N * N * sizeof(double));
    //inicializar las matrices
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            A[i * N + j] = i * N + j * 1.0;
            B[i * N + j] = i * N + j * 2.0;
        }
    }
    
    // Transponer B para mejor cache locality
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            B_T[j * N + i] = B[i * N + j];  // B_T[j][i] = B[i][j]
        }
    }

    //medir el tiempo de ejecución secuencial con block tiling
    gettimeofday(&start, NULL);
    // Inicializar C
    for (i = 0; i < N * N; i++) {
        C[i] = 0.0;
    }
    // Block tiling: dividir en bloques de tamaño BS para mejor cache locality
    for (int ii = 0; ii < N; ii += BS) {
        for (int jj = 0; jj < N; jj += BS) {
            for (int kk = 0; kk < N; kk += BS) {
                // Loops anidados dentro de los bloques
                for (i = ii; i < (ii + BS) && i < N; i++) {
                    int iN = i * N;
                    for (j = jj; j < (jj + BS) && j < N; j++) {
                        double sum = 0.0;
                        int jN = j * N;
                        for (int k = kk; k < (kk + BS) && k < N; k++) {
                            sum += A[iN + k] * B_T[jN + k];
                        }
                        C[iN + j] += sum;  // Acumular bloques
                    }
                }
            }
        }
    }
    gettimeofday(&end, NULL);
    timeticksseq = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1e6;
    printf("Tiempo secuencial: %f segundos\n", timeticksseq);

    //inicializar el atributo de los threads
    pthread_attr_init(&attr);
    
    // Reinicializar C para versión paralela
    for (i = 0; i < N * N; i++) {
        C[i] = 0.0;
    }

    //medir el tiempo de ejecución paralela
    gettimeofday(&start, NULL);
    
    //crear los threads
    for (i = 0; i < T; i++) {
        args[i].thread_id = i;
        args[i].start_row = i * (N / T);
        args[i].end_row = (i == T - 1) ? N : (i + 1) * (N / T);
        args[i].N = N;
        args[i].T = T;
        pthread_create(&threads[i], &attr, matrixProd, (void *) &args[i]);
    }

    //esperar a que terminen todos los threads
    for (i = 0; i < T; i++) {
        pthread_join(threads[i], NULL);
    }
    
    gettimeofday(&end, NULL);
    timetick = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1e6;
    
    printf("Tiempo paralelo (%d hilos): %f segundos\n", T, timetick);
    printf("Speedup: %f\n", timeticksseq / timetick);
    printf("Eficiencia: %f%%\n", (timeticksseq / timetick) / T * 100);
   
    //liberar memoria
    pthread_attr_destroy(&attr);
    free(A);
    free(B);
    free(B_T);
    free(C);
    
    return 0;
}

void * matrixProd(void * ptr) {
    // Desempacar parámetros locales
    thread_args_t *args = (thread_args_t *) ptr;
    int id = (int) args->thread_id;
    int start = (int)args->start_row;
    int end = (int)args->end_row;
    int N = (int)args->N;
    int T = (int)args->T;
    
    // Block tiling: C[i][j] = sum(A[i][k] * B[k][j])
    // Iterar sobre bloques ii (filas), jj (columnas), kk (profundidad)
    for (int ii = start; ii < end; ii += BS) {
        for (int jj = 0; jj < N; jj += BS) {
            for (int kk = 0; kk < N; kk += BS) {
                // Loops anidados dentro de los bloques
                for (int i = ii; i < (ii + BS) && i < N; i++) {
                    int iN = i * N;
                    for (int j = jj; j < (jj + BS) && j < N; j++) {
                        double sum = 0.0;
                        int jN = j * N;
                        for (int k = kk; k < (kk + BS) && k < N; k++) {
                            sum += A[iN + k] * B_T[jN + k];  // B_T acceso secuencial dentro de bloque
                        }
                        C[iN + j] += sum;  // Acumular contribuciones de bloques kk
                    }
                }
            }
        }
    }
    
    pthread_exit(NULL);
}