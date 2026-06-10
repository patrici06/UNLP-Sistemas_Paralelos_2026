#include <float.h>
#include <limits.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define coordinator 0

void blkmul(double *ablk, double *bblk, double *cblk, int n, int bs);
double dwalltime();

int main(int argc, char *argv[]) {
  int N, TB;
  int numProcs, rank;

  double tick[8];
  double Comm_time;
  double Total_time;
  double Time_comp;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (argc < 3) {
    if (rank == coordinator)
      printf("Error en los parametros\n");
    MPI_Finalize();
    exit(1);
  }

  N = atoi(argv[1]);
  TB = atoi(argv[2]);

  if (N % numProcs != 0 || N % TB != 0) {
    if (rank == coordinator)
      printf("Error: N debe ser divisible por numProcs y por TB\n");
    MPI_Finalize();
    exit(1);
  }

  int nodeSize = N / numProcs;
  int localElements = nodeSize * N;
  int totalElements = N * N;

  double *B_global = (double *)malloc(totalElements * sizeof(double));
  double *BT_global = (double *)malloc(totalElements * sizeof(double));
  double *BT_local = (double *)malloc(localElements * sizeof(double));

  double *A_local = (double *)malloc(localElements * sizeof(double));
  double *Tmat_local = (double *)malloc(localElements * sizeof(double));
  double *R_local = (double *)malloc(localElements * sizeof(double));

  double *A_global = NULL;
  double *R_global = NULL;

  Total_time = dwalltime();

  if (rank == coordinator) {
    A_global = (double *)malloc(totalElements * sizeof(double));
    R_global = (double *)malloc(totalElements * sizeof(double));
    for (int i = 0; i < totalElements; i++) {
      A_global[i] = 1.0 + (rand() % 10);
      B_global[i] = 1.0 + (rand() % 10);
    }
  }

  for (int i = 0; i < localElements; i++) {
    Tmat_local[i] = 0.0;
    R_local[i] = 0.0;
  }

  MPI_Barrier(MPI_COMM_WORLD);

  tick[0] = MPI_Wtime();

  MPI_Scatter(A_global, localElements, MPI_DOUBLE, A_local, localElements,
              MPI_DOUBLE, coordinator, MPI_COMM_WORLD); //https://mpitutorial.com/tutorials/mpi-scatter-gather-and-allgather/
  MPI_Bcast(B_global, totalElements, MPI_DOUBLE, coordinator, MPI_COMM_WORLD);

  tick[1] = MPI_Wtime();

  double l_maxA = -DBL_MAX, l_minA = DBL_MAX, l_sumA = 0.0;
  for (int i = 0; i < localElements; i++) {
    if (A_local[i] > l_maxA)
      l_maxA = A_local[i];
    if (A_local[i] < l_minA)
      l_minA = A_local[i];
    l_sumA += A_local[i];
  }

  int b_start = rank * localElements;
  double l_maxB = -DBL_MAX, l_minB = DBL_MAX, l_sumB = 0.0;
  for (int i = 0; i < localElements; i++) {
    if (B_global[b_start + i] > l_maxB)
      l_maxB = B_global[b_start + i];
    if (B_global[b_start + i] < l_minB)
      l_minB = B_global[b_start + i];
    l_sumB += B_global[b_start + i];
  }

  int node_offset = rank * nodeSize * N;
  int iN_trans, idx_origen_B;

  for (int i = 0; i < nodeSize; i++) {
    iN_trans = node_offset + (i * N);
    for (int j = 0; j < N; j++) {
      idx_origen_B = iN_trans + j;
      BT_local[j * nodeSize + i] = B_global[idx_origen_B];
    }
  }

  double g_maxA, g_minA, g_sumA;
  double g_maxB, g_minB, g_sumB;

  tick[2] = MPI_Wtime();

  MPI_Reduce(&l_maxA, &g_maxA, 1, MPI_DOUBLE, MPI_MAX, coordinator,
             MPI_COMM_WORLD);
  MPI_Reduce(&l_minA, &g_minA, 1, MPI_DOUBLE, MPI_MIN, coordinator,
             MPI_COMM_WORLD);
  MPI_Reduce(&l_sumA, &g_sumA, 1, MPI_DOUBLE, MPI_SUM, coordinator,
             MPI_COMM_WORLD);

  MPI_Reduce(&l_maxB, &g_maxB, 1, MPI_DOUBLE, MPI_MAX, coordinator,
             MPI_COMM_WORLD);
  MPI_Reduce(&l_minB, &g_minB, 1, MPI_DOUBLE, MPI_MIN, coordinator,
             MPI_COMM_WORLD);
  MPI_Reduce(&l_sumB, &g_sumB, 1, MPI_DOUBLE, MPI_SUM, coordinator,
             MPI_COMM_WORLD);

  MPI_Allgather(BT_local, localElements, MPI_DOUBLE, BT_global, localElements,
                MPI_DOUBLE, MPI_COMM_WORLD);

  tick[3] = MPI_Wtime();

  double factor = 0.0;
  if (rank == coordinator) {
    double promA = g_sumA / (double)totalElements;
    double promB = g_sumB / (double)totalElements;
    factor = (g_maxA * g_maxB - g_minA * g_minB) / (promA * promB);
  }

  tick[4] = MPI_Wtime();
  MPI_Bcast(&factor, 1, MPI_DOUBLE, coordinator, MPI_COMM_WORLD);
  tick[5] = MPI_Wtime();

  int iN_mul, jN_mul;

  for (int i = 0; i < nodeSize; i += TB) {
    iN_mul = i * N;
    for (int j = 0; j < N; j += TB) {
      jN_mul = j * N;
      for (int k = 0; k < N; k += TB) {
        blkmul(&A_local[iN_mul + k], &B_global[jN_mul + k],
               &Tmat_local[iN_mul + j], N, TB);
      }
    }
  }

  for (int i = 0; i < nodeSize; i += TB) {
    iN_mul = i * N;
    for (int j = 0; j < N; j += TB) {
      jN_mul = j * N;
      for (int k = 0; k < N; k += TB) {
        blkmul(&Tmat_local[iN_mul + k], &BT_global[jN_mul + k],
               &R_local[iN_mul + j], N, TB);
      }
    }
  }

  for (int i = 0; i < localElements; i++) {
    R_local[i] *= factor;
  }

  tick[6] = MPI_Wtime();
  MPI_Gather(R_local, localElements, MPI_DOUBLE, R_global, localElements,
             MPI_DOUBLE, coordinator, MPI_COMM_WORLD);
  tick[7] = MPI_Wtime();

  Total_time = dwalltime() - Total_time;
  Comm_time = (tick[1] - tick[0]) + (tick[3] - tick[2]) + (tick[5] - tick[4]) +
              (tick[7] - tick[6]);
  Time_comp = Total_time - Comm_time;

  if (rank == coordinator) {
    printf("=== BENCHMARK MPI RESUMEN ===\n");
    printf("Matriz N: %d | Procesos: %d\n", N, numProcs);
    printf("Tiempo Total      : %f segundos\n", Total_time);
    printf("Tiempo Comunicac. : %f segundos\n", Comm_time);
    printf("Tiempo Computo    : %f segundos\n", Time_comp);
    fflush(stdout);

    free(A_global);
    free(R_global);
  }

  free(B_global);
  free(BT_global);
  free(BT_local);
  free(A_local);
  free(Tmat_local);
  free(R_local);

  MPI_Finalize();
  return 0;
}

void blkmul(double *ablk, double *bblk, double *cblk, int n, int bs) {
  int i, j, k;
  for (i = 0; i < bs; i++) {
    int iN = i * n;
    for (j = 0; j < bs; j++) {
      int jN = j * n;
      double suma = 0.0;
      for (k = 0; k < bs; k++) {
        suma += ablk[iN + k] * bblk[jN + k];
      }
      cblk[iN + j] += suma;
    }
  }
}

double dwalltime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec / 1000000.0;
}