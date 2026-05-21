#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

typedef struct {
    int id, N, T, TB;
    double *A, *B, *BT, *Tmat, *R;
} thread_params;

pthread_mutex_t mutex_metrics;
pthread_barrier_t barrier_sync;
double g_maxA = -1.0, g_minA = 9999999, g_sumA = 0;
double g_maxB = -1.0, g_minB = 9999999, g_sumB = 0;
double factor_final;

double dwalltime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

void blkmul(double *ablk, double *bblk, double *cblk, int n, int bs) {
    int i, j, k;
    for (i = 0; i < bs; i++) {
        int iN = i * n;
        for (j = 0; j < bs; j++) {
            int jN = j * n;
            double suma = 0;
            for (k = 0; k < bs; k++) {
                suma += ablk[iN + k] * bblk[jN + k];
            }
            cblk[iN + j] += suma;
        }
    }
}

void* solve_pthreads(void* arg) {
    thread_params* p = (thread_params*)arg;

    int chunk = p->N / p->T; //repartimos la carga
    int start = p->id * chunk; //le digo donde incia
    int end = (p->id == p->T - 1) ? p->N : (p->id + 1) * chunk; //donde termina y si

    double l_maxA = -1.0, l_minA = 9999999, l_sumA = 0;
    double l_maxB = -1.0, l_minB = 9999999, l_sumB = 0;

    for (int i = start * p->N; i < end * p->N; i++) {
        if (p->A[i] > l_maxA) l_maxA = p->A[i];
        if (p->A[i] < l_minA) l_minA = p->A[i];
        l_sumA += p->A[i];
        if (p->B[i] > l_maxB) l_maxB = p->B[i];
        if (p->B[i] < l_minB) l_minB = p->B[i];
        l_sumB += p->B[i];
    }

    pthread_mutex_lock(&mutex_metrics);
    if (l_maxA > g_maxA) g_maxA = l_maxA;
    if (l_minA < g_minA) g_minA = l_minA;
    g_sumA += l_sumA;
    if (l_maxB > g_maxB) g_maxB = l_maxB;
    if (l_minB < g_minB) g_minB = l_minB;
    g_sumB += l_sumB;
    pthread_mutex_unlock(&mutex_metrics);

    pthread_barrier_wait(&barrier_sync);

    if (p->id == 0) {
        double promA = g_sumA / (p->N * p->N);
        double promB = g_sumB / (p->N * p->N);
        factor_final = (g_maxA * g_maxB - g_minA * g_minB) / (promA * promB);
    }

    pthread_barrier_wait(&barrier_sync);

    for (int i = start; i < end; i++) {
        int iN = i * p->N;
        for (int j = 0; j < p->N; j++) {
            p->BT[j * p->N + i] = p->B[iN + j];
        }
    }

    pthread_barrier_wait(&barrier_sync);

    for (int i = start; i < end; i += p->TB) {
        for (int j = 0; j < p->N; j += p->TB) {
            for (int k = 0; k < p->N; k += p->TB) {
                blkmul(&p->A[i * p->N + k], &p->BT[j * p->N + k], &p->Tmat[i * p->N + j], p->N, p->TB);
            }
        }
    }

    pthread_barrier_wait(&barrier_sync);

    for (int i = start; i < end; i += p->TB) {
        for (int j = 0; j < p->N; j += p->TB) {
            for (int k = 0; k < p->N; k += p->TB) {
                blkmul(&p->Tmat[i * p->N + k], &p->B[j * p->N + k], &p->R[i * p->N + j], p->N, p->TB);
            }
        }
    }

    for (int i = start * p->N; i < end * p->N; i++) {
        p->R[i] *= factor_final;
    }

    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printf("Uso: %s <N> <TB> <T (hilos)>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int TB = atoi(argv[2]);
    int T = atoi(argv[3]);

    double *A = (double*)malloc(N * N * sizeof(double));
    double *B = (double*)malloc(N * N * sizeof(double));
    double *BT = (double*)malloc(N * N * sizeof(double));
    double *Tmat = (double*)malloc(N * N * sizeof(double));
    double *R = (double*)malloc(N * N * sizeof(double));

    for (int i = 0; i < N * N; i++) {
        A[i] = 1.0; B[i] = 1.0; Tmat[i] = 0.0; R[i] = 0.0;
    }

    pthread_t hilos[T];
    thread_params params[T];
    pthread_mutex_init(&mutex_metrics, NULL);
    pthread_barrier_init(&barrier_sync, NULL, T);

    double tiempo = dwalltime();

    for (int i = 0; i < T; i++) {
        params[i].id = i; params[i].N = N; params[i].T = T; params[i].TB = TB;
        params[i].A = A; params[i].B = B; params[i].BT = BT; params[i].Tmat = Tmat; params[i].R = R;
        pthread_create(&hilos[i], NULL, solve_pthreads, &params[i]);
    }

    for (int i = 0; i < T; i++) {
        pthread_join(hilos[i], NULL);
    }

    tiempo = dwalltime() - tiempo;

    printf("N=%d, TB=%d, T=%d, Tiempo: %f segundos\n", N, TB, T, tiempo);

    pthread_mutex_destroy(&mutex_metrics);
    pthread_barrier_destroy(&barrier_sync);
    free(A); free(B); free(BT); free(Tmat); free(R);

    return 0;
}
