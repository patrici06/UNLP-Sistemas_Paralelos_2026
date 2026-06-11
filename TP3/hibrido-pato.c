#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <float.h>
#include <mpi.h>
#include <omp.h>

#define BS_FIXED 64
#define COORD 0

double dwalltime();
double sequential_times(int n);
void blkmulRowColRow(double *ablk, double *bblk, double *cblk, int n, int bs);
void blkmulRowColCol(double *ablk, double *bblk, double *cblk, int n, int bs);
void matmulblksRowColCol(double *a, double *b, double *c, int n, int bs);
void matmulblksRowColRow(double *a, double *b, double *c, int n, int bs);
void print_matrix(double *mat, int n, const char *name, int max_print);
int validate_result(double *a, double *b, double *r, int n, int BS, double constante);

double dwalltime() {
    double sec;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    sec = tv.tv_sec + tv.tv_usec / 1000000.0;
    return sec;
}

int main(int argc, char *argv[]) {
    int numProcs, rank, n, stripSize, BS = BS_FIXED, provided, print_matrices = 0;
    double tick[8], constante = 0.0;
    int stripStart;

    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (argc < 3 || (n = atoi(argv[1])) <= 0) {
        if (rank == COORD)
            printf("Usar: %s size procs [print]\n", argv[0]);
        MPI_Finalize(); return 1;
    }
    if (argc >= 4) print_matrices = atoi(argv[3]);

    if (n % numProcs != 0) {
        if (rank == COORD) printf("Error: N debe ser multiplo de numProcs\n");
        MPI_Finalize(); return 1;
    }
    stripSize = n / numProcs;
    stripStart = rank * stripSize;

    if (stripSize < BS) BS = stripSize;
    if (n < BS) BS = n;
    if (n % BS != 0) {
        if (rank == COORD) printf("Error: N debe ser multiplo de BS (%d)\n", BS);
        MPI_Finalize(); return 1;
    }

    // Buffers locales (cada proceso tiene su franja)
    double *A_local = (double *)malloc(stripSize * n * sizeof(double));
    double *B = (double *)malloc(n * n * sizeof(double));
    double *BT_local = (double *)malloc(stripSize * n * sizeof(double));
    double *BT_global = (double *)malloc(n * n * sizeof(double));
    double *T_local = (double *)malloc((size_t)stripSize * n * sizeof(double));
    double *R_local = (double *)malloc((size_t)stripSize * n * sizeof(double));
    double *A_global = NULL, *R_global = NULL;

    if (rank == COORD) {
        A_global = (double *)malloc(n * n * sizeof(double));
        R_global = (double *)malloc(n * n * sizeof(double));
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) {
                A_global[i * n + j] = (double)(i + j + 1);
                B[i * n + j] = (double)(i - j + 1);
            }
    }

    // Init en paralelo
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < stripSize * n; i++)
        T_local[i] = R_local[i] = 0.0;

    MPI_Barrier(MPI_COMM_WORLD);

    tick[0] = MPI_Wtime();

    // Comunicacion: distribuir datos (FUERA del parallel region)
    MPI_Scatter(A_global, stripSize * n, MPI_DOUBLE, A_local, stripSize * n,
                MPI_DOUBLE, COORD, MPI_COMM_WORLD);
    MPI_Bcast(B, n * n, MPI_DOUBLE, COORD, MPI_COMM_WORLD);

    tick[1] = MPI_Wtime();

    // Variables para stats
    double globalMinA, globalMaxA, globalSumA;
    double globalMinB, globalMaxB, globalSumB;
    double l_minA = DBL_MAX, l_maxA = -DBL_MAX, l_sumA = 0.0;
    double l_minB = DBL_MAX, l_maxB = -DBL_MAX, l_sumB = 0.0;

    #pragma omp parallel reduction(min:l_minA,l_minB) reduction(max:l_maxA,l_maxB) reduction(+:l_sumA,l_sumB)
    {
        // Stats de A (nuestra franja)
        #pragma omp for schedule(static)
        for (int i = 0; i < stripSize * n; i++) {
            double val = A_local[i];
            if (val < l_minA) l_minA = val;
            if (val > l_maxA) l_maxA = val;
            l_sumA += val;
        }

        // Stats de B (nuestra franja de filas)
        #pragma omp for schedule(static)
        for (int i = 0; i < stripSize * n; i++) {
            double val = B[stripStart * n + i];
            if (val < l_minB) l_minB = val;
            if (val > l_maxB) l_maxB = val;
            l_sumB += val;
        }

        // BT_local = B^T (transponer nuestra franja de B)
        #pragma omp for schedule(static)
        for (int i = 0; i < stripSize; i++) {
            int b_row = stripStart + i;
            for (int j = 0; j < n; j++)
                BT_local[i * n + j] = B[j * n + b_row];
        }

        // Comunicacion: solo el master
        #pragma omp master
        {
            tick[2] = MPI_Wtime();

            MPI_Reduce(&l_minA, &globalMinA, 1, MPI_DOUBLE, MPI_MIN, COORD, MPI_COMM_WORLD);
            MPI_Reduce(&l_maxA, &globalMaxA, 1, MPI_DOUBLE, MPI_MAX, COORD, MPI_COMM_WORLD);
            MPI_Reduce(&l_sumA, &globalSumA, 1, MPI_DOUBLE, MPI_SUM, COORD, MPI_COMM_WORLD);
            MPI_Reduce(&l_minB, &globalMinB, 1, MPI_DOUBLE, MPI_MIN, COORD, MPI_COMM_WORLD);
            MPI_Reduce(&l_maxB, &globalMaxB, 1, MPI_DOUBLE, MPI_MAX, COORD, MPI_COMM_WORLD);
            MPI_Reduce(&l_sumB, &globalSumB, 1, MPI_DOUBLE, MPI_SUM, COORD, MPI_COMM_WORLD);

            // BT_local -> BT_global (B^T completa via Allgather)
            // Que todos dispongan de BT completa para hacer el producto
            MPI_Allgather(BT_local, stripSize * n, MPI_DOUBLE,
                          BT_global, stripSize * n, MPI_DOUBLE, MPI_COMM_WORLD);

            tick[3] = MPI_Wtime();

            if (rank == COORD) {
                double promA = globalSumA / (double)(n * n);
                double promB = globalSumB / (double)(n * n);
                constante = (globalMaxA * globalMaxB - globalMinA * globalMinB) / (promA * promB);
            }

            tick[4] = MPI_Wtime();
            MPI_Bcast(&constante, 1, MPI_DOUBLE, COORD, MPI_COMM_WORLD);
            tick[5] = MPI_Wtime();
        }

        #pragma omp barrier

        // 1er matmul: T = A x B
        // blkmulRowColRow(A_block, BT_block, T_block) -> T += A x (BT)^T = A x B
        #pragma omp for schedule(static)
        for (int ii = 0; ii < stripSize; ii += BS) {
            int local_in = ii * n;
            for (int j = 0; j < n; j += BS) {
                int jn = j * n;
                for (int k = 0; k < n; k += BS) {
                    blkmulRowColRow(&A_local[local_in + k], &BT_global[jn + k],
                                    &T_local[local_in + j], n, BS);
                }
            }
        }

        // 2do matmul: R = T x B^T
        // blkmulRowColRow(T_block, B_block, R_block) -> R += T x B^T = A x B x B^T
        #pragma omp for schedule(static)
        for (int ii = 0; ii < stripSize; ii += BS) {
            int local_in = ii * n;
            for (int j = 0; j < n; j += BS) {
                int jn = j * n;
                for (int k = 0; k < n; k += BS) {
                    blkmulRowColRow(&T_local[local_in + k], &B[jn + k],
                                    &R_local[local_in + j], n, BS);
                }
            }
        }

        // R *= constante
        #pragma omp for schedule(static)
        for (int i = 0; i < stripSize * n; i++)
            R_local[i] *= constante;
    }

    tick[6] = MPI_Wtime();
    MPI_Gather(R_local, stripSize * n, MPI_DOUBLE, R_global, stripSize * n,
               MPI_DOUBLE, COORD, MPI_COMM_WORLD);
    tick[7] = MPI_Wtime();

    // Metricas (solo coordinador)
    if (rank == COORD) {
        double totalTime = tick[7] - tick[0];
        double gflops = (2.0 * n * n * n) / (totalTime * 1e9);
        double commTime = (tick[1] - tick[0]) + (tick[3] - tick[2])
                        + (tick[5] - tick[4]) + (tick[7] - tick[6]);
        double commPercent = (commTime / totalTime) * 100.0;

        double seq_time = sequential_times(n);
        int numThreads = omp_get_max_threads();
        char speedup_str[16], eff_str[16];
        if (seq_time > 0) {
            double speedup = seq_time / totalTime;
            double efficiency = (speedup / (numProcs * numThreads)) * 100.0;
            snprintf(speedup_str, sizeof(speedup_str), "%.4f", speedup);
            snprintf(eff_str, sizeof(eff_str), "%.2f%%", efficiency);
        } else {
            snprintf(speedup_str, sizeof(speedup_str), "N/A");
            snprintf(eff_str, sizeof(eff_str), "N/A");
        }

        // Formato: RESULT;N;P;Tiempo;GFLOPS;commOverhead%;speedup;eficiencia
        printf("RESULT;%d;%d;%lf;%lf;%.6f%%;%s;%s\n",
               n, numProcs, totalTime, gflops, commPercent,
               speedup_str, eff_str);

        if (n <= 128)
            validate_result(A_global, B, R_global, n, BS, constante);

        if (print_matrices && n <= 4) {
            printf("\nCONSTANTE = %.6f\n", constante);
            print_matrix(A_global, n, "MATRIZ A", n);
            print_matrix(B, n, "MATRIZ B", n);
            print_matrix(R_global, n, "MATRIZ R = A x B x B^T x k", n);
        }

        free(A_global);
        free(R_global);
    }

    free(A_local); free(B); free(BT_local); free(BT_global);
    free(T_local); free(R_local);
    MPI_Finalize();
    return 0;
}

// ========== Funciones auxiliares ==========

double sequential_times(int n) {
    switch(n) {
        case 512:  return 2.040698;
        case 1024: return 16.383931;
        case 2048: return 131.447305;
        case 4096: return 1052.775582;
        default:   return -1.0;
    }
}

// blkmulRowColRow: c += a x b^T  (segundo argumento accedido como b[j][k])
void blkmulRowColRow(double *ablk, double *bblk, double *cblk, int n, int bs) {
    for (int i = 0; i < bs; i++) {
        int in = i * n;
        for (int j = 0; j < bs; j++) {
            double sum = 0.0;
            int jn = j * n;
            for (int k = 0; k < bs; k++)
                sum += ablk[in + k] * bblk[k + jn];
            cblk[in + j] += sum;
        }
    }
}

// blkmulRowColCol: igual pero almacena C traspuesto (c[i + jn])
void blkmulRowColCol(double *ablk, double *bblk, double *cblk, int n, int bs) {
    for (int i = 0; i < bs; i++) {
        int in = i * n;
        for (int j = 0; j < bs; j++) {
            int jn = j * n;
            double sum = 0.0;
            for (int k = 0; k < bs; k++)
                sum += ablk[in + k] * bblk[jn + k];
            cblk[i + jn] += sum;
        }
    }
}

void matmulblksRowColCol(double *a, double *b, double *c, int n, int bs) {
    for (int i = 0; i < n; i += bs) {
        int in = i * n;
        for (int j = 0; j < n; j += bs) {
            int jn = j * n;
            for (int k = 0; k < n; k += bs)
                blkmulRowColCol(&a[in + k], &b[k + jn], &c[i + jn], n, bs);
        }
    }
}

void matmulblksRowColRow(double *a, double *b, double *c, int n, int bs) {
    for (int i = 0; i < n; i += bs) {
        int in = i * n;
        for (int j = 0; j < n; j += bs) {
            int jn = j * n;
            for (int k = 0; k < n; k += bs)
                blkmulRowColRow(&a[in + k], &b[jn + k], &c[in + j], n, bs);
        }
    }
}

void print_matrix(double *mat, int n, const char *name, int max_print) {
    int limit = (n < max_print) ? n : max_print;
    printf("\n%s (%dx%d):\n", name, n, n);
    for (int i = 0; i < limit; i++) {
        printf("[ ");
        for (int j = 0; j < limit; j++)
            printf("%8.2f ", mat[i * n + j]);
        if (limit < n) printf("... ");
        printf("]\n");
    }
    if (limit < n) printf("...\n");
}

int validate_result(double *a, double *b, double *r, int n, int BS, double constante) {
    double *refD = (double *)calloc((size_t)n * n, sizeof(double));
    double *refR = (double *)calloc((size_t)n * n, sizeof(double));

    matmulblksRowColCol(b, b, refD, n, BS);
    matmulblksRowColRow(a, refD, refR, n, BS);

    for (int i = 0; i < n * n; i++)
        refR[i] *= constante;

    int diff = 0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (fabs(r[i * n + j] - refR[i * n + j]) > 1e-10)
                diff++;

    printf("%s\n", diff == 0 ? "VALIDATION;OK" : "VALIDATION;ERROR");

    free(refD); free(refR);
    return diff;
}
