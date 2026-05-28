//Ejercicio 2
#include<stdio.h>
#include<stdlib.h>
#include<omp.h>
#include<math.h>


int main(int argc,char*argv[]){
 double x,scale;
 int i;
 int numThreads = atoi(argv[2]);
 int N=atoi(argv[1]);
 omp_set_num_threads(numThreads);
 scale=2.78;
 x=0.0;
 
 double *aux = (double*)malloc(N * sizeof(double));

 #pragma omp parallel for
 for(i=1;i<=N;i++){
	aux[i-1] = sqrt(i*scale);
 }

 #pragma omp parallel for ordered private(i)
 for(i=1;i<=N;i++){
	#pragma omp ordered
	x = x + aux[i-1] + 2*x;
 }

 printf("\n Resultado: %f \n",x);
 free(aux);

 return(0);
}

