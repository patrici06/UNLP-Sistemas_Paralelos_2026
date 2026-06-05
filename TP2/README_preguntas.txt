================================================================================
DOCUMENTO DE PREGUNTAS: Análisis de Paralelismo en Multiplicación de Matrices
================================================================================

ARCHIVO GENERADO: preguntas.md (60 KB, 2015 líneas)

CONTENIDO:
==========

El documento contiene 18 preguntas de evaluación universitaria organizadas en
7 secciones temáticas:

SECCIÓN 1: MODELOS DE PARALELISMO Y ARQUITECTURAS (3 preguntas)
  - Pregunta 1.1: Modelo Fork-Join vs Work-Sharing
  - Pregunta 1.2: Asignación de Trabajo a Hilos
  - Pregunta 1.3: Niveles de Paralelismo en OpenMP (collapse)

SECCIÓN 2: PRIMITIVAS DE SINCRONIZACIÓN (3 preguntas)
  - Pregunta 2.1: Mutex vs Barrera en Pthreads
  - Pregunta 2.2: Reduction vs Manual Aggregation
  - Pregunta 2.3: Schedule Strategies en OpenMP (static vs dynamic vs guided)

SECCIÓN 3: OPTIMIZACIONES Y PERFORMANCE (3 preguntas)
  - Pregunta 3.1: Block Tiling y Cache Locality
  - Pregunta 3.2: Métricas de Performance (Speedup, Eficiencia, Overhead)
  - Pregunta 3.3: Comparación Teórica de Performance (OpenMP vs Pthreads)

SECCIÓN 4: ANÁLISIS CRÍTICO DE DISEÑO (3 preguntas)
  - Pregunta 4.1: Descomposición de Etapas en Pthreads (4 barreras)
  - Pregunta 4.2: Seguridad de Concurrencia (Race conditions)
  - Pregunta 4.3: Diferencias Sutiles en Acumulación (IEEE 754, atomicidad)

SECCIÓN 5: SCALABILITY Y LIMITACIONES (2 preguntas)
  - Pregunta 5.1: Ley de Amdahl vs Ley de Gustafson (Strong vs Weak scaling)
  - Pregunta 5.2: Limitaciones de Escalabilidad Inherentes (NUMA, false sharing, bandwidth)

SECCIÓN 6: PREGUNTAS DE IMPLEMENTACIÓN DETALLADA (2 preguntas)
  - Pregunta 6.1: Implicit vs Explicit Synchronization (comparativa)
  - Pregunta 6.2: Variable Storage Orders (row-major vs column-major)

SECCIÓN 7: PREGUNTAS TEÓRICAS AVANZADAS (2 preguntas)
  - Pregunta 7.1: Afinidad de Procesadores (Processor Affinity, NUMA-aware)
  - Pregunta 7.2: False Sharing en Detail (caché line contention)

CARACTERÍSTICAS DE CADA PREGUNTA:
==================================

Cada pregunta incluye:
  1. Enunciado claro y específico
  2. Respuesta fundamentada en teoría paralela
  3. Ejemplos de código extraído de matrices.c, matrices-pthread.c, matrices-open-mp.c
  4. Análisis técnico profundo:
     - Comparativas en tablas
     - Gráficos ASCII de conceptos
     - Cálculos cuantitativos
     - Ejemplos numéricos reales
  5. Citas de referencias teóricas
  6. Conexión con material de clase (Clase 1-6)

TEMAS CUBIERTOS:
==================

Teoría Paralela:
  - Modelos de ejecución (fork-join, work-sharing)
  - Sincronización (mutex, barriers, reduction)
  - Descomposición de datos vs tareas
  - Granularidad de paralelismo

Performance y Optimización:
  - Amdahl's Law y Gustafson's Law
  - Métricas: Speedup, Eficiencia, Overhead
  - Block tiling y cache locality
  - Memory bandwidth limitaciones
  - Cache coherence (MESI/MOESI)
  - False sharing

Arquitecturas:
  - Memoria jerárquica (L1/L2/L3)
  - NUMA (Non-Uniform Memory Access)
  - Processor affinity
  - Cache line contention

Implementación Práctica:
  - Pthreads: pthread_create, pthread_join, pthread_barrier, pthread_mutex
  - OpenMP: pragma omp, reduction, collapse, schedule
  - Storage orders: row-major vs column-major
  - IEEE 754 floating-point (asociatividad)

NIVEL ACADÉMICO:
=================

Dificultad: Avanzado
Público objetivo: Estudiantes de grado/posgrado en:
  - Computación Paralela
  - Sistemas de Computación de Alto Rendimiento
  - Arquitectura de Computadores

Requisitos previos:
  - Conocimiento de programación en C
  - Nociones básicas de threads
  - Conceptos de sistemas operativos
  - Arquitectura de computadores (básico)

REFERENCIAS CITADAS:
====================

1. Amdahl (1967) - Amdahl's Law
2. Gustafson (1988) - Weak Scaling
3. OpenMP Specification (2021)
4. IEEE 754-2019 - Floating-point Standard
5. Butenhof (1997) - POSIX Threads Programming
6. Kirk & Hwu (2016) - Parallel Programming
7. Pacheco (2011) - Parallel Programming Introduction
8. AMD Epyc Documentation - NUMA Architecture
9. Material de clase local (Clase 1-6)

CÓMO USAR ESTE DOCUMENTO:
==========================

Para Profesores:
  - Usar preguntas como evaluación o estudio
  - Seleccionar por dificultad/tema según curricula
  - Adaptar enunciados según necesidad
  - Las respuestas son extensas pero pueden resumirse

Para Estudiantes:
  - Estudiar respuestas fundamentadas
  - Revisar ejemplos de código con anotaciones
  - Comprender conexión entre teoría y práctica
  - Preparación para exámenes/entrevistas técnicas

Para Investigadores:
  - Análisis detallado de trade-offs en paralelización
  - Benchmarking comparativo
  - Consideraciones de arquitectura (NUMA, cache)

ESTADÍSTICAS DEL DOCUMENTO:
============================

Total de líneas: 2015
Tamaño: 60 KB
Preguntas: 18
Respuestas: 18 (1 por pregunta, extensas)
Ejemplos de código: 80+
Tablas comparativas: 15+
Gráficos ASCII: 10+
Cálculos cuantitativos: 30+
Referencias: 9

GENERACIÓN:
===========

Generado por: OpenCode
Fecha: 2026-05-28
Basado en: Análisis de código fuente de tres implementaciones
  - matrices.c (secuencial)
  - matrices-pthread.c (Pthreads)
  - matrices-open-mp.c (OpenMP)

Material teórico consultado:
  - Clase 1-6 (teorías de sistemas paralelos)
  - Documentación OpenMP oficial
  - Especificación POSIX Threads
  - Manuales de arquitectura AMD Epyc

MEJORAS FUTURAS:
=================

Posibles expansiones:
  - Más preguntas sobre MPI (Message Passing)
  - Análisis de GPU/CUDA
  - Preguntas sobre análisis experimental
  - Guías de resolución paso-a-paso
  - Quiz interactivo
  - Código de referencia con anotaciones

================================================================================
