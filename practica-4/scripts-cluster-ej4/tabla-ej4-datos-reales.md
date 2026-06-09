# Ejercicio 4 - MPI Matrix Multiplication (Datos Reales del Cluster)

## Tabla resumen completa

| N    | P | T_Seq (s)  | T_Par (s)  | T_Comm (s) | Speedup  | Eficiencia |
|------|---|------------|------------|------------|----------|------------|
| 512  | 1 | 1.601323   | 1.601323   | 0.000004   | N/A      | N/A        |
| 512  | 4 | 1.601323   | 0.409775   | 0.009381   | 3.907810 | 0.976953   |
| 512  | 8 | 1.601323   | 0.293583   | 0.093361   | 5.454413 | 0.681802   |
| 1024 | 1 | 12.998812  | 12.998812  | 0.000005   | N/A      | N/A        |
| 1024 | 4 | 12.998812  | 3.301078   | 0.039613   | 3.937748 | 0.984437   |
| 1024 | 8 | 12.998812  | 2.002957   | 0.372433   | 6.489811 | 0.811226   |
| 2048 | 1 | 103.608182 | 103.608182 | 0.000005   | N/A      | N/A        |
| 2048 | 4 | 103.608182 | 26.541913  | 0.125132   | 3.903569 | 0.975892   |
| 2048 | 8 | 103.608182 | 14.721710  | 1.513615   | 7.037782 | 0.879723   |

## Comparativa por N = 512

| P | T_Seq (s) | T_Par (s) | Speedup  | Eficiencia |
|---|-----------|-----------|----------|------------|
| 4 | 1.601323  | 0.409775  | 3.907810 | 0.976953   |
| 8 | 1.601323  | 0.293583  | 5.454413 | 0.681802   |

## Comparativa por N = 1024

| P | T_Seq (s)  | T_Par (s) | Speedup  | Eficiencia |
|---|------------|-----------|----------|------------|
| 4 | 12.998812  | 3.301078  | 3.937748 | 0.984437   |
| 8 | 12.998812  | 2.002957  | 6.489811 | 0.811226   |

## Comparativa por N = 2048

| P | T_Seq (s)  | T_Par (s)  | Speedup  | Eficiencia |
|---|------------|------------|----------|------------|
| 4 | 103.608182 | 26.541913  | 3.903569 | 0.975892   |
| 8 | 103.608182 | 14.721710  | 7.037782 | 0.879723   |

## Observaciones

- **P = 4 (1 nodo):** Speedup cercano al ideal (~3.9 sobre 4) y eficiencia > 0.97 para N >= 1024. Excelente aprovechamiento del paralelismo intra-nodo.
- **P = 8 (2 nodos):** El speedup sigue creciendo pero la eficiencia cae a ~0.81-0.88, evidenciando overhead de comunicacion inter-nodo (T_Comm salta de ~0.04 a ~0.37 para N=1024, y de ~0.12 a ~1.51 para N=2048).
- **Escalabilidad:** A mayor N, mejor eficiencia en P=8 (0.68 para 512, 0.81 para 1024, 0.88 para 2048). El overhead de comunicacion se amortiza con mayor carga de trabajo por proceso.
- **Conclusion:** Para este algoritmo punto-a-punto, la configuracion de 1 nodo con 4 procesos es muy eficiente. Pasar a 2 nodos mejora el speedup absoluto pero con penalizacion en eficiencia debido al costo de comunicacion entre nodos.
