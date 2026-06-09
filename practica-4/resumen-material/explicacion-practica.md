### modelo de memoria distruibuida:
- Los Procesos son independientes, cada uno con su propio espacio de direcciones, que se comunican y se sincronzian mediante el intercamio de mensajes. 
    - No existe memoira compartida entre ellos. 
    - Un proceso no puede acceder al pescio de memoria de otro proceso
    - No Hilops porque comparten memoria de un mismo proceso y no representan unidades de memoria independiente. 
- Surge a partir de los primeros clusters monocore. 
- Herramietnas para desarrollo de aplicaciones: 
    - PVM -> ya no se usa
    - MPI -> estandar actual

### MPI 
Define sintaxis y semantica de funciones para pasaje de mensajes, no define los comandos. 

Es SPMD (Single Program, Multiplle data): todos los procesos ejecutan el mismo codigo.
Runtime System o Runtime enviroment (ORTE) gestiona el entorno de ejecucion. 
    
### MPI - Funcionamiento: 
- Suponemos: 
    - Un lcuster con varios nodos conectados por una red fisica
    - Distribucion OpenMPI
- Cada nodo debe conocer el archivo ejecutable(binario)
El programador debe compipuilar el codigo fuente y distribuir el binairo. MPI no lo hace. 

La compilacion con MPI: 
    mpicc -o miPrograma miPrograma.c
- La compilacion depende de las caracteristicas del cluster: 

- Cluster homogeneo: 
    - Nodos iguales 
    - Compilamos el codigo fuente en un nodo
    - Distribuimos el binario o lo compartimos mediante un sistema de archivos distribuido (NFS)

- Cluster Heterogeneo: 
    - Nodos diferentes
    - Compilar el codigo fuente en cada nodo. 
    - Cada nodo tendra su archivo binario. 

### MPI - Ejecucion: 
    mpirun -np NroProcesos  -machinefile maquinas miPrograma
- NroProcesos: numero total de procesos a crear
- Maquinas: archivo que contiene el nombnre de los nodos. 
    - Varia dependiendo de la distribucion

Archivo "maquinas": 
 maquinaA slots=2
 maquinaB slots=1
 maquinaC slots=1 

Aca slots: numero de procesos que se crean en cada nodo. 

- Habitualmente, se ejecuta desde un frontend (nodo de gestion y servicios)
- El runtime System ejecuta una copia del programa por unidad de procesamiento: 
    - Cada unidad de procesamiento ejecuta una copia del mismo programa.
- Cada proceso recibe un identificador unico (rank)
    - Generalmente, el orden del archivo de nodos.

### MPI - Estructura de programa: 
    tipo_t miVariable; 
    int main(int argc, char** argv){
        int miID; 
        int nrProcs; 
        MPI_Init(&argc, &argv); 
        MPI_Comm_rank(MPI_COMM_WORLD, &miID);
        MPI_Comm_size(MPI_COMM_WORLD, &nrProcs); 
        ...
        MPI_Finalize(); 
        return(0); 
    }

Recordar que una varible definida antes que el main no es una variable compartida entre los procesos!. 
Es un modelo de memoria distribuida NO HAY VARIABLES COMPARTIDAS. 
    
Cada unidad de procesamiento ejecuta una copia del mismo programa. Existira una copia de la variable para cada proceso. Variable local y privada. 

### Pthreads Caracteristicas del script: 

#!/bin/bash
#SBATCH -N 2     -> numero nde nodos a reservar
#SBATCH --exclusive
#SBATCH --tast-per-node=4 -> Procesos por nodo. 
#SBATCH -o directorioSalida/output.txt
#SBATCH -e directorioSalida/errors.txt
mpirun miAplicacionMPI parametros de programa -> Aplicacion MPI a ejecutar y sus respectivos parametros. 


chmod +x miScript.sh -> dar permisos de ejecucion 
sbatch script -> Ejecutar con el gestor de recursos. Retorna un JOBID

NUNCA ejecutar asi: ./script


### Modelo hibrido: 
Actuamente contamos con gran diversidad de arquitecturas de computo, Arquitecturas de memoria compartida y memoria distribuida combinadas fromando una arquitectura hibrida con mayor potencia de computo. 

Estrategia habitual es: 
    - Crear un Proceso MPI por nodo del cluster
    - Cada proceso crea varios hilso en el nodo. 
Compilar con MPI + Pthreads: mpicc -lpthread -o mpi_pthreads mpi_pthreads.c

Compilar con MPI + OpenMP: mpicc -fopenmp -o mpi_omp mpi_omp.c

Ejecutar como cualquier programa MPI (mpirun) parametrizando el numero de hilos. 

### MPI + Pthreads Caracterisitcas del script: 

#!/bin/bash
#SBATCH -N 2
#SBATCH -exclusive
#SBATCH -task-per-node=1
#SBATCH -o directorioSalida/output.txt
#Sbatch -e directorioSalida/errors.txt
mpirun --bind-to none miAplicacionMPI parametros de programa nroHilos

--bind-to-none para evitar bug de MPI
nuevamente chmod +x miScript.sh
           sbatch miScript

### MPI + OpenMP Caracteristicas del script: 
#!/bin/bash
#SBATCH -N 2 
#SBATCH -exclusive
#SBATCH -task-per-node=1
#SBATCH -o directorioSalida/output.txt
#SBATCH -e directorioSalida/errors.txt
export OMP_NUM_THREADS=4 -> usamos hilos por variable de entorno
mpirun --bind-to none miAplicacionMPI parametros de programa

ejecutamos como chmod +x miScrit.sh 
                sbatch script

    
