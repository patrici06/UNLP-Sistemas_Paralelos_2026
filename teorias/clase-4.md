# Analisis de Rendimiento en Sistemas Paralelos: 

### Metricas - Tiempo de ejecucion: 
- Un algoritmo secuencial se suele evaluar por su tiempo de ejecucion -> En general, es posible encontrar alguna ley asintotica del tiempo de ejecucion en funcion del tamano de datos de entrada. 
- El tiempo de ejecucion de un programa paralelo no solo depende del tamano de los datos de entrada sino tambien el numero de procesadores y de los parametros de comunicacion de la arquitectura de soporte -> es incorrecto analziar el algoritmo paralelo en forma aislada
En analisis se debe realizar a nivel de sistema paralelo. 

El tiempo de ejecucion secuencial Ts es el tiempo que transcurre desde el inicio hasta el fin de la ejecucion sobre una maquina empleando una unica unidad de procesamiento. 

El timepo de ejecucion paralela Tp resume la diferencia de tiempo entre que la primera tarea que comienza hasta que la ultima tarea haya completado su trabajo. 

### Fuentes de overhead: 
Usando el doble de recursos, se espera que un programa paralelo se ejecute en la mitad del tiempo, aunque en la practica esto es raro que ocurra. 
- Existen factores que generan overhead en los programas paralelos e impiden una mejora proporcional al aumento de la arquitectura: 
    + Ocio
    + Interaccion entre procesos
    + Computo Adicional
### Metricas - Speedup
El Speedup S refleja el beneficio de empelar procesamiento paralelo para resolver un problema dado comparado a realizarlo en forma secuencial. 
    formula: Sp(n) = Ts(n) / Tp(n)
Es una medida dde cuantas veces mas rapido pudimos resolver el problema empleando el algoritmo paralelo con p unidades de procesamiento comparado al algoritmo secuencial. 
Para un problema dado pueden existir diferentes algoritmos secuenciales los cuales pueden tener diferente tiempo de ejecucion y, a si vez, puedenm ser paralelizados de distintas maneras. 

Para computar el Speedup, siempre se debe considerar el mejor algoritmo secuencial. 

### Metricas - Speedup: 
Limites del Speedup, si Sp(n)<1 entonces el algoritmo paralelo tarda mas que el mejor algoritmo secuencial Sp(n) debe ser mayor a 1. 
 - El mejor resolutado es si somos capaces de distribuir el trabajo entre las unidades de procesamiento sin introducir ocio, interaccion ni computo adicional -> situacion poco usual. 
    + con p unidades de procesamiento -> Sp(n) = p (conocido como Speedup lineal, Speedup optimo, Speedup perfecto). 
- Teoricamente, siempre se cumple que Sp(n)<=p
    + Un speedup mayor a p solo es posible si cada unidad de procesamiento requiere menos de Ts(n) / p unidades de tiempo. 
Entonces podriamos construir un nuevo algoritmo secuencial que emule las p unidades de procesamiento usando una unica unidad fisica, resolveindo el problema en menos de Ts unidades de tiempo -> Contradiccion. 
En la practica a veces se puede dar Sp(n)>p (speedup superlineal)
    - Un motivo puede ser que la version paralela del algoritmo realice menos trabajho que la version secuencial. 
Un segundo motivo de Speedup superlineal es la combinacion de caracteristicas de hardware y distribucion de los datos del algoritmo paralelo que ponen en desventaja al algoritmo secuencial. 

Esto va relacionado que al paralelizar podemos incrementar la tasa de cache hits suponiendo que los procesadores son identicos. 

Lo visto hasta ahora asume que todas las unidades de procesamiento empleadas son identicas. 
En arquitecturas heterogeneas, el speedup se debe calcular considerando la potencia de computo total (pct) en lugar del numero de unidades de procesamiento p
            p-1
    + pct =  Σ pcr_i
            i=0 
    
    + pcr_i = pi / pm 

pct = Potencia de computo total
pcr = potencia de computo relativa
pi = potencia del procesador i 
pm = potencia del mejor procesador

### Metricas-Eficiencia
 Solo un sistema paralelo ideal con p unidades de procesamiento puede reportar speedups iguales a p -> En la practica es dificil que ocurra debido a las fuentes de overhead. 
La Eficiencia nes una medida de la fraccion de tiempo en la cual las unidades de procesamiento son empleadas en forma util
        Ep(n) = Sp(n) / Sopt
- En arquitecturas homogeneas Sopt = p mientras que en heterogeneas Sopt = pct 
- Si Sp(n) = p, Entonces Ep(n) = 1
- En la practica Sp(n)<=p lo que implica que Ep(n) <= 1
- Por definicion Ep(n) > 0, por lo tanto 0 <Ep(n) <=1

### Metricas - Overhead de las comunicaciones: 

El overhead de las comunicaciones de un sistema paralelo se define como la relacion entre el tiempo requerido por las comunicaciones de nuestra solucion y el tiempo total que esta requiera. 

    OCp(n) = Tcommp(n) / Tp(n) x 100

### Ley de Amdahl
- Los factores de overhead limitan los beneficios del procesamiento paralelo. 
- Una restriccion iomportante proveiene de aquellos secciones de codigo que no pueden ser paralelizadas => bloque de ejecucion secuencial.
- La ley de Amdahl permite estimar el Speedup alcanzable en aquellos programas paralelos que contienen partes secuenciales: 
- Dada una fraccion f, 0 <=f<=1 de un programa paralelo que debe ser ejecutada secuencialmente, el tiempo de ejecucion paralela se calcula como: 
    
    Tp(n) = f x Ts(n) + ((1-f)xTs(n)/p)
Entonces el Speedup ahora puede re-escribirse de la siguiente forma:
    S a p(n) = Ts(n)                 1
               -----            = --------
        fxTs(n)+(1-f)xTs(n)       f + (1-f)
                ----------            ------
                     p                   p
Es importante notar que, aunq con un numero infinito de unidades de procesamiento, el Speedup estara limitado a 1/f 
debemos tener en cuenta esta caracterticas si vamos a emplear una gran cantidad de unidades de procesamiento, aunque veremos mas adelante que esta estimacion puede ser considerada poco realista. 

### Escalabilidad: 
En el contexto del analisis del rendimiento, la escalabilidad hace referencia a la capacidad que tiene un sistema de mantener un nivel de Eficiencia fijo al incrementar tanto el numero de unidades de procesamiento como el tamano del problema a resolver -> En ese caso, se dice que el sistema es escalable.
 Entonces la escalabilida de un sistema paralelo es una medida de sucapacidad de incrementar el speedup en forma proporcional al numero de unidades de procesamiento empleadas. 

Casos especiales: 
- Escalabilidad fuerte: Cuando el incrementar el numero de unidades de procesamiento, no resulta necesario aumentar el tamano de problema para mantener la eficiencia en un valor fijo. 
- Escalabilidad debil: Cuando al incrementar el numero de unidades de procesamiento, resulta necesario tambien aumentar el tamano de problema para mantener la eficiencia en un valor fijo. 

El incremento en el Speedup por un tamano mayor de problema no es percibido por la ley de Amdahl

### Ley de Gustafson:
- El incremento en el Speedup por un tamano mayor de problema no es percibido por la ley de Amdahl

Gustafson vio que un multiprocesador mas grande usualmente permite resolver un problema de mayor tamano en un tiempo de ejecucion determinado -> el tamaño de problema seleccionado depende frecuentemente del numero de unidades de procesamiento disponibles. 

Al incrementar el tamaño del problema y el numero de unidades de procesamiento para mantener el timepo de ejecucion constante, la fraccion secuencial de los programas se mantiene fija o no crece en forma proporcional al tamaño de la entrada. 

Asumir entonces que el tamaño de problema es fijo tan valido como que el tiempo de ejecucion paralela lo es. 
Se re-escribio entonces la ecuacion para estimar el maximo speedup alcanzable (speedup escalado). 

continar resume mas adelante
