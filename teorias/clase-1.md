Clase 1: 

###Procesamiento paralelo: 
    + Es el uso de multiples unidades de procesamiento para resolver probelma computacional. 
    + El problema se divide en partes separadas que pueden ser resueltas en forma concurrente. 
    + Cada parte es luego dividida en una serie de instrucciones. 
    + Las instrucciones de cada parte se ejecutan simultaneamente sobre diferentes procesadores. 
    + Se requiere mecanismo de control/coordinacion global.
El procesamiento paralelo, permite resolver problemas grandes o complejos. Provee concurrencia implicitamente. 
    + Reducir tiempos de computo, lo que reduce costos. 
    + Hacer mejor uso del hardware. 
Los problemas de caracter demandante en computo tales como aplicaciones cientificas, industria o comercio son algunos de los ejemplos de sectores donde la paralizacion es deseada. 

### Procesamiento concurrente, paralelo y distribuido: 
    + Concurrente, aquel en el que multiples tareas puede estar avanzado en cualquier instante de tiempo. 
    + Paralelo es aquel en el que multiples tareas que se ejecutan simultaneamente cooperan para resolver un problema. 
    + Distribuido es aquel en el que multiples tareas que se ejecutan fisicamente en diferentes lugares cooperan para resolver uno o mas problemas. 

El procesamiento paralelo busca reducir el tiempo de ejecucion de un programa empleando multiples procesadores al mismo tiempo. 
    + El hardware brinda respuesta pero lo fundamental sigue siendo el software. 
    + Se tiene una gran dependencia del software. 
Un sistema distribuido es un conjunto de computadoras autonomas interconecatadas que cooperan compartiendo recursos (fisicos y datos). 
    + Pueden ejecutar multiples aplicaciones de diferentes usuarios
    + Sus caracteristicas y aplicaciones objetivos dependen de la granularidad de sus nodos y el grado de acoplamiento de los procesadores.
    Se caracteriza por problemas de no tener un reloj unico, se requiere planificacion, la escalabilidad depende de las comunicaciones heterogeneidad, seguridad de los datos. 
    
### Computo de alto rendimiento,consta dde sistemas de extraordinario poder computacional y de tecnicas de procesamietno paralelo para la resolucion de problemas complejos con alta demanda computacional. 
    
## Clasificaicon de las plataformas de computo paralelo: 
    Es fundamental conocer las caracteristicas de la arquitectura adyacente. 

    Podemos diferenciarlas por:
    + Por el mecanismo de control: 
        - Corresponde a la Taxonomia de Flynn
        - De acuerdo a como se especifica el paralelismo entre instrucciones y datos. 
    + Por la organizacion fisica: 
        - de acuerdo al espacio de direcciones que tiene cada procesador (es decir, como ve la memoria principal). 
    
### Por Mecanismo de control: 
    - SISD -> single instruction stream Single Data Stream
        instrucciones ejecutan de forma secuencial, una por ciclo de reloj. Los datos afectados son aquellos que hace refgerencia la instruccion en cuestion. Ejecucion determinisitca (ej mainframes)
    - SIMD -> Single Instruction Multipe Data Stream
        Todas las unidades de procesamiento ejecutan la misma instruccion sobre diferentes datos. Sicronica y deterministica. hardware simplificado. Unidades pueden desactibarse selectivamente. Ideal para problemas de alta granularidad (procesamiento de imagenes)
    - MISD -> Multiple Instruction Single Data Stream
        Las unidades de procesamient ejecutan diferentes instrucciones sobre mismo dato, no existen maquinas reales basadas en este modo.
    - MIMD -> Multiple Instruction Multiple Data Stream
        unidades de procesamiento ejecutan diferentes instrucciones sobre diferentes instrucciones sobre diferentes datos. Puede ser sincronica o asincronica, deterministica o no deterministica. Pueden ser maquinas de memoria compartida o de memoria distribuida, la clase mas comun de maquinas paralelas. 
d
### Por Mecanismo de la organizacion fisica: 
    Se realiza de acuerdo al espacio de direcciones que tiene cada procesador -> en otras palabras, tiene en cuanta la vision de la memoria principal por parte de cada procesador. 
    Las opciones son:
        + Memoria compartida
        + Memoria Distribuida
        + Memoria hibrida
    ### Memoria Compartida: 
        Estos pueden variar pero se caracterizan por la capacidad que tienen sus procesadores de acceder a toda la memoria como un unico espacio de direcciones global. 
        Cuentan con multiples procesadores que operan de forma independiente pero que comparten los mismos recursos de memoria, los cambios realizados por un procesaddor seran visibles para el resto. 
    Se necesita mecanismo de coherencia de cache. 
    Sub-clasificacion por modo de acceso a memoria: 
    - Acceso uniforme a memoria (UMA)
    - Acceso no uniforme a memoria (NUMA)
    Ventajas:
        la comunicacion de datos entre los procesadores es mas rapida y uniforme debido a su cercania, la programacion de estos sistemas sule ser facil debido al espacio de direcciones global. 
    Desventajas: 
        Se dene asegurar un corecto acceso a los datos por parte de los procesadores, no da escalabilidad entre las memorias y los procesadores, agregar mas procesadores incrementa el trafico de memoria y complejiza los mecanismos de coherencia de cache. 
   ### Memoria Distribuida: 
    Se caracterizan por requierir de una red de comunicaciones para poder conectar a sus procesadores, Cada procesador opera en forma independiente y tiene su propia memoria, no cuenta con espacio de direcciones global -> esto significa que los cambios realizados por un procesador no seran visibles para el resto. 
    Cuando un procesador necesita un dato que posee otro, el programador se vuelve responsable de definir como y cuando sera comunicado.    No se requiere un mecanismo de coherencia de cache, ya que las memorias son locales. 
    Redes de interconexion: Ethernet, infiniband, Myrinet. 
    Ventajas: 
        La memoria escala con el numero de procesadores, en proporcionalidad debido al espacio de direcciones distribuido. 
        Cada procesador accede de forma mas rapida a los datos, no hay necesidad de un protocolo de coherenca de cache. 
        Buena relacion costo-rendimiento se pueden construir a partir de maquinas convencionales. 
    Desventajas:    
        - Accso NUMA, los datos qu estan en nodos remotos toma tiempo.
        - El manejo de la conicaicon se vuelve explicito. 
        - El espacio de direccionamiento distribuido puede coindicionar la programacion. 

### Memoria hibrida: 
    son multiples maquinas de memoria compartida son inter-conectadas entre si para permitir que sus procesadores puesn comunicarse.
    Ventajas y desventajas: 
    + lo que es comuin a ambos modelos
    + soluciona el problema de escalabnilida de memoria compartida
    + aumenta la complejidad de programacion


## Modelos de programacion Paralela: 
    Generalmente tomamos 2 alternativas: 
    - Memoria compartida 
    - Pasaje de mensajes
### Modelo de memoria compartida: 
    en el modelo de memoria compartida, multiples tareas se ejecutan en forma concurrente, todas las tareas acceden a un mapa de memoria comun y ademas cada una puede tener memoria local "exclusiva" 
    La comunicacion y sincronizacion de estas tareas se realiza escribirndo y leyendo areas de memoria compartida. 
    Generalmente usado en plataformas de memoria compartida como multiprocesadores o multicores. 
    El programador en general no maneja la distribucion de los datosni lo relaciona a la comunicacion de los mismos. 
    + Ventaja: transparencia para el programador. La ubicacion de los datos, su replicacion y su migracion son transparentes. 
    + Desventaja: A veces es necesario trabajar sobre esos aspectos para mejorar el rendimiento. es dificil la prediccion de perdformance a partir de la lectura del algoritmo. 
    
### Modelo de Pasaje de Mensajes: 
    Consiste en p procesos (eventualmente procesadores), cada uno de ellos con su espacio de direcciones exclusivo. 
    Caracterisitca clave: Espacio de direcciones particionado: 
        + Cada dato pertenece a una particion 
        + toda interaccion requiere la cooperacion de dos procesos. 
    El intercambio de mensajes sirve para varios propositos: 
        - Intercambio explicito de datos (programador)
        - Sincronizar procesos. 
    Ejemplo de uso clusters
    Ventajas: 
    - El programador tiene control total pra lograr sistemas eficientes y escalables. 
    - Puede implementarse eficientemente en muchas arquitecturas paralelas. 
    - Mas facil de predecir el rendimiento. 
    Desventajas: 
    - Mayor complejidad al implementar estos algoritmos para lograr alto rendimiento. 
    
### Modelos de programacion y arquitecturas paralelas: 
      Se suele asociar cada modelo con su correspondiente arquitectura paralela. 
    Sin embargo estos modelos son abstracciones y deben de poder implementarse en cualquier plataforma paralela: 
    + un modelo de memoria compartida sobre una maquina de memoria distribuida
    + Un modelo de pasaje de mensajes sobre una maquina de memoria compartida. 
El uso de un modelo que no resulte natural a la arquitectura probablemente provoque una degradacion del rendimiento. 

### Pipelining: 
    Consiste basicamente en solapar las diferentes etapas de la ejecucion de instrucciones, reduciendo el tiempo de ejecucion total. 
### ILP - Pipelining: 
    El modelo de referencia es el de "linea de montaje"
    Para aumentar la velocidad del pipeline podemos dividirlo en etapas mas pequenas incrementando su profundidad. 
       - En el contexto de los procesadores, esto permite a su vez mayores frecuencias de reloj. 
       - Tener en cuanta que la velocidad de un pipeline esta limitada por la duracion de su etapa mas costosa -> Etapa mas pequenas aceleran el pipeline. 
### Pipelining y ejecucion especulativa: 
    Pipelines profundos requieren de tecnicas efectivas que sean capaces de predecir los saltos de forma especulativa. 
    + La penalizacion de un salto mal predicho se incrementa a medida que el pipeline es mas profundo -> Mas instrucciones adelantadas son las que se pierden. 
    + Estos factores limitan la profundidad de pipeline y su posible ganancia de rendimiento. 
    Como mejorar entonces la tasa de ejecucion de instrucciones? 
    + Una forma obvia consiste en usar multiples pipelines, durante cada ciclo de reloj, multiples instrucciones son emitidas en paralelo, las cuales son ejecutadas en multiples unidades fincionales. 
### Pipelining y ejecucion superescalar: 
    + No hay una unica manera de escribir un programa y la misma tiene incidencia en el rendimiento final. 
    + El ideal seria que todas las etapas esten activas en todo momento(maximo paralelismo). En la practica es muy dificil que ocurra. 

    + Al momento de realizar la planifcacion de instrucciones, se deben tener en cuenta: 
        - Dependencia verdadera de datos, el resultado de una instruccion es la entrada de la siguiente. 
        - Dependencia de recursos, dos operaciones requieren el mismo recurso. 
        - Dependencia de salto, las instrucciones a ejecutar despues de un salto condicional no pueden ser determinadas a proiori sin margen de error.
    + El planificador (hardware) analiza un conjunto de instrucciones de la cola de instrucciones a ejecutar y emite aquellas que pueden ser ejecutadas en forma concurrente, teniendo en cuenta las dependencias. 
    - Si las instrucciones son ejecutadas en el orden en que aparecen en la cola, se dice que la emision es en orden -> simple pero limita la emision de instrucciones
    - Si el procesador tiene la habilidad de reordenar las instrucciones en la cola, entonces se puede alcanzar el maximo rendimiento posible -> este modelo se conoce como fuera de orden y aunque es mas complejo, es el mas usado en la actualidad. 
    
### Instrucciones SIMD: 
    Incorporacion de unidades de procesamiento vectorial que permiten ejecutar una instruccion sobre diferentes datos en un ciclo de reloj. 

### Memory Wall, indica que llegara un momento en que el rendimiento estara dominado por la velocidad de la memoria (pone una pared fisica). 
### ILP Wall: 
    Si bien es posible agregar mas unidades funcionales al chip no se alcanzaria una mejora de rendimiento para las aplicaciones debido a no es posible extraer mas ILP de los programas. 
    Problemas: 
    + Limintaciones de ls compiladores
    + Dependendias entre instrucciones
    + Imposibilidad de predecir saltos
    + Cantidad limitada de paralelismo intrinseco
### Power Wall
    La potencia usada en el procesador se vuelve calor, si la temperatura aumenta se reduce necesariamente la velocidad de los transmisores y puede llevar fallos o destruccion del procesador. 
    El problema es que el crecimiento de tension, calor producido es cuadratico y esto significa que se llega a un punto verdaderamente imposible para nosotros refrigerarlo. 

###Procesadores multicore: 
    Desde su desarrollo, los procesadores multicore han sofisticado su diseno en las sucesivas familias. 
    Los primeros procesadores de este tipo eran practicamente dos procesadores mononucleo en la misma oblea. 
    Las siguientes generaciones han incrementado el numero de nucleos e incorporado niveles de chache L2 y L3. 
    
### Multi-hilado a nivel hardware: tecnica complementaria a los multicore: 
    un unico hilo de ejecucion no resulta suficiente para aprovechar la potencia de los procesadores superescalares. 
    El pipeline del procesador puede quedar atascado(stalled):
        + Al esperar el resultado de una operacion larga de punto flotante (o entera). 
        + Al esperar datos que deben llegar desde la memoria
    Mientras tanto, el resto de las unidades esperan en forma ociosa. 
    La tecnica de simultaneous Multi-Threading (SMT) consiste en mantener mas de un hilo de ejecucion al mismo tiempo en el procesador. 
    Los recursos asociados al estado del procesador son replicados una o mas veces. Manteniendo el numero original de recursos de ejecuciuon, solo requiere un pequeno incremento en el tamano del chip.
    Con esta replicacion el procesador parece tener multiples nucleos y por lo tanto puede ejecutar multiples flujos en parelo, sin importar si pertenecen al mismo programa o a diferentes. 
    El numero de replicaciones de estados determina el numero de procesadores logicos del procesador. 
    
Sin SMT solo un hilo de ejecucion puede estar activo en un determinado momento. 
En un procesador con SMT ambos hilos pueden estar activos al mismo tiempo, aunque no pueden usar la misma unidad funcional al mismo tiempo.
 
Los nucleos de un procesador multicore pueden tener multiples hilos hardware o no. 
Las posibles combinaciones: 
    + Monocore, sin SMT: procesadores mononucleos convencionales
    + Monocore con SMT
    + Multicore, sin SMT
    + Multicore con SMT
Numeros de hilo hardware: 2 o 4 usualmente, raramente mas. 
    
Es una buena idea buscare ejemplos de procesadores con y sin SMT. 

SMT puede mejorar la productividad del procesador siempre y cuando sea posible intercalar instrucciones de multiples hilos entre los pipelines. 
El escenario ideal seria tener multiples hilos que usen recursos de ejecucion diferentes -> Lamentablemente esto no es comun en la practica. en ocaciones el numero de referencia a memoria de un programa escacla con el numero de hilos por lo que se puede dar un mejor aprovechamiento del ancho de banda si tenemos un gran numero de hilos. 
La Desventaja de SMT es que si los hilos usan exactamente los mismos recursos, podria haber perdida de rendimiento por la competencia entre ellos. Por ejemplo, programas que solo computan en punto flotante. 

La ganancia por el uso de SMT depende fuertemente del programa a ejecutar. Generalmente lo que conviene es hacer pruebas con y sin SMT para evaluar el beneficio real. 

### Jerarquia de memoria en procesadores multicore: 
+ Los procesadores multicore suelen tener memorias caches de multiples niveles
    + El nivel 1 siempre es privado
    + Los siguientes niveles varian de acuerdo a la arquitectura.

### Jerarqui de memoria en procesadores multicore: 
    + Ventajas de las caches privadas: 
        - Al estar mas cerca de los cores, el acceso a los datos es mas rapido. 
        - Se reduce la competencia en el acceso a los recursos
    + Ventajas de las cache compartidas: 
        - Hilos en diferentes nucleos pueden compartir datos que estan en la misma cache
        - Mas espacio de cache disponible si un unico hilo, se ejecuta en el procesador. 
    
### Clusters
    Coleccion de computadoras individuales interconectadas via algun tipo de red, que trabajan en conjunto como un unico recurso integrado de computo. 
    + Cada nodo de procesamiento es un sistema de computo en si mismo, con hardware y sistemas operativos propio. 
    + La red de interconexion puede ser Ethernet o redes especificas de alta velocidad como Infiniban o Myrinet
    + Puede ser homogeno o heterogeneo
    + Ofrecen una buena relacion costo-rendimiento y son faciles de expandir. 
    
Los clusters se volvieron populares en la decada `90, eran clusters de tipo Beowulf. Actulmente, la mayoria de los grandes sistemas de computo se basan en clusters de nodos multi/many-core, conformado una arquitectura hibrida

+ Dado que los clusters son arquitecturas distribuidas, el modelo de programacion mas utilizado suele ser pasaje de mensajes. 
+ Tambien es posib le emplear el modelo de memoria compartida, aunque a costo de menor rendimiento.
+ Con la incorporacion de los procesadores multicore a los clustgers, sugirio un nuevo modelo de programacion que combina pasaje de mensajes con memoria compartida.  
