# Sistemas de memoria

## Memory Wall: limitaciones en el rendimiento del sistema de memoria

En la memoria hay dos parámetros fundamentales a tener en cuenta que son la latencia y el ancho de banda.

**Definiciones:**

- **Latencia:** es el tiempo que transcurre desde que se solicita el dato hasta que el mismo está disponible.
- **Ancho de Banda:** es la velocidad con la cual el sistema puede alimentar al procesador.

Una solución al problema propio de la latencia es el uso de cachés:

- **Las cachés:** Son memorias de alta velocidad y baja capacidad que usualmente están integradas al chip. Actúan como memoria intermedia entre los registros de la CPU y la memoria principal.

El objetivo es disminuir la latencia del sistema de memoria maximizando el número de datos que se acceden desde la caché.

Cuando la CPU necesita un dato, primero revisa si está en caché. Si lo encuentra se da un cache hit (baja la latencia). Sino se produce cache miss y debe ser buscado en RAM, queda en caché una vez recuperado.

La tasa de hits es importante porque incide directamente en la latencia global del sistema.

Recordar el principio de localidad temporal.

## Impacto del ancho de banda

Es la velocidad con la que los datos pueden ser transferidos desde la memoria al procesador y está determinado por el ancho de banda del bus de memoria como de las unidades de memoria.

Una técnica común para mejorar el ancho de banda del sistema consiste en incrementar el tamaño de los bloques de memoria que se transfieren por ciclo de reloj.

El sistema de memoria requiere / unidades de tiempo para obtener b unidades de datos (b es el tamaño del bloque medido en bits, bytes o words).

## Resumamos las ideas para mejorar el rendimiento del sistema de memoria

1. Explotar localidad espacial y temporal de datos es crítico para amortizar la latencia e incrementar el ancho de banda efectivo.
2. La relación entre el número de instrucciones y el número de accesos a memoria es un buen indicador temprano del rendimiento efectivo del sistema.
3. La organización de los datos en la memoria y la forma en que se estructura el código pueden impactar en el rendimiento del sistema.

## Arreglos Multidimensionales y Organización en Memoria

La estructura de datos más utilizada en HPC son arreglos multidimensionales.

Listas, árboles, grafos son otras opciones no tan implementadas.

Existen 2 maneras en que los datos de un arreglo son almacenados en memoria:

- Por filas
- Por columnas

**¿Cómo declarar un arreglo en C?**

Nos interesa particularmente que el arreglo:

- No imponga un tamaño máximo por la forma en que está declarado.
- Pueda elegir cómo se organizan sus elementos en memoria.
- Sus datos estén contiguos en memoria.
- Si su tamaño puede ser dinámico en ejecución, mejor.

Veamos cómo declararlos con su sintaxis:

```c
// Alternativa 1: Arreglo Estático
#define N 100
int main (int argc, char * argv[]){
    float matriz[N][N];
}
```

Este arreglo únicamente cumple la condición que todos sus datos estén contiguos en la memoria.

```c
// Alternativa 2: Arreglo de longitud variable
int main (int argc, char * argv[]){
    int n = 100;
    float matriz[n][n];
}
```

Nuevamente solo satisface que sus datos sean contiguos en la memoria.

```c
// Alternativa 3: Arreglo dinámico como vector de punteros a filas/columnas
#define N 100
int main (int argc, char * argv[]){
    float ** matriz = malloc(N*sizeof(float*));
    for(i=0; i<N; i++){
        matriz[i] = malloc(N*sizeof(float));
    }
}
```

Este cumple con todo menos que los datos sean contiguos en la memoria.

```c
// Alternativa 4: Arreglo dinámico como vector de elementos
#define N 100
int main (int argc, char * argv[]){
    float * matriz = malloc(N*N*sizeof(float));
}
```

Este cumple con todos los atributos deseados. ¡Una masa de bueno!

## Arreglo dinámico como vector de elementos

En general, al almacenar una matriz como un vector de elementos, el cálculo del índice para acceder a sus elementos depende de cómo estén organizados.

- Cuando una matriz está organizada por filas, se debe multiplicar el número de fila por la cantidad de elementos en cada fila (cantidad de columnas) y sumarle el número de columna.
  - Ej: matriz[i*N+j]
- Cuando una matriz está organizada por columnas, se debe multiplicar el número de columna por la cantidad de elementos de cada columna (cantidad de filas) y sumarle el número de fila.
  - Ej: matriz[j*N+i]

**Ventajas de arreglo dinámico como vector de elementos:**

- Favorece el aprovechamiento de la localidad de datos.
- Hace posible el uso de instrucciones SIMD.
- Facilita el intercambio de arreglos entre programas escritos en diferentes lenguajes.

## Coherencia de caché en arquitecturas multiprocesador

Las redes de interconexión proveen mecanismos para comunicar datos.

En máquinas de memoria compartida pueden existir múltiples copias del mismo dato → se requiere hardware específico para mantener la consistencia entre estas copias.

El mecanismo de coherencia debe asegurar que todas las operaciones realizadas sobre las múltiples copias son serializables → debe existir algún orden de ejecución secuencial que se corresponde con la planificación paralela.

## Protocolos de coherencia de caché

- Si un procesador lee un dato una vez y no vuelve a usarlo, un protocolo de actualización puede generar un overhead innecesario → en este caso es mejor un protocolo de invalidación.
- Si dos procesadores trabajan sobre la misma línea en forma alternada, un protocolo de actualización sea mejor opción → evita ocio por espera del dato actualizado.

Relación costo-beneficio: Los protocolos de actualización pueden producir overhead por comunicaciones innecesarias mientras que los de invalidación pueden producir ocio ante la espera del actualizar el dato.

En la actualidad la mayoría de los protocolos se basan en invalidación.

## Protocolos de coherencia de caché basados en invalidación

Esquema simple donde cada copia se asocia con uno de 3 estados: compartida (shared), inválida (invalid) o sucia (dirty).

- En compartida (shared): hay múltiples copias válidas del dato. Ante una escritura, pasa a estado sucio donde se produjo mientras que el resto se marca como inválida.
- En sucia (dirty): la copia es válida y se trabaja con esta.
- En inválida la copia no es válida. Ante una lectura se actualiza a partir de la copia válida.

## Implementación de protocolos de coherencia

Existen una variedad de mecanismos de hardware para implementar protocolos de coherencia de caché:

- Sistemas snoopy
- Sistemas basados en directorios
- Combinaciones de los dos anteriores.

### Implementación Snoopy

- Asociado usualmente a los sistemas multiprocesador interconectados por alguna red broadcast, como bus o anillo.
- La caché de cada procesador mantiene un conjunto de tags asociados a sus bloques, los cuales determinan su estado.
- Todos los procesadores monitorizan (snoop) el bus, lo que permite realizar las transacciones de estado en sus bloques.

Cuando el hw snoopy detecta una lectura sobre un bloque caché dirty, entonces toma el control del bus y cumple el pedido.

Cuando el hw snoopy detecta una escritura sobre un bloque de datos del cual tiene copia entonces la marca como inválida.

Snoopy se adoptó masivamente por ser un esquema simple, de bajo costo y buen rendimiento para operaciones locales.

Si cada procesador opera sobre datos disjuntos, entonces los mismos pueden ser cacheados:

- Ante operaciones de escritura, el dato es marcado como sucio. Al no haber operaciones de otros procesadores, las siguientes peticiones se satisfacen localmente.
- Ante operaciones de lectura, el dato es marcado como compartido. Las peticiones siguientes se satisfacen localmente en todos los casos.
- En ambos casos, el protocolo no agrega overhead adicional.
- Si diferentes procesadores realizan lecturas y escrituras sobre el mismo dato, se genera tráfico en el bus para poder mantener la coherencia de los datos.
- Al ser redes broadcast el mensaje de coherencia llega a todos los procesadores aún si no tienen el dato en cuestión.
- Como el bus a su vez tiene un ancho de banda limitado, se convierte en un cuello de botella.

Una solución obvia a este problema consiste en solo propagar las operaciones de coherencia a aquellos procesadores que tienen el dato involucrado, lo cual requiere mantener un registro de qué datos tiene cada procesador → Sistemas basados en directorios.

## Sistemas Basados en directorios

La memoria principal incorpora un directorio que mantiene información de estado (bits de presencia + estado) sobre los bloques de caché y los procesadores donde están cacheados.

La información contenida en el directorio permite que solo aquellos procesadores que tienen un determinado dato queden involucrados en las operaciones de coherencia.

Si los procesadores operan sobre datos disjuntos, las peticiones pueden cumplirse localmente (no agrega overhead).

Cuando múltiples procesadores leen y escriben los mismos datos, se generan operaciones de coherencia → provoca overhead adicional por la necesidad de mantener actualizado el directorio.

- Como el directorio está en memoria, si un programa paralelo requiere un gran número de operaciones de coherencia, se genera overhead por la competencia en el acceso al recurso.
- La cantidad de memoria requerida por el directorio puede ser un cuello de botella a medida que el número de procesadores crece.

Como el directorio representa un punto centralizado de acceso, una solución posible es particionarlo → Sistemas basados en directorios distribuidos.

## Sistemas Basados en directorios distribuidos

Se da en arquitecturas escalables, donde la memoria se encuentra físicamente distribuida.

- Cada procesador es responsable de mantener la coherencia de sus propios bloques (mantiene su propio directorio).
  - Cada bloque ahora tendrá un propietario.
  - Cuando un procesador desea leer un bloque por primera vez, debe pedírselo al propietario, quien redirige el pedido de acuerdo a la información del directorio.
  - Cuando un procesador escribe un bloque de memoria, envía una invalidación al propietario, quien luego la propaga a todos aquellos que tienen una copia.
- Como el directorio está distribuido, la competencia en el acceso al mismo se alivia → sistema más escalable.
- La latencia y el ancho de banda de interconexión se convierten ahora en cuellos de botella del rendimiento de estos sistemas.

## Costos de Comunicación

Uno de los mayores overheads en los programas paralelos proviene de la comunicación entre unidades de procesamiento.

El costo de la comunicación depende de múltiples factores y no solo del medio físico: modelo de programación, topología de red, manejo y ruteo de datos, protocolo de software asociados.

- Costos diferentes según la forma de comunicación:
  - Modelo simplificado para pasaje de mensajes: t_comm = t_s + m * t_w donde t_s es el tiempo requerido para preparar el mensaje, m es el tamaño del mensaje medido en words y t_w es el tiempo requerido para transmitir una palabra.
  - Memoria compartida: resulta difícil modelar costos por múltiples factores que escapan al control del programador.