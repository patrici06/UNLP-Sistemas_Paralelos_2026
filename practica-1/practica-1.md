1. El algoritmo fib.c resuelve la serie de Fibonacci, para un numero N dado, utilizando dos matodos: recursivos e iterativo. Analice los tiempos de ejecucion de ambos metodos. Cual es mas rapido? por que? ejecute N=1..50. 

Es mas rapido iterativo. esto es dado porque recursivo tiene un O(2^N). Ademas principalmente hay un Overhead de llamadas dado que se recrea todo el frame(contexto de la funcion), pasaje de mensajes, contador de programa todo. 

2. El algoritmo funcion.c resuelve, para un x dado una sumatoria, este da una solucion unica y luego una itearcion sobre la misma operacion matematica lo que da:  
 Resultados de la salida: 
patricio@fedora:~/Escritorio/sistemas-paralelos/Fuentes$ ./funcion_c_exe 
Funcion calculada...
 Tiempo total en segundos 0.3102397919 
 Tiempo promedio en segundos 0.0000000031 
Funcion calculada cada vez...
 Tiempo total en segundos 0.3151848316 
 Tiempo promedio en segundos 0.0000000032  
+ cual es mas rapida? porque? 
    Como se ve La mas rapida es la funcion calculada unica vez.
    Aunque la diferencia es infima con la iteracion, esto se debe a que por el principio de localidad temporal, los valores de X estan cacheados, por lo que realmente solo se ejecuta la funcion completa en la primer iteracion deln bucle, luego es lectura de cache y operaciones sobre la misma. 

3. Investigue en la documentacion del compilador o a traves de internet que opciones de optimicacion ofrece el compilador gcc flag O.
    Compile y ejecuyte el codigo de matrioces C, el cual resuelve una multiplicacion de matrices de NxN. Explore los diferentes niveles de optimizacion para distintos tamanos de matrices y luego responda. Que optimizaciones aplica el compilador? Cual es la ganancia respecto a la version sin optimizacion del compilador? Cual es la ganacia entre los distintos Niveles?
    + Optimizacion de gcc:
    Flags: 
        -O0 : No aplica ninguna optimizacion, Sin inlining, sin eliminacion de codigo muerto y variables preservadas para debug. 
        -O1 : Aplica optimizaciones sobre eliminacion de codigo muerto, propagacion de registros y optimizacion de pila. 
        -O2 : Aplica O1 y ademas optimizaciones globales y analisis interprocedural, Esto impolica analisis de aliasing, scheduling de instrucciones, common subexpression eliminatio global. 
        -O3 : Aplica O2 y agrega vectorizacion automatica, unrolling de loops, cloning de funciones. 
        -Os : Aplica casi todo -O2 y activa optimizaciones de compresion de codigo. 
        -Ofast : Optimizaciones sobre floating point. 
        -Og : optimizacion para debug, elimina codigo muerto, evita inlining agresivo, mantiene variables visibles. 
    
    Cada uno de estos flags brillan sobre todo segun el tipo de aplicacion a desarrollar o el entorno que nos encontremos. Ademas la arquitectura circundante es relevante. 
    Aqui las salidas con respecto al archivo: 
     - O Multiplicacion de matrices de 10x10. Tiempo en segundos 0.000055
    Multiplicacion de matrices resultado correcto
     - O1 Multiplicacion de matrices de 10x10. Tiempo en segundos 0.000003
    Multiplicacion de matrices resultado correcto
    - O2 Multiplicacion de matrices de 10x10. Tiempo en segundos 0.000002
    Multiplicacion de matrices resultado correcto
    - O3 Multiplicacion de matrices de 10x10. Tiempo en segundos 0.000002
    Multiplicacion de matrices resultado correcto
    - Os Multiplicacion de matrices de 10x10. Tiempo en segundos 0.000001
    Multiplicacion de matrices resultado correcto
    - Og Multiplicacion de matrices de 10x10. Tiempo en segundos 0.000002
    Multiplicacion de matrices resultado correcto

    El mejor en este caso es O3 dado que la multiplicacion de una matriz idealmente es 3 fors anidados, permite que la optimizacion de vectorificacion explite al maximo los pipelines.
    
4. Investigue si el tipo de dato usado en un codigo determinado puede incidir en si rendimiento, particularmente en el area de computo numerico o cientifico. A que factores afecta el tipo de datos para que influya el rendimiento?. 
Si el tipo de dato influye directamente en el rendimiento, esto porque cada tipo tiene un peso distinto en memoria, lo que afecta directamente a el uso de cache, ancho de banda de la memoria, cantidad de datos que pueden procesarse. 
Ademas segun el tipo de dato las operaciones aritmetricas son mas o menos costosas, por ejemplo los enteros son rapidas y sencillas para el procesador, mientras quer trabajar con punto flotante demanda mas tiempo del mismo. 
Se debe tener en cuenta la verctorizacion y SIMD, segun el tipo de dato puede impactar en estas tecnicas directamente. 
Algunos tipos de datos requieren alineacion en memoria. 

    principalmente estos problemas se ven en los tipos de datos double y float, dado que double es mucho mas preciso que float, por lo que en el ambito cientifico es necesario tener esto en cuenta.

5. Dada la ecuacion cuadratica x^2 - 4.0000000x + 3.9999999 = 0, sus raices son r1 = 2.00316228 y r2 = 1.999683772 (empleado 10 digitos para la parte decimal). 

    a. El algoritmo quadratic1.c computa las raices de esa ecuacion empleado los tipo  s de datos float y double. Compite y ejecuta el codigo, que diferencia nota en el resultado? 
        a. Noto que float redondea resultados

    b. El algoritmo quadratic2.c computa las raices de esta ecuacion, pero de forma repetida. Compile y ejecute el codigo variando la constante TIMES. Que diferencia nota en la ejecucion? 
        b. aumentar la constante TIMES, repite el calculo de las raices, provoca un aumento proporcional en el tiempo de ejecucion.

    c. El algoritmo quadratic3.c computa las raices de esta ecuacion, pero de forma repetida. Compile y ejecute el codigo variando la constante TIMES. Que diferencia nota en la ejecucion? que diferencias puede observar en el codigo con respecto a quadratic2.c?
        c. Al ejecutar y variar TIMES se observa nuevamente un aumento proporcional en el tiempo de ejecucion debido a la repeticion del calculo. 
        La ejecucion utilizando float es mas rapida. 
        La diferencia principal en el codigo es que utiliza funciones powf(), sqrtf() especificas para float, mientras que utiliza pow() y sqrt() que trabajan en double. Esto prooca conversiones implicitas de float a double y luego de vuelta a float, lo que incrementa el costo computacional. Al utilizar powf, sqrtf evitamos converiosnes lo que mejora el rendimiento.

6. Analice el codigo matrices.c y luego responda. Donde cree que seproduce demoras? como podria optimizarse el codigo? Al menos, considere los siguientes aspectos: 
    + Explotacion de la localidad de datos a traves de la reorganizacion interna de las matrices A, B, C (segun corresponda). Como conviene que esten organizadas las matrices? todas de la misma manera o alguna diferente? Afecta esto a la inicializacion? Seguramente le sea de utilidad probar inicialmente con tamanos pequenos y activar la impresion de matrices en pantalla para ver la relacion entre la organizacion logica y fisica de cada una.
    Respuesta:     
        La matriz A esta organizada por filas, a su vez la B tambien, lo cual es un error teniendo en cuenta que queremos realizar una multiplicacion de matrices. Para esto entonces es preferible inicializar la matriz B por columnas y recorrerla coherentemente por columnas. Esto muestra una mejora significable en el rendimiento. 

    + El uso de Setters y getters es una buena practica en la programacion orientada a objetos. Tiene sentido usarlo en este caso? Pruebe eliminando los llamados a funciones y reemplazandolos por sus correspondientes cuerpos. Cual es su impacto en le rendimiento?
        En este paradigma no tiene sentido hacer uso de funciones para getter y setters, Aqui solo agrega overhead en el lamamiento de funciones, carga mas stack frames, parametros y retornos en cache, sin mencionar las comprobaciones adicionales en cada llamamiento de funcion.
        La eliminacion de funciones afecta muy positivamente en el rendimiento. al rededor de un 10% mas rapido con N grande. 
    + Hay expresiones en el computo que se repiten?, pueden refactorizarse para no sr computadas de forma repetida? cual es su impacto en el rendimiento? 
        Si es posible, estas estan presentes en el acceso de los indices en el triple for. Su impacto en el rendimiento es grande, dado que se eliminan n^2 operaciones precalculando y haciendo uso de la localidad espacial.

    + En lugar de ir acumulando directamente sobre la posicion C[i,j] de la matriz resultado (linea 81), pruebe usar una variable local individual y finalizar el bucle mas interno, asigne su valor a C[i,j]. Esta modificacion impacta el rendimiento? porque?
        Es mas eficiente dado que realiza el calculo y lo cachea una unica vez sino son n^2 accesos indebidos a memoria, por el contrario llevar la variable local proporciona un unico acceso, sin calculos intermedios por iteraccion.

7. Analice y describa brevemente como funciona el algoritmo mmblk.c que resuelve 
la multiplicacion de matrices cuadradas de NxN utilizando una tecnica de 
multiplicacion por bloque. Luego, ejecute el algoritmo utilizando distintos 
tamanos de matrices y distintos tamanos de bloque(finalmente pruebe con potencias 
de 2). Finalmente, compare los tiempos con respecto a la multiplicacion de 
matrices optimizada del ejercicio, Segun el tamano de las matrices y del bloque 
elegido, responda. Cual es mas rapido? porque? cual seria el tamano de bloque 
optimo para un determinado tamano de matriz?. 
    
    Funciones principales:
    + matmulblks(): Itera sobre bloques externos (salto de bs)
    + blkmul(): Multiplica sub-bloques bs×bs (límite de bs)
    
    El algoritmo divide matrices n×n en bloques de bs×bs para mantener datos en 
    caché L1/L2, siguiendo el principio de localidad de datos.
    
    ¿Cuál es más rápido?
    matrices.c es ~4.5-5× más rápido que mmblk (sin copia).
    
    Resultados experimentales (1024×1024):
    + matrices.c:        1.0s   (4.70 GFLOPS) 
    + mmblk bs=32:       4.59s  (0.47 GFLOPS)
    + mmblk bs=64:       4.90s  (0.44 GFLOPS)
    + mmblk bs=128:      4.93s  (0.44 GFLOPS)
    + mmblk bs=256:      5.01s  (0.43 GFLOPS)
    
    ¿Por qué?
    matrices.c: Acceso secuencial a memoria [0→1→2→...→1023], localidad 
    perfecta, sin overhead de llamadas. L1 HITS ~95%.
    
    mmblk (sin copia): Acceso disperso [0→1024→2048→...], saltos de n 
    elementos (8KB). Genera muchos cache misses en L1 (~76% MISS). Si bien 
    divide en bloques, estos NO están contiguos en memoria, por lo que nunca 
    aprovecha la localidad espacial de caché.
    
    ¿Cuál sería el tamaño de bloque óptimo?
    
    El tamaño de bloque ÓPTIMO depende ÚNICAMENTE del hardware (caché L1), 
    NO del tamaño de la matriz:
    
    bs_óptimo = √(L1_size / (3 × 8 bytes))
    
    Para L1 típico de 32KB: bs ≈ √(32768/24) ≈ 37 → usar bs=32 o 64 (potencia de 2)
    
    Este bs es óptimo para matrices de 512×512, 1024×1024, 2048×2048, etc.
    
    La ÚNICA relación con n es que (n % bs) = 0 (n debe ser divisible por bs).
    
    Nota: Sin copia a memoria contigua, el bs óptimo solo logra ~10% de mejora,
    siendo mmblk aún 4.6× más lento. El problema fundamental es el acceso 
    disperso a memoria, no el tamaño de bloque.

8. Analice el codigo mmblk-const.c que tambien resuelve la multiplicacion de matrices cuadradas de NxN utilizando una tecnica de multiplicacion por bloques. Compile y ejecute codigo para los mismos tamanos de problema y de bloque que uso en el ejercicio anterior. Cual es mas rapido? porque? En caso de que mmblk-const.c sae el mas rapido? tiene alguna desventaja?
    Es mas rapido mmblk-const dado que: 
        - BS es conocido en tiempo de compilacion, en lugar de ejecucion.(optimiza el acceso a este valor)
        - i*BS se optimiza. 
        - Loop unrolling mas agresivo. 
        - Mejor inlining de funciones
        - Menos overhead de parametros. 
    Ofrece entonces mas ventajas en optimizaciones que puede ejecutar el compilador. 
    Las desventajas que presenta es: 
        - Falta de flexibilidad en BS
        - Menos testeble, para evaluar BS requiero recompilaciones. 
        - Menos escalabe en general. 
    La mejora es marginal. El problema principal de acceso parcial es ineficiente. 

9.Vuelva a resolver el ejercicio 6 pero ahora usando uno de los nodos del cluster remoto de la catedra. En caso de que ya lo haya usado, entonces emplee su computadora personal. Adicionalmente responda: 
El tamano del bloque optimo es el mismo? En caso positivo. considera que siempre sera asi? de que depende el tamano de bloque optimo para un sistema?  
No entiendo bien las preguntas porque este ejercicio no tiene relacion alguna.

10. Analice el algoritmo triangular.c que resuelve la multiplicacion de una matriz cuadrada por una matriz triangular inferior, ambas de NxN. Como se podria optimizar el codigo? Se pueden evitar operaciones?. Se puede reducir la memoria reservada? Implemente una solucion optimizada y compare los tiempos probando con diferentes tamanos de matrices. 
    - Se puede optmizar el codigo, se pueden evitar operaciones en las multiplicaciones. Es posible reducir la memoria reservada alocando solo (N *(N+1))/2 es posible reducir incluso los tiempos de comparado a el algoritmo original un 40% mas eficiente al reducir en un 40 el tamano de la matriz