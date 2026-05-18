#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
#include<math.h>
#include<string.h>
#include<stdbool.h>

#define BS 64

double dwalltime();

/* Multiply square matrices, blocked version */
void matmulblks(double *a, double *b, double *c, int n, int bs, int order);
//funciones especializadas para cada escenario de orden de almacenamiento
//Mejora el performance y aprovecha optimizaciones del compilador
void matmulblksRowColCol(double *a, double *b, double *c, int n, int bs);
void matmulblksRowColRow(double *a, double *b, double *c, int n, int bs);


/* Multiply (block)submatrices */
void blkmul(double *ablk, double *bblk, double *cblk, int n, int bs, int order);
//funciones especializadas para cada escenario de orden de almacenamiento
//Mejora el performance y aprovecha optimizaciones del compilador
void blkmulRowColCol(double *ablk, double *bblk, double *cblk, int n, int bs);
void blkmulRowColRow(double *ablk, double *bblk, double *cblk, int n, int bs);

/* Print matrix with specified storage order */
void print_matrix(double *mat, int n, const char *name, int order, bool print);

int main(int argc, char*argv[]) {
    double *a, *b, *d, *c, *r;
    int i, j, k, n;
    double MaxA, MinA, PromA, MaxB, MinB, PromB;
    double timetick, workTime;

    if ((argc != 2) || ((n = atoi(argv[1])) <= 0)) {
        printf("\nUsar: %s N\n", argv[0]);
        exit(1);
    }
    bool print = false;
    print = BS == 8;
    a = (double*) malloc(sizeof(double)*n*n);
    b = (double*) malloc(sizeof(double)*n*n);
    d = (double*) malloc(sizeof(double)*n*n);
    c = (double*) malloc(sizeof(double)*n*n);
    r = (double*) malloc(sizeof(double)*n*n);

    // Inicialización
    for (i = 0; i < n; i++) {
      for (j = 0; j < n; j++) {
        // Inicializamos A en row-major y B en row-major
        a[i*n + j] = (double)(i + j + 1);
        b[i*n + j] = (double)(i - j + 1);
       }
    }

    memset(d, 0, sizeof(double)*n*n);
    memset(c, 0, sizeof(double)*n*n);
    memset(r, 0, sizeof(double)*n*n);

    // TESTING: Print initial matrices
    if (print) {
        printf("\n========== TESTING: Matrices Iniciales ==========\n");
        print_matrix(a, n, "Matriz A (row-major)", 0, print);
        print_matrix(b, n, "Matriz B (row-major)", 0, print);
        printf("================================================\n\n");
    }

    timetick = dwalltime();

    //acceso secuencial para calcular maximo, minimo y promedio de A y B
    PromA = PromB = 0.0;
    MaxA = MinA = a[0];
    MaxB = MinB = b[0];

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
    //row, col, col
    //primero B x BT para obtener D con column major order
    //order = 2 representa el escenario donde B y BT(b recorrido por columnas) y D resulta en column major order
    //matmulblks(b, b, d, n, BS, 2);
    //luego A x D para obtener C con row major order 
    //order = 1 representa el escenario donde A y C son row major order y D es column major order
    //matmulblks(a, d, c, n, BS, 1);

    //solucion especializada para el escenario row, col, col
    matmulblksRowColCol(b, b, d, n, BS);
    //solucion especializada para el escenario row, col, row
    matmulblksRowColRow(a, d, c, n, BS);

    //recorrido lineal dado que es una constante matematica por una matriz
    double constante = ((MaxA * MaxB) - (MinA * MinB)) / (PromA * PromB);

    for (i = 0; i < n*n; i++) {
        r[i] = constante * c[i];
    }

    workTime = dwalltime() - timetick;

    // TESTING: Print intermediate results
    if (print) {
        printf("\n========== TESTING: D = B x B^T ==========\n");
        print_matrix(d, n, "Matriz D (column-major)", 2, print);
        printf("=========================================\n\n");
        printf("========== TESTING: C = A x D ==========\n");
        print_matrix(c, n, "Matriz C (row-major)", 0, print);
        printf("========================================\n\n");
        printf("\n========== TESTING: R = k * C ==========\n");
        printf("Constante k = %lf\n", constante);
        print_matrix(r, n, "Matriz R (row-major)", 0, print);
        printf("=======================================\n\n");
    }
    // =========================
    // VALIDACIÓN
    // =========================
    int nan_count = 0, inf_count = 0;

    for (i = 0; i < n*n; i++) {
        if (isnan(r[i])) nan_count++;
        if (isinf(r[i])) inf_count++;
    }

    printf("RESULT;%d;%lf;%lf\n", n, workTime, ((double)2*n*n*n)/(workTime*1e9));

    if (nan_count == 0 && inf_count == 0)
        printf("VALIDATION;OK\n");
    else
        printf("VALIDATION;ERROR;NaN=%d;Inf=%d\n", nan_count, inf_count);

    free(a);
    free(b);
    free(d);
    free(c);
    free(r);

    return 0;
}


void matmulblks(double *a, double *b, double *c, int n, int bs, int order) {
    int i, j, k;
    int in, jn, kn;
    
    // Estructura optimizada: for i, for k, for j (k es dimensión de reducción)
    // Esto aprovecha mejor la localidad caché
    
    for (i = 0; i < n; i += bs) {
        in = i*n;
        for (k = 0; k < n; k += bs) {
            kn = k*n;
            for (j = 0; j < n; j += bs) {
                jn = j*n;
                
                if (order == 0) {
                    // A, B, C todos row-major: C[i,j] = suma_k A[i,k] * B[k,j]
                    blkmul(&a[in + k], &b[kn + j], &c[in + j], n, bs, order);
                }
                else if (order == 1) {
                    // A, C row-major; D column-major: C[i,j] = suma_k A[i,k] * D[k,j]
                    // &b[k*n + j*n] accede a D[k,j] en column-major
                    blkmul(&a[in + k], &b[kn + jn], &c[in + j], n, bs, order);
                }
                else if (order == 2) {
                    // B × B^T: D[i,j] = suma_k B[i,k] * B[j,k]
                    // Resultado en column-major: D[i + j*n]
                    blkmul(&a[in + k], &b[jn + k], &c[i + jn], n, bs, order);
                }
                //esta solucion podria extenderse para todos los escenarios posibles con diferentes combinaciones de orden de almacenamiento
            }
        }
    }
}
//implementacion de las funciones especializadas para cada escenario de orden de almacenamiento
void matmulblksRowColCol(double *a, double *b, double *c, int n, int bs) {
    // Implementación especializada para A row-major, B column-major, C column-major
    // C[i + j*n] += A[i*k] * B[k + j*n]
    for (int i = 0; i < n; i += bs) {
        int in = i*n;
        for (int k = 0; k < n; k += bs) {
            int kn = k*n;
            for (int j = 0; j < n; j += bs) {
                blkmulRowColCol(&a[in + k], &b[kn + j*n], &c[i + j*n], n, bs);
            }
        }
    }
}
void matmulblksRowColRow(double *a, double *b, double *c, int n, int bs) {
    // Implementación especializada para A row-major, B row-major, C row-major
    // C[i*n + j] += A[i*n + k] * B[k*n + j]
    for (int i = 0; i < n; i += bs) {
        int in = i*n;
        for (int k = 0; k < n; k += bs) {
            int kn = k*n;
            for (int j = 0; j < n; j += bs) {
                blkmulRowColRow(&a[in + k], &b[kn + j], &c[in + j], n, bs);
            }
        }
    }
}

void blkmul(double *ablk, double *bblk, double *cblk, int n, int bs, int order) {
    int i, j, k;
    int in, jn, kn;
    
    // Estructura optimizada: reordena bucles para usar acumulador
    // Reduces escrituras a cblk acumulando en sum hasta terminar el bucle k
    
    for (i = 0; i < bs; i++) {
        in = i*n;
        for (j = 0; j < bs; j++) {
            jn = j*n;
            double sum = 0.0;  // Acumulador para reducir asignaciones
            
            for (k = 0; k < bs; k++) {
                kn = k*n;
                
                if (order == 0) {
                    // A, B, C row-major: C[i,j] += A[i,k] * B[k,j]
                    sum += ablk[in + k] * bblk[kn + j];
                }
                else if (order == 1) {
                    // A, C row-major; D column-major: C[i,j] += A[i,k] * D[k,j]
                    // D[k,j] en column-major está en bblk[k + j*n]
                    sum += ablk[in + k] * bblk[k + jn];
                }
                else if (order == 2) {
                    // B × B^T: D[i,j] += B[i,k] * B[j,k]
                    // Resultado en column-major: D[i + j*n]
                    sum += ablk[in + k] * bblk[jn + k];
                }
                //esta solucion podria extenderse para todos los escenarios posibles con diferentes combinaciones de orden de almacenamiento
            }

            // Asignación única después de terminar el bucle k
            if (order == 0 || order == 1) {
                cblk[in + j] += sum;
            } else if (order == 2) {
                cblk[i + jn] += sum;
            }
        }
    }
}
void blkmulRowColCol(double *ablk, double *bblk, double *cblk, int n, int bs) {
    // Implementación especializada para A row-major, B column-major, C column-major
    for (int i = 0; i < bs; i++) {
        int in = i*n;
        for (int j = 0; j < bs; j++) {
            int jn = j*n;
            double sum = 0.0;
            for (int k = 0; k < bs; k++) {
                sum += ablk[in + k] * bblk[k + jn];
            }
            cblk[i + jn] += sum;
        }
    }
}
void blkmulRowColRow(double *ablk, double *bblk, double *cblk, int n, int bs) {
    // Implementación especializada para A row-major, B column-major, C row-major
    for (int i = 0; i < bs; i++) {
        int in = i*n;
        for (int j = 0; j < bs; j++) {
            int jn = j*n;
            double sum = 0.0;
            for (int k = 0; k < bs; k++) {
                sum += ablk[in + k] * bblk[k + jn];
            }
            cblk[in + j] += sum;
        }
    }
}

// =========================
// TIMER
// =========================
double dwalltime()
{
    double sec;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    sec = tv.tv_sec + tv.tv_usec/1000000.0;
    return sec;
}

// =========================
// PRINT MATRIX FUNCTIONS
// =========================
/* Print matrix with specified storage order */
void print_matrix(double *mat, int n, const char *name, int order, bool print)
{
    if (print) {
        printf("\n%s (n=%d, order=%s):\n", name, n, order == 0 ? "row-major" : order == 1 ? "row-major" : "column-major");
        
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                double val;
                if (order == 0 || order == 1) {
                    // row-major
                    val = mat[i*n + j];
                } else {
                    // column-major (order == 2)
                    val = mat[i + j*n];
                }
                printf("%8.2f ", val);
            }
            printf("\n");
        }
    }
}
