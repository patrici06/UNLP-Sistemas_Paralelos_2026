### Agenda de la clase: 
+ Fundamento de programacion en pasaje de mensajes
+ Estandar MPI 

# Fundamentos de programacion en pasaje de mensajes: 
### Plataformas de memoria distribuida: 
Consisten de un conjunto de nodos de procesamiento, cada uno con su propio espacio de datos, Los nodos se comunican intercambiando mensajes tanto para sincronizacion como para comunicacion. 
Existen problemas de consistencia? -> Apostaria que si y que se debe hacer un esfuerzo para mantenerla. 
# Modelo de programacion asociado: pasaje de mensajes, es memoria distribuida. 

Consiste de un conjunto de procesos, donde cada uno cuenta con su propio espacio de direcciones(no se comparte).
Entonces es clave entender que el espacio de direcciones es particionado: 
    + Cada dato pertenece a una particion. 
    + Toda interaccion requiere la cooperacion de dos procesos. 
    
El intercambio de mensajes sirve para varios propositos: 
    + intecambio explicito de datos -> responsabilidad del programador
    + Sincronizacion de procesos. 
Generalmente usado en plataformas de memoria distribuida como clusters. 

+ Programabilidad -> En pasaje de mensajes se trabaja en bajo nivel. 
    - El programador es responsable de la distribucion de datos y el mapeo de procesos, asi como la comunicacion entre tareas. 
    - Dificil de programar, dificil de depurar, dificil de mantener. 
    - La mayoria de los programas se escriben siguiendo el modelo de Single Program Multiple Data (SPMD)
        - No todos ejecutan las mismas instrucciones (sentencias de seleccion). 
        - Los procesos no estan sincronizados en la ejecucion de cada sentencia. 

- Eficiencia -> el ajuste(Tuning) para mejorar el rendimiento puede ser optimo. 
    - Puede manejar el balance de carga, la redistribucion de datos y procesos (dinamicamente), replicar datos, entre otras tareas. 
    - Portabilidad -> Existen librerias para facilitar la ejecucion de codigo sobre diferentes arquitecturas (estandares) 
        - Buena portabilidad de codigo, NO necesariamente portabilidad de rendimiento.

Ventajas de dichos estandares: 
    - El programador tiene total control para lograr sistemas mas eficientes y escalables. 
    - Puede implementarse eficientemente en muchas arquitecturas paraleleas. 
    - Mas facil de predecir el rendimiento. 
    
Desventajas: 
    - Mayor complejidad al implementar estos algoritmos para lograr alto rendimiento. 
    
### Operaciones Send y Receive: 
    Los prototipos de las operaciones son: 
        Send (void *sendbuf, int nelems, int dest)
        Receive(void *recvbuf, int nelems, int source)
- La semantica del Send requiere que en P1 quede el valor 100 (no 0). 
- Existen diferentes protocolos para Send y Receive: bloqueante y no bloqueante. 

### Cuando una operacion de comunicacion es bloqueante: 
- Se devuelve el control al proceso llamador una ve que todos los recursos involucrados(por ejemplo buffer de envio/recepcion) pueden ser reutilizados -> aplica tanto al emisor como al receptor. 
- Se garantiza que todas transiciones de estados inciadas por las operaciones fueron completadas. 

Existe entonces Ociosidad en los procesos!. 
Hay entonces 2 alternativas: 
- Sin buffering
- Con buffering

### Send y Receive - Bloqueantes sin Buffering: 
En este modelo el send se bloque hasta que el receptor no termine el recive del mensaje. 
- Tiempos ocioso de los procesadores. 
- Deadlocks si las sentencias de comunicacion no coinciden. 


### Send y Receive - Bloquantes con buffering: 
En este modelo el send se bloquea hasta que el mensaje llega a un buffer prealocado del sistema (diferente al del receptor) (man in the middel)
- Transmision del mensaje: 
    Se tiene hardware para comunicacion asicronica(sin intervencion de la CPU) -> se comienza la transmision al buffer del receptor. 
    Sin hardware especial -> el emisor transmite el mensaje al buffer del receptor y recien ahi se desbloquea. 
        Entonces Sin hardware es sobre el proceso receptor, de tener hardware especial este asume el buffering y envio posterior. 
    
Protocolos con buffering reducen el tiempo ocioso de los procesadores pero aumentan el costo por manejo logicamente de buffers. 
- Tamano limitado de los buffers -> Osea una vez lleno bloquea al send hasta que haya lugar nuevamente. 
(productor consumidor)
- Buffering reduce la ocurrencia de deadlocks pero no los evita completamente!. 
Ejemplo: 
    P0
    receive(&a,1,1)
    send(&b,1,1)
    P1
    receive(&1,0)
    send(&b,1,0)

### Send y Receive - No bloqueantes: 
Para evitar overhead (ociosidad o manejo de buffer) se deveulve el control de la operacion  inmeditamente. 
    - No garantiza que los recursso involucrados puiedan ser reutilizados. 
    - No garantiza que todas transiciones de estados iniciadas por la operacion hayan sido completadas. 

Requiere un posterior chequeo para asegurarse la finalizacion de la comunicacion -> Deja en manos del programador asegurar la semantica del Send. 

Hay dos alternativas: 
    - Sin buffering: inicia comunicacion al llegar al receive. 
    - Con buffering: el emisor utiliza acceso directo a memoria (DMA) para copiar los datos a un buffer prealocado mientras el proceso continua su compuito (reduce el tiempo en que el dato no esta seguro). 

Un resumen de operaciones para operaciones Send y Receive: 
- Operaciones Bloqueantes con buffering:
    El emisor retoma el control una vez que los datos han sido copiados al buffer. 
    
- Operaciones no bloqueantes:
    El emisor retoma el control una vez que ha iniciado loa transferencia DMA al buffer, aun cuando podria no haberse completado. 

- Operaciones bloqueante Sin buffering: 
    El emisor se bloquea hasta que el receptor alcance el receive correspondiente

- Operaciones no bloqueante Sin buffering: 
   El emisor envia los datos cuando el receptor alcanza el receiv.
    
Entonces:

Operaciones bloqueante: La semantica del send y receive esta asegurada por la operacion correspondiente. 

Operaciones no bloquantes: El programador debe asegurar explicitametne el cumplimiento de la semantica de las operaciones. 

  
# Estandar Message Passing Interface (MPI): 
Estandar MPI 
En la decada del 90 existian multiples librerias para pasaje de mensajes (no compatibles obvio). Un grupo de representantes de universidades y de la industria se reunieron para desarrollar un estandar para programacion basada en pasaje de mensajes. 
MPI define una libreria estandar que puede ser empleada desde C o Fortran (potencialmente cualquie otro lenguaje). 
Existen diferentes implementaciones del MPI en la actualidad: 
    - OpenMPI
    - MPICH 
    - Intel MPI
    - IBM MPI 
aunque no todas soportan la especificacion en forma competa -> es algo que debemos onsiderar al elegir la implementacion MPI. 

+ Modelo Single Program Multiple Data (SPMD)
El estandar MPI define la sintaxis y la semantica de mas de 400 rutinas aunque basicamente con 6 podemos escribir progrmas paralelos basados en pasaje de mensajes: 
    - MPI_Init
    - MPI_Finalize
    - MPI_Comm_size
    - MPI_Comm_rank
    - MPI_Send
    - MPI_Recv
### MPI - Inicio y finalizacion de entorno: 
    - MPI_Init: Inicializa el entorno MPI. Debe ser invocada por todos los procesos antes que cualquier otro llamado a rutinas MPI. 
    MPI_Init (int *argc, char **argv)
algunas implementaciones de MPI requieren argc y argv para inicializar el entorno
    - MPI_Finalize: cierra el entorno MPI. debe ser invocado por todos los procesos on ultimo llamado a rutinas MPI. 
    MPI_Finalize()

### MPI - Comunicadores: 
+ Un comunicador define el dominio de comunicacion -> que procesos pueden comunicarse erntre si. 
+ Son variables del tipo MPI_Comm -> Almacena informacion sobre los procesos que pertenecen a el. 
+ Un proceso puede pertencer a muchos comunicadores. 
+ Existe un comunicador que incluye a todos los procesos de la aplicacion MPI_COMM_WORLD.
+ En cada operacion de transferencia se debe indicar el comunicador sobre el que se va a realizar. 

### MPI - Adquisicion de informacion: 
    + MPI_Comm_size: indica la cantidad de proceso en el comunicador. 
    MPI_Comm_size (MPI_Comm comunicador, int *cantidad). 

    + MPI_Comm_rank: indica el "rank"(identigicador) del proceso dentro de ese comunicador. 
     MPI_Comm_rank(MPI_Comm comunicador, int *rank)
        - rank es un valor entre [0..cantidad]
`       - Cada proceso puede tener un rank diferente en cada comunicador. 
    
### MPI - Operaciones de comunicacion: 
MPI soporta: 
    - Comnicaicones punto a punto: operaciones de comunicacion que involucran a dos procesos -> bloquante y no bloqueantes. 
    - Comunicaciones colectivas: operaciones de comunicacion que pueden involucrar a dos o mas procesos -> bloqueantes y no bloqueantes. 

### MPI - Comunicaciones putno a punto bloqueantes: 
- MPI_Send: rutina basica para enviar datos a otro proceso.
    MPI_Send: (void *buf, int cantidad, MPI_DataType tipoDato, int destino, int tag, MPI_Comm comunicador)
    Valor de Tag entre [0..MPI_TAG_UB]. 
Existen diferentes variantes para MPI_Send: 
    - MPI_Send: 
        - retorna el control uando el buffer emisor esta listo para ser reusado -> no significa que el receptor ya haya recibido. 
        - Podria involucrar uso de buffering o no -> depende de la implementacion. 
    - MPI_Bsend (Buffer send): 
        - Basdado en MPI_send   
        - Permite implementar buffering a nivel de usuario, ya que sea porque el sistema no lo implementa o porque se quiere tener mayor control sobre el mismo. 
    - MPI_Ssend (Synchonic Send): 
        - Retorna el control solo cuando el buffer del emisor esta listo para ser reusado y el proceso receptro ha comenzado a recibir el mensaje. 
    -MPI_Rsend(Ready Send):
        - Solo puede ser invocado si el proceso receptor se encuentra listo para recibir. De otro modo, la operacion es erronea y el resultado no esta garantizado. 
        - Puede mejorar el rendimiento de las comunicaciones a costo de mayor inseguridad en su uso. 

### Tipos de datos para las comunicaciones: 
    -----------------------------------------
    | MPI               | C                 |
    -----------------------------------------
    | MPI_CHAR          | signed char       |
    | MPI_INT           | signed short int  |
    | MPI_LONG          | signed long int   |
    | MPI_UNSIGNED_CHAR | unsigned char     | 
    | MPI_UNSIGNED_SHORT| unsigned short int|
    | MPI_UNSIGNED      | unisgned int      |
    | MPI_UNSIGNED_LONG | unsigned long int |
    | MPI_FLOAT         | float             |
    | MPI_DOUBLE        | double            | 
    | MPI_LONG_DOUBLE   | long double       |
    | MPI_BYTE          |                   | 
    | MPI_PACKED        |                   |
    -----------------------------------------

### MPI - Comunicaciones punto a punto bloqueantes: 
    - MPI_Recv: rutina basica para recibir datos de otro proceso. 
    MPI_Recv(void *buf, int cantidad, MPI_Datatype tipoDato, int origen, int tag, MPI_Comm comunicador, MPI_Status *estado)
        - Comodines MPI_ANY_SOURCE y MPI_ANY_TAG. 
        - Estructura MPI_Status
            typedef srtuct MPI_Status {
                                        int MPI_SOURCE; 
                                        int MPI_TAG; 
                                        int MPI_ERROR; 
                                        }

    - MPI_Get_count: rutina para obtener la cantidad de elementos recibidos. 
        MPI_Get_count(MPI_Status * estado, MPI_Datatype tipoDato, int *cantidad)

### Comunicaciones punto a punto Caso de deadlock
Ejemplo 1: 
    int a[10], b[10], cantProc, id; 
    MPI_Status estado; 
    MPI_Comm_size(MPI_COMM_WORLD, &cantProc); 
    MPI_Comm_rank(MPI_COMM_WORLD, &id); 
    MPI_Send (a, 10, MPI_INT, (id+1)%cantProc, 1, MPI_COMM_WORLD; 
    MPI_Recv(b, 10, MPI_INT, (id-1)+cantProc%cantProc,1,MPI_COMM_WORLD, estado); 
    
Ejemplo 2: 
    int a[10], b[10], identificador; 
    MPI_Status estado; 
    MPI_Comm-rank(MPI_COMM_WORLD, &identificador); 
    if(identificador == 0){
        MPI_Send(a, 10, MPI_INT, 1, 1, MPI_COMM_WORLD); 
        MPI_Send(a, 10, MPI_INT, 1, 2, MPI_COMM_WORLD); 
    }
    else{
        MPI_Recv(b,10, MPI_INT, 0, 2, MPI_COMM_WORLD, estado); 
        MPI_Recv(a, 10, MPI_INT, 0, 1, MPI_COMM_WORLD, estado); 
    }

### MPI Comunicaciones punto a punto no bloqueantes: 
- Comienzan las operaciones de comunicacion e inmediatemente devuelven el control (no garantiza que la operacion haya finalizado). 
    MPI_Isend(void *buf, int cantidad, MPI_Datatype tipoDato, int destino, int tag, MPI_Comm comunicador, MPI_Request *solicitud)
    MPI_Irecv(void *buf, int cantidad, MPI_Datatype tipoDato, int origen, int tag, MPI_Comm comunicador, MPI_Request * solicitud)

    MPI_Test: evalua si las operacion de comunicacion finalizo. 
        MPI_Test (MPI_Request *solicitud, int *flag, MPI_Status *estado)
    MPI_Wait:bloquea al proceso hasta que la operacion indicada en le Request haya finalizado. 
        MPI_Wait (MPI_Request *solicitud, MPI_Status *estado)
- Este tipo de comunicacion permite solapar computo con comunicacion. 
- Es responsabilidad de programador asegurar que la semantica de las operaciones se respete. 
    
// revisar ejemplos de la teoria pag 39

### MPI - Comunicaciones punto a punto - Orden y fairness: 
- Sobre orden: 
     - MPI asegura que los mensajes no se sobrepasan entre ellos. 
    - Si un proceso envia 2 mensajes seguidos a un mismo receptor (M1 y M2), 

### MPI - Comunicaciones punto a punto Orden y fairness
- Sobre orden: 
    - MPI asegura que los mensajes no se sobrepasan entre ellos.    
    - Si un proceso envia 2 mensajes seguidos a un mimso receptor (M1 y M2) y ambos coinciden con el mismo receive, el orden de recepcion sera: M1, M2. 
    - Si un proceso ejecuta 2 seguidos (R1, R2), y hay un mensaje pendiente que coincide con ambos, R1 recibira antes que R2. 
    
- Sobre fairness (justicia): 
    - MPI no asegura fairness -> es responsabilidad del programador que un proceso no sufra inanicion. 
    - Ejemplo: P0 le envia un mensaje a P2. Sin embargo, P1 envia otro mensaje a P2 que compite con el de P0 (coincide con el receive). P2 solo recibira uno de los 2 mensajes.

### MPI Comunicaciones colectivas: 
    - MPI provee un conjunto de funciones para realiar operaciones colectivas, sobre un grupo de procesos asociado con un comunicador. 
        - No solo facilitan la programacion sino que mejoran el rendimiento. 
        - Todos los procesos del comunicador deben llamar a la rutina colectiva. 
    - Tipos de operaciones colectivas: 
        - Sincronizacion: los procesos se bloquean hasta que todos hayan llegado a determinado punto del programa. 
        - Transferencia de datos: broadcast, scatter, gather y sus variantes. 
        - Computaciones colectivas: operaciones de reduccion. 

### MPI - Comunicaciones colectivas - Sincronizacion por barrera: 

- Sincronizacion en una barrera
    MPI_Barrier(MPI_Comm comunicador)

### MPI - Comunicaciones colectivas - Broadcast: 
- Broadcast: un proceso envia el mismo mensaje a todos los otros procesos del comunicador. 
    MPI_Bcast(void * buf, int cantidad, MPI_Datatype tipoDato, int origen, MPI_Comm)

### MPI - Comunicaciones colectivas - Gather
    Gather recolecta un vector de datos de cada proceso del comunicador (inclusive el destino) y los concatena en orden para dejar el resultado en un unico proceso. 
    
    MPI_Gather(void *sendbuf, int cantEnvio, MPI_Datatype tipoDatoEnvio, void *recvbuf, int cantRec, MPI_Datatype tipoDatoRec, int destino, MPI_Comm comunicador)

### MPI - Comunicaciones colectivas - Gatherv: 
- Gather requiere que todos los procesos aporten la misma cantidad de datos. Para casos en los que cada proceos envia una cantidad diferente, MPI cuenta con la variable Gatherv
    MPI_Gatherv (void *sendbuf, int cantEnvio, MPI_Datatype tipoDatoEnvio, void *recvbuf, int destino, MPI_Comm comunicador)

### MPI - Comunicaciones colectivas - Allgather: 
    Allgather: funciona como el Gather solo que el resultado es enviado a todos los procesos. 
    MPI_AllGather (void *sendbuf, int cantEnvio, MPI_Datatype tipoDatoEnvio, void *recvbuf, int cantRec, MPI_Datatype tipoDatoRec, MPI_Comm comunicador )

### MPI - Comunicaciones colectivas - Allgatherv
    MPI ofrece una variante de Allgather para aquellos casos en cada proceso puede enviar una cantidad diferente de datos. 
    MPI_Allgatherv (void *sendbuf, int cantEnvio, MPI_Datatype tipoDatoEnvio, void *recvbuf, int *cantsRec, int *desplazamientos, MPI_Datatype tipoDatoRec, MPI_Comm comunicador)

### MPI - Comunicaciones colectivas - Scatter
    Scatter: reparte un vector de datos entre todos los procesos (inclusive el mismo dueno del vector) de forma equitativa. 
    MPI_Scatter (void * sendbuf, int cantEnvio, MPI_Datatype tipoDatoEnvio, void *recvbuf, int cantRec, MPI_Datatype tipoDatoRec, int origen, MPI_Comm comunicador)

### MPI - Comunicaciones colectivas - Scatterv: 
Al igual que con Gather, MPI ofrece una variable de Scatter para aquellos casos en que hay que repartir una cantidad diferente a cada proceso: 
    MPI_Scatterv (void *sendbuf, int *cantsEnvio, int *desplazamientos, MPI_Datatype tipoDatoEnvio, void *recvbuf, int cantRec, MPI_Datatype tipoDatoRec, int origen, MPI_Comm comunicador)

### MPI - Comunicaciones colectivas - All to All: 
    All to All, cada proceso envia una parte de sus datos a cada uno de los otros procesos (inclusive a el mismo) y recibe de ellos una parte.
    - Todas las porciones son del mismo tamano
    - Es equivalente a realizar un Scatter + Gather. 
    MPI_Alltoall (void *sendbuf, int cantEnvio, MPI_Datatype tipoDatoEnvio, void *recvbuf, int cantRec, MPI_Datatype tipoDatoRec, MPI_Comm comunicador)
    MPI cuenta con la variable Allotoallv para los casos en que los procesos pueden enviar y recibir porciones de tamano diferente. 
    MPI_Alltoall(void *sendbuf, int *cantsEnvio, int *despEnvio, MPI_Datatype tipoDatoEnvio, void *recvbuf, int *cantsRec, int *despRec, MPI_Datatype tipoDatoRec, MPI_Comm comunicador)
    
### MPI - Comunicaciones colectivas Reducciones: 
    - Reduccion de todos a uno, combina los elementos enviados por cada uno de los procesos, inclusive el destino aplicando cierta operacion

    MPI_Reduce (void *sendbuf, void *recvbuf, int cantidad, MPI_Datatype tipoDato, MPI_Op operacion, int destino, MPI_Comm comunicador)

    - Reduccion de todos a todos: el resultado de la operacion de reduccion es enviado a todos los procesos. 
    MPI_Allreduce (void *sendbuf, void *recvbuf, int cantidad, MPI_Datatype tipoDato, MPI_Op operacion, MPI_Comm comunicador)

//Ejemplo muy bueno pagina 59 del uso de los Scatters y Reduce. 

### MPI - Grupos y Comunicadores: 
En Ocasiones, las operaciones de comunicacion entre los parocesos de un programa se realizan entre subconjunto de ellos -> MPI provee mecanismos para dividir el grupo de procesos asociado a un comunicador en varios subgrupos (cada uno con su correspondiente comunicador). 
    
    MPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm *newcomm)

=========== revisar porque es interesante ================

### MPI-2 y MPI-3
MPI-1 fue introducida en 1994, aunque el foro de MPI continuo trabajando en correcciones y extensiones de esa primera version.
MPI-2 fue publicada en 1998 como una version superadora de la primera. Incluyo funcionalidad para: 
    - Generacion dinamica de procesos 
    - Comunicaciones one-sided
    - Soporte para comunicaciones colectivas de a grupos
    - Soporte para C++
    - E/S paralela
MPI-3 fue aprobado en 2012. Incluyo funcionalidad para: 
    - Comunicaciones colectivas no bloqueantes
    - Mas soporte para comunicaciones one-sided
    - Mas soporte para comunicaciones colectivas de a grupos
    - Soporte para Frotran 2008

### MPI hoy: 
La mayoria de las implementaciones sigue la version 1.2 del estandar. 
Revisar las implementacionde de Java-MPI y Rust rsmpi
