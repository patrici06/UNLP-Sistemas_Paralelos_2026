Practica 2, Programacion con Pthreads: 
Para todos los puntos debemos responder una serie de preguntas: 
 - Que tipo de paralelismo le parece mas adecuado para este caso?, Funcional o datos? 
- Analice como distribui el trabajo entre hilos. Que tecnica de descomposicion aplicaria para obtener las tareas?
- Identifique las regiones de codigo que pueden ejecutarse en paralelo y las que no, Existen dependencias? Como seria el grafo de dependencias de tareas?
- Que algoritmo secuencial conviene utilizar? 
- Como debe presentar tablas y graficos de metricas? 
- Que puede decir sobre el rendimiento de su algoritmo ante cambios de T y N?
- Que puede decir sobre la escalabilidad?

1. Desarrolle un algoritmo paralelo que compute una suma de vectores Ai = Bi + Ci. Para ello, siga las pautas indicadas mas arriba. Mida el tiempo de ejecucion para distintos valores de N y T = {2, 4, 8}, y luego analice el rendimiento. 

Notas:
Que paralelismo es mas adecuado?
Para determinar esto analicemos el problema. Para hacer la suma de vectores los datos son independientes, la operacion es sencialla, cada elemento se procesa de la misma manera. Estos son indicadores que lo mejor es paralelismo de datos!.

Es importante denotar que lo correcto aqui es dividir la caga de trabajo por blockes y no por ciclos. 
Para la distribucion de la carga de trabajo deberiamos asignar N/T (T = hilos) a cada worker.
Esto aprovecha mucho mejor la linealidad de los datos en memoria, mejora los cache hits y es sencillo de implementar. 
 
A cada hilo, sera necesario mandarle quien es (pid) y para indicarle el principio y el final de su bloque de trabajo: 
inicio = thread_id * (N/T)
fin  = inicio + (N/T) -1

Por la naturaleza el ejercicio no existe dependencias entre los elementos de un mismo vector, no hay dependencias entre hilos, todos son independientes. 
la estructura del programa deberia quedar algo como: 
//parte secuencial 
Creacion de datos
Creacion de hilos
Iniciar medicion de tiempo
//region paralela: suma de vectores
Cada hilo ejecuta: A[i] = B[i] + C[i]
//region secuencial: sincronizamos 
Esperar que todos terminen (pthread_join)
Detener medicion de tiempo
Liberar memoria

Para todos los ejercicios debemos tener presente que debemos tener una medicion clara de las metricas relevantes para el campo. Estas son: 
1. Tiempo de ejecucion(ms)
2. Speedup:S(T) = tiempo_secuencial/Tiempo_paralelo_con_T_hilos
3. Eficiencia: E(T) = S(T) / T -> que tan bien aprovechamos los hilos. 

A tener en cuenta al analizar estas metricas debemos tener en cuenta que podriamos encontrar los siguientes problemas: 
- overhead de creacion/sincronizacion de hilos mayor al beneficio. 
- Contencion por cache entre hilos 
- desbalance de carga. 

solucion: ej-1.c

2. Desarrolle un algoritmo paralelo que compute la multiplicacion de matrices cuadradas NxN a partir de la version optimizada del ejercicio 6 de la practica anterior. Mida el tiempo de ejecucion para N={512,1024,2048,4096} y T={2,4,8} y luego analice el rendimiento. 

Notas: 
solucion: ej-2.c

3. Desarrolle un algoritmo paralelo que compute la multiplicacion de matrices cuadradas de NxN a partir de la version que computa por bloques. Mida el tiempo de ejecucion para N={512,1024,2048,4096} y T={2,4,8}y luego analice el rendimiento. Nota: puede asumir que el tamano de bloque (TB) es multiplo de N. 

Notas: 
solucion: ej-3.c

4. Desarrolle un algoritmo paralelo que cuente la cantidad de veces que un elemento X aparece dentro de un vector de N elementos enteros. Alfinalizar, la cantida de ocurrencis del elemento X debe quedar en una variable llamada ocurrencias. Para la sincronizacion emplee mutex-locks. Pruebe con diversos tamanos de N y T={2,4,8}. Analice el rendimiento. 

Notas:
solucion: ej-4.c

5. Desarrolle un algoritmo paralelo que calcule el valor promedio, minimo y maximo de los numeros almacenados en un vector de tamano N. Para sincronzacion emplee semaforos. Pruebe con diversos tamanos de N y T={2,4,5}. Analice rendimiento. 

Notas: 
solucion: ej-5.c


6. Si tuviese que resolver el ejercicio 1 en el caso de una arquitectura heterogenea, como los nuevos procesadores hibridos de intel, Utilizaria la misma estrategia de distribucion de trabajo o una diferente? Que debe tener en cuenta en este caso? 

Notas: 
 
