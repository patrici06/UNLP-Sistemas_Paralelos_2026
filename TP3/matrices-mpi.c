#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <string.h>
#include <mpi.h>

#define BS_FIXED 64
#define COORDINADOR 0

double sequential_times(int n);
double dwalltime();
void matmulblksRowColCol(double *a, double *b, double *c, int n, int bs);
void matmulblksRowColRow(double *a, double *b, double *c, int n, int bs);
void blkmulRowColCol(double *ablk, double *bblk, double *cblk, int n, int bs);
void blkmulRowColRow(double *ablk, double *bblk, double *cblk, int n, int bs);
void print_matrix(double *mat, int n, const char *name, int max_print);

double dwalltime() {
    double sec;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    sec = tv.tv_sec + tv.tv_usec/1000000.0;
    return sec;
}

int main(int argc, char* argv[]){
    int numProcs, rank, n, stripSize;
    double constante;
    int print_matrices = 0;

    if ((argc < 2) || ((n = atoi(argv[1])) <= 0)) {
        printf("\nUsar: %s size [print]\n  size: Dimension de las matrices\n  print: (opcional) 1 para imprimir matrices (solo n <= 4)\n", argv[0]);
        exit(1);
    }
    if (argc >= 3) {
        print_matrices = atoi(argv[2]);
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (n % numProcs != 0) {
        if (rank == COORDINADOR)
            printf("Error: n debe ser multiplo de numProcs\n");
        MPI_Finalize(); return 1;
    }
    stripSize = n / numProcs;
    int stripStart = rank * stripSize;

    int BS = BS_FIXED;
    if (stripSize < BS) BS = stripSize;
    if (n < BS) BS = n;
    if (n % BS != 0) {
        if (rank == COORDINADOR)
            printf("Error: n debe ser multiplo de BS (%d)\n", BS);
        MPI_Finalize(); return 1;
    }

    double *A_local = (double*)malloc(stripSize * n * sizeof(double));
    double *B = (double*)malloc(n * n * sizeof(double));
    double *D_local = (double*)calloc((size_t)stripSize * n, sizeof(double));
    double *D = (double*)malloc(n * n * sizeof(double));
    double *R_local = (double*)calloc((size_t)stripSize * n, sizeof(double));
    double *A_global = NULL, *R_global = NULL;

    if (rank == COORDINADOR) {
        A_global = (double*)malloc(n * n * sizeof(double));
        R_global = (double*)malloc(n * n * sizeof(double));
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) {
                A_global[i*n + j] = (double)(i + j + 1);
                B[i*n + j] = (double)(i - j + 1);
            }
    }

    double tick[6];
    MPI_Barrier(MPI_COMM_WORLD);
    tick[0] = MPI_Wtime();

    // Distribuir datos
    MPI_Scatter(A_global, stripSize*n, MPI_DOUBLE, A_local, stripSize*n,
                MPI_DOUBLE, COORDINADOR, MPI_COMM_WORLD);
    MPI_Bcast(B, n*n, MPI_DOUBLE, COORDINADOR, MPI_COMM_WORLD);
    tick[1] = MPI_Wtime();

    // Estadisticas sobre la porcion local
    double localMinA = A_local[0], localMaxA = A_local[0], localSumA = 0.0;
    double localMinB = B[stripStart*n], localMaxB = B[stripStart*n], localSumB = 0.0;

    for (int i = 0; i < stripSize*n; i++) {
        double valA = A_local[i];
        if (valA < localMinA) localMinA = valA;
        if (valA > localMaxA) localMaxA = valA;
        localSumA += valA;
    }
    for (int i = 0; i < stripSize*n; i++) {
        double valB = B[stripStart*n + i];
        if (valB < localMinB) localMinB = valB;
        if (valB > localMaxB) localMaxB = valB;
        localSumB += valB;
    }

    double globalMinA, globalMaxA, globalSumA;
    double globalMinB, globalMaxB, globalSumB;
    MPI_Reduce(&localMinA, &globalMinA, 1, MPI_DOUBLE, MPI_MIN, COORDINADOR, MPI_COMM_WORLD);
    MPI_Reduce(&localMaxA, &globalMaxA, 1, MPI_DOUBLE, MPI_MAX, COORDINADOR, MPI_COMM_WORLD);
    MPI_Reduce(&localSumA, &globalSumA, 1, MPI_DOUBLE, MPI_SUM, COORDINADOR, MPI_COMM_WORLD);
    MPI_Reduce(&localMinB, &globalMinB, 1, MPI_DOUBLE, MPI_MIN, COORDINADOR, MPI_COMM_WORLD);
    MPI_Reduce(&localMaxB, &globalMaxB, 1, MPI_DOUBLE, MPI_MAX, COORDINADOR, MPI_COMM_WORLD);
    MPI_Reduce(&localSumB, &globalSumB, 1, MPI_DOUBLE, MPI_SUM, COORDINADOR, MPI_COMM_WORLD);

    if (rank == COORDINADOR) {
        double promA = globalSumA / (n * n);
        double promB = globalSumB / (n * n);
        constante = (globalMaxA * globalMaxB - globalMinA * globalMinB) / (promA * promB);
    }
    MPI_Bcast(&constante, 1, MPI_DOUBLE, COORDINADOR, MPI_COMM_WORLD);
    tick[2] = MPI_Wtime();

    // Etapa 1: D = B x B^T (solo nuestra franja de filas, row-major)
    //   Dependemos de la simetria de D: en Stage 2 leer D[j][k] = D_actual[k][j]
    for (int i = stripStart; i < stripStart + stripSize; i += BS) {
        int in = i * n;
        int local_in = (i - stripStart) * n;
        for (int j = 0; j < n; j += BS) {
            int jn = j * n;
            for (int k = 0; k < n; k += BS) {
                blkmulRowColRow(&B[in + k], &B[k + jn], &D_local[local_in + j], n, BS);
            }
        }
    }
    tick[3] = MPI_Wtime();

    // Compartir D completa via Allgather
    MPI_Allgather(D_local, stripSize*n, MPI_DOUBLE, D, stripSize*n, MPI_DOUBLE, MPI_COMM_WORLD);

    // Etapa 2: R_local = A_local x D
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
    for (int i = 0; i < stripSize * n; i++)
        R_local[i] *= constante;

    tick[4] = MPI_Wtime();

    // Gather R
    MPI_Gather(R_local, stripSize*n, MPI_DOUBLE, R_global, stripSize*n,
               MPI_DOUBLE, COORDINADOR, MPI_COMM_WORLD);

    tick[5] = MPI_Wtime();

    if (rank == COORDINADOR) {
        double totalTime = tick[5] - tick[0];
        double gflops = (2.0 * n * n * n) / (totalTime * 1e9);
        double commTime = (tick[1] - tick[0])  // Scatter + Bcast
                        + (tick[4] - tick[3])  // Allgather D
                        + (tick[5] - tick[4]); // Gather R (incluye R compute)
        double commPercent = (commTime / totalTime) * 100.0;

        double seq_time = sequential_times(n);
        char speedup_str[16], eff_str[16];
        if (seq_time > 0) {
            double speedup = seq_time / totalTime;
            double efficiency = (speedup / numProcs) * 100.0;
            snprintf(speedup_str, sizeof(speedup_str), "%.4f", speedup);
            snprintf(eff_str, sizeof(eff_str), "%.2f%%", efficiency);
        } else {
            snprintf(speedup_str, sizeof(speedup_str), "N/A");
            snprintf(eff_str, sizeof(eff_str), "N/A");
        }

        printf("RESULT;%d;%d;%lf;%lf;%.6f%%;%s;%s\n",
               n, numProcs, totalTime, gflops, commPercent,
               speedup_str, eff_str);

        if (n <= 128) {
            double *refD = (double*)calloc(n*n, sizeof(double));
            double *refR = (double*)calloc(n*n, sizeof(double));
            matmulblksRowColCol(B, B, refD, n, BS);
            matmulblksRowColRow(A_global, refD, refR, n, BS);
            for (int i = 0; i < n*n; i++) refR[i] *= constante;

            int diff = 0;
            for (int i = 0; i < n; i++)
                for (int j = 0; j < n; j++)
                    if (fabs(R_global[i*n + j] - refR[i*n + j]) > 1e-10)
                        diff++;
            printf("%s\n", diff == 0 ? "VALIDATION;OK" : "VALIDATION;ERROR");
            free(refD); free(refR);
        }

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

    free(A_local); free(B); free(D_local); free(D); free(R_local);
    MPI_Finalize();
    return 0;
}

double sequential_times(int n) {
    switch(n) {
        case 512:  return 0.493915;
        case 1024: return 3.910157;
        case 2048: return 31.394781;
        case 4096: return 255.850330;
        default:   return -1.0;
    }
}

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

void blkmulRowColCol(double *ablk, double *bblk, double *cblk, int n, int bs) {
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

void blkmulRowColRow(double *ablk, double *bblk, double *cblk, int n, int bs) {
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

void print_matrix(double *mat, int n, const char *name, int max_print) {
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
