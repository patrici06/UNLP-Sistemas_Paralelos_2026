Cátedra: Sistemas Paralelos — Año 2026

# Práctica Nro. 4

## Programación con MPI / Programación híbrida

### Información útil para compilar y ejecutar

- Para compilar con OpenMPI, abra una consola y use `mpicc` empleando la siguiente sintaxis:

  ```
  mpicc archivofuente.c -o nombreBinario
  ```

- Para compilar con OpenMPI+OpenMP, abra una consola y use `mpicc` empleando la siguiente sintaxis:

  ```
  mpicc archivofuente.c -o nombreBinario -fopenmp
  ```

- Para ejecutar un binario en una máquina local, emplee la siguiente sintaxis:

  ```
  mpirun -np P nombreBinario arg1 arg2 ... argN
  ```

  donde `P` representa el número de procesos a generar.

- Para ejecutar en el cluster de la cátedra, siga las instrucciones detalladas en el instructivo.

### Pautas generales

- Para obtener el tiempo de ejecución de todos los algoritmos se debe utilizar la función provista por la cátedra (`dwalltime`).
- Por convención sólo deberá tomarse el tiempo de ejecución del procesamiento y comunicación de datos (se recomienda medir ambos por separado). Esto significa excluir del tiempo de ejecución:
  - Reserva y liberación de memoria.
  - Inicialización de estructuras de datos.
  - Impresión y verificación de resultados.
  - Impresión en pantalla (`printf`).
- Las pruebas deben realizarse de forma aislada a la ejecución de otras aplicaciones. Se debe ejecutar desde consola, sin otras aplicaciones ejecutándose al mismo tiempo.
- Además del algoritmo paralelo, debe implementar el algoritmo secuencial en el caso que corresponda.

### Ejercicios

1. Revisar el código `mpi-simple.c`. Compile y ejecute el código. Modifíquelo para que los procesos se comuniquen en forma de anillo: el proceso `i` debe enviar un mensaje al proceso `i+1`, a excepción del último, que debe comunicarse con el `0`.

  Para este ejercicio se opto por seguir con la comunicacion bloqueante con buffer:
  la solucion se encuentra en mpi-simple-solution.c
Se opto por que todos los procesos tengan igualdad de roles, dividiendo unicamente entre pares e impares para evitar deadlocks. 

2. Los códigos `blocking.c` y `non-blocking.c` siguen el patrón master-worker, donde los procesos worker le envían un mensaje de texto al master empleando operaciones de comunicación bloqueantes y no bloqueantes, respectivamente.
   - Compile y ejecute ambos códigos usando P={4,8,16} (no importa que el número de núcleos sea menor que la cantidad de procesos). ¿Cuál de los dos retorna antes el control?
   Estrictamente hablando la respuesta es non-blocking, porque las operacones de envio y recepcion no bloqueante devuelven el control de la llamada inmediatamente, mientrasue MPI_Recv bloquea hasa recibir el mensaje. 
   Esto influye en los tiempos totales?. 
   No en este patron master-worker con mensajes tan pequennos y sin trabajo extra entre la comunicacion no bloqueante y su wait, el tiempo total es practicamente igual, estoo ue MPI_Send ya terna rapido por poder satisfacer el envio en buffer. El master en non-bloking.c termina bloqueando en MPI_Wait de todos modos. 
   Cuando es util realmente el uso de no bloqueante? cuando queremos solapar computo util entre comunicaiones. 
   - En el caso de la versión no bloqueante, ¿qué sucede si se elimina la operación `MPI_Wait()` (línea 52)? ¿Se imprimen correctamente los mensajes enviados? ¿Por qué?
  Ahora se muestra el valor por defecto del buffer "No deberias estar leyendo este mensaje", eso porque no estamos garantizando que al recibir los workers se encuentre la informacion que realmente esperamos en el buffer. El receive retorna un espado de pending pero como no demoramos al proceso este finaliza sin recibir el mensaje.

  Entonces es importante quedarnos con que es MPI_Wait quien nos permite separar "iniciar la comunicacion" de "usar los datos de forma segura"

3. Los códigos `blocking-ring.c` y `non-blocking-ring.c` comunican entre procesos mediante un anillo, empleando operaciones bloqueantes y no bloqueantes, respectivamente. Compile y ejecute ambos códigos empleando P={4,8,16} (no importa que el número de núcleos sea menor que la cantidad de procesos) y N={10000000, 20000000, 40000000, ...}. ¿Cuál de los dos algoritmos requiere menos tiempo de comunicación? ¿Por qué?
   - Nota: Para el caso de P=16, agregue la línea `--overcommit` al script de SLURM y el flag `--oversubscribe` al comando `mpirun`.
   La versión no bloqueante requiere siempre menos tiempo de comunicación. La razón es estructural y se explica analizando cómo opera cada versión.

   **Versión bloqueante** (`blocking-ring.c`): la comunicación se organiza como un **pipeline secuencial**. El proceso P-1 envía primero al 0 (el protocolo eager de MPI_Send permite que retorne rápido para este primer salto). Recién cuando el 0 completa su `MPI_Recv`, puede enviar al 1; el 1 recibe y recién ahí envía al 2, y así sucesivamente. Cada salto depende del anterior, por lo que el tiempo total es la suma de todos los saltos: `T_bloq ≈ P × (latencia + transferencia)`. Crece linealmente con P.

   **Versión no bloqueante** (`non-blocking-ring.c`): cada proceso postea su `MPI_Isend` (al siguiente) y su `MPI_Irecv` (del anterior) **inmediatamente, sin esperar a nadie**. Como `MPI_Isend` e `MPI_Irecv` retornan en cuanto registran la operación (no bloquean hasta completar la transferencia), todos los procesos llaman a ambas funciones en simultáneo. En ese momento MPI ya tiene visibilidad completa del patrón de comunicación y puede iniciar todas las transferencias del anillo **concurrentemente en la red**. Luego cada proceso llama a `MPI_Wait` para asegurar que sus datos ya están disponibles, pero el grueso de la transferencia ya ocurrió en paralelo mientras los `MPI_Wait` se ejecutaban. El tiempo total es aproximadamente el del **salto más lento**: `T_noblq ≈ max_i(latencia_i + transferencia_i)`. No depende de la cantidad de procesos P, sino solo del tamaño de los datos y de la peor latencia del anillo.

   **Resultados en el cluster:**

   | N | P | Bloqueante (s) | No Bloqueante (s) | Mejora |
   |--:|--:|--:|--:|--:|
   | 10M | 4 | 0.104562 | 0.054842 | 1.91× |
   | 10M | 8 | 0.236479 | 0.073165 | 3.23× |
   | 10M | 16 | 0.813434 | 0.210898 | 3.86× |
   | 20M | 4 | 0.206114 | 0.095325 | 2.16× |
   | 20M | 8 | 0.466438 | 0.287590 | 1.62× |
   | 20M | 16 | 1.730703 | 0.475714 | 3.64× |
   | 40M | 4 | 0.409035 | 0.175593 | 2.33× |
   | 40M | 8 | 0.942097 | 0.505540 | 1.86× |
   | 40M | 16 | 6.493433 | 0.681735 | 9.52× |

   Los datos confirman el análisis: a medida que P crece, la versión bloqueante escala linealmente (más saltos serializados), mientras la no bloqueante se mantiene mucho más estable al solapar todas las transferencias. El caso más extremo es P=16, N=40M, donde la diferencia es de ~9.5× porque el pipeline bloqueante serializa 16 saltos con latencia de interconexión real, mientras el no bloqueante completa en lo que tarda el salto más lento de los 16 ejecutándose en paralelo.

4. El algoritmo `mpi_matmul.c` computa una multiplicación de matrices cuadradas empleando comunicaciones punto a punto:
   - Compile y ejecute el código empleando N={512,1024,2048} usando todos los núcleos de 1 y 2 nodos. ¿Mejora el speedup y la eficiencia al pasar de 1 a 2 nodos? ¿Qué sucede con el overhead por comunicaciones?
   En local, procesador con 4 cores fisicos:

   | N | P | T_Seq (s) | T_Par (s) | Speedup | Eficiencia |
   |---|---|-----------|-----------|---------|------------|
   | 512 | 2 | 0.628468 | 0.351085 | 1.790073 | 0.895036 |
   | 1024 | 2 | 5.019554 | 2.934756 | 1.710382 | 0.855191 |
   | 2048 | 2 | 43.964977 | 24.984936 | 1.759659 | 0.879829 |
   | 512 | 4 | 0.628468 | 0.263960 | 2.380921 | 0.595230 |
   | 1024 | 4 | 5.019554 | 2.187522 | 2.294630 | 0.573657 |
   | 2048 | 4 | 43.964977 | 16.880294 | 2.604514 | 0.651128 |

    Estas mediciones son locales, no representan nodos, es necesario validaciones en el cluster de la catedra. Pero nos sirven para hipotesis iniciales. Analizando los datos, vemos que el speed up no aumenta razonablemente en base a los procesos generados, esto puede ser una pista que el buffer, la demora de procesos, sincronzacion y bus de comunicacion pueden estar perjudicando al performance general del algoritmo, lo que digo es que no es necesariamente un overhead sino demoras innecesarias dado que se realiza punto a punto.
    Mediciones en el cluster: 
    
    ### Resultados Ejercicio 4 - Multiplicación de Matrices MPI

| N | Nodos | Procesos | Tₚ (s) | Tcomm (s) | Speedup | Eficiencia | OC (%) |
|---|--------|----------|---------|-----------|----------|------------|--------|
| 512  | 1 | 1 | 1.601323 | 0.000004 | N/A | N/A | 0.00 |
| 512  | 1 | 4 | 0.409775 | 0.009381 | 3.907810 | 0.976953 | 2.29 |
| 512  | 2 | 8 | 0.293583 | 0.093361 | 5.454413 | 0.681802 | 31.80 |
| 1024 | 1 | 1 | 12.998812 | 0.000005 | N/A | N/A | 0.00 |
| 1024 | 1 | 4 | 3.301078 | 0.039613 | 3.937748 | 0.984437 | 1.20 |
| 1024 | 2 | 8 | 2.002957 | 0.372433 | 6.489811 | 0.811226 | 18.59 |
| 2048 | 1 | 1 | 103.608182 | 0.000005 | N/A | N/A | 0.00 |
| 2048 | 1 | 4 | 26.541913 | 0.125132 | 3.903569 | 0.975892 | 0.47 |
| 2048 | 2 | 8 | 14.721710 | 1.513615 | 7.037782 | 0.879723 | 10.28 |

  Contestemos entonces las preguntas: 
  ¿Mejora el speedup y la eficiencia al pasar de 1 a 2 nodos? ¿Qué sucede con el overhead por comunicaciones?
  - Sobre el speedup: 
    Mejora pero no linealmente, el speedup aumenta porque hay mas poder de computo pero la ganancia decada proceso es menor a la ideal 
  - Sobre la eficiencia: 
    La eficiencia empeora. En especial al pasar a 2 nodo esto directamente por el overhead de comunicaciones internodo.
  - Sobre el overhead por comunicaciones: 
    El overhead de comunicaciones se dispara al pasar de intra-nodo a inter-nodo, esto por la latencia que produce el bus de interconexion, es imporante ver que mpi_matmul.c distribuye TODA la matriz B a todos los procesos y recolecta strips de C por punto a punto, lo que genera mucho trafico cuando los procesos estan en nodos distintos, esto incrementa el porcentaje de overhead de comunicaciones. 

   - Revise las secciones de código donde se realiza la comunicación de las matrices. Analice el patrón de comunicación y piense si es posible emplear comunicaciones colectivas en lugar de punto a punto. En ese caso, modifique el código original, compile y ejecute la nueva versión para los mismos casos de prueba que el punto anterior. ¿Se mejora la legibilidad? ¿Se logra mejorar el rendimiento? ¿Por qué?
   Si, se es mas se deben usar comunicaciones colectivas. 
   Ahora mismo se usa comunicacion punto a punto para: 
   Distribuior strips de A desde el coordinador a cada worker. Enviar la matriz B completa a cada worker. Recolectar strips de C desde cada worker al coordinador. 

  Entonces que reemplazos tenemos para estas comunicaciones: 
  - Para MPI_Send (strips de A) -> MPI_Scatter
    + Este distribuye porciones de un arreglo desde el root/coordinador a todos los procesos. 
  - Para MPI_Send (B completa) -> MPI_Bcast 
    + envia el mismo dato a todos los procesos. 
  - MPI_Recv (strips de C) -> Gather
    + Recolecta porciones de todos los procesos en el root/coordinador
  
  Obviamente se producen mejoras con esto, sin embargo hay consideraciones mejorables respecto a como se realiza la paralelizacion en base a datos el problema se presenta claramente en que se paralelizan salidas y al haber independencia de datos aqui se podria hacer perfectamente strips en la matriz B, el producto se hace con BxT pero podriamos logicamente manejarla asi y evitarnos saturar el bus de comunicaciones. 

5. Desarrolle un algoritmo paralelo que dado un vector V de tamaño N obtenga el valor máximo, el valor mínimo y valor promedio de sus elementos. Para todos los ejercicios, considere lo siguiente al momento de desarrollar código:
   - ¿Qué tipo de paralelismo le parece más adecuado para este caso? ¿Funcional o de datos?
   + Claramente en datos, usariamos una estrategia de maestro -> envia mensaje con area de trabajo, recepcion de parciales y calculos finales. escalvos -> recepcion de area de trabajo y envio de datos locales. Seria ideal usar comunicaciones colectivas para este proposito.

   - Analice cómo distribuir el trabajo entre los hilos. ¿Qué técnica de descomposición aplicaría para obtener las tareas? ¿Es la misma que se usó para la multiplicación de matrices?
   + La tecnica de descomposicion seria en base a paralelizar salidas. 

   - Identifique las regiones de código que pueden ejecutarse en paralelo y las que no. ¿Existen dependencias? ¿Cómo sería el grafo de dependencias de tareas?
   + Los calculos parciales son las paralelizables, la asignacion de areas y recoleccion no.
    representacion del grafo
                / metricas_sub_V_0 \
    ->porciones - metricas_sub_V_1  -> agrupacion 
                \ metricas_sub_V_2 /

   - Luego, compile y ejecute para P={2,4,8,16} variando el valor de N. Analice el rendimiento, considerando:
     - ¿Qué algoritmo secuencial conviene utilizar?
     - ¿Cómo debe presentar tablas y gráficos de métricas?
     - ¿Qué puede decir sobre el rendimiento de su algoritmo ante cambios en P y N?
     - ¿Qué puede decir sobre escalabilidad?
     - ¿Cuál es la relación entre el overhead de comunicaciones y la eficiencia?

6. Desarrolle una versión híbrida (MPI+OpenMP) de la multiplicación de matrices. Replique el análisis realizado para el algoritmo puro MPI (ejercicio 4). Analice el rendimiento de las dos versiones, considerando las mismas preguntas que en el ejercicio anterior (ejercicio 5). Luego, compare sus rendimientos. ¿Cuál es mejor? ¿Por qué?

---

Facultad de Informática — Universidad Nacional de La Plata
