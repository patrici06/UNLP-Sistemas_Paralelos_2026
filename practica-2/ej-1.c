#include<stdio.h>
#include<stdlib.h>
#include<float.h>
#include<pthread.h>
#include<sys/time.h>

void * vectorAdd(void * ptr) ;
double * A, *B, *C;
int N, T;

void * vectorAdd(void * ptr) {
    int id = *((int *) ptr);
    int start = id * (N / T);
    int end = (id == T - 1) ? N : (id + 1) * (N / T);
    
    for (int i = start; i < end; i++) {
        C[i] = A[i] + B[i];
    }
    
    pthread_exit(NULL);
} 

int main (int argc, char * argv[]){
    int i, j; 
    struct timeval start, end;
    double timetick, timeticksseq; 

    //control de parametros 
    if (argc != 3) {
        printf("Uso: %s N T\n  N: tamaño de los vectores\n", argv[0]);
        printf("  T: número de hilos\n");
        exit(1);
    }
    N = atoi(argv[1]);
    T = atoi(argv[2]);
    pthread_attr_t attr;
    pthread_t threads[T];
    int ids[T];
    //reservar memoria para los vectores
    A = (double *) malloc(N * sizeof(double));
    B = (double *) malloc(N * sizeof(double));
    C = (double *) malloc(N * sizeof(double));
    //inicializar los vectores
    for (i = 0; i < N; i++) {
        A[i] = i * 1.0;
        B[i] = i * 2.0;
    }
    //medir el tiempo de ejecución secuencial
    gettimeofday(&start, NULL);
    for (i = 0; i < N; i++) {
        C[i] = A[i] + B[i];
    }
    gettimeofday(&end, NULL);
    timeticksseq = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1e6;
    printf("Tiempo secuencial: %f segundos\n", timeticksseq);

    //inicializar el atributo de los threads
    pthread_attr_init(&attr);

    //medir el tiempo de ejecución paralela
    gettimeofday(&start, NULL);
    
    //crear los threads
    for (i = 0; i < T; i++) {
        ids[i] = i;
        pthread_create(&threads[i], &attr, vectorAdd, (void *) &ids[i]);
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
    printf ("Eficiencia: %f\n", (timeticksseq / timetick) / T);
    
    //liberar memoria
    pthread_attr_destroy(&attr);
    free(A);
    free(B);
    free(C);
    
    return 0;
}