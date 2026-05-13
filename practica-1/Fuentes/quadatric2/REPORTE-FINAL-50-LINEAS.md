# REPORTE FINAL: Por qué SIMD es menos eficiente en Double que Float

## Síntesis Técnica (50 líneas)

### El Problema Observado
En la máquina local, con compilación -O2, double y float tienen rendimiento similar (~5.8s).
En el cluster remoto, float es hasta 2x más rápido que double, especialmente con TIMES=200.

### Causa Raíz 1: Vectorización SIMD
La arquitectura AVX2/SSE disponible en ambas máquinas vectoriza automáticamente con -O3 -march=native.
Los registros SIMD de 256 bits pueden contener: 8 floats (4 bytes cada uno) o 4 doubles (8 bytes cada uno).
Float alcanza 4 elementos por ciclo, double solo 2 elementos por ciclo.
Esto proporciona el doble de throughput aritmético con float.

### Causa Raíz 2: Ancho de Banda de Memoria
El algoritmo requiere 3 arrays: fa[], fb[], fc[] con 10,000,000 elementos.
Arrays de float: 120 MB totales (40 MB × 3 arrays).
Arrays de double: 240 MB totales (80 MB × 3 arrays).
Double consume 2x ancho de banda de memoria comparado con float.

### Causa Raíz 3: Eficiencia de Cache
Con línea de cache de 64 bytes:
Float cabe 16 elementos por línea, double solo 8.
Float tiene mejor localidad espacial y tasas de cache hit más altas.
Double experimenta más cache misses bajo carga SIMD intensiva.

### Por Qué el Cluster es Peor
El cluster probablemente tiene: compilador más agresivo, CPU moderno con mejor soporte SIMD, o memoria bandwidth más limitada.
Con TIMES=200, la diferencia alcanza 2x (40.64s double vs 20.47s float).
En máquina local, el efecto SIMD es 8.8%, en cluster amplificado a 98% por limitaciones de BW.

### Mito sobre Registros
Se pensaba que "double no entra en registros", pero esto es incorrecto.
Registros AVX2 de 256 bits contienen tanto 4 doubles como 8 floats.
El problema real es throughput insuficiente: double solo procesa la mitad de operaciones por ciclo.

### Conclusión
SIMD es menos eficiente en double porque: (1) mitad de throughput vectorizado, 
(2) doble consumo de memoria, (3) peor eficiencia de cache.
El cluster amplifica estos factores cuando el ancho de banda de memoria es cuello de botella.
La solución es usar float explícitamente con powf() y sqrtf() en lugar de pow() y sqrt().
