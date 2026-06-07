#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <string.h>
#include <mpi.h>
#include <omp.h>

#define BS_FIXED 64
#define COORDINADOR 0

// Tiempos secuenciales de referencia (para calcular speedup)
// Estos valores se actualizan manualmente cuando se ejecuta matrices-open-mp.c
// con OMP_NUM_THREADS=1 para obtener tiempo secuencial puro
double sequential_times(int n) {
    // HARDCODED: Valores de tiempo secuencial en segundos (matrices-open-mp.c)
    // Formato: switch case con tiempos medidos (en segundos)
    // Para actualizar: ejecutar ./matrices-open-mp N 1
    switch(n) {
        case 4:    return -1.0;         // N/A - muy rápido
        case 64:   return -1.0;         // N/A - por definir
        case 128:  return 0.0080;       // Estimado: 2x MPI(np=2)
        case 256:  return 0.0500;       // Estimado: 2x MPI(np=2)
        case 512:  return 0.3600;       // Estimado: 2x MPI(np=2)
        case 1024: return 2.8800;       // Estimado: 2x MPI(np=2)
        default:   return -1.0;         // N/A (no disponible)
    }
} 

// ========================================
// Vamos a definir como deberia de plantearse el problema 
// Tenemos etapas nuevamente, esta son dentro de si mismas independientes
// Tenemos que paralelizar en todas las etapas por datos, y nuestro objetivo sera paralelizar las salidas. 
// ========================================
//Funciones heredadas de TP2, veremos si tienen sentido luego
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


int main(int argc, char* argv[]){
	int i, 
    j,
    k,
    numProcs, 
    rank,
    n, 
    stripSize,
    check=1;
	double *a, *b, *d, *r;
	MPI_Status status;
    //variables necesarias para calcular overhead de comunicacion, que tedioso que es
	double commTime, totalTime, tick[14];
    double constante;
	
    /* Lee parametros de la linea de comando */
    int print_matrices = 0;
	if ((argc < 2) || ((n = atoi(argv[1])) <= 0) ) {
	    printf("\nUsar: %s size [print]\n  size: Dimension de las matrices\n  print: (opcional) 1 para imprimir matrices (solo n <= 4)\n", argv[0]);
		exit(1);
	}
    if (argc >= 3) {
        print_matrices = atoi(argv[2]);
    }
    
    // Ajustar BS si n < BS_FIXED (para permitir n pequeño)
    // El flag print_matrices solo controla si se imprime
    int BS_temp = BS_FIXED;
    if (n < BS_FIXED) {
        BS_temp = n;
    }
    /* Inicializa MPI con soporte para threading (MPI_THREAD_FUNNELED)
       Necesario para usar OpenMP en paralelo con MPI
       Main thread: MPI calls (Scatter, Reduce, Allgather, Gather)
       Worker threads: OpenMP parallelism (sin MPI calls) */
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
    
    // Verificar que el sistema MPI soporta el nivel de threading requerido
    if (provided < MPI_THREAD_FUNNELED) {
        fprintf(stderr, "Error: MPI no soporta MPI_THREAD_FUNNELED\n");
        fprintf(stderr, "       Nivel soportado: %d (requerido: %d)\n", provided, MPI_THREAD_FUNNELED);
        fprintf(stderr, "       Este programa requiere MPI con soporte de threading\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // BS se usa del ajuste hecho arriba basado en print_matrices
    int BS = BS_temp;
    
    if (n % BS != 0) {
        if (rank == COORDINADOR)
            printf("Error: n (%d) debe ser multiplo de BS (%d)\n", n, BS);
        MPI_Finalize();
        return 1;
    }
    if (n % numProcs != 0) {
        if (rank == COORDINADOR)
            printf("Error: n (%d) debe ser multiplo de numProcs (%d)\n", n, numProcs);
        MPI_Finalize();
        return 1;
    }
    stripSize = n / numProcs;
    if (stripSize < BS) {
        if (rank == COORDINADOR)
            printf("Error: n/numProcs (%d) debe ser >= BS (%d)\n", stripSize, BS);
        MPI_Finalize();
        return 1;
    }

    /* Reservamos espacio en memoria de las matrices */        
    a = (double*) malloc(sizeof(double)*n*n);
    b = (double*) malloc(sizeof(double)*n*n);
    d = (double*) malloc(sizeof(double)*n*n);
    r = (double*) malloc(sizeof(double)*n*n);
    // Inicialización
    for (i = 0; i < n; i++) {
      for (j = 0; j < n; j++) {
        // Inicializamos A en row-major y B en row-major
        a[i*n + j] = (double)(i + j + 1);
        b[i*n + j] = (double)(i - j + 1);
        d[i + j*n] = 0.0;
        r[i*n + j] = 0.0;
       }
    }

    // Reservamos espacio para las porciones locales que recibiremos via Scatter
    // Logicamente tratamos la matriz n x n como un arreglo 1D de tamaño n*n
    double *localA = (double*)malloc(sizeof(double)*stripSize*n);
    double *localB = (double*)malloc(sizeof(double)*stripSize*n);

    // Sincronizamos todos los procesos antes de medir
    MPI_Barrier(MPI_COMM_WORLD);

    tick[0] = MPI_Wtime();

    //Etapa 0: Cálculo de estadísticas en paralelo usando MPI_Scatter
    //Trabajamos la amtriz en bloques contiguos de stripSize*n elementos
    MPI_Scatter(a, stripSize*n, MPI_DOUBLE, localA, stripSize*n, MPI_DOUBLE, COORDINADOR, MPI_COMM_WORLD);
    MPI_Scatter(b, stripSize*n, MPI_DOUBLE, localB, stripSize*n, MPI_DOUBLE, COORDINADOR, MPI_COMM_WORLD);

    tick[1] = MPI_Wtime();

    // Variables locales para estadisticas sobre la porcion recibida
    double localMinA = localA[0];
    double localMaxA = localA[0];
    double localSumA = 0.0;
    double localMinB = localB[0];
    double localMaxB = localB[0];
    double localSumB = 0.0;

    // Calculo local de estadisticas recorriendo el arreglo lineal local
    for (i = 0; i < stripSize*n; i++) {
        double valA = localA[i];
        double valB = localB[i];

        if (valA < localMinA) localMinA = valA;
        if (valA > localMaxA) localMaxA = valA;
        localSumA += valA;

        if (valB < localMinB) localMinB = valB;
        if (valB > localMaxB) localMaxB = valB;
        localSumB += valB;
    }

    tick[2] = MPI_Wtime();

    // Usar MPI_Reduce para calcular min, max, suma de forma eficiente
    double globalMinA = localMinA;
    double globalMaxA = localMaxA;
    double globalSumA = localSumA;
    double globalMinB = localMinB;
    double globalMaxB = localMaxB;
    double globalSumB = localSumB;

    // MPI_Reduce con operaciones predefinidas
    MPI_Reduce(&localMinA, &globalMinA, 1, MPI_DOUBLE, MPI_MIN, COORDINADOR, MPI_COMM_WORLD);
    MPI_Reduce(&localMaxA, &globalMaxA, 1, MPI_DOUBLE, MPI_MAX, COORDINADOR, MPI_COMM_WORLD);
    MPI_Reduce(&localSumA, &globalSumA, 1, MPI_DOUBLE, MPI_SUM, COORDINADOR, MPI_COMM_WORLD);
    MPI_Reduce(&localMinB, &globalMinB, 1, MPI_DOUBLE, MPI_MIN, COORDINADOR, MPI_COMM_WORLD);
    MPI_Reduce(&localMaxB, &globalMaxB, 1, MPI_DOUBLE, MPI_MAX, COORDINADOR, MPI_COMM_WORLD);
    MPI_Reduce(&localSumB, &globalSumB, 1, MPI_DOUBLE, MPI_SUM, COORDINADOR, MPI_COMM_WORLD);

    tick[3] = MPI_Wtime();

    if (rank == COORDINADOR) {
        double promA = globalSumA / (n * n);
        double promB = globalSumB / (n * n);
        
        // Calculamos la constante que usaremos en Etapa 3
        constante = ((globalMaxA * globalMaxB) - (globalMinA * globalMinB)) / (promA * promB);
    }

    // Broadcasteamos la constante a todos los procesos (contamos este overhead)
    double bcast_start = MPI_Wtime();
    MPI_Bcast(&constante, 1, MPI_DOUBLE, COORDINADOR, MPI_COMM_WORLD);
    double bcast_end = MPI_Wtime();

    // ========================================
    // Etapa 1: Producto de B por B^T -> D
    // ========================================
    // Cada proceso tiene solo su franja de B en localB (stripSize filas).
    // Para calcular D = B * B^T, cada proceso necesita la matriz B completa
    // (todas las filas), no solo su franja. Por lo tanto, usamos MPI_Allgather
    // para distribuir B completa a todos los procesos.
    // Luego cada proceso calcula su franja de D (filas stripStart..stripEnd-1)
    // usando matmulblksRowColCol (bloques row-col-col), que produce salida
    // row-major. Esto hace que cada franja sea contigua en memoria, ideal
    // para MPI_Gather.
    // Finalmente MPI_Gather recolecta las franjas de D en el coordinador.

    int stripStart = rank * stripSize;
    double *fullB = (double*)malloc(sizeof(double) * n * n);

    tick[4] = MPI_Wtime();

    // Compartimos B completa con todos los procesos via Allgather
    // Allgather es la mejor opcion porque todos los procesos necesitan B completa,
    // y Allgather la obtiene con implementacion interna ring (topologia de anillo)
    MPI_Allgather(localB, stripSize*n, MPI_DOUBLE, fullB, stripSize*n, MPI_DOUBLE, MPI_COMM_WORLD);

    tick[5] = MPI_Wtime();

    // Producto B * B^T -> D (row-col-col)
    // Usamos matmulblksRowColCol(a=B, b=B, c=D) que da salida column-major.
    // D[i][j] = sum_k B[i][k] * B[j][k]  (B^T equivale a acceder a B[j][k])
    // Cada proceso calcula su franja de filas de D (en column-major)
    // Fila i en column-major: d[i + 0*n], d[i + 1*n], ..., d[i + (n-1)*n]
    for (i = stripStart; i < stripStart + stripSize; i += BS) {
        int in = i * n;
        for (j = 0; j < n; j += BS) {
            int jn = j * n;
            for (k = 0; k < n; k += BS) {
                blkmulRowColCol(&fullB[in + k], &fullB[k + jn], &d[i + jn], n, BS);
            }
        }
    }

    tick[6] = MPI_Wtime();

    // Etapa 1 finaliza: D está distribuido en column-major
    // Cada proceso tiene su franja de filas (stripStart..stripStart+stripSize-1) 
    // almacenadas en column-major: d[i + j*n]

    free(fullB);

    // Etapa 2: Producto de A por D -> R
    // D está distribuido en column-major (desde Etapa 1 con matmulblksRowColCol).
    // MPI_Allgather D para obtener D completa en column-major.
    // Usar blkmulRowColRow directamente SIN transposición.

    tick[7] = MPI_Wtime();

    // Allgather D: cada proceso envía su franja de filas (stripSize filas en column-major)
    // Linearizar: para cada columna j, las filas stripStart..stripStart+stripSize-1
    double *sendD = (double*)malloc(sizeof(double) * stripSize * n);
    
    // Copiar franja de d (column-major) a sendD linealmente
    for (j = 0; j < n; j++) {
        for (i = 0; i < stripSize; i++) {
            sendD[i + j * stripSize] = d[stripStart + i + j * n];
        }
    }
    
    // Allgather: d recibe D completa, bloques de cada proceso
    MPI_Allgather(sendD, stripSize*n, MPI_DOUBLE, d, stripSize*n, MPI_DOUBLE, MPI_COMM_WORLD);
    free(sendD);

    tick[8] = MPI_Wtime();

    // Reorganizar d para column-major correcto
    // d contiene bloques lineales: [p=0: filas stripSize linearizadas], [p=1: ...], etc.
    // Necesitamos: dFull[i + j*n] donde i es fila global, j es columna
    double *dFull = (double*)malloc(sizeof(double) * n * n);
    for (int p = 0; p < numProcs; p++) {
        int pStripStart = p * stripSize;
        int blockOffset = p * stripSize * n;
        for (j = 0; j < n; j++) {
            for (i = 0; i < stripSize; i++) {
                // d[blockOffset + i + j*stripSize] contiene la fila pStripStart+i, columna j
                dFull[pStripStart + i + j * n] = d[blockOffset + i + j * stripSize];
            }
        }
    }

    // Computar R = A × D para las filas locales [stripStart, stripStart+stripSize)
    // usando blkmulRowColRow: A en row-major, D (dFull) en column-major, R en row-major
    for (i = stripStart; i < stripStart + stripSize; i += BS) {
        int in = i * n;
        for (j = 0; j < n; j += BS) {
            int jn = j * n;
            for (k = 0; k < n; k += BS) {
                blkmulRowColRow(&a[in + k], &dFull[jn + k], &r[in + j], n, BS);
            }
        }
    }
    
    free(dFull);

    tick[9] = MPI_Wtime();

    // ========================================
    // Etapa 3: Multiplicación escalar R = constante × R
    // ========================================
    // Trivialmente paralelizada: cada proceso multiplica su franja

    tick[10] = MPI_Wtime();

    for (i = stripStart*n; i < (stripStart+stripSize)*n; i++) {
        r[i] *= constante;
    }

    tick[11] = MPI_Wtime();

    // ========================================
    // Gather R final al coordinador
    // ========================================
    tick[12] = MPI_Wtime();

    double *sendR = (double*)malloc(sizeof(double) * stripSize * n);
    // Copiar franja local de r a sendR mediante loop explícito
    for (i = 0; i < stripSize * n; i++) {
        sendR[i] = r[stripStart*n + i];
    }
    MPI_Gather(sendR, stripSize*n, MPI_DOUBLE, r, stripSize*n, MPI_DOUBLE, COORDINADOR, MPI_COMM_WORLD);
    free(sendR);

    tick[13] = MPI_Wtime();

    // Impresión de resultado y validación (antes de liberar memoria)
    if (rank == COORDINADOR) {
        double totalWorkTime = tick[13] - tick[0];
        double gflops = ((double)2*n*n*n) / (totalWorkTime * 1e9);
        
        // Calcular overhead de comunicaciones total - TODOS los puntos de comunicación
        // Etapa 0: 
        //   - tick[0..1]: MPI_Scatter (A,B)
        //   - tick[2..3]: MPI_Reduce (min, max, sum)
        //   - bcast_start..bcast_end: MPI_Bcast (constante)
        // Etapa 1: tick[4..5]: MPI_Allgather (B completa)
        // Etapa 2: tick[7..8]: MPI_Allgather (D completa)
        // Gather final: tick[12..13]: MPI_Gather (R)
        double commOverhead = (tick[1] - tick[0]) +           // Scatter A,B
                              (tick[3] - tick[2]) +           // Reduce (min,max,sum)
                              (bcast_end - bcast_start) +     // Bcast constante
                              (tick[5] - tick[4]) +           // Allgather B
                              (tick[8] - tick[7]) +           // Allgather D
                              (tick[13] - tick[12]);          // Gather R final
        
        double commPercent = (commOverhead / totalWorkTime) * 100.0;
        
        // Calcular speedup respecto a secuencial
        double seq_time = sequential_times(n);
        char speedup_str[16];
        if (seq_time > 0) {
            double speedup = seq_time / totalWorkTime;
            snprintf(speedup_str, sizeof(speedup_str), "%.4f", speedup);
        } else {
            snprintf(speedup_str, sizeof(speedup_str), "N/A");
        }
        
        printf("RESULT;%d;%lf;%lf;%.2f%%;%.6f;%s\n", n, totalWorkTime, gflops, commPercent, constante, speedup_str);

        // ========================================
        // Validación contra referencia secuencial
        // ========================================
        if (n <= 128) {
            // Computar referencia secuencial para validación
            double *refD = (double*)calloc(n * n, sizeof(double));
            double *refR = (double*)calloc(n * n, sizeof(double));
            
            matmulblksRowColCol(b, b, refD, n, BS);
            matmulblksRowColRow(a, refD, refR, n, BS);
            
            // Aplicar constante a referencia
            for (i = 0; i < n*n; i++) {
                refR[i] *= constante;
            }
            
            // Comparar R contra refR
            int diff_count = 0;
            for (i = 0; i < n; i++) {
                for (j = 0; j < n; j++) {
                    // r: row-major, refR: row-major (ambas)
                    if (fabs(r[i*n + j] - refR[i*n + j]) > 1e-10) {
                        diff_count++;
                    }
                }
            }
            
            if (diff_count == 0)
                printf("VALIDATION;OK\n");
            else
                printf("VALIDATION;ERROR;%d diferencias en R\n", diff_count);
            
            free(refD);
            free(refR);
        }

        // ========================================
        // Impresión de matrices si print_matrices=1
        // ========================================
        if (print_matrices && n <= 4) {
            printf("\nCONSTANTE = %.6f\n", constante);
            print_matrix(d, n, "MATRIZ D = B x B^T", n);
            print_matrix(r, n, "MATRIZ R = A x D x k", n);
        }
    }

    // Liberar buffers y finalizar MPI
    free(a);
    free(b);
    free(d);
    free(r);
    free(localA);
    free(localB);

    MPI_Finalize();
    return 0;
}

// ========================================
// FUNCTION: matmulblksRowColCol
// Multiplicación B x B^T -> D (row-col-col)
// Paralelizada a nivel de bloques
// ========================================
void matmulblksRowColCol(double *a, double *b, double *c, int n, int bs) {
    int i, j, k;
    for (i = 0; i < n; i += bs) {
        int in = i*n;
        for (j = 0; j < n; j += bs) {
            int jn = j*n;    
            for (k = 0; k < n; k += bs) {
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
    for (i = 0; i < n; i += bs) {
        int in = i*n;    
        for (j = 0; j < n; j += bs) {
            int jn = j*n;
            for (k = 0; k < n; k += bs) {
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
    #pragma omp parallel for schedule(static)
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
    #pragma omp parallel for schedule(static)
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