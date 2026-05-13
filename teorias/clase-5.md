# Memoria compartida - Pthreads

## Temas de la clase

- Fundamentos de programación en memoria compartida.
- Estándar Pthreads.

## Programación en Memoria Compartida

**Plataformas de memoria compartida:**

- Los procesadores se comunican leyendo y escribiendo variables en un espacio de datos común (memoria compartida).
- Los módulos de memoria pueden ser locales, exclusivos a un procesador, o globales, comunes a todos los procesadores.
- Sub-clasificación por modo de acceso a la memoria:
  - Acceso uniforme a memoria (UMA)
  - Acceso no uniforme a memoria (NUMA)
- Se necesita un mecanismo de coherencia de caché.
- Modelo de programación asociado: memoria compartida. Pasaje de mensajes también es posible.

### Modelo de memoria compartida

En este modelo es responsabilidad del programador evitar los deadlock, así como toda sincronización disminuye la eficiencia.

La localidad de los datos será significativa para el rendimiento.

En algunos lenguajes el programador actúa sobre la localidad de los datos, en otros tendrá que re-estructurar el código.

En general el programador no maneja la distribución de los datos ni lo relacionado a la comunicación de los mismos.

Esto ofrece las ventajas de transparencia para el programador. La ubicación de los datos, su replicación y su migración son transparentes. Aunque presenta la desventaja que a veces es necesario trabajar sobre estos aspectos para mejorar el rendimiento. Además, resulta difícil la predicción de performance a partir de la lectura del algoritmo.

Los modelos de programación proveen un soporte para expresar la concurrencia y sincronización:

- Los modelos basados en procesos presuponen datos locales de cada proceso.
- Los modelos basados en threads o procesos livianos presuponen que toda la memoria es global → Pthreads.
- Los modelos basados en directivas extienden el modelo basado en threads para facilitar su manejo → OpenMP.

### Fundamentos del modelo de hilos

Un thread es un único hilo de control en el flujo de un programa.

Un ejemplo es la multiplicación de matrices.

- Todos los hilos tienen acceso a una memoria compartida global.
- Los hilos a su vez tienen su propio espacio de memoria privada.

### Ventaja del modelo de hilos frente al de procesos

- **"Livianaje" → Rendimiento:** Los hilos son más livianos que los procesos; su intercomunicación es más rápida por compartir memoria y su cambio de contexto resulta menos costoso.
- **Ocultamiento de latencia → Multitasking:** Múltiples hilos de ejecución contribuyen a reducir la latencia ocasionada por los accesos a memoria, la E/S y la comunicación.
- **Planificación y balance de carga:** Las APIs de hilos suelen permitir la creación de una gran cantidad de tareas concurrentes, que luego pueden ser mapeadas dinámicamente a través de primitivas a nivel de sistema → minimiza el overhead por ociosidad. Al mismo tiempo, facilita la distribución de trabajo ante cargas irregulares.
- **Facilidad de programación y uso extendido:** Más fácil de programar que pasaje de mensajes. (No requiere el manejo de la comunicación de datos).
- **Portabilidad:** Permite migrar aplicaciones entre arquitecturas. Útil para desarrollo.

---

# Sistemas Paralelos - Clase 5

## Pthreads

Las rutinas más usadas de Pthreads son:

- Manejo de threads: creación, terminación, join, asignación y recuperación, etc.
- Mutex.
- Variables de Condición.

### Creación de hilos

Inicialmente un único hilo (main). Todo otro hilo debe ser declarado por el programador bajo la función:

```c
int pthread_create (pthread_t *threadhandle,
                    const pthread_attr_t *attribute,
                    void * (*thread_function)(void *),
                    void *arg);
```

Los parámetros son:

- **thread_handle:** Es la dirección de un objeto pthread_t, el cual representa al hilo.
- **attribute:** Es la dirección de un objeto pthread_attr. NULL para valores por defecto.
- **thread_function:** Es la función que contiene el código que ejecutará el hilo creado.
- **arg:** Es el único argumento que se le puede pasar directamente al hilo creado. Debe ser de tipo void *.

### Creación de hilos

Una vez creados, son pares y pueden crear otros hilos. No existen jerarquías o dependencias predefinidas entre hilos.

### Terminación de hilos

Para finalizar la ejecución un hilo debe invocar:

```c
void pthread_exit(void *res);
```

Esta función finaliza la ejecución del hilo y retorna un valor que puede ser leído por otro hilo (generalmente el creador).

### Join de hilos

El hilo que invoca a la función pthread_create continúa con su ejecución luego del llamado → Se requiere sincronización para evitar que el programa termine de forma incorrecta.

Para esto contamos con la función pthread_join. Esta función bloquea al hilo llamado hasta que el hilo especificado como argumento termine su ejecución.

El flujo que se nos propone para la ejecución es:

→ Secuencial → Concurrente/Paralela → Secuencial

Usaremos el parámetro *arg para cosas tales como id, información de quién soy idealmente.

Al pasar parámetros es explícitamente necesario el casteo de tipos de los mismos.

### Pasaje de parámetros a los hilos

En el caso de que haya múltiples parámetros a cada hilo, hay al menos dos posibilidades:

- Mantener uno o más arreglos globales y pasarle el id a cada hilo para que sepa a qué posición debe acceder (concurrente).
- Pasarle un struct a cada hijo que contenga todos los argumentos que necesita (pasar un registro).

### Primitivas de exclusión mutua

Comunicación implícita → Se pone el esfuerzo en sincronizar tareas concurrentes. Recordar que el acceso a la memoria es compartido por todos los hilos (comparten globales).

Cuando múltiples hilos tratan de manejar los mismos datos se dan problemas de pisamiento, malos resultados.

Cuando se da pisamiento o incoherencias, esto depende del scheduling de los hilos → Condiciones de carrera (Race conditions).

**¿Cómo entonces tratamos las secciones críticas?**

- **Mutex locks** (bloqueo por exclusión mutua):
  - Los mutex locks tienen dos estados: locked (bloqueado) y unlocked (desbloqueado). En cualquier instante, solo un hilo puede bloquear un mutex_lock (acción atómica).
  - Para entrar entonces a la sección crítica un hilo debe lograr ganar el acceso. Cuando un hilo sale de la sección crítica debe desbloquearlo.
  - **TODOS LOS mutex_lock deben inicializarse como desbloqueados.**

Pthreads provee las siguientes funciones para hacer uso:

```c
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_init(pthread_mutex_t *mutex,
                      const pthread_mutexattr_t *lock_attr);
```

### Pthreads Tipos de exclusión

Soporta tres tipos de locks: Normal, Recursive y Error Check.

- Un mutex con el atributo **Normal** NO permite que un hilo que lo tiene bloqueado vuelva a hacer un lock sobre el (deadlock).
- Un mutex con el atributo **Recursive** Sí permite que un hilo que lo tiene bloqueado vuelve a hacer un lock sobre el (simplemente incrementa una cuenta de control).
- Un mutex con el atributo **Error Check** responde con un reporte de error al intento de un segundo bloqueo por el mismo hilo.

El tipo de mutex puede setearse entre los atributos antes de su inicialización.

### Overhead por el uso de locks

Los locks representan puntos de serialización. Si dentro de las secciones críticas ponemos segmentos largos de programa tendremos una degradación importante en el rendimiento.

A menudo se puede reducir el overhead por espera ociosa, utilizando la función pthread_mutex_trylock, la cual retorna el control informando si pudo hacer o no el lock:

```c
int pthread_mutex_trylock(pthread_mutex_t *mutex_lock);
```

Evita tiempos ociosos. Menos costoso por no tener que manejar las colas de espera.

### Primitivas para sincronización por condición

Los locks representan un mecanismo útil para sincronizar hilos. Sin embargo, un uso indiscriminado de los mismos puede provocar un overhead inaceptable. Exclusión mutua muchas veces genera busy waiting.

Una solución posible a este problema consiste en emplear variables de condición. Estas permiten que los hilos se autobloqueen hasta que se alcance determinado estado.

Cada variable condición estará asociada con un predicado (estado). Cuando el predicado es true, la variable condición se utiliza para avisar al/los hilos que están esperando por el cambio de estado de la condición.

Una única variable condición puede asociarse a varios predicados, aunque no es recomendable.

Una variable condición siempre tiene un lock asociado a ella. Cada hilo bloquea este lock y evalúa el predicado asociado a la variable compartida. Si el predicado es falso, el hilo espera en la variable condición (se duerme) por lo que no usa CPU. Al usar Variables de condición en lugar de locks, estamos reemplazando un mecanismo de sincronización basado en consultas (polling) por uno de interrupciones.

### Pthreads → Primitivas para sincronización por condición

La API provee las funciones:

```c
int pthread_cond_wait(pthread_cond_t *cond,
                     pthread_mutex_t *mutex);
```

El llamado a esta función bloquea al hilo hasta tanto reciba una señal de otro hilo o sea interrumpido por el sistema operativo.

Para poder invocarla, el hilo debe tener el control del mutex asociado.

Una vez dormido en la variable condición, el mutex se libera para que otros puedan usarlo.

Cuando el hilo se despierta, espera a que el mutex esté disponible nuevamente para continuar su ejecución.

```c
int pthread_cond_signal(pthread_cond_t *cond);
```

El llamado a esta función despierta al hilo que esté dormido en la variable condición. (El hilo a despertar depende de las políticas de planificación).

Para poder invocarla, el hilo debe tener el control del mutex asociado.

Usualmente el mutex asociado se libera (permitiendo que otros puedan usarlo).

Las funciones para manejar las Variables Condición:

```c
int pthread_cond_init(pthread_cond_t *cond,
                    const pthread_condattr_t *attr);
int pthread_cond_destroy(pthread_cond_t *cond);
```

Tenemos además wait y signal:

```c
int pthread_cond_timewait(pthread_cond_t *cond,
                         pthread_mutex_t *mutex,
                         const struct timespec *abstime);
```

El hilo se duerme una determinada cantidad de tiempo.

```c
int pthread_cond_broadcast(pthread_cond_t *cond);
```

Se despiertan todos los hilos que estén dormidos en la variable condición.

### Barreras con Pthreads

Proveé las siguientes funciones para implementar puntos de sincronización de múltiples hilos:

```c
int pthread_barrier_wait(pthread_barrier_t *barrier);
```

El hilo se queda bloqueado hasta tanto el número de hilos implicados en la barrera hayan alcanzado este punto.

El número de hilos asociados a una barrera se especifica en su inicialización:

```c
int pthread_barrier_init(pthread_barrier_t *restrict barrier,
                        const pthread_barrierattr_t *restrict attr,
                        unsigned count);
int pthread_barrier_destroy(pthread_barrier_t *barrier);
```

### Semáforos

Es una estructura de datos que permite sincronizar hilos (tanto para exclusión mutua como para sincronización por condición).

POSIX definió una API para esto, se puede emplear con Pthreads, aún sin ser parte del estándar.

Los tipos de datos y funciones se encuentran en semaphore.h.

Para la declaración de un semáforo, se usa el tipo sem_t.

**Inicialización de semáforos:**

```c
int sem_init(sem_t *sem, int pshared, unsigned int value);
```

**Decrementar semáforo (P):**

```c
int sem_wait(sem_t *sem);
```

**Incrementar semáforo (V):**

```c
int sem_post(sem_t *sem);
```

**Destruir semáforo:**

```c
void sem_destroy(sem_t *sem);
```

### Planificación de hilos

El sistema operativo es responsable de planificar la ejecución de los hilos. Sin embargo uno puede influenciarlo usando los atributos de planificación. La prioridad de planificación de un hilo determina el nivel de privilegio que tendrá el mismo sobre los demás en la planificación.

El planificador mantiene una cola separada de hilos por cada prioridad definida. Al momento de seleccionar un hilo a ejecutar, se elige alguno que esté listo de la cola con mayor prioridad.

Si existieran varios hilos posibles en la cola seleccionada, se elige uno de ellos de acuerdo a la política de planificación.

### Pthreads - Planificación de hilos

Para asignar y recuperar atributos de planificación Disponemos:

```c
int pthread_attr_getschedparam(const pthread_attr_t *attr,
                               struct sched_param *param);
int pthread_attr_setschedparam(pthread_attr_t *attr,
                              const struct sched_param *param);
```

Para asignar y recuperar la prioridad mínima y máxima de una determinada política de planificación, se pueden usar las funciones:

```c
int sched_get_priority_min(int policy);
int sched_get_priority_max(int policy);
```

La política de planificación determina cómo se ejecutan y comparten recursos de hilos de una misma prioridad.

Pthread soporta tres políticas de planificación:

- **SCHED_FIFO:** Una vez en ejecución, el hilo se ejecuta hasta que termine, se bloquee o hasta que un hilo de mayor prioridad pueda ejecutarse. Los hilos de la misma prioridad son ejecutados en orden.
- **SCHED_RR** (round-robin): Similar a FIFO pero los hilos se ejecutan a sumo determinada cantidad de tiempo configurable.
- **SCHED_OTHER:** Política adicional, no definida en el estándar. Su comportamiento depende completamente de la implementación.

### Multihilado en otros lenguajes

**En Python:**

```python
import threading
import numpy as np

# Tamaño del vector
N = 1000

# Crear dos vectores aleatorios
a = np.random.rand(N)
b = np.random.rand(N)

# Número de hilos a usar
num_threads = 4

# Vector donde se almacenará la suma
result = np.zeros(N)

# Función que realiza la suma en un rango de índices
def sum_vectors(start, end):
    for i in range(start, end):
        result[i] = a[i] + b[i]

# Crear y lanzar los hilos
threads = []
chunk_size = N // num_threads
for i in range(num_threads):
    start = i * chunk_size
    end = N if i == num_threads - 1 else (i + 1) * chunk_size
    thread = threading.Thread(target=sum_vectors, args=(start, end))
    threads.append(thread)
    thread.start()

# Esperar a que todos los hilos terminen
for thread in threads:
    thread.join()

# Verificar el resultado con numpy
expected = a + b
print("Diferencia máxima:", np.max(np.abs(result - expected)))
```

**En Java:**

```java
import java.util.Random;

class VectorSumThread extends Thread {
    private double[] a, b, result;
    private int start, end;

    public VectorSumThread(double[] a, double[] b, double[] result, int start, int end) {
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

public class SumVectorsMultithreading {
    private static final int N = 1000;
    private static final int NUM_THREADS = 4;

    public static void main(String[] args) {
        double[] a = new double[N];
        double[] b = new double[N];
        double[] result = new double[N];
        Random rand = new Random();

        // Inicializar los vectores con valores aleatorios
        for (int i = 0; i < N; i++) {
            a[i] = rand.nextDouble();
            b[i] = rand.nextDouble();
        }

        VectorSumThread[] threads = new VectorSumThread[NUM_THREADS];
        int chunkSize = N / NUM_THREADS;

        // Crear y ejecutar los hilos
        for (int i = 0; i < NUM_THREADS; i++) {
            int start = i * chunkSize;
            int end = (i == NUM_THREADS - 1) ? N : (i + 1) * chunkSize;
            threads[i] = new VectorSumThread(a, b, result, start, end);
            threads[i].start();
        }

        // Esperar a que todos los hilos terminen
        for (VectorSumThread thread : threads) {
            try {
                thread.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        // Mostrar algunos valores para verificar el resultado
        System.out.println("Primeros 10 valores de la suma:");
        for (int i = 0; i < 10; i++) {
            System.out.printf("%.4f + %.4f = %.4f\n", a[i], b[i], result[i]);
        }
    }
}
```