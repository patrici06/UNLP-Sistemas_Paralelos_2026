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
        case 4:    return 0;
        case 64:   return 0;
        case 128:  return 0;
        case 256:  return 0;
        case 512:  return 0;
        case 1024: return 0;
        default:   return 0;
    }
}

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
	double commTime, totalTime, tick[14];
    double constante;

    int print_matrices = 0;
	if ((argc < 2) || ((n = atoi(argv[1])) <= 0) ) {
	    printf("\nUsar: %s size [print]\n  size: Dimension de las matrices\n  print: (opcional) 1 para imprimir matrices (solo n <= 4)\n", argv[0]);
		exit(1);
	}
    if (argc >= 3) {
        print_matrices = atoi(argv[2]);
    }

    int BS_temp = BS_FIXED;
    if (n < BS_FIXED) {
        BS_temp = n;
    }
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);


    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

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

    a = (double*) malloc(sizeof(double)*n*n);
    b = (double*) malloc(sizeof(double)*n*n);
    d = (double*) malloc(sizeof(double)*n*n);
    r = (double*) malloc(sizeof(double)*n*n);
    for (i = 0; i < n; i++) {
      for (j = 0; j < n; j++) {
        a[i*n + j] = (double)(i + j + 1);
        b[i*n + j] = (double)(i - j + 1);
        d[i + j*n] = 0.0;
        r[i*n + j] = 0.0;
       }
    }

    double *localA = (double*)malloc(sizeof(double)*stripSize*n);
    double *localB = (double*)malloc(sizeof(double)*stripSize*n);

    MPI_Barrier(MPI_COMM_WORLD);

    tick[0] = MPI_Wtime();

    MPI_Scatter(a, stripSize*n, MPI_DOUBLE, localA, stripSize*n, MPI_DOUBLE, COORDINADOR, MPI_COMM_WORLD);
    MPI_Scatter(b, stripSize*n, MPI_DOUBLE, localB, stripSize*n, MPI_DOUBLE, COORDINADOR, MPI_COMM_WORLD);

    tick[1] = MPI_Wtime();

    double localMinA = localA[0];
    double localMaxA = localA[0];
    double localSumA = 0.0;
    double localMinB = localB[0];
    double localMaxB = localB[0];
    double localSumB = 0.0;

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

    double globalMinA = localMinA;
    double globalMaxA = localMaxA;
    double globalSumA = localSumA;
    double globalMinB = localMinB;
    double globalMaxB = localMaxB;
    double globalSumB = localSumB;

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

        constante = ((globalMaxA * globalMaxB) - (globalMinA * globalMinB)) / (promA * promB);
    }

    double bcast_start = MPI_Wtime();
    MPI_Bcast(&constante, 1, MPI_DOUBLE, COORDINADOR, MPI_COMM_WORLD);
    double bcast_end = MPI_Wtime();

    // Etapa 1: Producto B * B^T -> D
    // D = B * B^T es simetrica: D[i][j] = D[j][i]
    // Usamos blkmulRowColRow para que D quede en row-major directamente.
    // Esto permite que el posterior MPI_Allgather sea directo (sin copia ni
    // linearizacion), pues las franjas de filas ya son contiguas en memoria.
    // Por la simetria, D[j][k] = D[k][j], lo que se aprovecha en Etapa 2.

    int stripStart = rank * stripSize;
    double *fullB = (double*)malloc(sizeof(double) * n * n);

    tick[4] = MPI_Wtime();

    MPI_Allgather(localB, stripSize*n, MPI_DOUBLE, fullB, stripSize*n, MPI_DOUBLE, MPI_COMM_WORLD);

    tick[5] = MPI_Wtime();

    // Producto B * B^T -> D (row-major)
    // D[i][j] = sum_k B[i][k] * B[j][k]
    // Computamos en row-major: cada fila i esta en d[i*n + 0..n-1]
    // Las filas stripStart..stripStart+stripSize-1 son contiguas.
    #pragma omp parallel for schedule(static)
    for (i = stripStart; i < stripStart + stripSize; i += BS) {
        int in = i * n;
        for (j = 0; j < n; j += BS) {
            int jn = j * n;
            for (k = 0; k < n; k += BS) {
                blkmulRowColRow(&fullB[in + k], &fullB[k + jn], &d[in + j], n, BS);
            }
        }
    }

    tick[6] = MPI_Wtime();

    // D ya esta en row-major: cada proceso tiene sus filas en d[stripStart*n..]
    // Contiguo en memoria, ideal para Allgather directo

    free(fullB);

    // Etapa 2: Producto A * D -> R
    // D completa mediante Allgather directo (row-major, franjas contiguas)
    // Luego R = A * D: R[i][j] = sum_k A[i][k] * D[k][j]
    // D[j][k] = D[k][j] por simetria, entonces usamos d[jn + k] = D[j][k]

    tick[7] = MPI_Wtime();

    // Allgather directo con MPI_IN_PLACE: cada proceso ya tiene sus filas en
    // la posicion correcta dentro de d (d[rank*stripSize*n])
    MPI_Allgather(MPI_IN_PLACE, 0, MPI_DATATYPE_NULL, d, stripSize*n, MPI_DOUBLE, MPI_COMM_WORLD);

    tick[8] = MPI_Wtime();

    // Computar R = A x D para las filas locales [stripStart, stripStart+stripSize)
    // D en row-major: d[jn + k] = D[j][k] = D[k][j] (simetria)
    // A en row-major, R en row-major
    #pragma omp parallel for schedule(static)
    for (i = stripStart; i < stripStart + stripSize; i += BS) {
        int in = i * n;
        for (j = 0; j < n; j += BS) {
            int jn = j * n;
            for (k = 0; k < n; k += BS) {
                blkmulRowColRow(&a[in + k], &d[jn + k], &r[in + j], n, BS);
            }
        }
    }

    tick[9] = MPI_Wtime();

    // Etapa 3: Multiplicacion escalar R = constante x R
    tick[10] = MPI_Wtime();

    for (i = stripStart*n; i < (stripStart+stripSize)*n; i++) {
        r[i] *= constante;
    }

    tick[11] = MPI_Wtime();

    // Gather R final al coordinador
    tick[12] = MPI_Wtime();

    double *sendR = (double*)malloc(sizeof(double) * stripSize * n);
    for (i = 0; i < stripSize * n; i++) {
        sendR[i] = r[stripStart*n + i];
    }
    MPI_Gather(sendR, stripSize*n, MPI_DOUBLE, r, stripSize*n, MPI_DOUBLE, COORDINADOR, MPI_COMM_WORLD);
    free(sendR);

    tick[13] = MPI_Wtime();

    if (rank == COORDINADOR) {
        double totalWorkTime = tick[13] - tick[0];
        double gflops = ((double)2*n*n*n) / (totalWorkTime * 1e9);

        double commOverhead = (tick[1] - tick[0]) +
                              (tick[3] - tick[2]) +
                              (bcast_end - bcast_start) +
                              (tick[5] - tick[4]) +
                              (tick[8] - tick[7]) +
                              (tick[13] - tick[12]);

        double commPercent = (commOverhead / totalWorkTime) * 100.0;

        double seq_time = sequential_times(n);
        char speedup_str[16];
        if (seq_time > 0) {
            double speedup = seq_time / totalWorkTime;
            snprintf(speedup_str, sizeof(speedup_str), "%.4f", speedup);
        } else {
            snprintf(speedup_str, sizeof(speedup_str), "N/A");
        }

        printf("RESULT;%d;%lf;%lf;%.6f%%;%s\n", n, totalWorkTime, gflops, commPercent, speedup_str);

        if (n <= 128) {
            double *refD = (double*)calloc(n * n, sizeof(double));
            double *refR = (double*)calloc(n * n, sizeof(double));

            matmulblksRowColRow(b, b, refD, n, BS);
            matmulblksRowColRow(a, refD, refR, n, BS);

            for (i = 0; i < n*n; i++) {
                refR[i] *= constante;
            }

            int diff_count = 0;
            for (i = 0; i < n; i++) {
                for (j = 0; j < n; j++) {
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

        if (print_matrices && n <= 4) {
            printf("\nCONSTANTE = %.6f\n", constante);
            print_matrix(d, n, "MATRIZ D = B x B^T", n);
            print_matrix(r, n, "MATRIZ R = A x D x k", n);
        }
    }

    free(a);
    free(b);
    free(d);
    free(r);
    free(localA);
    free(localB);

    MPI_Finalize();
    return 0;
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
