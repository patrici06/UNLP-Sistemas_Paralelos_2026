#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <string.h>
#include <mpi.h>
#include <omp.h>

#define BS_FIXED 64
#define COORDINADOR 0

double sequential_times(int n) {
    switch(n) {
        //estos casos no son representativos 
        //usaremos los de la primer version temporalmente
        case 512:  return 2.040698;
        case 1024: return 16.383931;
        case 2048: return 131.447305;
        case 4096: return 1052.775582;
        default:   return -1.0;
    }
}

double dwalltime();
void matmulblksRowColCol(double *a, double *b, double *c, int n, int bs);
void matmulblksRowColRow(double *a, double *b, double *c, int n, int bs);
void blkmulRowColCol(double *ablk, double *bblk, double *cblk, int n, int ldc, int bs);
void blkmulRowColRow(double *ablk, double *bblk, double *cblk, int n, int bs);
void blkmulRowRowRow(double *ablk, double *bblk, double *cblk, int n, int bs);
void print_matrix(double *mat, int n, const char *name, int max_print);

double dwalltime() {
    double sec;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    sec = tv.tv_sec + tv.tv_usec / 1000000.0;
    return sec;
}


int main(int argc, char *argv[]) {
    int numProcs, rank, n, stripSize;
    double *a = NULL, *b = NULL,
    //d probablemente lo eliminemos
    *d = NULL,
    *r = NULL;
    MPI_Status status;
    double tick[14];
    
    double constante;

    int p, print_matrices = 0;
    if ((argc < 3) || ((n = atoi(argv[1])) <= 0) || ((p = atoi(argv[2])) <= 0)) {
        printf("\nUsar: %s size procs [print]\n"
               "  size: Dimension de las matrices\n"
               "  procs: Cantidad de procesos MPI\n"
               "  print: (opcional) 1 para imprimir matrices (solo n <= 4)\n",
               argv[0]);
        exit(1);
    }
    if (argc >= 4)
        print_matrices = atoi(argv[3]);

    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (p != numProcs) {
        if (rank == COORDINADOR)
            printf("Error: cantidad de procesos indicada (%d) no coincide con MPI_Comm_size (%d)\n", p, numProcs);
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

    int BS = BS_FIXED;
    if (stripSize < BS)
        BS = stripSize;
    if (n < BS)
        BS = n;
    if (n % BS != 0) {
        if (rank == COORDINADOR)
            printf("Error: n (%d) debe ser multiplo de BS (%d)\n", n, BS);
        MPI_Finalize();
        return 1;
    }

    int stripStart = rank * stripSize;

    //solo el coordinador tiene las matrices completas
    if (rank == COORDINADOR) {
        a = (double *)malloc(sizeof(double) * n * n);
        b = (double *)malloc(sizeof(double) * n * n);
        d = (double *)malloc(sizeof(double) * n * n);
        r = (double *)malloc(sizeof(double) * n * n);

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                a[i * n + j] = (double)(i + j + 1);
                b[i * n + j] = (double)(i - j + 1);
            }
        }
    }

    // Todos tienen sus franjas locales (tras Scatter)
    double *localA = (double *)malloc(sizeof(double) * stripSize * n);
    double *localB = (double *)malloc(sizeof(double) * stripSize * n);

    MPI_Barrier(MPI_COMM_WORLD);

    // Variables compartidas entre hilos del parallel region
    double localMinA, localMaxA, localSumA;
    double localMinB, localMaxB, localSumB;
    double globalMinA, globalMaxA, globalSumA;
    double globalMinB, globalMaxB, globalSumB;
    double *fullB = NULL;
    double *Bcomp;
    double *partialD;
    double *fullD;
    double *localR;

    tick[0] = MPI_Wtime();

    MPI_Scatter(a, stripSize * n, MPI_DOUBLE, localA, stripSize * n,
                        MPI_DOUBLE, COORDINADOR, MPI_COMM_WORLD);
    MPI_Bcast(rank == COORDINADOR ? b : fullB, n * n, MPI_DOUBLE,
                      COORDINADOR, MPI_COMM_WORLD);
    tick[1] = MPI_Wtime();
    //calculo fuera de la region paralela 
    //region paralela global, hacemos reuso de hilos 
    #pragma omp parallel
    {
        localMinA = localA[0]; localMaxA = localA[0]; localSumA = 0.0;
        localMinB = localB[0]; localMaxB = localB[0]; localSumB = 0.0;
        
        // ETAPA 0: Estadisticas (todos los hilos)
        #pragma omp for reduction(min:localMinA) reduction(max:localMaxA) reduction(+:localSumA) \
            reduction(min:localMinB) reduction(max:localMaxB) reduction(+:localSumB)
        for (int idx = 0; idx < stripSize * n; idx++) {
            double valA = localA[idx], valB = localB[idx];
            if (valA < localMinA) localMinA = valA;
            if (valA > localMaxA) localMaxA = valA;
            localSumA += valA;
            if (valB < localMinB) localMinB = valB;
            if (valB > localMaxB) localMaxB = valB;
            localSumB += valB;
        }

        #pragma omp master
        {
            MPI_Reduce(&localMinA, &globalMinA, 1, MPI_DOUBLE, MPI_MIN,
                       COORDINADOR, MPI_COMM_WORLD);
            MPI_Reduce(&localMaxA, &globalMaxA, 1, MPI_DOUBLE, MPI_MAX,
                       COORDINADOR, MPI_COMM_WORLD);
            MPI_Reduce(&localSumA, &globalSumA, 1, MPI_DOUBLE, MPI_SUM,
                       COORDINADOR, MPI_COMM_WORLD);
            MPI_Reduce(&localMinB, &globalMinB, 1, MPI_DOUBLE, MPI_MIN,
                       COORDINADOR, MPI_COMM_WORLD);
            MPI_Reduce(&localMaxB, &globalMaxB, 1, MPI_DOUBLE, MPI_MAX,
                       COORDINADOR, MPI_COMM_WORLD);
            MPI_Reduce(&localSumB, &globalSumB, 1, MPI_DOUBLE, MPI_SUM,
                       COORDINADOR, MPI_COMM_WORLD);

            if (rank == COORDINADOR) {
                double promA = globalSumA / (double)(n * n);
                double promB = globalSumB / (double)(n * n);
                constante = ((globalMaxA * globalMaxB) - (globalMinA * globalMinB))
                            / (promA * promB);
            }
            MPI_Bcast(&constante, 1, MPI_DOUBLE, COORDINADOR, MPI_COMM_WORLD);
        }

        #pragma omp barrier

        // ETAPA 1: Setup (master)
        #pragma omp master
        {
            if (rank != COORDINADOR)
                fullB = (double *)malloc(sizeof(double) * n * n);

            tick[4] = MPI_Wtime();

            
            tick[5] = MPI_Wtime();

            Bcomp = (rank == COORDINADOR) ? b : fullB;
            partialD = (double *)calloc((size_t)stripSize * n, sizeof(double));
        }
        #pragma omp barrier
        //como todo lo realizamos en una porcion de la matriz, 
        //transponemos nuestra parte y usamos esta transpuesta. 
        double * BT = (double *)malloc(sizeof(double) * stripSize * n);

        // D = B x B^T (todos los hilos)
        #pragma omp for schedule(static) //grano grueso
        for (int i = stripStart; i < stripStart + stripSize; i += BS) {
            int in = i * n;
            for (int j = 0; j < n; j += BS) {
                int jn = j * n;
                for (int k = 0; k < n; k += BS) {
                    blkmulRowColRow(&Bcomp[in + k], &Bcomp[k + jn],
                                    &partialD[(i - stripStart) * n + j], n, BS);
                }
            }
        }

        #pragma omp master
        {
            tick[6] = MPI_Wtime();
            if (fullB) free(fullB);
            tick[7] = MPI_Wtime();

            fullD = (double *)malloc(sizeof(double) * n * n);
            MPI_Allgather(partialD, stripSize * n, MPI_DOUBLE,
                          fullD, stripSize * n, MPI_DOUBLE, MPI_COMM_WORLD);
            free(partialD);

            tick[8] = MPI_Wtime();
        }
        #pragma omp barrier

        // ETAPA 2: Setup (master)
        #pragma omp master
        {
            localR = (double *)calloc((size_t)stripSize * n, sizeof(double));
        }
        #pragma omp barrier

        // R = A x D (todos los hilos)
        #pragma omp for schedule(static)
        for (int i = 0; i < stripSize; i += BS) {
            int in = i * n;
            for (int j = 0; j < n; j += BS) {
                int jn = j * n;
                for (int k = 0; k < n; k += BS) {
                    blkmulRowColRow(&localA[in + k], &fullD[k + jn],
                                    &localR[in + j], n, BS);
                }
            }
        }

        #pragma omp master
        {
            tick[9] = MPI_Wtime();
            tick[10] = MPI_Wtime();
        }

        // ETAPA 3: R = constante * R (todos los hilos)
        #pragma omp for
        for (int idx = 0; idx < stripSize * n; idx++) {
            localR[idx] *= constante;
        }

        #pragma omp master
        {
            tick[11] = MPI_Wtime();
            tick[12] = MPI_Wtime();
            MPI_Gather(localR, stripSize * n, MPI_DOUBLE,
                       r, stripSize * n, MPI_DOUBLE, COORDINADOR, MPI_COMM_WORLD);
            tick[13] = MPI_Wtime();
        }
        //es solo un Gather, no es necesario sincronizar con una barrera, el master se encarga de medir el tiempo de esta etapa y luego cada proceso puede continuar con su trabajo de liberar memoria local.
    
    //liberamos todo
    free(localA);
    free(localB);
    free(fullD);
    free(localR);
    free(fullB);
    free(partialD);
    free(Bcomp);
    free(d);
    free(r);
    free(a);
    free(b);
    }
    //  (solo coordinador)
    if (rank == COORDINADOR) {
        double totalWorkTime = tick[13] - tick[0];
        double gflops = (2.0 * n * n * n) / (totalWorkTime * 1e9);

        double commOverhead = (tick[1] - tick[0])
                            + (tick[5] - tick[4])
                            + (tick[8] - tick[7])
                            + (tick[13] - tick[12]);
        double commPercent = (commOverhead / totalWorkTime) * 100.0;

        double seq_time = sequential_times(n);
        char speedup_str[16], efficiency_str[16];
        if (seq_time > 0) {
            double speedup = seq_time / totalWorkTime;
            double efficiency = (speedup / numProcs) * 100.0;
            snprintf(speedup_str, sizeof(speedup_str), "%.4f", speedup);
            snprintf(efficiency_str, sizeof(efficiency_str), "%.2f%%", efficiency);
        } else {
            snprintf(speedup_str, sizeof(speedup_str), "N/A");
            snprintf(efficiency_str, sizeof(efficiency_str), "N/A");
        }

        printf("RESULT;%d;%d;%lf;%lf;%.6f%%;%s;%s\n",
               n, numProcs, totalWorkTime, gflops,
               commPercent, speedup_str, efficiency_str);

        if (n <= 128) {
            double *refD = (double *)calloc((size_t)n * n, sizeof(double));
            double *refR = (double *)calloc((size_t)n * n, sizeof(double));

            matmulblksRowColCol(b, b, refD, n, BS);
            matmulblksRowColRow(a, refD, refR, n, BS);

            for (int i = 0; i < n * n; i++)
                refR[i] *= constante;

            int diff_count = 0;
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    if (fabs(r[i * n + j] - refR[i * n + j]) > 1e-10)
                        diff_count++;
                }
            }

            printf("%s\n", diff_count == 0 ? "VALIDATION;OK" : "VALIDATION;ERROR");

            free(refD);
            free(refR);
        }

        if (print_matrices && n <= 4) {
            printf("\nCONSTANTE = %.6f\n", constante);
            print_matrix(a, n, "MATRIZ A", n);
            print_matrix(b, n, "MATRIZ B", n);
            print_matrix(fullD, n, "MATRIZ D = B x B^T", n);
            printf("MATRIZ D fisicamente:\n");
            for (int i = 0; i < n * n; i++)
                printf("%.2f ", fullD[i]);
            print_matrix(r, n, "MATRIZ R = A x D x k", n);
        }
    }
     //Cada proceso libera su memoria local, el coordinador libera las matrices completas
     if (rank == COORDINADOR) {
         free(a);
         free(b);
         free(d);
         free(r);
     }
    free(localA);
    free(localB);
    free(fullD);
    free(localR);

    MPI_Finalize();
    return 0;
}

// ======================================================================
// FUNCIONES DE MULTIPLICACION POR BLOQUES
// ======================================================================

void matmulblksRowColCol(double *a, double *b, double *c, int n, int bs) {
    for (int i = 0; i < n; i += bs) {
        int in = i * n;
        for (int j = 0; j < n; j += bs) {
            int jn = j * n;
            for (int k = 0; k < n; k += bs) {
                blkmulRowColCol(&a[in + k], &b[k + jn], &c[i + jn], n, n, bs);
            }
        }
    }
}

void matmulblksRowColRow(double *a, double *b, double *c, int n, int bs) {
    for (int i = 0; i < n; i += bs) {
        int in = i * n;
        for (int j = 0; j < n; j += bs) {
            int jn = j * n;
            for (int k = 0; k < n; k += bs) {
                blkmulRowColRow(&a[in + k], &b[jn + k], &c[in + j], n, bs);
            }
        }
    }
}

// void matmulblksRowRowRow(double *a, double *b, double *c, int n, int bs) {
//     for (int i = 0; i < n; i += bs) {
//         int in = i * n;
//         for (int j = 0; j < n; j += bs) {
//             int jn = j * n;
//             for (int k = 0; k < n; k += bs) {
//                 blkmulRowRowRow(&a[in + k], &b[j * n], &c[in + j], n, bs);
//             }
//         }
//     }
// }

/*
 * blkmulRowColCol: C += A × B  (bloques)
 *   A: row-major, stride n
 *   B: row-major, stride n
 *   C: column-major, stride ldc (parametrizable)
 *   ldc = n    para escribir en una matriz n×n column-major clasica
 *   ldc = ss   para escribir en un buffer column-major stride
 *              (ss = stripSize)
 */

void blkmulRowColCol(double *ablk, double *bblk, double *cblk,
                     int n, int ldc, int bs)
{
    for (int i = 0; i < bs; i++) {
        int in = i * n;
        for (int j = 0; j < bs; j++) {
            int jn = j * n;
            int jc = j * ldc;
            double sum = 0.0;
            for (int k = 0; k < bs; k++) {
                sum += ablk[in + k] * bblk[jn + k];
            }
            //desplazamiento necesario para que al Allgather posterior quede en formato column-major clasico, 
            //cuestiones que no puedo controlar de MPI que me obliga a escribir en formato compacto 
            cblk[i + jc] += sum;
        }
    }
}

void blkmulRowColRow(double *ablk, double *bblk, double *cblk, int n, int bs) {
    for (int i = 0; i < bs; i++) {
        int in = i * n;
        for (int j = 0; j < bs; j++) {
            double sum = 0.0;
            int jn = j * n;
            for (int k = 0; k < bs; k++) {
                // ablk: row-major, A[i+i_local][k+k_local] = ablk[in + k_local]
                // bblk apunta a fullD[k + j*n], acceso column-major: D[k+k_local][j+j_local] = bblk[k_local + jn]
                sum += ablk[in + k] * bblk[k + jn];
            }
            cblk[in + j] += sum;
        }
    }
}

void blkmulRowRowRow(double *ablk, double *bblk, double *cblk, int n, int bs) {
    // C += A × B, todos en row-major
    // ablk[i*n + k] = A[i][k]
    // bblk[k*n + j] = B[k][j]
    // cblk[i*n + j] = C[i][j]
    for (int i = 0; i < bs; i++) {
        int in = i * n;
        for (int j = 0; j < bs; j++) {
            double sum = 0.0;
            for (int k = 0; k < bs; k++) {
                sum += ablk[in + k] * bblk[k * n + j];
            }
            cblk[in + j] += sum;
        }
    }
}

void print_matrix(double *mat, int n, const char *name, int max_print) {
    int limit = (n < max_print) ? n : max_print;
    printf("\n%s (%dx%d):\n", name, n, n);
    for (int i = 0; i < limit; i++) {
        printf("[ ");
        for (int j = 0; j < limit; j++) {
            printf("%8.2f ", mat[i * n + j]);
        }
        if (limit < n)
            printf("... ");
        printf("]\n");
    }
    if (limit < n)
        printf("...\n");
}
