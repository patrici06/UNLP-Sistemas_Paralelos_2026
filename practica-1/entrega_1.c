#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
#include<math.h>
#include<string.h>

#define BS 64

double dwalltime();

int main(int argc, char*argv[]) {
    double *a, *b, *e, *bt, *r, *c;
    int i, j, k, n;
    double MaxA, MinA, PromA, MaxB, MinB, PromB = 0.0;
    double timetick, workTime;

    // controla los argumentos al programa
    if ((argc != 2) || ((n = atoi(argv[1])) <= 0)) {
        printf("\nUsar: %s N\n  N: Dimension de la matriz (NxN)\n", argv[0]);
        exit(1);
    }
    
    // reserva memoria para las matrices
    a  = (double*)malloc(sizeof(double)*n*n);
    b  = (double*)malloc(sizeof(double)*n*n);
    e  = (double*)malloc(sizeof(double)*n*n);
    bt = (double*)malloc(sizeof(double)*n*n);
    c  = (double*)malloc(sizeof(double)*n*n);
    r  = (double*)malloc(sizeof(double)*n*n);
    
    // inicializa las matrices a y b con valores aleatorios entre 0 y 100
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            // Inicializo por filas
            a[i*n + j] = rand() % 101;
            // Inicializo por columnas para optimizar el acceso a memoria
            b[i + j*n] = rand() % 101;
        }
    }
    
    // inicializar matrices de resultado a 0
    memset(e, 0, n*n*sizeof(double));
    memset(c, 0, n*n*sizeof(double));
    memset(r, 0, n*n*sizeof(double));
    
    // ============================================================
    // DESDE ACA
    // ============================================================
    timetick = dwalltime();
    
    PromA = 0.0;
    PromB = 0.0;
    MaxA = MinA = a[0];
    MaxB = MinB = b[0];
    
    // hacemos una pasada para obtener el maximo, minimo y promedio de las matrices a y b
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            double valA = a[i*n + j];
            double valB = b[i + j*n];
            if (valA > MaxA) MaxA = valA;
            if (valA < MinA) MinA = valA;
            PromA += valA;
            if (valB > MaxB) MaxB = valB;
            if (valB < MinB) MinB = valB;
            PromB += valB;
        }
    }
    PromA /= (n*n);
    PromB /= (n*n);
    
    // primer multiplicacion de matrices por bloques obtener la matriz e
    for (i = 0; i < n; i+= BS) {
        for (j = 0; j < n; j+= BS) {
            for (k = 0; k < n; k+= BS) {
                // procesamiento del bloque a y el bloque b para obtener el bloque e
                int x, y, z;
                for (x = 0; x < BS; x++) {
                    for (y = 0; y < BS; y++) {
                        double sum = 0.0;
                        int jumpRow = (i+x)*n;
                        int jumpCol = (j+y)*n;
                        for (z = 0; z < BS; z++) {
                            sum += a[jumpRow + (k+z)] * b[(k+z) + jumpCol];
                        }
                        e[jumpRow + (j+y)] += sum;
                    }
                }
            }
        }
    }
    
    // transponemos la matriz b para optimizar el acceso a memoria en la siguiente multiplicacion
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            bt[j*n + i] = b[i + j*n];
        }
    }
    
    // ahora con bt listo, y con e resuelta hacemos la multiplicacion para obtener la matriz c
    for (i = 0; i < n; i+= BS) {
        for (j = 0; j < n; j+= BS) {
            for (k = 0; k < n; k+= BS) {
                // procesamiento del bloque e y el bloque bt para obtener el bloque c
                int x, y, z;
                for (x = 0; x < BS; x++) {
                    for (y = 0; y < BS; y++) {
                        double sum = 0.0;
                        int jumpRow = (i+x)*n;
                        int jumpCol = (j+y)*n;
                        for (z = 0; z < BS; z++) {
                            sum += e[jumpRow + (k+z)] * bt[(j+y)*n + (k+z)];
                        }
                        c[jumpRow + (j+y)] += sum;
                    }
                }
            }
        }
    }
    
    // ahora con el producto matricial listo calculamos la constante a resolver y calculamos finalmente r
    double constante = ((MaxA * MaxB) - (MinA * MinB)) / (PromA * PromB);
    // recorremos de forma lineal la matriz c dado que el producto con una constante es de orden n^2, por nuestra disposicion es posible un acceso lineal a memoria
    for (i = 0; i < n*n; i++) {
        r[i] = constante * c[i];
    }
    
    workTime = dwalltime() - timetick;
    // ============================================================
    // HASTA ACA
    // ============================================================
    
    // ============================================================
    // VALIDACIÓN - Verificar que no haya valores NaN o Inf en la matriz r
    // ============================================================
    int validation_ok = 1;
    int nan_count = 0, inf_count = 0;
    
    for (i = 0; i < n*n; i++) {
        if (isnan(r[i])) {
            nan_count++;
            validation_ok = 0;
        }
        if (isinf(r[i])) {
            inf_count++;
            validation_ok = 0;
        }
    }
    
    if (validation_ok) {
        printf("RESULT;%d;%lf;%lf\n", n, workTime, ((double)2*n*n*n)/(workTime*1e9));
        printf("VALIDATION;OK;NaN_count=0;Inf_count=0\n");
    } else {
        printf("RESULT;%d;%lf;%lf\n", n, workTime, ((double)2*n*n*n)/(workTime*1e9));
        printf("VALIDATION;ERROR;NaN_count=%d;Inf_count=%d\n", nan_count, inf_count);
    }
    
    free(a);
    free(b);
    free(e);
    free(bt);
    free(c);
    free(r);
    
    return 0;
}

/*****************************************************************/

double dwalltime()
{
    double sec;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    sec = tv.tv_sec + tv.tv_usec/1000000.0;
    return sec;
}