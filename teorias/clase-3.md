# Clase 3: 

###Temas de la clase: Diseno y modelos de algoritmos paralelos: 

### Diseno de algoritmos paralelos:
    - Un algoritmo secuencial es ideal para resolver un problema en una unica unidad de procesamiento. 
    - Un algoritmo paralelo es un esquema para resolver problemas empleado multiples unidades de procesamiento.     
    El desarrollar un algoritmo paralelo es mas que determinar los pasos a resolver el problema -> el programador debe al menos considerar la concurrencia y especificar que pasos pueden llevarse a cabo simultaneamente

En la practica, el diseno de algoritmos paralelos puede incluir algunas de las siguientes actividades: 
    - Identificar porciones de trabajo (tareas) que puedan resolverse en paralelo. 
    - Asignar tareas a procesos que ejecutan en diferentes procesadores. 
    - Distribuir datos de entrada, de salida e intermedios asociados con el programa. 
    - Administrar accesos a datos compartidos. 
    - Sincronizar procesos en diferentes etapas del programa.
Puede existir diferentes opciones para cada paso aunque usualmente pocas combinaciones son las que producen buen rendimiento. 

### Pasos Fundamentales: Descomposicion en tareas y Mapeo de tareas a procesos. 

### Descomposicion de tareas: 
    Es el proceso de dividir el computo en partes mas pequenas(tareas)a, de las cuales algunas o todas podran ser potencialmente ejecutadas en paralelo. 
    - Se trata de definir un gran numero de tareas pequenas para obtener una descomposicion de grano fino -> brindar mayor flexibilidad a los potenciales algoritmos paralelos. 
    - Probablemente, en etapas posteriores, la evaluacion de los requerimientos de comunicacion, la plataforma destino, o cuestiones de ingenieria de software, pueden llevar a descartar algunas opciones de descomposicion consideradas inicialmente. 
        + En esos casos, la particion original es revisada y sus tareas son aglomeradas para incrementar su tamano o granularidad. 
    - La descomposicion se puede realizar de diferentes modos. Una priomera aproximacion consiste en pensar tareas de igual codigo (paralelismo de datos o dominio) o tareas de codigo diferente(paralelismo funcional). 
### Descomposicion de datos: 
    Consiste en descomponer los datos asociados a un problema de pequenas porciones (usualmente del mismo tamano) y luego asociarle el computo relacionado a las mismas para generar las tareas. 
    - Esta division llevara a un numero determinado de tareas, donde cada una comprende de algunos datos y operaciones a realizar sobre los mismos. 
    - Una operacion puede requerir datos de diferetenes tareas, lo que implicara comunicacion y sincronzacion. 
    - Diferentes particiones son posibles de acuerdo a la estructura de datos disponible. 
### Descomposicion Funcional: 
    Se enfoca en el computo a realizar mas que en los datos -> Divide al computo en tareas disjuntas y luego examina los datos. 
    - Los requerimientos de datos pueden ser disjuntos (caso ideal) o superponerse significativamente (peor caso -> comunicacion requerida para evitar replicacion de datos). 
    - La descomposicion de datos es la mas antigua y a su vez la mas usada. De todas formas la descomposicion funcional tiene valor como una forma diferente de pensar los problemas. 
        + Enfocarse en el computo a realizar facilita la estructuracion del programa y el descubrimiento de oportunidades de optimizacion (situacion no tan obvia cuando uno se enfoca en los datos). 
### Descomposicion de tareas: 
    Si el problema lo permite, todas las tareas seran independientes-> este es el caso ideal ya que todas podrian computarse a la vez. 
    - En general esto no es usual y exite algun tipo de dependencia entre las tareas. 
    - Un grafo de Dependencias de tareas (GDT) puede ser util para expresar las dependencias entre tareas y su orden relativo. 
        + Es un grafo aciclico dirigido en el que los nodos representan las tareas y las aritas indican las dependencias entre las mismas. 
        + El grafo puede ser disconexo e inclusive no tener aristas
### Granularidad de las tareas: 
    El numeor y el tamano de las tareas en las que un problema se divide determinan la granularidad de la descomposicion. 
    - Grano fino: gran numero de pequenas tareas 
    - Grano grueso: pequeno numero de grandes tareas
### Grano de concurrencia: 
    El numero de tareas que se ejecutan en paralelo indica el grado de concurrencia de la descomposicion.
    - Como este numeor puede variar en ejecucion, un dato interesante a conocer es el maximo grado de concurrencia alcanzable por una determinada descomposicion.   
    - El grado de concurrencia promedio representa el numero promedio de tareas que pueden ejecutarse simultaneamente durante todo el programa. 
    - Una caracterisitca del grafo de dependencias de tareas que determina el grado de concurrencia promedio para una determinada granularidad es el camino critico. 
    - Camino critico es el camini dirigido mas largo entre un nodo inicial y un nodo final
    - La suima de ls pesos de los nodos que integran el camino critico se conoce como longitud del camino critico -> si los pesos indican el tiempo requerido por una tarea, entonces representan el tiempo minimo requerido para resolver el problema. 
    Grado de concurrencia promedio = Peso total / Longitud del camino critico. 
Un camino critico mas corto favorece a un mayor grado de concurrencia. 

Se debe tener en cuenta que las tareas deben comunicarse y sincronizar -> esto significa overhead que limita el speedup alcanzable.

Un adecuado balance entre computo y comunicacion definira el rendimiento alcanzable. 
    
### Aglomeracion de tareas: 
    Este paso consiste en analziar si conviene combinar/aglomerar varias tareas para obtener un numero de tareas pero de mayor tamano. Tambien se analiza si vale la pena replicar datos o computo. 
    - En general, el numeor final ade tareas como resultado de la aglomeracion deberia ser igual al numero de procesadores a emplear. 
    - 3 objetivos, a veces conflictivos entre ellos, que guian las decisiones de aglomeracion y replicacion: 
        + Incremento de la granularidad: al combinar varias tareas relacionadas, se elimina la necesidad de comunicar datos entre ellas. 
        + Preservacion de la flexibilidad: al combinar varias tareas se puede limitar la escalabilidad del algoritmo. Si un algoritmo es capaz de crear un numero variable de tareas, entonces posee un mayor grado de portabilidad y escalabilidad. 
        + Reduccion de costos de desarrollo: en ocasiones, el costo desde el punto de vista del proceso de ingenieria de software, puede ser muy elevado para la ganacia asociada. 

### Tecnicas de descomposicion: 
    Como descomponer un problema en tareas y subtareas? 
    - No existe una unica forma
    - Algunas tecniucas posibles: 
        + Proposito general: 
            - Descomposicion recursiva
            - Descomposiscion basada en los datos
        + Proposito especifico: 
            - Descomposicion exploratoria
            - Descomposicion especulativa
### Recursiva: 
    En general se ajusta muyb bien a los problemas que se pueden resolver mediante la estrategia divide y venceras. 
    - En problema inicial es es dividido en un conjunto de subproblemas independientes. Luego, cada uno de estos subproblemas son recursivamente descompuestos en otros subproblemas independientes mas pequenos hasta alcanzar una determinada granularidad. 

    - En ocasiones, puede requerirse alguna fase de combinacion de resultados parciales. 
    A veces puede resultar necesario reestructurar el computo de un algoritmo para que sea posible aplicar esta descomposicion. 
    Las versiones iterativas se pueden re-escribir en forma recursiva siguiendo la estrategia divide y venceras. 

### Basada en los Datos: 
    Generalmente usada en problemas que operan sobre grandes estructuras de datos. 
    - Requiere de dos pasos: 
        + Particionar los datos que se procesan 
        + Usar la particion anterior para inducir una descomposicion del computo de tareas. 
    - El particionamiento de los datos se puede realizar de diferentes maneras -> Se debe analizar las diferentes variantes y elegir la que lleve a una descomposicion natural y de buen rendimiento. 
 
### Basada en los datos de salida   
    - Resulta natural cuando cada elemento de la salida de un programa (resultados) se puede calcular en forma independiente como funcion de los datos de entrada. 
    - Una particion de los datos de salida lleva inmediatamente a una descomposicion en tareas, donde a cada tarea se le asocia el computo relacionado a la proporcion asignada. 
    - por ejemplo multiplicacion de matrices. 
    - En general una determinada descomposicion de datos lleva a un dada descomposicion del computo de tareas, pero puede haber mas de una opcion. 

### Basada en los datos de entrada: 
    - Particionar los datos de salida no siempre es posible, por ejemplo: 
        + cuando se computa al maximo, el minimo o la suma de una lista de numeros -> La salida es un unico numero. 
        + Cuando se ordena un vector de numeros, los elementos individuales de la salida no se pueden determinar de antemano. 
    - En estos casos,. resulta natural particionar los datos de entrada e inducir concurrenia a partid de ellos. 
    - A cada tarea se le asigna un aporcion de los datos de entradas y sera responsable de realizar todos los computos asociados a la misma. En ocasiones, se puede requerir de algun paso posterior de reduccion de salidas parciales. 
    - Ejemplos: ocntar ocurrencias en un vector; busqueda de un elemento en un vector; descomposicion estatica para realizar una ordenacion, etc.

### Exploratoria:
    Se suele emplear en aquellos problemas cuyas solucion involucra una busqueda en un espacio de soluciones.
    - Para realizar la descomposicion, se particiona el espacio de busqueda en porciones mas pequenas y realiza una busqueda concurrente en cada una de ellas hasta encontrar la solucioon objetivo.
    -Ejemplos clasicos son los problemas de optimizacion (buscar la mejor configuracion para un determinado conjunto de parametros) y juegos (ajedrez). 
    - Un metodo usual para resolver el problema en paralelo consiste en desarrollar algunos niveles desde la configuracion inicial en forma secuencial. Luego, cada nodo es asignado a un atarea para realizar la bisqueda de forma concurrente. Cuando una la encuentra, le avisa al resto. 
+ Diferencia con descomposicion basada en los datos: las tareas son ejecutadas completamente, en exploratorio no.

### Especulativa: 
    Se emplea cuando un programa podria tomar uno o varios caminos quie aplican computo significativo pero la decision depende de la salida de algun computo anterior. 
    - Pensar en un case con multiples opciones que son evaluadas al mismo tiempo pero antes de tener el valor de la entrada. Cuando la entrada del case esta disponible, se descartan las opciones incorrectas y se continua la ejecucion. 
+ Diferencia con exploratoria, la salida del las multiples tareas que salen de una bifurcacion son desconocidas. 

### Hibrida:
Las tecnicas de descomposicion vistas no son exclusivas y se pueden combinar. 
    - En ocasiones, un programa se estructura en multiples etapas y cada etapa puede ser descompuesta de forma diferente.
    Un ejemplo es la busqueda del minimo en un vector. 

### Etapa de mapeo de tareas a procesos: 
Caracteristicas de las tareas: 
    - Las tecnicas de descomposicion analizadas permiten identificar la concurrecia disponible en un problema y descomponerlo en tareas que podran ser ejecutadas en paralelo. 
    - El proximo paso en el diseno de algoritmo paralelos consiste en el mapeo (asignacion) de las tareas a los procesos del programa. 
    - Para realizar un buen mapeo, hay que tener en cuenta las caracteristicas de las tareas: 
        + Modo de generacion 
        + tamano y conocimiento del mismo
        + Volumen de datos asociado

### Modo de generacion:  
Las tareas que constituyen un programa se pueden generar en forma estatica o dinamica. 
    - En la generacion estatica, las tareas que se generan se conocen previo a la ejecucion
        + Ej: son la multiplicacion de nmatrices o la busqueda del minimo de una lista de numeros. 
    - En la generacion dinamica, las tareas se generan durante la ejecucion, por lo que no se conoce de antemano cual sera el numero final. 
        + Ej: quicksort recursivo
### tamano y conocimiento del mismo: 
     Las tareas que constituyen un programa pueden ser uniformes o no uniformes. 
        + Cuando las tareas requieren aproximadamente el mismo tiempo de computo, se dice que son uniformes. Ejemplo: multiplicacion de matrices.
        + Cuando el tiempo requerido entre una tarea y otra puede variar significativamente se dice que son no uniformes. Ejemplo: quicksort recursivo. 
    - Conocer el tamano de las tareas previo a la ejecucion es otro factor que puede influir en el mapeo. Ejemplos: 
        + En la multiplicacion de matrices conocemos el tamanoa de cada tarea previo a la ejecucion 
        + En le puzzle-15 no es posible saberlo, no sabemos cuantos movimientos debemos realizar para llegar a la configuracion final. 

### Volumen de datos asociado: 
    - El volumen de datos asociado a una tarea tiene que ver muchas veces con la granularidad elegida. 
    - A su vez, la granularidad impacta directamente en la relacion computo-comunicacion: 
        + Usualmente con bajos niveles de comunicacion se tiende a afinar la granularidad y a asignar un menor voluimen de datos por proceso. 
        + Cuando tenemos mucho intercambio de datos, se suele optar por aumentar la granularidad o emplear memoria compartida. 
    
### Tecnicas de mapeo de tareas a procesos: 
Una ves que el computo ha sido descompuesto en tareas, el siguiente paso consiste en mapearlas a los procesos del programa. 
    - El mapeo debe llevarse a cabo buscando que el tiempo requerido para completar las tareas sea el minimo posible. Para ello se deben considerar 2 estrategias: 
        + Asignar tareas independientes en diferentes procesadores para lograr un mayor grano de concurrencia. 
        + Asignar tareas que se comunican frecuentemente en el mismo procesador reducir overhead y mejorar localidad. 
    - Estas 2 estrategias entran en conflicto entre si y la clave esta en encontrar un balance adecuado. 
    - El proble a de encontrar un mapeo optimo es NP-completo -> esto significa que no existe un algoritmo de compolejidad polimional que evalue los diferentes compromisos entre las estrategias en el caso general y determine cual es el mejor. 
    - Sin embargo, existen heuristicas para deteminadas clases de problema que suele dar buen resultado. 
    
### Mapeo de tareas a procesos: 
    Para analizar el mapeo, debemos tener en cuenta el grafo de dependencias
    - Las dependencias entre las tareas puede condicionar el balance de carga entre los procesos
    - La interaccion entre tareas debe tender a minimizar la comunicacion entre los procesos.
    - Una carga balanceada no necesariamente significa minimo tiempo de ejecucion. 
    
### Balance de carga: 
    - Las tecnicas de mapeo usadas en algoritmos paralelos se pueden clasificar en estaticas y dinamicas. 
    ### Tecnicas Estaticas: 
        + Distribuyen las tareas entre los procesos previo a la ejecucion. 
        + Es fundamental conocer las caracteristicas de las tareas
        + Para casos complejos se emplean heuristicas
        + En general los algoritmos son mas faciles de disenar y programar.
    ### Tecnicas Dinamicas: 
        + Distribuyen las tareas entre los procesos durante la ejecucion. 
        + Si las tareas se generan dinamicamente, entonmces deben mapearse dinamicamente tambien. 
        + Si no se conoce de antemano el tamano de las tareas, el mapeo dinamico suele dar mejor resultado. 
        + Si el volumen de datos asociado a cada tarea es grande pero el computo no es significativo,un mapeo dinamico podria incurrir en un alto overhead por la migracion de datos. 
        
### Esquema de mapeo estatico: 
    - El mapeo estatico suele ser utilizado en problemas que emplean descomposicion basada en los datos. 
    - Como las tareas estan fuertemente relacionadas con los datos, mapear los datos a los procesos es de alguna forma equivalente a mapear las tareas a los procesos. 
    (se pueden distribuir por filas o columnas Distribucion de bloques 1D)
    - El mapeo estatico suele ser utilizado en problemas que emplean descomposicion basada en los datos. 
    - Como las tareas estan fuertemente relacionadas con los datos, mapear los datos a los proecesos es de alguna forma equivalente a mapear las tareas a procesos. 
        (Distribucion por bloques 2D)
    - El mapeo estatico suele ser utilizado en problemas que emplean descomposicion basada en los datos.
    - Como las tareas estan fuertemente relacionadas con los datos, mapear los datos a los processo es de alguna forma equivalente a mapear las tareas a los procesos. 
        (Distribucion por bloques ciclicas)
    - El mapeo estatico suele ser utilizado en problemas que emplean descomposicion basada en los datos. 
    - Como las tareas estan fuertemente relacionadas con los datos,mapear losd atos a los procesos es de alguna forma equivalente a mapear las tareas a los procesos.
        (Distribucion por bloques aleatorios)
### Esquemas de mapeo Dinamico: 
    - El mapeo dinamico es necesario cuando: 
        + Emplear un mapeo estatico puede llevar a una distribucion desbalanceada de trabajo entre los procesos. 
        + El grafo de dependencias  de tareas es dinamico en si mismo
    - Se suele referir a sus tecnicas como balance de carga dinamico, ya que equilibrar la carga entre procesos es la principal razon de su uso. 
    - Los esquemas de mapeo dinamicos se clasifican en centralizados o distribuidos. 
    ### Esquemas centralizados: 
        En estos: 
            - Existe proceso especial master que administra las tareas a realizar; al resto de procesos denominamos workers
            - Cuando un proceso worker no tiene trabajo, le pide al master que le asigne una tarea y asi sucesivamente hasta que no queden tareas por completar. 
            - Suele ser mas facil implementar que los esquemas distribuidos pero sufren de esccalabilidad limitada -> el master se puede volver un cuello de botella cuando la cantidad de procesos es muy grande. 
    ### Esquenmas distribuidos: 
        En estos: 
            - Se evita el cuello de botella potencial del master, delegando la distribucion entre varios procesos pares. 
            - Dificil implementacion 
            - Los problemas son de sincronizacion
                + Dificil determinar las responsabilidades y los momentos de asignacion, comunicacion, etc. 

### Metodos para reducir overhead de las interacciones: 
    Overhead de las interacciones, reducir el overhead asociado a las interacciones entre procesos es un factor clave para mejorar la eficiencia de los programas paralelos. 
    - Existen diferentes metodos: 
        + Minimizar volumenes de datos intercambiados -> A mayor volumen de datos intercambiados, mayor tiempo de comunicacion 
        + Minimiozar frecuencia de las interacciones -> Cada interaccion tiene un costo inicial de preparacion. Siempre que sea posible, conviene combinar varias comunicaciones en una sola. 
        + Minimizar competencia entre recursos y zonas criticas (hotspots): Evitar posibles cuellos de botella mediante el uso de tecnica descentralizadas. Replicar datos si es necesario. 

### Metodos para reducir overhead de las interacciones:     
Existen diferentes metodos: 
    - Solapar computo con comunicaciones: mediante el uso de operaciones no bloqueantes en pasaje de mensajes y tecnicas de multi-hilado y prebusqueda en memoria compartida. 
    - Replicar datos o computo: si permite reducir la interacciones (mesajes o sincronzicion)
    - Usar operacuiones de comunicacion colectiva
    - Solapar comunicaciones con otras comunicaciones: siempre y cuando el hardware de soporte lo permitia, solapar diferentes comunicaciones puede reducir overhead. 

### Modelos de algoritmos paralelos: 
    Un modelo de algoritmo representa una estructura usual de codigo que combina tecniacas de descomposicion de problema y de mapeo de tareas junto a la aplicacion de metodos para minimizar overhead.
Existen diferentes modelos. Los mas comunes: 
    - Maestro-Esclavo.
    - Pipeline
    - Single Program Multiple Data
    - Divide y venceras. 
    
### Modelo Maestro-Esclavo: 
- El proceso Maestro es el responsable de generar trabajo y asignarselo a los workers. 
- Dos opciones para distribucion de trabajo: 
    + Si el maestro puede estimar de antemano el tamano de las tareas, un mapeo estatico sera una buena opcion. (ej multiplicacion de matrices). 
    + El mapeo dinamico si es elegida -> tareas pequenas son asignadas a los workers en multiples instancias. (ej ordenar vector). 

Se debe tener en cuenta que el Master puede convertirse en un cuello de botella, si las tareas son muy pequenas o los workers son muy rapidos -> la granularidad de las tareas debe ser elegida de forma tal que el tiempo de procesar la tarea sea mucho mayor que su comunicacion o sincronizacion asociada. 

- Puede ser generalziada a multiples niveles. 
- Resulta adecuado tanto para memoria compartida como pasaje de mensajes. 

### Modelo Pipeline: 
    - El computo se descompose en una secuencia de procesos. 
    - Los datos suelen ser particionados y pasados entre los procesos, donde cada uno realiza una tarea sobre ellos. 
    - Usualmente se organizan en forma de arreglo lineal o multidimensional. 
    - Organizaciones menos comunes incluyentes arboles o grafos. 
    - Un pipeline puede ser visto como una cadena de productores y consumidores. 
        + cada proceso consume los datos que genera el anterior pipe
        + Al mismo timepo, produce los datos que seran consumidos por el siguiente proceso. 
    - El balane  de carga depende de la granularidad de las tareas
        + A mayor granularidad, mas tiempo tardara el pipeline en llenarse (paralelismo ideal)
        + A menor gratitud, mayor interaccion entre los procesos del pipeline
    - Usado generalmente en procesamiento de imagenes

### Modelo Single Program Multiple Data (SPMD): 
    Cada proceso realiza el mismo computo sobre una porcion de datos diferentes
        + Mediante sentencias condicionales es posible que los procesos tomen diferentes caminos. 
    - En general, la carga de trabajo es proporcional a la cantidad de datos asignados a un proceso. 
        + Dificultades en problemas irregulares o donbde la arquitectura de soporte heterogenea
    - El computo puede involucrar diferentes fases, las cuales son usualmente intercaladas con comunicacion/sincronizacion. 
    - Resulta adecuado tanto en memoria compartida como en pasaje de mensajes. 
    - En memoria compartida, el esfuerzo de programacion suele ser menor. 
    - En pasaje de Mensajes: 
        + Cuando el espacion de direcciones esta particionado, usualmente se tiene un mayor control sobre la ubicacion de los datos -> mayor localidad de datos. 
        + El overhead de las comunicaciones puede ser aliviado mediante el uso de operaciones no bloqueantes, siempre y cuando las dependencias lo permitan.

### Modelo Divide y Venceras: 
    - Dividir: Fase den la que se particiona sucesivamente el problema en sub-problemas mas pequenos hasta obtener una granularidad deseada.
    - Conquistar: fase en la que se resuelven subproblemas en forma independiente. 
    - En ocaciones se requiere una fase adicional de combinacion de resultados parciales para llegar al resultado final. 

