#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <string.h>
#include "matmul_blocked.h"

#define BS 32

/* Timer function */
double dwalltime();

/* Comparison function for validation */
int compare_matrices(double *m1, double *m2, int n, double tolerance, int order_m1, int order_m2);

/* Print matrix (for debugging) */
void print_matrix(double *mat, int n, int order);

/* ============================================================
   MAIN PROGRAM
   ============================================================ */
int main(int argc, char *argv[])
{
    double *a, *b, *c, *c_rm, *c_cm, *c_mixed;
    int i, j, k, n;
    int test_mode = 0;  /* 0: all tests, 1: row-major only, 2: col-major only, 3: mixed only */
    double timetick, timeRowMajor, timeColMajor, timeMixed;
    double gflopRM, gflopCM, gflopMixed;
    
    /* Check command line parameters */
    if ((argc < 2) || ((n = atoi(argv[1])) <= 0)) {
        printf("\nUsage: %s N [mode]\n", argv[0]);
        printf("  N: matrix dimension (should be multiple of %d for optimal performance)\n", BS);
        printf("  mode (optional):\n");
        printf("    0: all tests (default)\n");
        printf("    1: row-major only\n");
        printf("    2: column-major only\n");
        printf("    3: mixed order only\n");
        exit(1);
    }
    
    if (argc >= 3) {
        test_mode = atoi(argv[2]);
        if (test_mode < 0 || test_mode > 3) {
            printf("Invalid mode. Using default (0).\n");
            test_mode = 0;
        }
    }
    
    printf("\n========== BLOCKED MATRIX MULTIPLICATION ==========\n");
    printf("Matrix dimension: %d x %d\n", n, n);
    printf("Block size (BS): %d\n", BS);
    printf("Total elements per matrix: %d\n", n*n);
    printf("===================================================\n\n");
    
    /* Allocate memory */
    a = (double *) malloc(n * n * sizeof(double));
    b = (double *) malloc(n * n * sizeof(double));
    c_rm = (double *) malloc(n * n * sizeof(double));
    c_cm = (double *) malloc(n * n * sizeof(double));
    c_mixed = (double *) malloc(n * n * sizeof(double));
    
    if (!a || !b || !c_rm || !c_cm || !c_mixed) {
        printf("Error: Memory allocation failed!\n");
        exit(1);
    }
    
    /* Initialize matrices with test values */
    printf("Initializing matrices...\n");
    initmat_rowmajor(a, n, 1.0);      /* Matrix A: all 1's */
    initmat_rowmajor(b, n, 1.0);      /* Matrix B: all 1's */
    
    printf("Expected result: C[i][j] = n = %d (for C = A * B with all 1's)\n\n", n);
    
    /* ============================================================
       TEST 1: ROW-MAJOR ORDER
       ============================================================ */
    if (test_mode == 0 || test_mode == 1) {
        printf("--- TEST 1: ROW-MAJOR ORDER ---\n");
        printf("Access pattern: mat[i*n + j]\n");
        printf("Multiplying matrices in row-major format...\n");
        
        timetick = dwalltime();
        matmulblks_rowmajor(a, b, c_rm, n);
        timeRowMajor = dwalltime() - timetick;
        
        gflopRM = ((double)2 * n * n * n) / (timeRowMajor * 1e9);
        printf("Time: %lf seconds\n", timeRowMajor);
        printf("Performance: %lf GFLOP/s\n", gflopRM);
        
        /* Validation */
        printf("Validating results...\n");
        int valid_rm = 1;
        int error_count_rm = 0;
        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++) {
                if (fabs(c_rm[i*n + j] - n) > 1e-6) {
                    if (error_count_rm < 5) {
                        printf("  Error at [%d,%d]: expected %d, got %lf\n", 
                               i, j, n, c_rm[i*n + j]);
                    }
                    error_count_rm++;
                    valid_rm = 0;
                }
            }
        }
        
        if (valid_rm) {
            printf("Status: OK ✓\n\n");
        } else {
            printf("Status: ERROR - %d mismatches\n\n", error_count_rm);
        }
    }
    
    /* ============================================================
       TEST 2: COLUMN-MAJOR ORDER
       ============================================================ */
    if (test_mode == 0 || test_mode == 2) {
        printf("--- TEST 2: COLUMN-MAJOR ORDER ---\n");
        printf("Access pattern: mat[i + j*n]\n");
        printf("Converting matrices to column-major format...\n");
        
        /* Convert A and B to column-major for this test */
        double *a_cm = (double *) malloc(n * n * sizeof(double));
        double *b_cm = (double *) malloc(n * n * sizeof(double));
        
        /* Convert from row-major to column-major */
        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++) {
                a_cm[i + j*n] = a[i*n + j];
                b_cm[i + j*n] = b[i*n + j];
            }
        }
        
        printf("Multiplying matrices in column-major format...\n");
        timetick = dwalltime();
        matmulblks_colmajor(a_cm, b_cm, c_cm, n);
        timeColMajor = dwalltime() - timetick;
        
        gflopCM = ((double)2 * n * n * n) / (timeColMajor * 1e9);
        printf("Time: %lf seconds\n", timeColMajor);
        printf("Performance: %lf GFLOP/s\n", gflopCM);
        
        /* Validation */
        printf("Validating results...\n");
        int valid_cm = 1;
        int error_count_cm = 0;
        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++) {
                if (fabs(c_cm[i + j*n] - n) > 1e-6) {
                    if (error_count_cm < 5) {
                        printf("  Error at [%d,%d]: expected %d, got %lf\n", 
                               i, j, n, c_cm[i + j*n]);
                    }
                    error_count_cm++;
                    valid_cm = 0;
                }
            }
        }
        
        if (valid_cm) {
            printf("Status: OK ✓\n\n");
        } else {
            printf("Status: ERROR - %d mismatches\n\n", error_count_cm);
        }
        
        free(a_cm);
        free(b_cm);
    }
    
    /* ============================================================
       TEST 3: MIXED ORDER (A row-major, B column-major)
       ============================================================ */
    if (test_mode == 0 || test_mode == 3) {
        printf("--- TEST 3: MIXED ORDER ---\n");
        printf("Matrix A: row-major [i*n + j]\n");
        printf("Matrix B: column-major [i + j*n]\n");
        printf("Matrix C: row-major [i*n + j]\n");
        
        /* Convert B to column-major */
        double *b_cm = (double *) malloc(n * n * sizeof(double));
        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++) {
                b_cm[i + j*n] = b[i*n + j];
            }
        }
        
        printf("Multiplying A (row-major) * B (column-major)...\n");
        timetick = dwalltime();
        matmulblks_mixed(a, b_cm, c_mixed, n);
        timeMixed = dwalltime() - timetick;
        
        gflopMixed = ((double)2 * n * n * n) / (timeMixed * 1e9);
        printf("Time: %lf seconds\n", timeMixed);
        printf("Performance: %lf GFLOP/s\n", gflopMixed);
        
        /* Validation */
        printf("Validating results...\n");
        int valid_mixed = 1;
        int error_count_mixed = 0;
        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++) {
                if (fabs(c_mixed[i*n + j] - n) > 1e-6) {
                    if (error_count_mixed < 5) {
                        printf("  Error at [%d,%d]: expected %d, got %lf\n", 
                               i, j, n, c_mixed[i*n + j]);
                    }
                    error_count_mixed++;
                    valid_mixed = 0;
                }
            }
        }
        
        if (valid_mixed) {
            printf("Status: OK ✓\n\n");
        } else {
            printf("Status: ERROR - %d mismatches\n\n", error_count_mixed);
        }
        
        free(b_cm);
    }
    
    /* ============================================================
       PERFORMANCE SUMMARY
       ============================================================ */
    if (test_mode == 0) {
        printf("========== PERFORMANCE SUMMARY ==========\n");
        printf("Row-major:      %10.6lf sec  |  %10.6lf GFLOP/s\n", timeRowMajor, gflopRM);
        printf("Column-major:   %10.6lf sec  |  %10.6lf GFLOP/s\n", timeColMajor, gflopCM);
        printf("Mixed order:    %10.6lf sec  |  %10.6lf GFLOP/s\n", timeMixed, gflopMixed);
        printf("=========================================\n\n");
        
        /* Comparison */
        if (timeRowMajor < timeColMajor && timeRowMajor < timeMixed) {
            printf("Best performance: ROW-MAJOR (%.2f%% faster than col-major)\n",
                   ((timeColMajor - timeRowMajor) / timeRowMajor) * 100);
        } else if (timeColMajor < timeRowMajor && timeColMajor < timeMixed) {
            printf("Best performance: COLUMN-MAJOR (%.2f%% faster than row-major)\n",
                   ((timeRowMajor - timeColMajor) / timeColMajor) * 100);
        } else {
            printf("Best performance: MIXED ORDER\n");
        }
    }
    
    /* Output in CSV format for easy analysis */
    printf("\nCSV Output:\n");
    if (test_mode == 0) {
        printf("MATMUL;%d;%d;%lf;%lf;%lf;%lf;%lf;%lf\n", 
               n, BS, timeRowMajor, gflopRM, timeColMajor, gflopCM, timeMixed, gflopMixed);
    } else if (test_mode == 1) {
        printf("MATMUL-RM;%d;%d;%lf;%lf\n", n, BS, timeRowMajor, gflopRM);
    } else if (test_mode == 2) {
        printf("MATMUL-CM;%d;%d;%lf;%lf\n", n, BS, timeColMajor, gflopCM);
    } else if (test_mode == 3) {
        printf("MATMUL-MIXED;%d;%d;%lf;%lf\n", n, BS, timeMixed, gflopMixed);
    }
    
    /* Free memory */
    free(a);
    free(b);
    free(c_rm);
    free(c_cm);
    free(c_mixed);
    
    return 0;
}

/* ============================================================
   UTILITY FUNCTIONS
   ============================================================ */

/**
 * Get wall time in seconds since some arbitrary point in the past
 */
double dwalltime()
{
    double sec;
    struct timeval tv;
    
    gettimeofday(&tv, NULL);
    sec = tv.tv_sec + tv.tv_usec / 1000000.0;
    return sec;
}

/**
 * Compare two matrices with specified tolerance
 * order: 0 for row-major, 1 for column-major
 */
int compare_matrices(double *m1, double *m2, int n, double tolerance, int order_m1, int order_m2)
{
    int i, j;
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            double val1, val2;
            
            if (order_m1 == 0)
                val1 = m1[i*n + j];
            else
                val1 = m1[i + j*n];
            
            if (order_m2 == 0)
                val2 = m2[i*n + j];
            else
                val2 = m2[i + j*n];
            
            if (fabs(val1 - val2) > tolerance) {
                return 0;  /* Matrices differ */
            }
        }
    }
    
    return 1;  /* Matrices are equal */
}

/**
 * Print matrix for debugging purposes
 * order: 0 for row-major, 1 for column-major
 */
void print_matrix(double *mat, int n, int order)
{
    int i, j;
    printf("\n");
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            double val;
            if (order == 0)
                val = mat[i*n + j];
            else
                val = mat[i + j*n];
            
            printf("%8.2f ", val);
        }
        printf("\n");
    }
    printf("\n");
}
