#include<stdio.h>
#include<sys/time.h>
#include<stdlib.h>

int N=100;

//Para calcular tiempo
double dwalltime(){
        double sec;
        struct timeval tv;

        gettimeofday(&tv,NULL);
        sec = tv.tv_sec + tv.tv_usec/1000000.0;
        return sec;
}

int main(int argc,char* argv[]){

 double *A,*BT,*C;
 int i,j,k;
 int check = 1;
 double timetick;

 //Controla los argumentos al programa
 if ((argc != 2) || ((N = atoi(argv[1])) <= 0) ) {
    printf("\nUsar: %s n\n  n: Dimension de la matriz (nxn X nxn)\n", argv[0]);
    exit(1);
  }
 
 //Aloca memoria para las matrices
 A=(double*)malloc(sizeof(double)*N*N);
 //como es una matriz triangular solo necesitamos la mitad de la matriz
  int matrizAlloc = (N*(N+1))/2;
 BT=(double*)malloc(sizeof(double)*matrizAlloc);
//sospecho que no es necesario alocar BT como N^2, dado que solo usamos la mitad de la matriz
 C=(double*)malloc(sizeof(double)*N*N);

 //Inicializa las matrices
 //La matriz A se inicializa todas las columnas en 0 menos la ultima
 //La matriz BT se inicializa triangular inferior con 1
 //Esto permite que el resultado sean todos 1
 //La matriz C se inicializa en 0
 for(i=0;i<N;i++){
  for(j=0;j<N;j++){

   if(j<N-1){
    A[i*N+j]=0;
   }else{
    A[i*N+j]=1;
   }
   C[i*N+j]=0;	
  }
 }
 //Inicializa la matriz BT triangular inferior con 1
 for(j=0;j<N;j++){
  for(i=j;i<N;i++){
    int index = j*N - j*(j+1)/2 + i;
   if(i>=j){
    BT[index]=N;
   }else{
    BT[index]=0;
   }
  }
}
 //Realiza la multiplicacion 
 timetick = dwalltime();

 for(i=0;i<N;i++){
     //Primeras optimizaciones evitar re calculaciones de indices   
     int in = i*N;
  for(j=0;j<N;j++){
        int jn = j*N;
        double suma = 0;
   //loop desde j porque BT es triangular inferior, Nos desplazamos correctamente sobre las columnas
   // -```
   // --``    //- es un valor valido
   // ---`    //` es 0 dado que es matriz triangular (0)
   // ----
   // k itera las filas sobre el desplazamiento de columnas
   for(k=j;k<N;k++){
        //
        int indexBT = j*N - j*(j+1)/2 + k;
        suma += A[in+k]*BT[indexBT];
   }
        C[in+j] = suma;
  }
 }

 printf("Tiempo en segundos %f\n", dwalltime() - timetick);

 //Verifica el resultado
 for(i=0;i<N;i++){
  for(j=0;j<N;j++){
   check = check && (C[i*N+j]==N);
  }
 }

 if(check){
  printf("Multiplicacion de matriz triangular correcta\n");
 }else{
  printf("Multiplicacion de matriz triangular erroneo\n");
 }
 
 free(A);
 free(BT);
 free(C);

 return(0);
}
