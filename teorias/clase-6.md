# Sistemas paralelos Clase 6:

### Estandar OpenMP:

**Modelo de programacion basado en directivas: Open MP:** 

- Modelo basado en threads → primitivas de bajo nivel.
- Modelos basados en directivas → constructores de alto nivel → la idea es liberar al programador del manejo explicito de hilos.

OpenMP 

Es un estanda de programacion paralela basado en directivas que puede ser usado en C, C++ y Fortran. Tiene 3 componentes primarios: 

- Directivas
- Funciones de librerias
- Variables de entorno

Sus directivas proveen soporte ara concurrencia, sincronizacion y manejo de datos obviando el uso explicito dde locks, variables condicion, alcance de los datos e inicializacion de threads. 

Sus directivas se traducen a codigo Pthreads

OpenMP se disenno en 1997 por el consorcio demismo nombre y aun se mantiene por el mismo. 

Su motivacion radica en proveer un mayor nivel de abstraccion al manejo de hilos para poder facilitar grandes programas paralelos. 

Soporte de compiladores: 

- Soporte de GNU es el compilador GCC → de caracter open Source
- Proveedor Intel ICC → de caracter privado

### Caracterisitcas basicas de OpenMP:

- Sintaxis de las directivas:
    
    ```c
    #pragma omp nombre_directiva [lista de clausulas]
    ```
    

### Modelo Fork-Join:

Comenzamos con un unico hilo. 

- FORK al encontrar un constructor paralelo, el hilo maestro crea un grupo de hilos.
- El bloque encerrada por el constructor de la region paralela es ejecutada en paralelo entre todos los hilos.
- JOIN cuando el conjunto de hilos finaliza el bloque paralelo, se sincronizan y terminan, continuando unicamente le hilo master.

### Costructor parallel:

Es el constructor mas importante, permite especificar un bloque de codigo que sera ejecutado en paralelo (region paralela)

Asegura la creacion de un equipo de ilos aunque la distribucion de trabajo dentro de la region paralela es responsabilidad del programador. 
dentro de la region paralela, cada hilo mantiene un ID unico (0 siemre es hilo master). 

Al final de la region paralela, hay una barrera implicita → solo el hilo master continua su ejecucion. 

```c
#pragma omp parallel [lista de cláusulas] 
{ … }

```

### Clausulas private y fistprivate:

Admite clausulas que determinan cuales datos seran privados a cada hilo y cuales seran compartidos entre todos los hilos de una region paralela. 

Las variables privadas de un hilo se especifican mediante la clausula private:

```c
#pragma omp parallel private(lista_de_variables)
{ … }
```

Esta clausula crea una copia local a cada hilo de cada variable especificada resetando su tipo y tamanno. Esta copia local solo puede ser accedida y modificada por el hilo que la posee. 
Variable: firstprivate. 

### Clausulas shared y default:

Las variables que son compartidas entre todos los hilos de un equipo se esecifican en la clausula shared: 

```c
#pragma omp parallel shared(lista_de_variables)
{ … }
```

En este caso, todos los hilos podran leer y modificar la variable original. Por defecto todas las variables son compatidas → para alterar este comportamiento, se puede emplear la clausula default:

```c
#pragma omp parallel default(shared|private|none)
{ … }
```

### Clausula num_threads:

Es para esecificar el numero de hilos a crear: 

```c
#pragma omp parallel num_threads(T)
{ … }
```

En caos de ausencia, el numero de hilos a crear lo determina la variable de entorno OMP_NUM_THREADS. 

### Clausula if:

Esta clausula permite condiciona la generacion de hilos a la evaluacion de una expresion escalar. 
Si la evaluacion de la expresion escalar resulta falsa, entonces el codigo se ejecutara de form secuencial. 
Puede ser util ara paralelizar solo cuando vale la pena. 

### Constructors para trabajo compartido:

La directiva paaralllel puede ser utilizada en conjunto con otras directivas ara especificar concurrrencia entre iteraciones y tareas (constructores de trabajo compartido) → No crea nuevos threads. 

Diferentes tipos de cosntructores: 

- Directiva for → divide las iteraciones de un bucle entre hilos (paralelismo de datos)
- Directiva sections → Divide el trabajo en secciones separadas (paralelismo funcional)

### Constructor For:

Sintaxis: 

```c
#pragma omp for [lista de cláusulas]
for ( init_exp; check_exp; mod_exp) 
```

El uso de esta directiva impone algunas restrucciones: 

- Interaciones deben ser independientes entre si.
- El numero de iteraciones debe ser conocido de antemano
- La variable indice se vuelve privada por defecto y no puede ser modificada por los hilos dentro del bucle
- No se puede usar break dentro de las iteraciones

El buble paralelo finaliza con una sincronizacion implicita entre todos los hilos que lo integran. 

Las clausulas disponibles son: 

- Shared, private, firstprivate
- lastprivate, funciona como private solo que la variable original queda con el valor de la ultima iteracion del bucle.
- reduction, realiza una operacion de reduccion usando el operador undicado con las multiples copias de la variable correspondiente.
- nowait → evita la barra implicita al final del bucle.
- schedule (politica [,chunk]): especifica como se distribuyen las iteraciones entre los hilos.
    - static: divide en bloques de chunk iteraciones y las asigna en forma RR. Cuando chunk no se especifica se dividen las iteraciones en bloques de tamanno aproximado.
    - dynamic: divide en bloques chunk iteraciones y las asigna bajo demanda. Cuando chunck no se especifica, las iteraciones son asignadas de a 1.
    - guided: basado en dynamic ero decrementando chink a medida que avanza el bucle. Cuando chunk=1, el bloque de iteraciones se asigna en forma proporcional a las iteraciones pendiente y los hilos que integran el bucle. Cuando chuck = k>1, el bloque se asigna de igual manera pero nunca menor a k.
    - auto: se delega la eleccion al compilador o al sistema
    - runtime: la planificacion la determina la variable de entorno OMP_SCHEDULE

### Constructor Sections:

```jsx
#pragma omp sections [lista de clausulas]
{
	#pragma omp section
	{//bloque estructurado}
	[#pragma omp section]
	{//bloque estructurado}
}
```

Util para la distribucion de trabajo no-interactivo. Por ejemplo, paralelismo funcional. Cada bloque de codigo indicado por la directiva section es independiente de los demas y es ejecutado una sola vez por un unico hilo, pudiendo hacerlo en paralelo con el resto de los hilos. 

Existe una barrera implica al final de sections. 

Clausulas disponibles: shared, private, fistprivate, lastprivate, reduction, nowait. 

### Combinacion de directivas:

Las directivas for y sections se pueden combinar con directivas parallel:

```c
#pragma omp parallel default (private) shared (n)
{
	#pragma omp for
	for(i=0; i<n;i++){
		/* cuerpo del bucle paralelo */
	}
}
```

Es equivalente a: 

```c
#pragma omp paralleel for default (private) share (n)
for (i=0; i<n;i++){
	/*cuerpo del bucle paralelo*/
}
```

- Se requiere que la variable de entorno OMP_NESTED tenga valor True. en algunos casos para que pueda ejecutarse de forma paralela.

### Constructores para sincronizacion:

- OpenMP provee de constructores de alto nivel para diferentes tipo de sincronizacion:
    - Ejecucion serial
    - Barreras
    - Secciones criticas
    - Atomicidad

**Constructor Single:** 

Permite que un bloque de codigo sea ejecutado por un unico hilo dentro de una region paralela. 

```c
#pragma omp single [lista de clausulas]
{/*bloque estructurado*/}
```

El bloque es ejecutado por el primer hilo del equipo que llega a ese punto de ejecucion; el resto de los hilos espera al final del bloque (hay una barrera implicita). 

Clausulas disponibles: private, fistprivate, nowait

### Constructor master:

Es una variable de la directiva single: el bloque de codigo siempre ejecutado por el hilo master. 

```c
#pragma omp master
{/*bloque estructurado*/}
```

A diferencia de single, no hay barrera implicita al final del bloque. 

### Constructor barrier:

Implementa un punto de sincronizacion global entre todos los hilos de un equipo(barrera). 

```c
#pragma omp barrier
```

Se debe usar con cuidado: 

- Puede causar deadlock.
- Incide en el rendimiento.

### Constructor Critical:

Permite implementar regiones criticas en forma sencilla. 

```c
#pragma omp critical [nombre]
{ /*bloque estructurado*/ }
```

Garantiza que, en cualquier punto de ejecucion del programa, a lo sumo un hilo estara dentro de la seccion critica nombre.

Si un hilo alcanza un bloque critical y ya hay otro que la misma, el hilo espera a que la seccion critica se libere. 

El nombre es opcional. Si no se especifica uno, entonces se usa un nombre por defecto que es el mismo para todas las secciones criticas que no tengan nombre (no es conveniente). 

### Constructor ordered:

Util para aquellos casos en que resulta necesario ejecutar cierto segmento de codigo en el mismo orden en que lo haria la version secuencial.

```c
#pragma omp ordered
{/*bloque estructurado*/}
```

Se emplea en el ambito de una directiva for o parallel for. 

- Requiere incluir clausula ordered

Tener en cuenta que la directiva ordered representa un punto de serializacion en la ejecucion → el bloque de codigo debe contener la minima cantidad de instruccione posibles. 

Solo tiene esntido si los hilos realizan trabajo significativo fuera del bloque constructor ordered. 

### Directiva flush:

se adopta un modelo relajado de consistencia de memoria, las variables suelen ser actualizadas en los registros o en la memoria cache, demorando su modificacion en la memoria principal. Si bien esto puede mejorar el rendimiento, tambien puede provocar una vista incosistente de la memoria para un hilo. 

La directiva flush representa un punto de sincronizacion de la memoria: 

- Todas las escrituras pendientes en memoria principal seran asentadas.
- Todas las lecturas pendiente seran realizadas desde memoria principal.

```c
#pragma omp flush [(lista de variables)]
```

NO suele ser muy usada ya que muchos de las directivas OpenMP incluyen un flush implicito:

- En la directiva barrier,
- a la entrada y a la salida de critical, ordered, parallel, parallel for, parallel sections;
- y a la salida de las directivas for, sections y single.

Excepciones: la clausula nowait exluye a flush; tampoco esta presente a la entrada de for, sections y single; ni a la entrada o salida de master. 

### Funciones de libreria:

Ademas de las directivas, OpenMP soporta una serie de funciones que permite al programador controlar la ejecucion del programo con mayor nivel de abstraccion de Pthreads. 

Funciones basicas: 

- void omp_set_threads (int num_threads);
    - Setea el valor de la variable de entorno OMP_NUM_THREADS, determinando el numeor de hilos que seran generados en las regiones paralelas que no especificen la clausula num_threads
- int omp_get_num_threads();
    - Retorna el numero de hilos de la region paralela actual.
- int omp_get_max_threads();
    - Retorna la cantidad maximo de hilos que se podrian generar en una region paralela.
- int omp_get_thread_num();
    - Retorna el ID del hilo que la invoco dentro de la region paralela actual.
- int omp_get_num_procs();
    - Retorna el numero de procesadores disponibles en el sistema.
- int omp_in_parallel();
    - Retorna true si el hilo que lo invoco esta dentro de una region paralela; Falso de lo contrario.

Funciones para controlar y monitorizar la creacion de hilos: 

- void omp_set_dynamic (int dynamic_threads);
    - Habilita o inhabilita el ajuste dinamico del numero de hilos a generar en las proximas regiones paralelas.
- int omp_get_dynamic();
    - Retorna true si el ajuste dinamico esta habilitado
- void omp_set_nested(int nested);
    - Habilita o inhabilita el paralelismo anidado.
- void omp_get_nested();
    - Retorna True si el paralelismo anidado esta habilitado

Funciones para controlar la planificacion de hilos: 

- void omp_set_schedule(omp_sched_t kind, int chunk);
    - permite configurar dinamicamente la planificacion a usar cuando se especifica runtime en la clasusula shcedule.
- void omp_get_schedule(omp_sched_t * kind, int * chunk);
    - retorna la planificacion que se usara cuando se especifica runtime en la clausula schedule.

OpenMP tambien ofrece funciones para el uso de locks, para aquellos casos en que las directuvas citical y atomic no sean suficientes/convenientes. 

El tipo de dato para los locks es omp_lock_t y ls funciones disposibles son 

- void omp_
    
    init_lock
    
    destroy_lock
    
    set_lock
    
    unset_lock
    
    test_lock
    

Funcionan en forma equivalente a los locks de pthreads. 

Ofrece funciones para la exclusion mutua recursiva. 

El tipo de dato para estas es omp_nest_lock_t

Las funciones son las mismas pero para si. 

Funcionan equivalente a los lock recursivos de Pthreads. 

### Variables de entorno:

Cuenta con un conjunto de variables de entorno. 

- OMP_NUM_THREADS
- OMP_DYNAMIC
- OMT_NESTED
- OMP_SCHEDULE

### OpenMP 3.0 Tasking:

El uso de trareas se introduce en la version 3.0

Una tarea es una unidad de trabajo (porcion de codigo) cuya ejecucion puede ser diferida en el tiempo. Se compone de: 

- Codigo a ejecutar
- Entorno de datos
- Variables de control internas

Pensado para paralelizar problemas irregulaes: 

- Bucles while
- Bucles for que no tienen una cantidad conocida de iteraciones
- Algoritmos recursivos
- otros

### OpenMP 3.0: Constructor task

- El programador identifica las tareas encerrando los bloques de codigo correspondientes bajo la directiva task → Se asume que todas las tareas son independientes entre si.
    
    ```c
    #pragma omp task [lista de clausulas]
    {/*bloque estructurado*/}
    ```
    
- Cuando un hilo encuentra un constructor task, el sistema de ejecucion genera una nueva tarea.
- El momento en que esta tarea se ejecute dependera del sistema de ejecucion, el cual puede ser inmediato o diferido
- Se permite el anidamiento de tareas → Una tarea puede generar otras tareas.

Clausulas disponibles: 

- shared, private, fitsprivate, default.
- untied: por defecto la tarea es ejecutada de inicio a fin por el mismo hilo (no necesariamente quien lo genera). untied permite que la tarea pueda ser completada por mas de un hilo
