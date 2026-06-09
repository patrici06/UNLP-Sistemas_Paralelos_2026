# Modelo de programación sobre memoria distribuida (MPI)
# Modelo de programación híbrido (MPI + Pthreads – MPI + OpenMP)

Facultad de Informática — Universidad Nacional de La Plata
Prof. Adrian Pousa (apousa@lidi.info.unlp.edu.ar)

---

## 1. Modelo de memoria distribuida

- Procesos independientes, cada uno con su propio espacio de direcciones, que se comunican y se sincronizan mediante el intercambio de mensajes.
  - No existe memoria compartida entre ellos.
  - Un proceso no puede acceder al espacio de memoria de otro proceso.
  - No hilos porque comparten memoria de un mismo proceso y no representan unidades de memoria independiente.
- Surge a partir de los primeros clusters monocore.
- Herramientas para el desarrollo de aplicaciones utilizando este modelo:
  - PVM - 1989: Impulsó el uso de clusters. Hoy en desuso.
  - MPI: Estándar vigente. C/C++ - Fortran.
  - Otras: Sockets, RMI, etc.

```
Red de comunicación
| Nodo    | Nodo    | ... | Nodo    |
| Memoria | Memoria |     | Memoria |
| P0      | P1      |     | Pp      |
```

## 2. MPI — Funcionamiento, Compilación, Ejecución, Script de ejecución en cluster

### MPI

- **Estándar** Message Passing Interface (MPI): define sintaxis y semántica de funciones para pasaje de mensajes. No define los comandos.
- Varias distribuciones que implementan el estándar en C/C++ y Fortran:
  - OpenMPI
  - Mpich
  - Otros...
  - Para otros lenguajes (Java o Python) son wrappers bajo lenguaje C.
- Nace en 1994 como iniciativa de un consorcio, MPI Forum:
  - IBM, Intel, Cray Research, Argonne National Laboratory, Universidad de Illinois…
- SPMD (Single Program, Multiple Data): todos los procesos ejecutan el mismo código.
- Runtime system o Runtime environment (ORTE) gestiona el entorno de ejecución.

[1]: http://www.mpi-forum.org

### Funcionamiento

- Suponemos:
  - Un cluster con varios nodos conectados por una red física.
  - Distribución OpenMPI.
- Cada nodo debe conocer el archivo ejecutable (binario).
- El programador debe compilar el código fuente y distribuir el binario.
  - MPI no lo hace.

```
miPrograma.c  ──Compilar──►  MiPrograma (binario)
int i;                        bin
If cond
Printf                        Cluster
…                               …
```

### Compilación

- Compilación MPI:

  ```
  mpicc -o miPrograma miPrograma.c
  ```

- La compilación depende de las características del cluster:

| Cluster Homogéneo | Cluster Heterogéneo |
|---|---|
| Nodos iguales. | Nodos diferentes. |
| Compilar el código fuente en un nodo. | Compilar el código fuente en cada nodo. |
| Distribuir el binario o compartirlo mediante un sistema de archivos distribuido (NFS). | Cada nodo tendrá su archivo binario. |

### Ejecución

- Ejecución MPI:

  ```
  mpirun -np NrProcesos –machinefile maquinas miPrograma
  ```

  - `NrProcesos`: número total de procesos a crear.
  - `maquinas`: archivo que contiene el nombre de los nodos.
    - Varía dependiendo de la distribución.

```
Cluster                          Archivo "maquinas"
Maquina A   Maquina B   Maquina C  maquinaA slots=2    slots: número de procesos que
                                   maquinaB slots=1    se crearán en cada nodo
                                   maquinaC slots=1
```

- Habitualmente, se ejecuta desde un frontend (nodo de gestión y servicios).
- El Runtime System ejecuta una copia del programa por unidad de procesamiento:
  - Cada unidad de procesamiento ejecuta una copia del mismo programa.
- Cada proceso recibe un identificador único (rank).
  - Generalmente, en el orden del archivo de nodos.

```
Frontend
Archivo "maquinas"     Cluster
maquinaA slots=2
BIN                    Maquina A         Maquina B    Maquina C
…                      Core    Core      Core         Core
maquinaB slots=1       Proceso Proceso   Proceso      Proceso
maquinaC slots=1       Rank 0  Rank 1    Rank 2       Rank 3
mpirun -np 4 -machinefile maquinas miPrograma
MiPrograma MiPrograma MiPrograma MiPrograma
```

### Estructura de programa

```
tipo_t miVariable;     NO ES UNA VARIABLE COMPARTIDA
int main(int argc, char** argv){
    int miID;          ...entre los procesos.
    int nrProcs;       En un modelo de memoria distribuida:
    MPI_Init(&argc, &argv);            NO HAY VARIABLES COMPARTIDAS
    MPI_Comm_rank(MPI_COMM_WORLD, &miID);
    MPI_Comm_size(MPI_COMM_WORLD, &nrProcs);
    ……                 Recordar:
    MPI_Finalize();    Cada unidad de procesamiento ejecuta una copia
    return(0);         del mismo programa.
}                      Existirá una copia de la variable para cada proceso.
                       VARIABLE LOCAL Y PRIVADA
```

### Script de ejecución (SLURM) — MPI

```bash
#!/bin/bash
#SBATCH -N 2                   # Número de nodos a reservar.
#SBATCH --exclusive
#SBATCH --tasks-per-node=4     # Procesos por nodo.
#SBATCH -o directorioSalida/output.txt
#SBATCH -e directorioSalida/errors.txt

mpirun miAplicacionMPI parámetros de programa   # Aplicación MPI a ejecutar y sus parámetros.

# Dar permisos de ejecución.
chmod +x miScript.sh

# Ejecutar con el gestor de recursos. NO ejecutar así: ./script
sbatch script                   # Retorna un JOBID
```

## 3. Modelo híbrido — Estrategia, Script de ejecución en cluster

### Modelo de programación híbrido

- Actualmente, contamos con una gran diversidad de arquitecturas de cómputo.
- Arquitecturas de memoria compartida y memoria distribuida combinadas formando una arquitectura híbrida con mayor potencia de cómputo.
- Podemos combinar:
  - MPI + Pthreads
  - MPI + OpenMP

```
Red de comunicación
WAN o LAN (Ethernet, Infiniband, Myrinet)

Core Core   Core Core   Core Core   Core Core
Core Core   Core Core   Core Core   Core Core
```

### Estrategia

- Estrategia habitual MPI+Pthreads y MPI+OpenMP es:
  - Crear un proceso MPI por nodo del cluster.
  - Cada proceso crea varios hilos en el nodo.

```
Compilar MPI + Pthreads: mpicc -lpthread -o mpi_pthreads mpi_pthreads.c
Compilar MPI + OpenMP:   mpicc -fopenmp -o mpi_omp mpi_omp.c

Ejecutar como cualquier programa MPI (mpirun) parametrizando el número de hilos.
```

### Script de ejecución (SLURM) — MPI + Pthreads

```bash
#!/bin/bash
#SBATCH -N 2                   # Número de nodos a reservar.
#SBATCH --exclusive
#SBATCH --tasks-per-node=1     # Un proceso por nodo.
#SBATCH -o directorioSalida/output.txt
#SBATCH -e directorioSalida/errors.txt

mpirun --bind-to none miAplicacionMPI parámetros de programa nroHilos
# --bind-to-none para evitar bug de MPI

# Dar permisos de ejecución.
chmod +x miScript.sh
sbatch script                   # Retorna un JOBID
```

### Script de ejecución (SLURM) — MPI + OpenMP

```bash
#!/bin/bash
#SBATCH -N 2                   # Número de nodos a reservar.
#SBATCH --exclusive
#SBATCH --tasks-per-node=1     # Un proceso por nodo.
#SBATCH -o directorioSalida/output.txt
#SBATCH -e directorioSalida/errors.txt

export OMP_NUM_THREADS=4       # Hilos por proceso.

mpirun --bind-to none miAplicacionMPI parámetros de programa
# --bind-to-none para evitar bug de MPI

# Dar permisos de ejecución.
chmod +x miScript.sh
sbatch script                   # Retorna un JOBID
```
