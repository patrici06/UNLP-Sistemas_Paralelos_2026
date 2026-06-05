Agenda de la clase: 
- Programacion en modelo hibrido. 

### Programacion en plataformas hibridas
# Modelo Hibrido

### Fundamentos del modelo hibrido: 
La incorporacion de procesadores multicore a las arquitecturas de cluster tradicionales dio origen a una nueva arquitectura paralela: cluster de multicores
- Arquitecturas hibridas
- Jerarquicas de dos niveles
Tanto la comunidad cientifica como la industria se interezaron en investigar modelos de comunicacion hibridos -> modelos que permitan comuncarse tanto con pasaje de mensajes como memoria compartida. 

- Los paradigmas de programacion tradicionales (pasaje de mensajes y memoria compartida) no se adaptan naturalmente a los cluster de multicores. 
 
- Se espera que un modelo hibrido (combinacion de pasaje de mensajes con memoria compartida) explote mejor sus caracteristicas. 

La idea basica detras de esto es: 
    - Las tareas que se encuentran en el mismo nodo se comunican y sincronizan por memoria compartida. 
    - Las tareas que se encuentran en diferentes nodos se comunican y sincronizan por pasaje de mensajes. 

La combinacion de MPI con OpenMP o Pthreads permite explotar el paralelismo jerarquico inherente a los clusters de multicores o a la aplicacion. 

El modelo de programacion hibrido puede cinrementar el rendimiento y la escalabilidad de una aplicacion
Sin embargo esto no ocurre en todos los casos, antes de desarrollar una aplicacion paralela empleado el modelo hibrido, debe analizarse si el mismo puede resultar util o no. 

Razones para utilizar el modelo hibrido: 
- Al aprovechar la memoria compatida dentro de cada nodo: 
    - Reducen overhead de las comunicaciones MPI    
    - Reducen los requerimientos de memoria de la aplicacion. 
- Algunas aplicaciones presentan dos niveles de paralelismo: 
    - Paralelismo de grano grueso: gran cantidad de computo que puede ser realizado de forma independiente + algun intercambio de informacion ocasional entre los procesos de la aplicacion -> MPI
    - Paralelismo de grano fino, disponible a nivel de bucle -> OpenMP
    - El modelo hibrido puede resultar adecuado para explotar estos multiples niveles de paralelismo. 
Algunas aplicaciones presentan una carga de trabajo desbalanceada a nivel de MPI, la cual puede resultar dificil de equilibrar
Balancear la carga en forma dinamica con OpenMP resulta mas sencillo de lograr, Pthreads tambien es una opcion aunque el costo de programacion es mayor. 

### Razones para no utilizar el modelo hibrido: 
    Algunas aplicaciones solo presentan un unico nivel de paralelismo -> la introduccion de paralelismo jerarquico no provee beneficios para aumentar la complejidad de programacion
    Al introducir OpenMp o Pthreads a un codio MPI existente tambien se introducen desventajas: 
    - Overhead adicional por la creacion, sincronizacion y destruccion de hilos. 
    - Dependencia de la calidad del compilador y soporte en ejecucion para OpenMP/Pthreads. 
    - Cuestiones relacionadas al uso de memoria compartida como ubicacion de los datos en memoria y conflictos en el acceso a los mismos.

### Esquemas del modelo hibrido: 
Existen diferentes esquemas para paralelizar una aplicacion utilizando el modelo hibrido. 
La clasificacion se realiza teniendo en cuenta que hilo/s envia/n mensajes entre los proesos MPI y en que momento lo hacen. 
- Sin solapamiento de computo y comunicaciones
- Con solapamiento de computo y comunicaciones

### Modelos Hibrido Sin solapamiento de computo y comunicaciones: 
    - Tambien conocido como master-only o modo vector
    - Emplea un proceso MPI por nodo y OpenMP o Pthreads sobre los nucleso de cada nodo. 
    - Las llamdas a las rutinas MPI son realizadas fuera de las regiones paralelas de OpenMP o del codigo de los hilos creados con Pthreads. 
ejemplo de codigo: 
    /* hilo maestro */
    MPI_Recv(); /* recibir datos */     
    #pragma omp parallel
    {
        /* Ejecucion multi-hilada */
    }
    /* hilo maestro */
    MPI_Send(); 
Ventajas de este modelo: 
    - No hay intercambio de mensajes dentro de cada nodo.
    - La topologia de los procesos MPI ya no es una cuestion relevante a la hora de optimizar el rendimiento de la aplicacion. 
Desventajas: 
    - Mientras el hilo master se comunica, el resto de los hilos esta ocioso -> overhead. 
    - Un unico hilo probablemente no sea capaz de aprovechar todo el ancho de banda disponible de la red de comunicacion.

### Modelo hibrido - COn solapamiento de computo y comunicaciones: 
Una forma de evitar el ocio de los hilos durante las comunicaciones MPI consiste en permitir que mas de un hilo pueda comunicarse en paralelo a otros que realicen computo util. 
ejemplo: 
#pragma omp parallel private(mi_id)
{
    mi_id = omp_get_thread_num(); 
    if(mi_id ..) /* hilo de comunicacion */
            MPI_Send();
    else 
        if (mi_id ...) /* hilo de comunicacion */
            MPI_Recv(); 
        else{
            /* computo */
        }

} 


Ventajas:   
    - Se reduce el tiempo ocioso que los hilos podrian incurrir 
    -  se aprovecha el ancho de banda de la red. 
Desventajas: 
    - Requiere mayor esfuerzo de programacion 
    - Se debe equilibrar la carga de trabajo entre los hilos que comunican y los que no lo hacen. 
    
== Continuar pagina 16 ==
### Soporte MPI para programacion Hibrida: 
Las librerias de MPI varian en su soporte para las comunicaciones de los hilos. 
MPI especifica 4 niveles diferentes: 
    - MPI_THREAD_SINGLE (Nivel 0): Sin soporte para hilos
    - MPI_THREAD_FUNNELED(Nivel 1): Los procesos pueden ser multi hilados pero todas las comunicaciones las realizara el hilo master. 
    - MPI_THREAD_SERIALIZED(Nivel 2): Los procesos pueden ser multi-hilados y los diferentes hilos pueden ejecutar rutinas MPI pero solo una a la vez; Los llamados a MPI no pueden ser realizados en simultaneo por 2 hilos. 
    - MPI_THREAD_MULTIPLE (Nivel 3): Multiples hilos pueden realizar comunicaciones, sin restricciones. 

MPI_Init debe reeemplazarse por MPI_Init_thread para procesos multi-hilados: 
    MPI_Init_thread( int *argc, char ***argv, int required, int *provided)

//pagina 18 define como podemos configurar estas cuestiones. 

### Caso de estudio: Reduccion a suma en cluster de multicores: 
    Debemos desarrollar un algoritmo paralelo para computar la reduccion a suma de un vector. 
    La arquitectura de sporte de un cluster de 2 nodos donde cada nodo tiene 2 procesadores quad-core (4 nucleos por nodo). 
   
### Sobre Las sumarizaciones en cluster multicores: 
    
En la version de MPI se generan 16 Procesos (uno por cada nucleo), cada uno con un espacio de direcciones independiente. Esto implica que la suma parcial de cada proceso debe comunicarse via pasaje de mensaje a traves de la red de interconexion, requiriendo una operacion colectiva de reduccion (MPI_Reduce) que involucra a los 16 procesos. El costo de comunicacion es elecado dado que cada proceso participa en el intercambio de mensajes, y la sicronizacion es global al completar las operaciones colectivas!. 

En la version hibrida master-only se generan unicamente 2 procesos MPI (uno por nodo), y dentro de cada nodo se crean N hilos OpenMP que comparten todo su espacio de memoria. La suma parcial por nodo se obtiene mediante una reduccion en memoria compartida (sin intervencion de la red), y unicamente los procesos maestro intercambian un unico mensaje MPI para combiar los resultados parciales.

La creacion de hilos OpenMP tiene costos inferior al de procesos MPI (menor overhead de creacion, contexto y aislamiento) y la sincronizacion entree hilos se resuelce con barreras a nivel de nodo, sin participacion de la red. 

Conclusiones: el sistema hibrido establece una jerarquia de procesos, aca es importante entender las responsabilidades, MPI crea, sincroniza y comunicacion de procesos. Mientras que OpenMP/Pthread hilos, hilos siempre locales. Procesos poseen mecanismo de comunicacion global. 
 
