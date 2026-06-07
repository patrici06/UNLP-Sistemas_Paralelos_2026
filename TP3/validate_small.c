#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define BS 64

void matmulblksRowColCol(double *a, double *b, double *c, int n, int bs);
void matmulblksRowColRow(double *a, double *b, double *c, int n, int bs);
void blkmulRowColCol(double *ablk, double *bblk, double *cblk, int n, int bs);
void blkmulRowColRow(double *ablk, double *bblk, double *cblk, int n, int bs);
void print_matrix(double *mat, int n, const char *name, int max_print);

int main() {
    int n = 4;
    int i, j;
    double *a, *b, *d, *r;
    double MaxA, MinA, PromA, MaxB, MinB, PromB, constante;

    a = (double*) malloc(sizeof(double)*n*n);
    b = (double*) malloc(sizeof(double)*n*n);
    d = (double*) malloc(sizeof(double)*n*n);
    r = (double*) malloc(sizeof(double)*n*n);

    // Inicialización igual a matrices-open-mp
    for (i = 0; i < n*n; i++) {
        a[i] = (double)((i / n) + (i % n) + 1);
        b[i] = (double)((i / n) - (i % n) + 1);
        d[i] = 0.0;
        r[i] = 0.0;
    }

    printf("Matriz A (4x4):\n");
    print_matrix(a, n, "A", n);
    printf("\nMatriz B (4x4):\n");
    print_matrix(b, n, "B", n);

    // Etapa 0: Estadísticas
    MaxA = MinA = a[0];
    MaxB = MinB = b[0];
    PromA = 0.0;
    PromB = 0.0;

    for (i = 0; i < n*n; i++) {
        double valA = a[i];
        double valB = b[i];
        if (valA > MaxA) MaxA = valA;
        if (valA < MinA) MinA = valA;
        PromA += valA;
        if (valB > MaxB) MaxB = valB;
        if (valB < MinB) MinB = valB;
        PromB += valB;
    }
    PromA /= (n*n);
    PromB /= (n*n);
    constante = ((MaxA * MaxB) - (MinA * MinB)) / (PromA * PromB);

    printf("\nEstadísticas:\n");
    printf("A: Min=%.2f, Max=%.2f, Prom=%.2f\n", MinA, MaxA, PromA);
    printf("B: Min=%.2f, Max=%.2f, Prom=%.2f\n", MinB, MaxB, PromB);
    printf("Constante = %.6f\n", constante);

    // Etapa 1: D = B × B^T (usando BS=2 para n=4)
    int bs = (n < BS) ? n : BS;
    matmulblksRowColCol(b, b, d, n, bs);

    printf("\nMatriz D (B × B^T):\n");
    print_matrix(d, n, "D", n);

    // Etapa 2: R = A × D
    matmulblksRowColRow(a, d, r, n, bs);

    printf("\nMatriz R (A × D):\n");
    print_matrix(r, n, "R", n);

    // Etapa 3: R *= constante
    for (i = 0; i < n*n; i++) {
        r[i] *= constante;
    }

    printf("\nMatriz R final (R × constante):\n");
    print_matrix(r, n, "R_final", n);

    free(a);
    free(b);
    free(d);
    free(r);

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

void print_matrix(double *mat, int n, const char *name, int max_print) {
    int limit = (n < max_print) ? n : max_print;
    printf("%s (%dx%d):\n", name, n, n);
    
    for (int i = 0; i < limit; i++) {
        printf("[ ");
        for (int j = 0; j < limit; j++) {
            printf("%8.2f ", mat[i*n + j]);
        }
        printf("]\n");
    }
}
