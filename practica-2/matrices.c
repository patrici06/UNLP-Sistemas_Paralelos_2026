#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
#include<math.h>
#include<string.h>

#define BS 32
#define ROWXROWECOL 0
#define ROWXCOLEROW 1
double dwalltime();
void blkmul(double *ablk, double *bblk, double *cblk, int n, int bs, int type);
void matmulblks(double *a, double *b, double *c, int n, int bs, int type);

int main(int argc, char*argv[]) {
    double *a, *b, *d, *c, *r;
    int i, j, k, n;
    double MaxA, MinA, PromA, MaxB, MinB, PromB;
    double timetick, workTime;

    if ((argc != 2) || ((n = atoi(argv[1])) <= 0)) {
        printf("\nUsar: %s N\n", argv[0]);
        exit(1);
    }

    a = (double*) malloc(sizeof(double)*n*n);
    b = (double*) malloc(sizeof(double)*n*n);
    d = (double*) malloc(sizeof(double)*n*n);
    c = (double*) malloc(sizeof(double)*n*n);
    r = (double*) malloc(sizeof(double)*n*n);

    // a y b en row-major coherente con accesos posteriores
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            a[i*n + j] = rand() % 101;
            b[i*n + j] = rand() % 101;
        }
    }

    // d en column-major (coherente con d[z + y*n])
    for (j = 0; j < n; j++) {
        for (i = 0; i < n; i++) {
            d[i + j*n] = 0.0;
        }
    }

    // c y r en row-major
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            c[i*n + j] = 0.0;
            r[i*n + j] = 0.0;
        }
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

    //primero resolvemos b que nos permite explitar la localidad
    matmulblks(b, b, d, n, BS, 0);
            
    //primero resolvemos b que nos permite explotar la localidad temporal en la multiplicación de bloques
    // for (i = 0; i < n; i += BS) {
    //     for (j = 0; j < n; j += BS) {    
    //         for (k = 0; k < n; k += BS) {
    //             for (int x = i; x < i+BS && x < n; x++) {
    //                 for (int y = j; y < j+BS && y < n; y++) {

    //                     double sum = 0.0;

    //                     for (int z = k; z < k+BS && z < n; z++) {
    //                         sum += b[x*n + z] * b[y*n + z];
    //                     }
    //                     //cargamos D en column-major para optimizar el acceso a memoria en la siguiente multiplicación
    //                     d[x + y*n] += sum;
    //                 }
    //             }
    //         }
    //     }
    // }
    // calculamos por bloques el producto de A (row-major) por D (column-major) para obtener C (row-major)
    matmulblks(a,d,c,n,BS,1);
    // for (i = 0; i < n; i += BS) {
    //     for (j = 0; j < n; j += BS) {
    //         for (k = 0; k < n; k += BS) {

    //             for (int x = i; x < i+BS && x < n; x++) {
    //                 for (int y = j; y < j+BS && y < n; y++) {

    //                     double sum = 0.0;

    //                     for (int z = k; z < k+BS && z < n; z++) {
    //                         sum += a[x*n + z] * d[z + y*n];
    //                     }

    //                     c[x*n + y] += sum;
    //                 }
    //             }
    //         }
    //     }
    // }
    //recorrido lineal dado que es una constante matematica por una matriz
    double constante = ((MaxA * MaxB) - (MinA * MinB)) / (PromA * PromB);

    for (i = 0; i < n*n; i++) {
        r[i] = constante * c[i];
    }

    workTime = dwalltime() - timetick;

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
// ========================
//  FUNCIONES
// ========================
/* Multiply square matrices, blocked version */
void matmulblks(double *a, double *b, double *c, int n, int bs, int type)
{
  int i, j, k;    /* Guess what... */
  // c ya viene inicializada
  for (i = 0; i < n; i += bs)
  {
    for (j = 0; j < n; j += bs)
    {
      for  (k = 0; k < n; k += bs)
      {
        blkmul(&a[i*n + k], &b[j*n + k], &c[i*n + j], n, bs, type);
      }
    }
  }
}

/*****************************************************************/

/* Multiply (block)submatrices */
void blkmul(double *ablk, double *bblk, double *cblk, int n, int bs, int type)
{
  int i, j, k;    /* Guess what... again... */

  for (i = 0; i < bs; i++)
  {
    for (j = 0; j < bs; j++)
    {
      for  (k = 0; k < bs; k++)
      {
        if(type == ROWXROWECOL){
            cblk[i + j*n] += ablk[i*n + k] * bblk[j*n + k];
        }
        if(type == ROWXCOLEROW){
            cblk[i*n + j] += ablk[i*n + k] * bblk[j + n*k];
        }
    }
    }
  }
}
    