# Análisis de Rendimiento en Sistemas Paralelos

## Métricas - Tiempo de ejecución

- Un algoritmo secuencial se suele evaluar por su tiempo de ejecución → En general, es posible encontrar alguna ley asintótica del tiempo de ejecución en función del tamaño de datos de entrada.
- El tiempo de ejecución de un programa paralelo no solo depende del tamaño de los datos de entrada sino también el número de procesadores y de los parámetros de comunicación de la arquitectura de soporte → Es incorrecto analizar el algoritmo paralelo en forma aislada.

El análisis se debe realizar a nivel de sistema paralelo.

El tiempo de ejecución secuencial Ts es el tiempo que transcurre desde el inicio hasta el fin de la ejecución sobre una máquina empleando una única unidad de procesamiento.

El tiempo de ejecución paralela Tp resume la diferencia de tiempo entre que la primera tarea que comienza hasta que la última tarea haya completado su trabajo.

## Fuentes de overhead

Usando el doble de recursos, se espera que un programa paralelo se ejecute en la mitad del tiempo, aunque en la práctica esto es raro que ocurra.

- Existen factores que generan overhead en los programas paralelos e impiden una mejora proporcional al aumento de la arquitectura:
  - Ocio
  - Interacción entre procesos
  - Cómputo adicional

## Métricas - Speedup

El Speedup S refleja el beneficio de emplear procesamiento paralelo para resolver un problema dado comparado a realizarlo en forma secuencial.

**Fórmula:** Sp(n) = Ts(n) / Tp(n)

Es una medida de cuántas veces más rápido pudimos resolver el problema empleando el algoritmo paralelo con p unidades de procesamiento comparado al algoritmo secuencial.

Para un problema dado pueden existir diferentes algoritmos secuenciales los cuales pueden tener diferente tiempo de ejecución y, a su vez, pueden ser paralelizados de distintas maneras.

Para computar el Speedup, siempre se debe considerar el mejor algoritmo secuencial.

## Métricas - Speedup

Límites del Speedup: si Sp(n) < 1 entonces el algoritmo paralelo tarda más que el mejor algoritmo secuencial. Sp(n) debe ser mayor a 1.

- El mejor resultado es si somos capaces de distribuir el trabajo entre las unidades de procesamiento sin introducir ocio, interacción ni cómputo adicional → situación poco usual.
  - Con p unidades de procesamiento → Sp(n) = p (conocido como Speedup lineal, Speedup óptimo, Speedup perfecto).
- Teóricamente, siempre se cumple que Sp(n) ≤ p.
  - Un speedup mayor a p solo es posible si cada unidad de procesamiento requiere menos de Ts(n) / p unidades de tiempo.
- Entonces podríamos construir un nuevo algoritmo secuencial que emule las p unidades de procesamiento usando una única unidad física, resolviendo el problema en menos de Ts unidades de tiempo → Contradicción.
- En la práctica a veces se puede dar Sp(n) > p (speedup superlineal).
  - Un motivo puede ser que la versión paralela del algoritmo realice menos trabajo que la versión secuencial.
  - Un segundo motivo de Speedup superlineal es la combinación de características de hardware y distribución de los datos del algoritmo paralelo que ponen en desventaja al algoritmo secuencial.

Esto va relacionado con que al paralelizar podemos incrementar la tasa de cache hits suponiendo que los procesadores son idénticos.

Lo visto hasta ahora asume que todas las unidades de procesamiento empleadas son idénticas.

En arquitecturas heterogéneas, el speedup se debe calcular considerando la potencia de cómputo total (pct) en lugar del número de unidades de procesamiento p.

```
pct = Σ pcr_i (para i = 0 hasta p-1)

pcr_i = pi / pm
```

Donde:

- pct = Potencia de cómputo total
- pcr = Potencia de cómputo relativa
- pi = Potencia del procesador i
- pm = Potencia del mejor procesador

## Métricas - Eficiencia

Solo un sistema paralelo ideal con p unidades de procesamiento puede reportar speedups iguales a p → En la práctica es difícil que ocurra debido a las fuentes de overhead.

La Eficiencia es una medida de la fracción de tiempo en la cual las unidades de procesamiento son empleadas en forma útil.

**Fórmula:** Ep(n) = Sp(n) / Sopt

- En arquitecturas homogéneas Sopt = p mientras que en heterogéneas Sopt = pct.
- Si Sp(n) = p, entonces Ep(n) = 1.
- En la práctica Sp(n) ≤ p lo que implica que Ep(n) ≤ 1.
- Por definición Ep(n) > 0, por lo tanto 0 < Ep(n) ≤ 1.

## Métricas - Overhead de las comunicaciones

El overhead de las comunicaciones de un sistema paralelo se define como la relación entre el tiempo requerido por las comunicaciones de nuestra solución y el tiempo total que esta requiere.

**Fórmula:** OCp(n) = Tcommp(n) / Tp(n) × 100

## Ley de Amdahl

- Los factores de overhead limitan los beneficios del procesamiento paralelo.
- Una restricción importante proviene de aquellas secciones de código que no pueden ser paralelizadas → bloque de ejecución secuencial.
- La ley de Amdahl permite estimar el Speedup alcanzable en aquellos programas paralelos que contienen partes secuenciales:
- Dada una fracción f, 0 ≤ f ≤ 1 de un programa paralelo que debe ser ejecutada secuencialmente, el tiempo de ejecución paralela se calcula como:

**Tp(n) = f × Ts(n) + ((1-f) × Ts(n) / p)**

Entonces el Speedup ahora puede re-escribirse de la siguiente forma:

```
Sp(n) = Ts(n) / (f × Ts(n) + (1-f) × Ts(n) / p)
      = 1 / (f + (1-f) / p)
```

Es importante notar que, aún con un número infinito de unidades de procesamiento, el Speedup estará limitado a 1/f.

Debemostenener en cuenta esta característica si vamos a emplear una gran cantidad de unidades de procesamiento, aunque veremos más adelante que esta estimación puede ser considerada poco realista.

## Escalabilidad

En el contexto del análisis del rendimiento, la escalabilidad hace referencia a la capacidad que tiene un sistema de mantener un nivel de Eficiencia fijo al incrementar tanto el número de unidades de procesamiento como el tamaño del problema a resolver → En ese caso, se dice que el sistema es escalable.

Entonces la escalabilidad de un sistema paralelo es una medida de su capacidad de incrementar el speedup en forma proporcional al número de unidades de procesamiento empleadas.

**Casos especiales:**

- **Escalabilidad fuerte:** Cuando al incrementar el número de unidades de procesamiento, no resulta necesario aumentar el tamaño de problema para mantener la eficiencia en un valor fijo.
- **Escalabilidad débil:** Cuando al incrementar el número de unidades de procesamiento, resulta necesario también aumentar el tamaño de problema para mantener la eficiencia en un valor fijo.

El incremento en el Speedup por un tamaño mayor de problema no es percibido por la ley de Amdahl.

## Ley de Gustafson

- El incremento en el Speedup por un tamaño mayor de problema no es percibido por la ley de Amdahl.

Gustafson vio que un multiprocesador más grande usualmente permite resolver un problema de mayor tamaño en un tiempo de ejecución determinado → El tamaño de problema seleccionado depende frecuentemente del número de unidades de procesamiento disponibles.

Al incrementar el tamaño del problema y el número de unidades de procesamiento para mantener el tiempo de ejecución constante, la fracción secuencial de los programas se mantiene fija o no crece en forma proporcional al tamaño de la entrada.

Asumir entonces que el tamaño de problema es fijo tan válido como que el tiempo de ejecución paralela lo es.

Se re-escribió entonces la ecuación para estimar el máximo speedup alcanzable (speedup escalado).