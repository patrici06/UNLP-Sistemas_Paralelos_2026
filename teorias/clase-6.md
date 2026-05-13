# Sistemas Paralelos - Clase 6

## Estandar OpenMP

**Modelo de programación basado en directivas: OpenMP**

- Modelo basado en threads → primitivas de bajo nivel.
- Modelos basados en directivas → constructores de alto nivel → la idea es liberar al programador del manejo explícito de hilos.

OpenMP es un estándar de programación paralela basado en directivas que puede ser usado en C, C++ y Fortran. Tiene 3 componentes primarios:

- Directivas
- Funciones de librerías
- Variables de entorno

Sus directivas proveen soporte para concurrencia, sincronización y manejo de datos obviando el uso explícito de locks, variables condición, alcance de los datos e inicialización de threads.

Sus directivas se traducen a código Pthreads.

OpenMP se diseñó en 1997 por el consorcio del mismo nombre y aún se mantiene por el mismo.

Su motivación radica en proveer un mayor nivel de abstracción al manejo de hilos para poder facilitar grandes programas paralelos.

**Soporte de compiladores:**

- Soporte de GNU es el compilador GCC → de carácter open source
- Proveedor Intel ICC → de carácter privado

---

## Características básicas de OpenMP

- Sintaxis de las directivas:
    
```c
#pragma omp nombre_directiva [lista de clausulas]
```

---

## Modelo Fork-Join

Comenzamos con un único hilo. 

- **FORK**: al encontrar un constructor paralelo, el hilo maestro crea un grupo de hilos.
- El bloque encerrado por el constructor de la región paralela es ejecutado en paralelo entre todos los hilos.
- **JOIN**: cuando el conjunto de hilos finaliza el bloque paralelo, se sincronizan y terminan, continuando únicamente el hilo master.

---

## Constructor parallel

Es el constructor más importante, permite especificar un bloque de código que será ejecutado en paralelo (región paralela).

Asegura la creación de un equipo de hilos aunque la distribución de trabajo dentro de la región paralela es responsabilidad del programador. 

Dentro de la región paralela, cada hilo mantiene un ID único (0 siempre es hilo master). 

Al final de la región paralela, hay una barrera implícita → solo el hilo master continúa su ejecución. 

```c
#pragma omp parallel [lista de cláusulas] 
{ … }
```

---

## Cláusulas private y firstprivate

Admite cláusulas que determinan qué datos serán privados a cada hilo y cuáles serán compartidos entre todos los hilos de una región paralela. 

Las variables privadas de un hilo se especifican mediante la cláusula **private**:

```c
#pragma omp parallel private(lista_de_variables)
{ … }
```

Esta cláusula crea una copia local a cada hilo de cada variable especificada reseteando su tipo y tamaño. Esta copia local solo puede ser accedida y modificada por el hilo que la posee. 

**Variable: firstprivate.**

---

### Cláusulas shared y default:

Las variables que son compartidas entre todos los hilos de un equipo se especifican en la cláusula **shared**: 

```c
#pragma omp parallel shared(lista_de_variables)
{ … }
```

En este caso, todos los hilos podrán leer y modificar la variable original. Por defecto todas las variables son compartidas → para alterar este comportamiento, se puede emplear la cláusula **default**:

```c
#pragma omp parallel default(shared|private|none)
{ … }
```

---

## Cláusula num_threads

Es para especificar el número de hilos a crear:

```c
#pragma omp parallel num_threads(T)
{ … }
```

En caso de ausencia, el número de hilos a crear lo determina la variable de entorno **OMP_NUM_THREADS**.

---

## Cláusula if

Esta cláusula permite condicionar la generación de hilos a la evaluación de una expresión escalar.
Si la evaluación de la expresión escalar resulta falsa, entonces el código se ejecutará de forma secuencial.
Puede ser útil para paralelizar solo cuando vale la pena.

---

## Constructores para trabajo compartido

La directiva **parallel** puede ser utilizada en conjunto con otras directivas para especificar concurrencia entre iteraciones y tareas (constructores de trabajo compartido) → No crea nuevos threads.

Diferentes tipos de constructores:

- **Directiva for** → divide las iteraciones de un bucle entre hilos (paralelismo de datos)
- **Directiva sections** → Divide el trabajo en secciones separadas (paralelismo funcional)

---

## Constructor For

Sintaxis: 

```c
#pragma omp for [lista de cláusulas]
for ( init_exp; check_exp; mod_exp) 
```

El uso de esta directiva impone algunas restricciones: 

- Iteraciones deben ser independientes entre sí.
- El número de iteraciones debe ser conocido de antemano
- La variable índice se vuelve privada por defecto y no puede ser modificada por los hilos dentro del bucle
- No se puede usar **break** dentro de las iteraciones

El bucle paralelo finaliza con una sincronización implícita entre todos los hilos que lo integran.

Las cláusulas disponibles son:

- **shared**, **private**, **firstprivate**
- **lastprivate**: funciona como private solo que la variable original queda con el valor de la última iteración del bucle.
- **reduction**: realiza una operación de reducción usando el operador indicado con las múltiples copias de la variable correspondiente.
- **nowait** → evita la barrera implícita al final del bucle.
- **schedule (política [,chunk])**: especifica cómo se distribuyen las iteraciones entre los hilos.
  - **static**: divide en bloques de chunk iteraciones y las asigna en forma RR. Cuando chunk no se especifica se dividen las iteraciones en bloques de tamaño aproximado.
  - **dynamic**: divide en bloques chunk iteraciones y las asigna bajo demanda. Cuando chunk no se especifica, las iteraciones son asignadas de a 1.
  - **guided**: basado en dynamic pero decrementando chunk a medida que avanza el bucle. Cuando chunk=1, el bloque de iteraciones se asigna en forma proporcional a las iteraciones pendientes y los hilos que integran el bucle. Cuando chunk = k>1, el bloque se asigna de igual manera pero nunca menor a k.
  - **auto**: se delega la elección al compilador o al sistema
  - **runtime**: la planificación la determina la variable de entorno **OMP_SCHEDULE**

---

## Constructor Sections

```c
#pragma omp sections [lista de clausulas]
{
    #pragma omp section
    { /*bloque estructurado*/ }
    [#pragma omp section]
    { /*bloque estructurado*/ }
}
```

Util para la distribución de trabajo no interactivo. Por ejemplo, paralelismo funcional. Cada bloque de código indicado por la directiva **section** es independiente de los demás y es ejecutado una sola vez por un único hilo, pudiendo hacerlo en paralelo con el resto de los hilos.

Existe una barrera implícita al final de **sections**.

Cláusulas disponibles: **shared**, **private**, **firstprivate**, **lastprivate**, **reduction**, **nowait**.

---

## Combinación de directivas

Las directivas **for** y **sections** se pueden combinar con directivas **parallel**:

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
#pragma omp parallel for default (private) shared (n)
for (i=0; i<n;i++){
	/*cuerpo del bucle paralelo*/
}
```

- Se requiere que la variable de entorno **OMP_NESTED** tenga valor **True** en algunos casos para que pueda ejecutarse de forma paralela.

---

## Constructores para sincronización

OpenMP provee de constructores de alto nivel para diferentes tipos de sincronización:

- Ejecución serial
- Barreras
- Secciones críticas
- Atomicidad

### Constructor Single

Permite que un bloque de código sea ejecutado por un único hilo dentro de una región paralela. 

```c
#pragma omp single [lista de clausulas]
{/*bloque estructurado*/}
```

El bloque es ejecutado por el primer hilo del equipo que llega a ese punto de ejecución; el resto de los hilos espera al final del bloque (hay una barrera implícita). 

Cláusulas disponibles: **private**, **firstprivate**, **nowait**

---

### Constructor master

Es una variante de la directiva **single**: el bloque de código siempre ejecutado por el hilo master. 

```c
#pragma omp master
{/*bloque estructurado*/}
```

A diferencia de **single**, no hay barrera implícita al final del bloque. 

---

### Constructor barrier

Implementa un punto de sincronización global entre todos los hilos de un equipo (barrera). 

```c
#pragma omp barrier
```

Se debe usar con cuidado: 

- Puede causar deadlock.
- Incide en el rendimiento.

---

### Constructor Critical

Permite implementar regiones críticas en forma sencilla. 

```c
#pragma omp critical [nombre]
{ /*bloque estructurado*/ }
```

Garantiza que, en cualquier punto de ejecución del programa, a lo sumo un hilo estará dentro de la sección crítica **nombre**.

Si un hilo alcanza un bloque **critical** y ya hay otro que la misma, el hilo espera a que la sección crítica se libere. 

El nombre es opcional. Si no se especifica uno, entonces se usa un nombre por defecto que es el mismo para todas las secciones críticas que no tengan nombre (no es conveniente). 

---

### Constructor ordered

Util para aquellos casos en que resulta necesario ejecutar cierto segmento de código en el mismo orden en que lo haría la versión secuencial.

```c
#pragma omp ordered
{/*bloque estructurado*/}
```

Se emplea en el ámbito de una directiva **for** o **parallel for**. 

- Requiere incluir cláusula **ordered**

Tener en cuenta que la directiva **ordered** representa un punto de serialización en la ejecución → el bloque de código debe contener la mínima cantidad de instrucciones posibles. 

Solo tiene sentido si los hilos realizan trabajo significativo fuera del bloque constructor **ordered**. 

---

### Directiva flush

Se adopta un modelo relajado de consistencia de memoria, las variables suelen ser actualizadas en los registros o en la caché, demorando su modificación en la memoria principal. Si bien esto puede mejorar el rendimiento, también puede provocar una vista inconsistente de la memoria para un hilo. 

La directiva **flush** representa un punto de sincronización de la memoria: 

- Todas las escrituras pendientes en memoria principal serán asentadas.
- Todas las lecturas pendientes serán realizadas desde memoria principal.

```c
#pragma omp flush [(lista de variables)]
```

NO suele ser muy usada ya que muchos de las directivas OpenMP incluyen un **flush** implícito:

- En la directiva **barrier**,
- a la entrada y a la salida de **critical**, **ordered**, **parallel**, **parallel for**, **parallel sections**;
- y a la salida de las directivas **for**, **sections** y **single**.

Excepciones: la cláusula **nowait** excluye a **flush**; tampoco está presente a la entrada de **for**, **sections** y **single**; ni a la entrada o salida de **master**. 

---

## Funciones de librería

Además de las directivas, OpenMP soporta una serie de funciones que permite al programador controlar la ejecución del programa con mayor nivel de abstracción de Pthreads. 

### Funciones básicas

- `void omp_set_threads (int num_threads);`
  - Setea el valor de la variable de entorno **OMP_NUM_THREADS**, determinando el número de hilos que serán generados en las regiones paralelas que no especifican la cláusula **num_threads**
- `int omp_get_num_threads();`
  - Retorna el número de hilos de la región paralela actual.
- `int omp_get_max_threads();`
  - Retorna la cantidad máxima de hilos que se podrían generar en una región paralela.
- `int omp_get_thread_num();`
  - Retorna el ID del hilo que lo invocó dentro de la región paralela actual.
- `int omp_get_num_procs();`
  - Retorna el número de procesadores disponibles en el sistema.
- `int omp_in_parallel();`
  - Retorna true si el hilo que lo invocó está dentro de una región paralela; falso de lo contrario.

### Funciones para controlar y monitorizar la creación de hilos

- `void omp_set_dynamic (int dynamic_threads);`
  - Habilita o inhabilita el ajuste dinámico del número de hilos a generar en las próximas regiones paralelas.
- `int omp_get_dynamic();`
  - Retorna true si el ajuste dinámico está habilitado
- `void omp_set_nested(int nested);`
  - Habilita o inhabilita el paralelismo anidado.
- `void omp_get_nested();`
  - Retorna true si el paralelismo anidado está habilitado

### Funciones para controlar la planificación de hilos

- `void omp_set_schedule(omp_sched_t kind, int chunk);`
  - Permite configurar dinámicamente la planificación a usar cuando se especifica **runtime** en la cláusula **schedule**.
- `void omp_get_schedule(omp_sched_t * kind, int * chunk);`
  - Retorna la planificación que se usará cuando se especifica **runtime** en la cláusula **schedule**.

### Locks

OpenMP también ofrece funciones para el uso de locks, para aquellos casos en que las directivas **critical** y **atomic** no sean suficientes/convenientes. 

El tipo de dato para los locks es **omp_lock_t** y las funciones disponibles son:

- `omp_init_lock`
- `omp_destroy_lock`
- `omp_set_lock`
- `omp_unset_lock`
- `omp_test_lock`

Funcionan en forma equivalente a los locks de pthreads. 

Ofrece funciones para la exclusión mutua recursiva. 

El tipo de dato para estas es **omp_nest_lock_t**

Las funciones son las mismas pero para sí. 

Funcionan equivalente a los locks recursivos de Pthreads. 

---

## Variables de entorno

Cuenta con un conjunto de variables de entorno:

- **OMP_NUM_THREADS**
- **OMP_DYNAMIC**
- **OMP_NESTED**
- **OMP_SCHEDULE**

---

## OpenMP 3.0 Tasking

El uso de tareas se introduce en la versión 3.0

Una tarea es una unidad de trabajo (porción de código) cuya ejecución puede ser diferida en el tiempo. Se compone de:

- Código a ejecutar
- Entorno de datos
- Variables de control internas

Pensado para paralelizar problemas irregulares:

- Bucles while
- Bucles for que no tienen una cantidad conocida de iteraciones
- Algoritmos recursivos
- Otros

---

## Constructor task

- El programador identifica las tareas encerrando los bloques de código correspondientes bajo la directiva **task** → Se asume que todas las tareas son independientes entre sí.
    
```c
#pragma omp task [lista de clausulas]
{/*bloque estructurado*/}
```

- Cuando un hilo encuentra un constructor **task**, el sistema de ejecución genera una nueva tarea.
- El momento en que esta tarea se ejecute dependerá del sistema de ejecución, el cual puede ser inmediato o diferido.
- Se permite el anidamiento de tareas → Una tarea puede generar otras tareas.

**Cláusulas disponibles:**

- **shared**, **private**, **firstprivate**, **default**
- **untied**: por defecto la tarea es ejecutada de inicio a fin por el mismo hilo (no necesariamente quien lo genera). **untied** permite que la tarea pueda ser completada por más de un hilo