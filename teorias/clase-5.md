### Memoria compartida //Pthreads:
Temas de la clase: 
    - Fundamentos de programacion en memoria compartida. 
    - Estandar Pthreads

## Programacion en Memoria Compartida: 
* Plataformas de memoria compartida *
- Los proecsadores se comunican leyendo y escribiendo variables en un espacio de datos comun (memoria compartida). 
- Los modulos de memoria pueden ser locales, exclusivos a un procesador. o globales, comunes a todos los procesadores. 
- Sub-clasificacion por modo de accceso a la memoria. 
    - Acceso uniforme a memoria UMA
    - Acceso no unifome a memoria NUMA
- Se necesta un mecanismo de coherencia de cache
- Modelo de programacion asociado: memoria compartida. Pasaje de mensajes tambien es posible. 
    
### Modelo de memoria compartida. 
    En este modelo es responsabilidad del programador evitar los deadlock, asi como toda sincronizacion disminuye la eficiencia. 
    La localidad de los datos sera significante para el rendimiento. 
    En algunos lenguajes el programador actua sobre la localiadd de los datos, en otros tendra que re-estructurar el codigo. 
    En general el programador no maneja la distribucion de ls datos ni lo relacionado a la comunicacion de los mismos. 
    
Esto ofrece las ventajas de transparencia para el programador. La ubicacion de los datos, su replicacion y su migracion son transparentes. Aunque presenta la desventaja que a veces es necesario trabajar sobre estos aspectos para mejorar el rendimiento. Ademas, resulta dificil la prediccion de performance a partir de la lectura del algoritmo. 

Los modelos de programacion proveen un soporte para expresar la concurrencia y sincronizacion:  
    - Los modelos basados en procesos suponen datos locales de cada proceso. 
    - Los modelos basados en threads o procesos livianos suponen que toda la memoria es global -> Pthreads. 
    - Los modelos basados en directivas extienden el modelo basado en threads para facilitar su manejo -> OpenMP. 
    
### Fundamentos del modelo de hilos: 
    Un thread es un unico hilo de control en el flujo de un programa.   
    Un ejemplo es la multiplicacion de matrices. 
    
 - Todos los hilos tienen acceso a una memoria compartida global. 
 - Los hilos a su vez tienen su propio espacio de memoria privada. 
### Ventaja del modelo de hilos frente al de procesos: 
    - "Livianidad" -> Rendimiento: Los hilos son mas livianos que los procesos; su intercomunicacion es mas rapida por compartir memoria y su cambio de contexto resulta menos costoso. 
    - Ocultamiento de latencia -> MUlti-tasking: multiples hilos de ejecucion contribuyen a reducir la lantencia ocasionada por los accesos a memoria, la E/S y la comunicacion. 
    - Planificacion y balance de carga: Las APIs de hilos suelen permitir la creacion de una gran cantidad de tareas concurrentes, que luego pueden ser mapeadas dinamicamente a traves de primiticas a nivel de sistema -> minimiza el overhead por ociosidad. Al mismo tiempo, facilita la distribucion de trabajo ante cargas irregulares. 
    - Facilidad de programacion y uso extendido: Mas facil de programar que pasaje de mensajes. (No requiere el menejo de la comunicacion de datos). 
    - Portabilidad: Permite migrar aplicaciones entre arquitecturas. Util para desarrollo. 
    
# Sistemas Paralelos clase 6

### Pthreads:

Las rutinas mas usadas de Pthreads son: 

- Manejo de threads: creacion, terminacion, join, asignacion y recuperacion, etc.
- Mutex
- Variables de Condicion.

Creacion de hilos: 

Inicialmente unico hilo (main), todo otro hilo debe ser declarado por el programador bajo la funcion: 

```jsx
int pthread_create (ptread_t *threadhandle, 
										const ptread_attr_t *attribute, 
										void * (*thread_function)(void *), 
										void *arg);
```

Los parametros son: 

- thread_heandle, es la direccion de un objeto pthread_t, el cual representa al hilo.
- attribute es la direccion de un objeto pthread_attr. NULL para valores por defecto.
- thread_function es la funcion que contiene el codigo que ejecutara el hilo creado.
- arg es el unico argumento que se le puede pasar directamente al hilo creado. Debe ser de tipo void *.

Creacion de hilos: 

Una vez creados, son pares y pueden crear otros hilos. No existen jerarquias o dependencias predefinidas entre hilos. 

Terminacion de hilos: 

Para finalizar la ejecucion un hilo debe invocar: 

```jsx
int pthread_exit (void *res); 
```

Esta funcion finaliza la ejecucion ddel hilo y retorna un valor que puede ser leido por otro hilo (generalmente el creador). 

Join de hilos: 

El hilo que invoca a la funcion pthread_create continua con su ejecucion luego del llamado → se requiere sincronizacion para evitar que el programa termine de forma incorrecta. 

Para esto contamos con la funcion pthread_join. Esta funcion Bloquea al hilo llamado hasta que el hilo especificado como argumento termine su ejecucion. 

El flujo que se nos propone para la ejecucion es: 

→ Secuencial → concurrente/Paralela → Secuencial

Usaremos el parametro *arg para cosas tales como id, informacion de quien soy idealmente. 

Al pasar parametros es explicitamente necesario el casteo de tipos de los mismos. 

### Pasaje de parametros a los hilos:

En el caso de que haya multiples parametros a cada hilo, hay al menos dos posibilidades: 

- Mantener uno o mas arreglos globales y pasarle el id a cada hilo para que sepa a que posicion debe acceder (concurrente).
- Pasarle un struct a cada hijo que contenga todos los argumentos que necesita (pasar un registro).

### Primitivas de Exclusion mutua:

Comunicacion implicita → se pone el esfuerzo en sincronizar tareas concurrentes. Recordar que el acceso a la memoria es compartido por todos los hilos (comparten globales). 

Cuando multiples hilos tratan de manejar los mismos datos se dan problemas de pisamiento, malos resultados. 

Cuando se da pisamientos o incoherencias, Esto depende del scheduling de los hilos → Condiciones de carrera (Race conditions). 

Como entonces tratamos las secciones criticas?

- mutex_locks (bloqueo por exclusion mutua)
    
    mutexs locks tiene dos estados: locked (bloqueado) y unlocked (desbloqueado). En cualquier instante, solo un hilo puede bloquear un mutex_lock(Accion atomica). 
    
    Para entrar entonces a la seccion critica un hilo debe lograr ganar el acceso, cuando un hilo entonces sale de la seccion critica debe desbloquearlo. 
    
    **TODOS LOS mutex_lock deben inicializarse como desbloqueados.** 
    

Pthread provee las siguientes funciones para hacer uso: 

```jsx
int pthread_mutex_lock (pthread_mutex_t *mutex); 
int pthread_mutex_unlock (pthread_mutex_t *mutex); 
int pthread_mutex_init (pthread_mutex_t *mutex, 
												const ptread_mutexattr_t *lock_attr
												); 

```

### Pthreads Tipos de exclusion:

Soporta tres tipos de locks: Normal, Recursive y Error Check. 

- Un mutex con el atributo **Normal** NO permite que un hilo que lo tienen bloqueado vuelva a hacer un lock sobre el (deadlock).
- Un mutex con el atributo Recursive Si permite que un hilo que lo tienen bloqueado vuelva a hacer un lock sobre el (simplemente incrementa una cuenta de control).
- Un mutex con el atributo Error Check responde con un reporte de error al intento de un segundo bloqueo por el mismo hilo.

El tipo de mutex puede setearse entre los atributos antes de su inicializacion. 

### Overhead por el uso de locks:

Los locks representan puntos de serializacion, si dentro de las secciones criticas ponemos segmentos lagos de programa tendremos una degradacion importante en el rendimiento. 
A menudo se puede reducir el overhead por espera ociosa, utilizando la funcion phtread_mutex_trylock, la cual retorna el control informando si pudo hacer o no el lock: 

```jsx
int pthread_mutex_trylock (pthread_mutex_t *mutex_lock)
```

Evita tiempos ociosos. Menos costoso por no tener que manejar las colas de espera. 

Cuando usamos? nose flaco no me dijiste. 

### Primitivas para sincronizacion por condicion:

Los locks reresentan un mecanismo util para sincronizar hilos. Sin embargo, un uso indiscriminado de los mismos puede provocar un overhead inaceptable. Exclusion mutua muchas veces genera busy waiting

Una solucion posible a este roblema consiste en emplear variables de condicion. Estas permite que los hilos se autobloqueen hasta que se alcanze determinado estado.

Cada variable condicoon estara asociada con un predicado (estado). Cuando el predicado es true, la variabl econdicion utiliza para avisar al/los hilos que estan esperando por el cambio de estado de la condicion. 

Una unica variable condicon puede asociarse a varios predicados, aunque no es recomendable. 

Una viriable condicion siempre tiene un lock asociada a ella. Cada hilo bloquea este lock y evalua el predicado asociado a la variable compartida. Si el predicado es falso, el hilo espera en la variable condicion(se duerme) por lo que no usa CPU. Al usar Variables de condicion en lugar de locks, estamos reemplazando un mecanismo de sincronizacion basado en consultas (polling) por uno  de interruciones. 

### Pthreads → primitivas para sincronizacion por condicion:

La api provee las funciones: 

```jsx
int pthread_cond_wait (phtread_cond_t *cond, 
												pthread_mutex_t *mutex
											)
```

El llamado a esta funcion bloquea la hilo hasta tanto reciba una senal de otro hilo o sea interrumpido por el sistema operativo.

Para poder invocarla, el hilo debe tener el control del mutex asociado. 

Una vez dormido en la variable condicion, el mutex se libera para que otros puedan usarlo. 

Cuando el hilo se despierta, espera a que el mutex este disponible nuevamente para continuar su ejecucion. 

```jsx
int pthread_cond_signal (pthread_cond_t *cond)
```

El llamado a esta funcion despierta al hilo que este dormido en la variable condicion. (El hilo a despertar depende de las politicas de planificacion). 

Para poder invocarla, el hilo debe tener el control del mutex asociado

Usualmente el mutex asociado se libera (permitiendo que otros puedan usarlo). 

Las funciones para manejar las Variables Condicion:
```jsx
int pthread_cond_init (pthread_cond_t *cond, 
												const pthread_condattr_t *attr
												)
int pthread_cond_destroy(pthread_cond_t *cond)
```

Tenemos ademas wait y signal: 

```jsx
int phtread_cond_timewait (pthread_cond_t *cond, 
pthread_mutex_t *mutex,
 const struct timespec *abstime)
```

El hilo se duerme una determinada cantidad de tiempo. 

```jsx
int pthread_cond_broadcast(pthread_cond_t *cond)
```

Se despiertan todos los hilos que esten dormidos en la variable condicion. 

### Barreras con Pthreads:

Provee las siguientes funciones para implementar puntos de sincronizacion de multiples hilos. 

```c
int pthread_barrier_wait(pthread_barrier_t *barrier);
```

El hilo se queda bloqueado hasta tanto el numero de hilos implicados en la barrera hayan alcanzado este punto. 

El número de hilos asociados a una barrera se especifica en su inicializacion: 

```c
int pthread_barrier_init(pthread_barrier_t *restrict barrier,
const pthread_barrierattr_t *restrict attr,
 unsigned count);
int pthread_barrier_destroy(pthread_barrier_t *barrier);
```

### Semaforos:

es una estructura de datos que permite sincronizar hilos (tanto para exclusion mutua como para sincronizacion por condicion). 

POSIX definidio una API para esto, se puede emplear con Pthreads, aun sin ser parte del estandar. 

Los tipos de datos y funciones se encuentran semaphore.h

Para la declaracion de un semaforo, se usa el tipo sem_t

Inicialización de semáforos: 

```c
int sem_init (sem_t *sem, int pshared, unsigned int value);
```

Decrementar semáforo (P): 

```c
int sem_wait(sem_t *sem);
```

Incrementar semáforo (V): 

```c
int sem_post(sem_t *sem);
```

Destruir Semáforo: 

```c
void sem_destroy(sem_t *sem);
```

### Planificación de hilos:

El sistema operativo es responsabel de planificarr la ejecuciuon de los hilos. Sin embargo uno puedde influenciarlo usando los atributos de planificacion. La prioridad de planificacion de un hilo determina el nivel de privilegio que tendra el mismo sobre los demas en la planificacion. 

El planificado mantiene una cola separada de hilos por cada prioridad definida. Al momento de selecciona un hilo a ejecutar, se elige alguno que este listo de la cola con mayor prioridad. 

Si existieran varios hilos posibles en la cola seleccionada, se elije uno de ellos de acuerdo  la politica de planificacion. 

### Phtreads - Planificacion de hilos:

Para asignar y recuperar atributos de planificacion disponemos: 

```c
int pthread_attr_getschedparam (const pthread attr t *attr,
struct sched param *param)
int pthread_attr_setschedparam (pthread attr t *attr,
const struct sched param *param)
```

Para asignar y recuperar la prioridad mínima y máxima de una determinada política de planificación, se pueden usar las funciones: 

```c
int sched_get_priority_min (int policy)
int sched_get_priority_max (int policy) 

```

La política de planificacion determina como se ejecutan y comparten recursos de hilos de una misma prioridad. 

Pthread soporta tres politicas de planificacion: 

- SCHED_FIFO, una vez en ejecucion, el hilo se ejecuta hasta que termin, se bloquee o hasta que un hilo de mayor prioridad pueda ejecutarse. Los hilos de la misma prioridad son ejecutados en orden.
- SCHED_RR(round-robin): Similar a FIFO pero los hilos se ejecutan a sumo ddeterminada cantidad de tiempo configurable.
- SCHED_OTHER: Politica adicional, no definida en el estandar. Su comportamiento depende completamente de la implementacion.

### Multihilado en otros lenguajes:

- En Python:

```python
import threading
import numpy as np
# Tamaño del vector
N = 1000
# Crear dos vectores aleatorios
a = np.random.rand(N)
b = np.random.rand(N)
# Crear y lanzar los hilos
threads = []
chunk_size = N // num_threads
for i in range(num_threads):
start = i * chunk_size
53
end = N if i == num_threads- 1 else (i + 1) * chunk_size
thread = threading.Thread(target=sum_vectors, args=(start, end))
threads.append(thread)
# Vector donde se almacenará la 
suma
result = np.zeros(N)
# Número de hilos a usar
num_threads = 4
# Función que realiza la suma en 
un rango de índices
def sum_vectors(start, end):
for i in range(start, end):
result[i] = a[i] + b[i]
thread.start()
# Esperar a que todos los hilos terminen
for thread in threads:
thread.join()
# Verificar el resultado con numpy
expected = a + b
print("Diferencia máxima:", np.max(np.abs(result- expected)))
```

- JAVA:

```java
import java.util.Random;
class VectorSumThread extends
public class SumVectorsMultithreading {
private static final int N = 1000;
private static final int NUM_THREADS = 4;
54
// Esperar a que todos los hilos 
terminen
Thread {
private double[] a, b, result;
private int start, end;
public VectorSumThread(double[]
a, double[] b, double[] result, int
start, int end) {
this.a = a;
this.b = b;
this.result = result;
this.start = start;
this.end = end;
}
@Override
public void run() {
for (int i = start; i < end; i++) {
result[i] = a[i] + b[i];
}
}
}
public static void main(String[] args) {
double[] a = new double[N];
double[] b = new double[N];
double[] result = new double[N];
Random rand = new Random();
// Inicializar los vectores con valores 
aleatorios
for (int i = 0; i < N; i++) {
a[i] = rand.nextDouble();
b[i] = rand.nextDouble();
}
VectorSumThread[] threads = new
VectorSumThread[NUM_THREADS];
int chunkSize = N / NUM_THREADS;
// Crear y ejecutar los hilos
for (int i = 0; i < NUM_THREADS; i++) {
int start = i * chunkSize;
int end = (i == NUM_THREADS - 1) ? N :
(i + 1) * chunkSize;
threads[i] = new VectorSumThread(a, b,
result, start, end);
threads[i].start();
}
for (VectorSumThread thread :
threads) {
try {
thread.join();
} catch (InterruptedException e) {
e.printStackTrace();
}
}
// Mostrar algunos valores para 
verificar el resultado
System.out.println("Primeros 10 
valores de la suma:");
for (int i = 0; i < 10; i++) {
System.out.printf("%.4f + %.4f = 
%.4f\n", a[i], b[i], result[i]);
}
}

```

