# Sistemas de memoria: 
### Memory Wall: limitaciones en el rendimiento del sistema de memoria: 
    En la memoria hay dos parametros fundamentales a tener en cuenta que son la latencia y el ancho de banda. 
    Definiciones: 
        Latencia es tiempo que transcurre desde que se solicita el dato hasta que el mismo esta disponible. 
        Ancho de Banda es la velocidad con la cual el sistema puede alimentar al procesador. 

Una solucion al proble a propio de la latencia es el uso de caches: 
    + Las cache: 
    Son memorias de alta velocidad y baja capacidad que usualmente estan integradas al chip. Actua como memoria intermedia entre los registros de la CPU y la memoria principal. 

El objetivo es disminuir la latencia del sistema de memoria maximizando el numero de datos que se acceden desde la cache. 
Cuando la CPU necesita un dato, primero revisa si esta en cacho, si lo encuentra se da un cache hit, baja la latencia. sino se produce cache miss y debe ser buscado en RAM, queda en cache una vez recuperado. 

La tasa de hits es importante porque incide directamente en la latencia global del sistema. 
Recordar el principio de localidad temporal. 
    
### Impacto del ancho de banda
    es la velocidad con la que los datos pueden ser transferidos desde la memoria al procesador y esta determinado por el ancho de banda del bus de memoria como de las unidades de memoria. 
    Una tecnica comun para mejorar el ancho de banda del sistema consiste en incrementar el tamano de los bloques de memoria que se transfieren por ciclo de reloj. 
    El sistema de memoria requiere / unidades de tiempo para obtener b unidades de datos (b es el tamano del bloque medido en bits, bytes o words). 
    
### Resumamos las ideas para mejorar el rendimiento del sistema de memoria: 
    1. Explotar localidad espacial y temporal de datos es critico para amortizar la latencia e incrementar el ancho de banda efectivo. 
    2. La realizacion entre el numero de instrucciones y el numero de accesos a memoria es un buen indicador temprano del rendimiento efectivo del sistema. 
    3. La organizacion de los datos en la memoria y la forma en que se estructura el codigo pueden impactar en el rendimiento del sistema. 
    
### Arreglos Multidimensionales y Organizacion en Memoria: 
    La estructuda de datos mas utilizada en HPC son arreglos multidimensionales: 
        Listas, arboles, grafos son otras opciones no tan implementadas
    
    Existen 2 maneras en que los datos de un arreglo son almacenados en memoria: 
        + Por filas
        + Por Columas
    
    Como declarar un arreglo en C? 
        nos interesa particularmente que el arreglo: 
        - No imponga un tamano maximo por la forma en que esta declarado.   
        - Pueda elegir como se organizan sus elementos en memoria
        - Sus datos esten contiguos en memoria 
        - Si su tamano puede ser dinamico en ejecucion mejor. 
    Veamos como declararlos con su sintaxis: 
    //Alternativa 1 Arreglo Estatico: 
    #define N 100
    int main (int argc, char * argv[]){
    
        float matriz[N][N];
    }
    //
    Este arreglo unicamente cumple la condicion que todos sus datos esten contiguos en la memoria. 
    //Alternativa 2 arreglo de longitud variable: 
    int main (int argc, char * argv[]){
        int n = 100; 
        float matriz[n][n];
    }
    //
    Nuevamente solo satisface que sus datos sean contiguos en la memoria
    //Alternativa 3 (arreglo dinamico como vector de punterlos a filas/columnas). 
    #define N 100
    int main (int argc, char * argv[]){
            float ** matriz = malloc(N*sizeof(float*)); 
            for(i=0; i< N; i++){
                matriz[i] = malloc(N*sizeof(float*)); 
            }
    }
    // Este cumple con todo menos que los datos sean contiguos en la memoria. 
    
    //Alternativa 4 arreglo dinamico como vector de elementos: 
    #define N 100
    int main (int argc, char * argv[]){
        float * matriz = malloc(N*N*sizeof(float));
    }
    // Este cumple con todos los atributos deseados, una masa de bueno. 
    
### Arreglo dinamico como vector de elementos: 
    En general, al almacenar una matriz como un vector de elementos, el calculo del indice para acceder a sus elementos depende de como esten organizados.  
    Cuando una matriz esta organizada por filas, se debe multiplicar el numero de fila por la cantidad de elementos en cada fila (cantidad de columnas) y sumarle el numero de columna. 
        ej: matriz [i*N+j]
    Cuando una matriz esta organizada por columnas, se debe multiplicar el numero de columna por la cantidad de elementos de cada columna (cantidad de filas y sumarle el numero de fila. 
        ej: matriz [j*N+i]

Ventajas de arreglo dinamico como vector de elementos: 
    + favorece el aprovechamiento de la localidad de datos 
    + Hace posible el uso de instrucciones SIMD 
    + Facilita el intercambio de arreglos entre programas escritos en diferentes lenguajes. 
    
### Coherencia de cache en arquitecturas multiprocesador:
    Las redes de interconexion proveen mecanismos para comunicar datos. 
    En maquinas de memoria compartida pueden existir mutiples copias del mismo dato -> se requiere hardware especifico para mantener la consistencia entre estas copias. 
    El mecanismo de coherencia debe asegurar que todas las operaciones realizadas sobre las multiples copias son serializables -> tiene que existir algun orden de ejecucion secuancial que se corresponde con la planificacion paralela. 
    
### Protocolos de coherencia de cache: 
    + Si un procesador lee un dato una vez y no vuelve a usarlo, un protocolo dde actualizacion pueden generar un overhead innecesario -> en este caso es mejor un protocolo de invalidacion. 
    + Si dos procesadores trabajan sobre la misma viaje en forma alternada, un protocolo de actualizacion sea mejor opcion -> evita ocio por espera del dato actualizado.   
Relacion costo-beneficio: Los protocolos de actualizaicon pueden producir overhead por comunicaciones innecesarias mientras que los de validacion pueden producir ocio ante la espera del actualizar el dato. 
En la actualidad la mayoria de protocolos se basan en invalidacion. 
    
### Protocolos de coherencia de cache basados en invalidacion: 
    Esquema simple donde cada copia se asocia con uno de 3 estadosL compartida(shared), invalida(invalid) o sucia (dirty)
    
    - En compartida (shared): hay multiples copias validas del dato. Ante una escritura, pasa a estado sucia donde se produjo mientras que el resto se marca como invalida. 
    - En Sucia(dirty): la copa es valida y se trabaja con esta. 
    - En invalida la copia no es valida. Ante una lectura se actualiza a partir de la copia valida. 

#Implementacion de protocolos de coherecia: 
    Existen una variedad de mecanisms de hardware para implementar protocolos de coherencia de cache: 
    - Sistemas snoopy
    - Sistemas basados en directorios
    - Combinaciones de los dos anteriores. 
### Implementacion Snoopy: 
    - Asociado usualmente a los sistemas multiprocesador interconectados por alguna red broadcast, como bus o anillo. 
    - La cache de cada procesador mantiene un conjunto de tags asociados a sus bloques, los cuales deterinan su estado. 
    - Todos los procesadores monitorizan (snoop) el bus, lo que permite realizar las trnasaccciones de estado en sus bloques. 
Cuando el hw snoopy detecta una lectura sobre un bloque cache dirty, entonces toma el control del bus y cumple el pedido.
Cuando el hw snoopy detecta una escritura sobre un bloque de datos del cual tiene copia entonces la marca como invalida.    

Snoopy se adopto masivamente por ser un esquema simple, de bajo costo y buen rendimiento para operaciones locales. 
Si cada procesador opera sobre datos disjuntos, entonces los mismos pueden ser cacheados: 
    - Ante operaciones de escritura, el dato es marcado como sucio, al no haber operaciones de otros procesadores, las siguientes peticiones se satisfacen localmente. 
    - Ante operaciones de lectura, el datoes marcado como compartido. Las peticiones siguientes se satisfacen localmente en todos los casos. 
    - En ambos casos, el protocolo no agrega overhead adicional. 
    - Si diferentes procesadores realizan lecturas y escrituras sobre el mismo dato, se genera trafico en el bus para poder mantener la coherencia de los datos. 
    - Al ser redes broadcast el mensaje de coherencia llega a todos los procesadores aun si no tienen el dato en cuestion. 
    - Como el bus a su vez tiene un ancho de banda limintado, se convierte en un cuello de botella. 

Una solucion obvia a este problema consiste en solo propagar las operaciones de coherencia a aquellos procesadores que tienen el dato invlucrado, lo cual requiere mantener un registro de que datos tiene cada procesador -> Sistemas basados en directorios. 

### Sistemas Basados en directorios: 
    La memoria principal incorpora un directorio que mantiene informacion de estado (bits de presencia + estado) sobre los bloques de cache y los procesadores donde estan cacheados. 
    La informacion contenida en el directorio permite que solo aquellos procesadores que tienen un determinado dato queden involucrados en las operaciones de coherencia. 
    Si los procesadores operan sobre datos dijunto, las peticiones pueden cumplirse localmente (no agrega overhead). 
    Cuando multiples procesadores leen y escriben los mismos datos, se generan operacioens de coherencia -> provoca overhead adicional por la necesidad de mantener actualizado el directorio. 
    + como el directorio esta en memoria, si un programa paralelo requiere un gran numero de operaciones de coherencia, se genera overhead por la competencia en el acceso al recurso. 
    + La cantidad de memoria requireida por el directorio puede ser un cuello de botella a medida que el numero de procesadores crece. 

Como el directorio representa un punto centralizado de acceso, una solucion posible es particionarlo -> Sistemas basados en directorios distribuidos. 

### Sistemas Basados en directorios distribuidos: 
    Se da enb arquitecturas escalables, donde la memoria se encuentra fisicamente distribuida. 
    + Cada procesador es responsable de mantener la coherencia de sus propios bloques (mantiene su propio directorio). 
        - Cada bloque ahora tendra un propietario
        - Cuando un procesador desea leer un bloque por primera vez, debe perdiselo al propietario, quien redirige el pedido de acuerdo a la informacion del directorio. 
        - Cuando un procesador escribe un bloque de memoria, envia una invalidacion al propietario, quien lego la propaga a todos aquellos que tiene una copia. 
    + Como el directorio esta distribuido, la competencia en el acceso al mismo se alivia -> sistema mas escalable. 
    + La latencia y el ancho de banda de interconexion se convierten ahora en cuellos de botella del rendimiento de estos sistemas. 

### Costos de Comunicacion: 
    Uno de los mayores overheads en los programas paralelos proviene de la comunicacion entre unidades de procesamiento. 
    El costo de la comunicacion depende de multiples factores y no solo del medio fisico: modelo de programacion, topologia de red, manejo y ruteo de datos, protocolo de software asociados. 
    + Constos diferentes segun la forma de comunicacion: 
        + Modelo simplificado para pasaje de mensaje t_comm = t_s + m t_w donde t_s es el ntiempo requeridop para preparar el mensaje, m es el tanao del mensaje medido en wordsy t_w es el tiempo requerido para transmitir una palabra. 
        + Memoria compartida: resulta dificil modelar costos por multiples factores que escapan al control del programador.      
