#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <string.h>
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
    double tick[6], constante = 0.0;
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

    double *A_local = (double *)malloc(stripSize * n * sizeof(double));
    double *B = (double *)malloc(n * n * sizeof(double));
    double *D_local = (double *)calloc((size_t)stripSize * n, sizeof(double));
    double *D = (double *)malloc(n * n * sizeof(double));
    double *R_local = (double *)calloc((size_t)stripSize * n, sizeof(double));
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

    MPI_Barrier(MPI_COMM_WORLD);

    tick[0] = MPI_Wtime();

    // Comunicacion fuera del parallel region
    MPI_Scatter(A_global, stripSize * n, MPI_DOUBLE, A_local, stripSize * n,
                MPI_DOUBLE, COORD, MPI_COMM_WORLD);
    MPI_Bcast(B, n * n, MPI_DOUBLE, COORD, MPI_COMM_WORLD);

    tick[1] = MPI_Wtime();

    double globalMinA, globalMaxA, globalSumA;
    double globalMinB, globalMaxB, globalSumB;
    double l_minA = DBL_MAX, l_maxA = -DBL_MAX, l_sumA = 0.0;
    double l_minB = DBL_MAX, l_maxB = -DBL_MAX, l_sumB = 0.0;

    #pragma omp parallel reduction(min:l_minA,l_minB) reduction(max:l_maxA,l_maxB) reduction(+:l_sumA,l_sumB)
    {
        // Stats de A
        #pragma omp for schedule(static)
        for (int i = 0; i < stripSize * n; i++) {
            double val = A_local[i];
            if (val < l_minA) l_minA = val;
            if (val > l_maxA) l_maxA = val;
            l_sumA += val;
        }

        // Stats de B
        #pragma omp for schedule(static)
        for (int i = 0; i < stripSize * n; i++) {
            double val = B[stripStart * n + i];
            if (val < l_minB) l_minB = val;
            if (val > l_maxB) l_maxB = val;
            l_sumB += val;
        }

        #pragma omp master
        {
            MPI_Reduce(&l_minA, &globalMinA, 1, MPI_DOUBLE, MPI_MIN, COORD, MPI_COMM_WORLD);
            MPI_Reduce(&l_maxA, &globalMaxA, 1, MPI_DOUBLE, MPI_MAX, COORD, MPI_COMM_WORLD);
            MPI_Reduce(&l_sumA, &globalSumA, 1, MPI_DOUBLE, MPI_SUM, COORD, MPI_COMM_WORLD);
            MPI_Reduce(&l_minB, &globalMinB, 1, MPI_DOUBLE, MPI_MIN, COORD, MPI_COMM_WORLD);
            MPI_Reduce(&l_maxB, &globalMaxB, 1, MPI_DOUBLE, MPI_MAX, COORD, MPI_COMM_WORLD);
            MPI_Reduce(&l_sumB, &globalSumB, 1, MPI_DOUBLE, MPI_SUM, COORD, MPI_COMM_WORLD);

            if (rank == COORD) {
                double promA = globalSumA / (double)(n * n);
                double promB = globalSumB / (double)(n * n);
                constante = (globalMaxA * globalMaxB - globalMinA * globalMinB) / (promA * promB);
            }
            MPI_Bcast(&constante, 1, MPI_DOUBLE, COORD, MPI_COMM_WORLD);
        }

        #pragma omp barrier

        // D = B x B^T (solo nuestra franja de filas, row-major)
        //   Usamos simetria de D: en Stage 2 leer D[j][k] = D_actual[k][j]
        #pragma omp for schedule(static)
        for (int ii = 0; ii < stripSize; ii += BS) {
            int i = stripStart + ii;
            int in = i * n;
            int local_in = ii * n;
            for (int j = 0; j < n; j += BS) {
                int jn = j * n;
                for (int k = 0; k < n; k += BS) {
                    blkmulRowColRow(&B[in + k], &B[k + jn],
                                    &D_local[local_in + j], n, BS);
                }
            }
        }
    }

    tick[2] = MPI_Wtime();
    // Compartir D completa via Allgather
    MPI_Allgather(D_local, stripSize * n, MPI_DOUBLE,
                  D, stripSize * n, MPI_DOUBLE, MPI_COMM_WORLD);
    tick[3] = MPI_Wtime();

    // R_local = A_local x D
    #pragma omp parallel for schedule(static)
    for (int ii = 0; ii < stripSize; ii += BS) {
        int local_in = ii * n;
        for (int j = 0; j < n; j += BS) {
            int jn = j * n;
            for (int k = 0; k < n; k += BS) {
                blkmulRowColRow(&A_local[local_in + k], &D[jn + k],
                                &R_local[local_in + j], n, BS);
            }
        }
    }

    // R *= constante
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < stripSize * n; i++)
        R_local[i] *= constante;

    tick[4] = MPI_Wtime();

    MPI_Gather(R_local, stripSize * n, MPI_DOUBLE, R_global, stripSize * n,
               MPI_DOUBLE, COORD, MPI_COMM_WORLD);

    tick[5] = MPI_Wtime();

    if (rank == COORD) {
        double totalTime = tick[5] - tick[0];
        double gflops = (2.0 * n * n * n) / (totalTime * 1e9);
        double commTime = (tick[1] - tick[0])  // Scatter + Bcast
                        + (tick[3] - tick[2])  // Allgather D
                        + (tick[5] - tick[4]); // Gather R
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

        printf("RESULT;%d;%d;%lf;%lf;%.6f%%;%s;%s\n",
               n, numProcs, totalTime, gflops, commPercent,
               speedup_str, eff_str);

        if (n <= 128)
            validate_result(A_global, B, R_global, n, BS, constante);

        if (print_matrices && n <= 4) {
            printf("\nCONSTANTE = %.6f\n", constante);
            print_matrix(A_global, n, "MATRIZ A", n);
            print_matrix(B, n, "MATRIZ B", n);
            print_matrix(D, n, "MATRIZ D = B x B^T", n);
            print_matrix(R_global, n, "MATRIZ R = A x D x k", n);
        }

        free(A_global);
        free(R_global);
    }

    free(A_local); free(B); free(D_local); free(D);
    free(R_local);
    MPI_Finalize();
    return 0;
}

double sequential_times(int n) {
    switch(n) {
        case 512:  return 0.493326;
        case 1024: return 3.931001;
        case 2048: return 31.406464;
        case 4096: return 254.568441;
        default:   return -1.0;
    }
}

void blkmulRowColRow(double *ablk, double *bblk, double *cblk, int n, int bs) {
    for (int i = 0; i < bs; i++) {
        int in = i * n;
        for (int j = 0; j < bs; j++) {
            double sum = 0.0;
            for (int k = 0; k < bs; k++)
                sum += ablk[in + k] * bblk[k + j * n];
            cblk[in + j] += sum;
        }
    }
}

void blkmulRowColCol(double *ablk, double *bblk, double *cblk, int n, int bs) {
    for (int i = 0; i < bs; i++) {
        int in = i * n;
        for (int j = 0; j < bs; j++) {
            double sum = 0.0;
            for (int k = 0; k < bs; k++)
                sum += ablk[in + k] * bblk[j * n + k];
            cblk[i + j * n] += sum;
        }
    }
}

void matmulblksRowColCol(double *a, double *b, double *c, int n, int bs) {
    for (int i = 0; i < n; i += bs) {
        int in = i * n;
        for (int j = 0; j < n; j += bs) {
            for (int k = 0; k < n; k += bs)
                blkmulRowColCol(&a[in + k], &b[k + j * n], &c[i + j * n], n, bs);
        }
    }
}

void matmulblksRowColRow(double *a, double *b, double *c, int n, int bs) {
    for (int i = 0; i < n; i += bs) {
        int in = i * n;
        for (int j = 0; j < n; j += bs) {
            for (int k = 0; k < n; k += bs)
                blkmulRowColRow(&a[in + k], &b[j * n + k], &c[in + j], n, bs);
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
