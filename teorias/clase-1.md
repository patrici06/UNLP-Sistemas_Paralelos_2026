# Clase 1

## Procesamiento paralelo

- Es el uso de múltiples unidades de procesamiento para resolver problema computacional.
- El problema se divide en partes separadas que pueden ser resueltas en forma concurrente.
- Cada parte es luego dividida en una serie de instrucciones.
- Las instrucciones de cada parte se ejecutan simultáneamente sobre diferentes procesadores.
- Se requiere mecanismo de control/coordinación global.

El procesamiento paralelo, permite resolver problemas grandes o complejos. Provee concurrencia implícitamente.

- Reducir tiempos de cómputo, lo que reduce costos.
- Hacer mejor uso del hardware.

Los problemas de carácter demandante en cómputo tales como aplicaciones científicas, industria o comercio son algunos de los ejemplos de sectores donde la paralelización es deseada.

## Procesamiento concurrente, paralelo y distribuido

- Concurrente: aquel en el que múltiples tareas puede estar avanzado en cualquier instante de tiempo.
- Paralelo: aquel en el que múltiples tareas que se ejecutan simultáneamente cooperan para resolver un problema.
- Distribuido: aquel en el que múltiples tareas que se ejecutan físicamente en diferentes lugares cooperan para resolver uno o más problemas.

El procesamiento paralelo busca reducir el tiempo de ejecución de un programa empleando múltiples procesadores al mismo tiempo.

- El hardware brine respuesta pero lo fundamental sigue siendo el software.
- Se tiene una gran dependencia del software.

Un sistema distribuido es un conjunto de computadoras autónomas interconectadas que cooperan compartiendo recursos (físicos y datos).

- Pueden ejecutar múltiples aplicaciones de diferentes usuarios.
- Sus características y aplicaciones objetivos dependen de la granularidad de sus nodos y el grado de acoplamiento de los procesadores.

Se caracteriza por problemas de no tener un reloj único, se requiere planificación, la escalabilidad depende de las comunicaciones, heterogeneidad, seguridad de los datos.

## Cómputo de alto rendimiento

Consta de sistemas de extraordinario poder computacional y de técnicas de procesamiento paralelo para la resolución de problemas complejos con alta demanda computacional.

## Clasificación de las plataformas de cómputo paralelo

Es fundamental conocer las características de la arquitectura adyacente.

Podemos diferenciarlas por:

- Por el mecanismo de control:
  - Corresponde a la Taxonomía de Flynn
  - De acuerdo a cómo se especifica el paralelismo entre instrucciones y datos.
- Por la organización física:
  - De acuerdo al espacio de direcciones que tiene cada procesador (es decir, cómo ve la memoria principal).

### Por Mecanismo de control

- **SISD** → Single Instruction Stream Single Data Stream: instrucciones ejecutan de forma secuencial, una por ciclo de reloj. Los datos afectados son aquellos que hace referencia la instrucción en cuestión. Ejecución determinística (ej: mainframes).
- **SIMD** → Single Instruction Multiple Data Stream: Todas las unidades de procesamiento ejecutan la misma instrucción sobre diferentes datos. Síncrona y determinística. Hardware simplificado. Unidades pueden desactivarse selectivamente. Ideal para problemas de alta granularidad (procesamiento de imágenes).
- **MISD** → Multiple Instruction Single Data Stream: Las unidades de procesamiento ejecutan diferentes instrucciones sobre mismo dato. No existen máquinas reales basadas en este modo.
- **MIMD** → Multiple Instruction Multiple Data Stream: Unidades de procesamiento ejecutan diferentes instrucciones sobre diferentes datos. Puede ser síncrona o asíncrona, determinística o no determinística. Pueden ser máquinas de memoria compartida o de memoria distribuida. La clase más común de máquinas paralelas.

### Por Mecanismo de la organización física

Se realiza de acuerdo al espacio de direcciones que tiene cada procesador → en otras palabras, tiene en cuenta la visión de la memoria principal por parte de cada procesador.

Las opciones son:

- Memoria compartida
- Memoria distribuida
- Memoria híbrida

### Memoria compartida

Estos pueden variar pero se caracterizan por la capacidad que tienen sus procesadores de acceder a toda la memoria como un único espacio de direcciones global.

Cuentan con múltiples procesadores que operan de forma independiente pero que comparten los mismos recursos de memoria. Los cambios realizados por un procesador serán visibles para el resto.

Se necesita mecanismo de coherencia de caché.

Sub-clasificación por modo de acceso a memoria:

- Acceso uniforme a memoria (UMA)
- Acceso no uniforme a memoria (NUMA)

**Ventajas:**

- La comunicación de datos entre los procesadores es más rápida y uniforme debido a su cercanía.
- La programación de estos sistemas suele ser fácil debido al espacio de direcciones global.

**Desventajas:**

- Se debe asegurar un correcto acceso a los datos por parte de los procesadores.
- No da escalabilidad entre las memorias y los procesadores.
- Agregar más procesadores incrementa el tráfico de memoria y complejiza los mecanismos de coherencia de caché.

### Memoria distribuida

Se caracterizan por requerir de una red de comunicaciones para poder conectar a sus procesadores. Cada procesador opera en forma independiente y tiene su propia memoria. No cuenta con espacio de direcciones global → esto significa que los cambios realizados por un procesador no serán visibles para el resto.

Cuando un procesador necesita un dato que posee otro, el programador se vuelve responsable de definir cómo y cuando será comunicado. No se requiere un mecanismo de coherencia de caché, ya que las memorias son locales.

Redes de interconexión: Ethernet, InfiniBand, Myrinet.

**Ventajas:**

- La memoria escala con el número de procesadores, en proporcionalidad debido al espacio de direcciones distribuido.
- Cada procesador accede de forma más rápida a los datos.
- No hay necesidad de un protocolo de coherencia de caché.
- Buena relación costo-rendimiento. Se pueden construir a partir de máquinas convencionales.

**Desventajas:**

- Acceso NUMA: los datos que están en nodos remotos toma tiempo.
- El manejo de la comunicación se vuelve explícito.
- El espacio de direccionamiento distribuido puede condicionar la programación.

### Memoria híbrida

Son múltiples máquinas de memoria compartida interconectadas entre sí para permitir que sus procesadores puedan comunicarse.

**Ventajas y desventajas:**

- Lo que es común a ambos modelos.
- Soluciona el problema de escalabilidad de memoria compartida.
- Aumenta la complejidad de programación.

## Modelos de programación paralela

Generalmente tomamos 2 alternativas:

- Memoria compartida
- Pasaje de mensajes

### Modelo de memoria compartida

En el modelo de memoria compartida, múltiples tareas se ejecutan en forma concurrente. Todas las tareas acceden a un mapa de memoria común y además cada una puede tener memoria local "exclusiva".

La comunicación y sincronización de estas tareas se realiza escribiendo y leyendo áreas de memoria compartida.

Generalmente usado en plataformas de memoria compartida como multiprocesadores o multicores.

El programador en general no maneja la distribución de los datos ni lo relacionado a la comunicación de los mismos.

**Ventajas:** Transparencia para el programador. La ubicación de los datos, su replicación y su migración son transparentes.

**Desventajas:** A veces es necesario trabajar sobre esos aspectos para mejorar el rendimiento. Es difícil la predicción de performance a partir de la lectura del algoritmo.

### Modelo de Pasaje de Mensajes

Consiste en p procesos (eventualmente procesadores), cada uno de ellos con su espacio de direcciones exclusivo.

**Característica clave:** Espacio de direcciones particionado:

- Cada dato pertenece a una partición.
- Toda interacción requiere la cooperación de dos procesos.

El intercambio de mensajes sirve para varios propósitos:

- Intercambio explícito de datos (programador).
- Sincronizar procesos.

Ejemplo de uso: clusters.

**Ventajas:**

- El programador tiene control total para lograr sistemas eficientes y escalables.
- Puede implementarse eficientemente en muchas arquitecturas paralelas.
- Más fácil de predecir el rendimiento.

**Desventajas:**

- Mayor complejidad al implementar estos algoritmos para lograr alto rendimiento.

### Modelos de programación y arquitecturas paralelas

Se suele asociar cada modelo con su correspondiente arquitectura paralela.

Sin embargo estos modelos son abstracciones y deben de poder implementarse en cualquier plataforma paralela:

- Un modelo de memoria compartida sobre una máquina de memoria distribuida.
- Un modelo de pasaje de mensajes sobre una máquina de memoria compartida.

El uso de un modelo que no resulte natural a la arquitectura probablemente provoque una degradación del rendimiento.

### Pipelining

Consiste basically en solapar las diferentes etapas de la ejecución de instrucciones, reduciendo el tiempo de ejecución total.

### ILP - Pipelining

El modelo de referencia es el de "línea de montaje".

Para aumentar la velocidad del pipeline podemos dividirlo en etapas más pequeñas incrementando su profundidad.

- En el contexto de los procesadores, esto permite a su vez mayores frecuencias de reloj.
- Tener en cuenta que la velocidad de un pipeline está limitada por la duración de su etapa más costosa → Etapas más pequeñas aceleran el pipeline.

### Pipelining y ejecución especulativa

Pipelines profundos requieren de técnicas efectivas que sean capaces de predecir los saltos de forma especulativa.

- La penalización de un salto mal predicho se incrementa a medida que el pipeline es más profundo → Más instrucciones adelantadas son las que se pierden.
- Estos factores limitan la profundidad de pipeline y su posible ganancia de rendimiento.

**¿Cómo mejorar entonces la tasa de ejecución de instrucciones?**

- Una forma obvia consiste en usar múltiples pipelines. Durante cada ciclo de reloj, múltiples instrucciones son emitidas en paralelo, las cuales son ejecutadas en múltiples unidades funcionales.

### Pipelining y ejecución superescalar

- No hay una única manera de escribir un programa y la misma tiene incidencia en el rendimiento final.
- El ideal sería que todas las etapas estén activas en todo momento (máximo paralelismo). En la práctica es muy difícil que ocurra.
- Al momento de realizar la planificación de instrucciones, se deben tener en cuenta:
  - Dependencia verdadera de datos: el resultado de una instrucción es la entrada de la siguiente.
  - Dependencia de recursos: dos operaciones requieren el mismo recurso.
  - Dependencia de salto: las instrucciones a ejecutar después de un salto condicional no pueden ser determinadas a priori sin margen de error.
- El planificador (hardware) analiza un conjunto de instrucciones de la cola de instrucciones a ejecutar y emite aquellas que pueden ser ejecutadas en forma concurrente, teniendo en cuenta las dependencias.
- Si las instrucciones son ejecutadas en el orden en que aparecen en la cola, se dice que la emisión es en orden → simple pero limita la emisión de instrucciones.
- Si el procesador tiene la habilidad de reordenar las instrucciones en la cola, entonces se puede alcanzar el máximo rendimiento posible → este modelo se conoce como fuera de orden y aunque es más complejo, es el más usado en la actualidad.

### Instrucciones SIMD

Incorporación de unidades de procesamiento vectorial que permiten ejecutar una instrucción sobre diferentes datos en un ciclo de reloj.

### Memory Wall

Indica que llegarán un momento en que el rendimiento estará dominado por la velocidad de la memoria (pone una pared física).

### ILP Wall

Si bien es posible agregar más unidades funcionales al chip no se alcanzaría una mejora de rendimiento para las aplicaciones debido a que no es posible extraer más ILP de los programas.

**Problemas:**

- Limitaciones de los compiladores.
- Dependencias entre instrucciones.
- Imposibilidad de predecir saltos.
- Cantidad limitada de paralelismo intrínseco.

### Power Wall

La potencia usada en el procesador se vuelve calor. Si la temperatura aumenta se reduce necesariamente la velocidad de los transistores y puede llevar a fallos o destrucción del procesador.

El problema es que el crecimiento de tensión, calor producido es cuadrático y esto significa que se llega a un punto verdaderamente imposible para refrigerarlo.

### Procesadores multicore

Desde su desarrollo, los procesadores multicore han sofisticado su diseño en las sucesivas generaciones.

Los primeros procesadores de este tipo eran prácticamente dos procesadores mononúcleo en la misma oblea.

Las siguientes generaciones han incrementado el número de núcleos e incorporado niveles de caché L2 y L3.

### Multi-hilado a nivel hardware

Técnica complementaria a los multicore: un único hilo de ejecución no resulta suficiente para aprovechar la potencia de los procesadores superescalares.

El pipeline del procesador puede quedar atascado (stalled):

- Al esperar el resultado de una operación larga de punto flotante (o entera).
- Al esperar datos que deben llegar desde la memoria.

Mientras tanto, el resto de las unidades esperan en forma ociosa.

La técnica de Simultaneous Multi-Threading (SMT) consiste en mantener más de un hilo de ejecución al mismo tiempo en el procesador.

Los recursos asociados al estado del procesador son replicados una o más veces. Manteniendo el número original de recursos de ejecución, solo requiere un pequeño incremento en el tamaño del chip.

Con esta replicación el procesador parece tener múltiples núcleos y por lo tanto puede ejecutar múltiples flujos en paralelo, sin importar si pertenecen al mismo programa o a diferentes.

El número de replicaciones de estados determina el número de procesadores lógicos del procesador.

Sin SMT solo un hilo de ejecución puede estar activo en un determinado momento.

En un procesador con SMT ambos hilos pueden estar activos al mismo tiempo, aunque no pueden usar la misma unidad funcional al mismo tiempo.

Los núcleos de un procesador multicore pueden tener múltiples hilos hardware o no.

Las posibles combinaciones:

- Monocore, sin SMT: procesadores mononúcleo convencionales
- Monocore con SMT
- Multicore, sin SMT
- Multicore con SMT

Número de hilos hardware: 2 o 4 usualmente, raramente más.

Es una buena idea buscar ejemplos de procesadores con y sin SMT.

SMT puede mejorar la productividad del procesador siempre y cuando sea posible intercalar instrucciones de múltiples hilos entre los pipelines.

El escenario ideal sería tener múltiples hilos que usen recursos de ejecución diferentes → Lamentablemente esto no es común en la práctica. En ocasiones el número de referencias a memoria de un programa escala con el número de hilos por lo que se puede dar un mejor aprovechamiento del ancho de banda si tenemos un gran número de hilos.

La desventaja de SMT es que si los hilos usan exactamente los mismos recursos, podría haber pérdida de rendimiento por la competencia entre ellos. Por ejemplo, programas que solo computan en punto flotante.

La ganancia por el uso de SMT depende fuertemente del programa a ejecutar. Generalmente lo que conviene es hacer pruebas con y sin SMT para evaluar el beneficio real.

### Jerarquía de memoria en procesadores multicore

- Los procesadores multicore suelen tener memorias caché de múltiples niveles:
  - El nivel 1 siempre es privado.
  - Los siguientes niveles varían de acuerdo a la arquitectura.

### Jerarquía de memoria en procesadores multicore

**Ventajas de las cachés privadas:**

- Al estar más cerca de los cores, el acceso a los datos es más rápido.
- Se reduce la competencia en el acceso a los recursos.

**Ventajas de las cachés compartidas:**

- Hilos en diferentes núcleos pueden acceder a datos que están en la misma caché.
- Más espacio de caché disponible si un único hilo se ejecuta en el procesador.

### Clusters

Colección de computadoras individuales interconectadas vía algún tipo de red, que trabajan en conjunto como un único recurso integrado de cómputo.

- Cada nodo de procesamiento es un sistema de cómputo en sí mismo, con hardware y sistemas operativos propio.
- La red de interconexión puede ser Ethernet o redes específicas de alta velocidad como InfiniBand o Myrinet.
- Puede ser homogéneo o heterogéneo.
- Ofrecen una buena relación costo-rendimiento y son fáciles de expandir.

Los clusters se volvieron populares en la década del '90. Eran clusters de tipo Beowulf. Actualment, la mayoría de los grandes sistemas de cómputo se basan en clusters de nodos multi/many-core, conformando una arquitectura híbrida.

- Dado que los clusters son arquitecturas distribuidas, el modelo de programación más utilizado suele ser pasaje de mensajes.
- También es posible emplear el modelo de memoria compartida, aunque a costo de menor rendimiento.
- Con la incorporación de los procesadores multicore a los clusters, sugirió un nuevo modelo de programación que combina pasaje de mensajes con memoria compartida.
