# Clase 3

## Temas de la clase: Diseño y modelos de algoritmos paralelos

## Diseño de algoritmos paralelos

- Un algoritmo secuencial es ideal para resolver un problema en una única unidad de procesamiento.
- Un algoritmo paralelo es un esquema para resolver problemas empleando múltiples unidades de procesamiento.

El desarrollar un algoritmo paralelo es más que determinar los pasos a resolver el problema → el programador debe al menos considerar la concurrencia y especificar qué pasos pueden llevarse a cabo simultáneamente.

En la práctica, el diseño de algoritmos paralelos puede incluir algunas de las siguientes actividades:

- Identificar porciones de trabajo (tareas) que puedan resolverse en paralelo.
- Asignar tareas a procesos que ejecutan en diferentes procesadores.
- Distribuir datos de entrada, de salida e intermedios asociados con el programa.
- Administrar accesos a datos compartidos.
- Sincronizar procesos en diferentes etapas del programa.

Puede existir diferentes opciones para cada paso aunque usualmente pocas combinaciones son las que producen buen rendimiento.

## Pasos Fundamentales: Descomposición en tareas y Mapeo de tareas a procesos

## Descomposición de tareas

Es el proceso de dividir el cómputo en partes más pequeñas (tareas), de las cuales algunas o todas podrán ser potencialmente ejecutadas en paralelo.

- Se trata de definir un gran número de tareas pequeñas para obtener una descomposición de grano fino → brindar mayor flexibilidad a los potenciales algoritmos paralelos.
- Probablemente, en etapas posteriores, la evaluación de los requerimientos de comunicación, la plataforma destino, o cuestiones de ingeniería de software, pueden llevar a descartar algunas opciones de descomposición consideradas inicialmente.
  - En esos casos, la partición original es revisada y sus tareas son aglomeradas para incrementar su tamaño o granularidad.
- La descomposición se puede realizar de diferentes modos. Una primera aproximación consiste en pensar tareas de igual código (paralelismo de datos o dominio) o tareas de código diferente (paralelismo funcional).

### Descomposición de datos

Consiste en descomponer los datos asociados a un problema de pequeñas porciones (usualmente del mismo tamaño) y luego asociarle el cómputo relacionado a las mismas para generar las tareas.

- Esta división llevará a un número determinado de tareas, donde cada una comprende de algunos datos y operaciones a realizar sobre los mismos.
- Una operación puede requerir datos de diferentes tareas, lo que implicará comunicación y sincronización.
- Diferentes particiones son posibles de acuerdo a la estructura de datos disponible.

### Descomposición funcional

Se enfoca en el cómputo a realizar más que en los datos → Divide al cómputo en tareas disjuntas y luego examina los datos.

- Los requerimientos de datos pueden ser disjuntos (caso ideal) o superponerse significativamente (peor caso → comunicación requerida para evitar replicación de datos).
- La descomposición de datos es la más antigua y a su vez la más usada. De todas formas la descomposición funcional tiene valor como una forma diferente de pensar los problemas.
  - Enfocarse en el cómputo a realizar facilita la estructuración del programa y el descubrimiento de oportunidades de optimización (situación no tan obvia cuando uno se enfoca en los datos).

### Descomposición de tareas

Si el problema lo permite, todas las tareas serán independientes → este es el caso ideal ya que todas podrían computarse a la vez.

- En general esto no es usual y existe algún tipo de dependencia entre las tareas.
- Un grafo de Dependencias de tareas (GDT) puede ser útil para expresar las dependencias entre tareas y su orden relativo.
  - Es un grafo acíclico dirigido en el que los nodos representan las tareas y las aristas indican las dependencias entre las mismas.
  - El grafo puede ser disconexo e inclusive no tener aristas.

### Granularidad de las tareas

El número y el tamaño de las tareas en las que un problema se divide determinan la granularidad de la descomposición.

- Grano fino: gran número de pequeñas tareas.
- Grano grueso: pequeño número de grandes tareas.

### Grado de concurrencia

El número de tareas que se ejecutan en paralelo indica el grado de concurrencia de la descomposición.

- Como este número puede variar en ejecución, un dato interesante a conocer es el máximo grado de concurrencia alcanzable por una determinada descomposición.
- El grado de concurrencia promedio representa el número promedio de tareas que pueden ejecutarse simultáneamente durante todo el programa.
- Una característica del grafo de dependencias de tareas que determina el grado de concurrencia promedio para una determinada granularidad es el camino crítico.
- Camino crítico es el camino dirigido más largo entre un nodo inicial y un nodo final.
- La suma de los pesos de los nodos que integran el camino crítico se conoce como longitud del camino crítico → si los pesos indican el tiempo requerido por una tarea, entonces representan el tiempo mínimo requerido para resolver el problema.

**Grado de concurrencia promedio = Peso total / Longitud del camino crítico.**

Un camino crítico más corto favorece a un mayor grado de concurrencia.

Se debe tener en cuenta que las tareas deben comunicarse y sincronizar → esto significa overhead que limita el speedup alcanzable.

Un adecuado balance entre cómputo y comunicación definirá el rendimiento alcanzable.

## Aglomeración de tareas

Este paso consiste en analizar si conviene combinar/aglomerar varias tareas para obtener un número menor de tareas pero de mayor tamaño. También se analiza si vale la pena replicar datos o cómputo.

- En general, el número final de tareas como resultado de la aglomeración debería ser igual al número de procesadores a emplear.
- 3 objetivos, a veces conflictivos entre ellos, que guían las decisiones de aglomeración y replicación:
  - Incremento de la granularidad: al combinar varias tareas relacionadas, se elimina la necesidad de comunicar datos entre ellas.
  - Preservación de la flexibilidad: al combinar varias tareas se puede limitar la escalabilidad del algoritmo. Si un algoritmo es capaz de crear un número variable de tareas, entonces posee un mayor grado de portabilidad y escalabilidad.
  - Reducción de costos de desarrollo: en ocasiones, el costo desde el punto de vista del proceso de ingeniería de software, puede ser muy elevado para la ganancia asociada.

## Técnicas de descomposición

¿Cómo descomponer un problema en tareas y subtareas?

- No existe una única forma.
- Algunas técnicas posibles:
  - Propósito general:
    - Descomposición recursiva
    - Descomposición basada en los datos
  - Propósito específico:
    - Descomposición exploratoria
    - Descomposición especulativa

### Recursiva

En general se ajusta muy bien a los problemas que se pueden resolver mediante la estrategia divide y vencerás.

- El problema inicial es dividido en un conjunto de subproblemas independientes. Luego, cada uno de estos subproblemas son recursivamente descompuestos en otros subproblemas independientes más pequeños hasta alcanzar una determinada granularidad.
- En ocasiones, puede requerirse alguna fase de combinación de resultados parciales.
- A veces puede resultar necesario reestructurar el cómputo de un algoritmo para que sea posible aplicar esta descomposición.
- Las versiones iterativas se pueden re-escribir en forma recursiva siguiendo la estrategia divide y vencerás.

### Basada en los datos

Generalmente usada en problemas que operan sobre grandes estructuras de datos.

- Requiere de dos pasos:
  - Particionar los datos que se procesan.
  - Usar la partición anterior para inducir una descomposición del cómputo de tareas.
- El particionamiento de los datos se puede realizar de diferentes maneras → Se debe analizar las diferentes variantes y elegir la que lleve a una descomposición natural y de buen rendimiento.

### Basada en los datos de salida

- Resulta natural cuando cada elemento de la salida de un programa (resultados) se puede calcular en forma independiente como función de los datos de entrada.
- Una partición de los datos de salida lleva inmediatamente a una descomposición en tareas, donde a cada tarea se le asocia el cómputo relacionado a la proporción asignada.
- Por ejemplo: multiplicación de matrices.
- En general una determinada descomposición de datos lleva a una descomposición del cómputo de tareas, pero puede haber más de una opción.

### Basada en los datos de entrada

- Particionar los datos de salida no siempre es posible, por ejemplo:
  - Cuando se computa el máximo, el mínimo o la suma de una lista de números → La salida es un único número.
  - Cuando se ordena un vector de números, los elementos individuales de la salida no se pueden determinar de antemano.
- En estos casos, resulta natural particionar los datos de entrada e inducir concurrencia a partir de ellos.
- A cada tarea se le asigna una porción de los datos de entrada y será responsable de realizar todos los cómputos asociados a la misma. En ocasiones, se puede requerir de algún paso posterior de reducción de salidas parciales.
- Ejemplos: contar ocurrencias en un vector; búsqueda de un elemento en un vector; descomposición estática para realizar una ordenación, etc.

### Exploratoria

Se suele emplear en aquellos problemas cuya solución involucra una búsqueda en un espacio de soluciones.

- Para realizar la descomposición, se particiona el espacio de búsqueda en porciones más pequeñas y realiza una búsqueda concurrente en cada una de ellas hasta encontrar la solución objetivo.
- Ejemplos clásicos son los problemas de optimización (buscar la mejor configuración para un determinado conjunto de parámetros) y juegos (ajedrez).
- Un método usual para resolver el problema en paralelo consiste en desarrollar algunos niveles desde la configuración inicial en forma secuencial. Luego, cada nodo es asignado a una tarea para realizar la búsqueda de forma concurrente. Cuando una la encuentra, le avisa al resto.

Diferencia con descomposición basada en los datos: las tareas son ejecutadas completamente, en exploratorio no.

### Especulativa

Se emplea cuando un programa podría tomar uno o varios caminos que aplican cómputo significativo pero la decisión depende de la salida de algún cómputo anterior.

- Pensar en un case con múltiples opciones que son evaluadas al mismo tiempo pero antes de tener el valor de la entrada. Cuando la entrada del case está disponible, se descartan las opciones incorrectas y se continúa la ejecución.

Diferencia con exploratoria: la salida de las múltiples tareas que salen de una bifurcación son desconocidas.

### Híbrida

Las técnicas de descomposición vistas no son exclusivas y se pueden combinar.

- En ocasiones, un programa se estructura en múltiples etapas y cada etapa puede ser descompuesta de forma diferente.

Un ejemplo es la búsqueda del mínimo en un vector.

## Etapa de mapeo de tareas a procesos

**Características de las tareas:**

- Las técnicas de descomposición analizadas permiten identificar la concurrencia disponible en un problema y descomponerlo en tareas que podrán ser ejecutadas en paralelo.
- El próximo paso en el diseño de algoritmos paralelos consiste en el mapeo (asignación) de las tareas a los procesos del programa.
- Para realizar un buen mapeo, hay que tener en cuenta las características de las tareas:
  - Modo de generación.
  - Tamaño y conocimiento del mismo.
  - Volumen de datos asociado.

### Modo de generación

Las tareas que constituyen un programa se pueden generar en forma estática o dinámica.

- En la generación estática, las tareas que se generan se conocen previo a la ejecución.
  - Ej: son la multiplicación de matrices o la búsqueda del mínimo de una lista de números.
- En la generación dinámica, las tareas se generan durante la ejecución, por lo que no se conoce de antemano cuál será el número final.
  - Ej: quicksort recursivo.

### Tamaño y conocimiento del mismo

Las tareas que constituyen un programa pueden ser uniformes o no uniformes.

- Cuando las tareas requieren aproximadamente el mismo tiempo de cómputo, se dice que son uniformes. Ejemplo: multiplicación de matrices.
- Cuando el tiempo requerido entre una tarea y otra puede variar significativamente se dice que son no uniformes. Ejemplo: quicksort recursivo.
- Conocer el tamaño de las tareas previo a la ejecución es otro factor que puede influir en el mapeo. Ejemplos:
  - En la multiplicación de matrices conocemos el tamaño de cada tarea previo a la ejecución.
  - En el puzzle-15 no es posible saberlo, no sabemos cuántos movimientos debemos realizar para llegar a la configuración final.

### Volumen de datos asociado

- El volumen de datos asociado a una tarea tiene que ver muchas veces con la granularidad elegida.
- A su vez, la granularidad impacta directamente en la relación cómputo-comunicación:
  - Usualmente con bajos niveles de comunicación se tiende a afinar la granularidad y a asignar un menor volumen de datos por proceso.
  - Cuando tenemos mucho intercambio de datos, se suele optar por aumentar la granularidad o emplear memoria compartida.

## Técnicas de mapeo de tareas a procesos

Una vez que el cómputo ha sido descompuesto en tareas, el siguiente paso consiste en mapearlas a los procesos del programa.

- El mapeo debe llevarse a cabo buscando que el tiempo requerido para completar las tareas sea el mínimo posible. Para ello se deben considerar 2 estrategias:
  - Asignar tareas independientes en diferentes procesadores para lograr un mayor grado de concurrencia.
  - Asignar tareas que se comunican frecuentemente en el mismo procesador para reducir overhead y mejorar localidad.
- Estas 2 estrategias entran en conflicto entre sí y la clave está en encontrar un balance adecuado.
- El problema de encontrar un mapeo óptimo es NP-completo → esto significa que no existe un algoritmo de complejidad polinomial que evalúe los diferentes compromisos entre las estrategias en el caso general y determine cuál es el mejor.
- Sin embargo, existen heurísticas para determinadas clases de problema que suelen dar buen resultado.

## Mapeo de tareas a procesos

Para analizar el mapeo, debemos tener en cuenta el grafo de dependencias.

- Las dependencias entre las tareas puede condicionar el balance de carga entre los procesos.
- La interacción entre tareas debe tender a minimizar la comunicación entre los procesos.
- Una carga balanceada no necesariamente significa mínimo tiempo de ejecución.

## Balance de carga

- Las técnicas de mapeo usadas en algoritmos paralelos se pueden clasificar en estáticas y dinámicas.
- **Técnicas Estáticas:**
  - Distribuyen las tareas entre los procesos previo a la ejecución.
  - Es fundamental conocer las características de las tareas.
  - Para casos complejos se emplean heurísticas.
  - En general los algoritmos son más fáciles de diseñar y programar.
- **Técnicas Dinámicas:**
  - Distribuyen las tareas entre los procesos durante la ejecución.
  - Si las tareas se generan dinámicamente, entonces deben mapearse dinámicamente también.
  - Si no se conoce de antemano el tamaño de las tareas, el mapeo dinámico suele dar mejor resultado.
  - Si el volumen de datos asociado a cada tarea es grande pero el cómputo no es significativo, un mapeo dinámico podría incurrir en un alto overhead por la migración de datos.

## Esquemas de mapeo estático

- El mapeo estático suele ser utilizado en problemas que emplean descomposición basada en los datos.
- Como las tareas están fuertemente relacionadas con los datos, mapear los datos a los procesos es de alguna forma equivalente a mapear las tareas a los procesos.

(Distribución de bloques 1D)

- El mapeo estático suele ser utilizado en problemas que emplean descomposición basada en los datos.
- Como las tareas están fuertemente relacionadas con los datos, mapear los datos a los procesos es de alguna forma equivalente a mapear las tareas a procesos.

(Distribución por bloques 2D)

- El mapeo estático suele ser utilizado en problemas que emplean descomposición basada en los datos.
- Como las tareas están fuertemente relacionadas con los datos, mapear los datos a los procesos es de alguna forma equivalente a mapear las tareas a los procesos.

(Distribución por bloques cíclicas)

- El mapeo estático suele ser utilizado en problemas que emplean descomposición basada en los datos.
- Como las tareas están fuertemente relacionadas con los datos, mapear los datos a los procesos es de alguna forma equivalente a mapear las tareas a los procesos.

(Distribución por bloques aleatorios)

## Esquemas de mapeo dinámico

- El mapeo dinámico es necesario cuando:
  - Emplear un mapeo estático puede llevar a una distribución desbalanceada de trabajo entre los procesos.
  - El grafo de dependencias de tareas es dinámico en sí mismo.
- Se suele referir a sus técnicas como balance de carga dinámico, ya que equilibrar la carga entre procesos es la principal razón de su uso.
- Los esquemas de mapeo dinámicos se clasifican en centralizados o distribuidos.

### Esquemas centralizados

En estos:

- Existe un proceso especial master que administra las tareas a realizar; al resto de procesos denominamos workers.
- Cuando un proceso worker no tiene trabajo, le pide al master que le asigne una tarea y así sucesivamente hasta que no queden tareas por completar.
- Suele ser más fácil implementar que los esquemas distribuidos pero sufren de escalabilidad limitada → el master se puede volver un cuello de botella cuando la cantidad de procesos es muy grande.

### Esquemas distribuidos

En estos:

- Se evita el cuello de botella potencial del master, delegando la distribución entre varios procesos pares.
- Difícil implementación.
- Los problemas son de sincronización.
  - Difícil determinar las responsabilidades y los momentos de asignación, comunicación, etc.

## Métodos para reducir overhead de las interacciones

Overhead de las interacciones: reducir el overhead asociado a las interacciones entre procesos es un factor clave para mejorar la eficiencia de los programas paralelos.

- Existen diferentes métodos:
  - Minimizar volúmenes de datos intercambiados → A mayor volumen de datos intercambiados, mayor tiempo de comunicación.
  - Minimizar frecuencia de las interacciones → Cada interacción tiene un costo inicial de preparación. Siempre que sea posible, conviene combinar varias comunicaciones en una sola.
  - Minimizar competencia entre recursos y zonas críticas (hotspots): Evitar posibles cuellos de botella mediante el uso de técnicas descentralizadas. Replicar datos si es necesario.

## Métodos para reducir overhead de las interacciones

Existen diferentes métodos:

- Solapar cómputo con comunicaciones: mediante el uso de operaciones no bloqueantes en pasaje de mensajes y técnicas de multi-hilado y pre-búsqueda en memoria compartida.
- Replicar datos o cómputo: si permite reducir las interacciones (mensajes o sincronización).
- Usar operaciones de comunicación colectiva.
- Solapar comunicaciones con otras comunicaciones: siempre y cuando el hardware de soporte lo permita, solapar diferentes comunicaciones puede reducir overhead.

## Modelos de algoritmos paralelos

Un modelo de algoritmo representa una estructura usual de código que combina técnicas de descomposición de problema y de mapeo de tareas junto a la aplicación de métodos para minimizar overhead.

Existen diferentes modelos. Los más comunes:

- Maestro-Esclavo.
- Pipeline.
- Single Program Multiple Data (SPMD).
- Divide y vencerás.

## Modelo Maestro-Esclavo

- El proceso Maestro es el responsable de generar trabajo y asignárselo a los workers.
- Dos opciones para distribución de trabajo:
  - Si el maestro puede estimar de antemano el tamaño de las tareas, un mapeo estático será una buena opción. (Ej: multiplicación de matrices).
  - El mapeo dinámico si es elegida → tareas pequeñas son asignadas a los workers en múltiples instancias. (Ej: ordenar vector).

Se debe tener en cuenta que el Master puede convertirse en un cuello de botella, si las tareas son muy pequeñas o los workers son muy rápidos → la granularidad de las tareas debe ser elegida de forma tal que el tiempo de procesar la tarea sea mucho mayor que su comunicación o sincronización asociada.

- Puede ser generalizada a múltiples niveles.
- Resulta adecuado tanto para memoria compartida como pasaje de mensajes.

## Modelo Pipeline

- El cómputo se descompone en una secuencia de datos.
- Los datos suelen ser particionados y pasados entre los procesos, donde cada uno realiza una tarea sobre ellos.
- Usualmente se organizan en forma de arreglo lineal o multidimensional.
- Organizaciones menos comunes incluyen árboles o grafos.
- Un pipeline puede ser visto como una cadena de productores y consumidores.
  - Cada proceso consume los datos que genera el anterior pipe.
  - Al mismo tiempo, produce los datos que serán consumidos por el siguiente proceso.
- El balance de carga depende de la granularidad de las tareas.
  - A mayor granularidad, más tiempo tardará el pipeline en llenarse (paralelismo ideal).
  - A menor granularidad, mayor interacción entre los procesos del pipeline.
- Usado generalmente en procesamiento de imágenes.

## Modelo Single Program Multiple Data (SPMD)

Cada proceso realiza el mismo cómputo sobre una porción de datos diferentes.

- Mediante sentencias condicionales es posible que los procesos tomen diferentes caminos.
- En general, la carga de trabajo es proporcional a la cantidad de datos asignados a un proceso.
  - Dificultades en problemas irregulares o donde la arquitectura de soporte es heterogénea.
- El cómputo puede involucrar diferentes fases, las cuales son usualmente intercaladas con comunicación/sincronización.
- Resulta adecuado tanto en memoria compartida como en pasaje de mensajes.
- En memoria compartida, el esfuerzo de programación suele ser menor.
- En Pasaje de Mensajes:
  - Cuando el espacio de direcciones está particionado, usualmente se tiene un mayor control sobre la ubicación de los datos → mayor localidad de datos.
  - El overhead de las comunicaciones puede ser aliviado mediante el uso de operaciones no bloqueantes, siempre y cuando las dependencias lo permitan.

## Modelo Divide y Vencerás

- **Dividir:** Fase en la que se particiona sucesivamente el problema en sub-problemas más pequeños hasta obtener una granularidad deseada.
- **Conquistar:** Fase en la que se resuelven subproblemas en forma independiente.
- En ocasiones se requiere una fase adicional de combinación de resultados parciales para llegar al resultado final.