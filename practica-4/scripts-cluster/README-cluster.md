# Cluster Scripts - Guia de Uso

Este directorio contiene los scripts preparados para ejecutar los ejercicios de la **Practica 4** en el **cluster remoto** utilizando **SLURM** como gestor de recursos.

## Estructura

```
scripts-cluster/
├── ej1-mpi-simple.slurm          # Ejercicio 1: anillo con mpi-simple-solucion.c
├── ej2-blocking.slurm            # Ejercicio 2: master-worker bloqueante
├── ej2-non-blocking.slurm        # Ejercicio 2: master-worker no bloqueante
├── ej3-blocking-ring.slurm       # Ejercicio 3: anillo bloqueante
├── ej3-non-blocking-ring.slurm   # Ejercicio 3: anillo no bloqueante
├── ej4-mpi-matmul.slurm          # Ejercicio 4: multiplicacion de matrices MPI
├── ej5-vector-stats.slurm        # Ejercicio 5: vector max/min/promedio (plantilla)
├── ej6-hybrid-matmul.slurm       # Ejercicio 6: MPI + OpenMP matrices (plantilla)
├── run-all.sh                    # Envia todos los jobs al cluster
├── collect-metrics.sh            # Extrae metricas y genera tablas
├── logs-cluster/                 # Logs de salida de SLURM y de los programas
└── tablas-cluster/               # Tablas y CSV generados por collect-metrics.sh
```

## Requisitos previos

- Acceso al cluster de la catedra.
- Modulo de OpenMPI (y OpenMP si se usa ejercicio 6) cargado.
- Los fuentes deben estar en `../Fuentes/`.

## Como ejecutar

### 1. Enviar todos los jobs de una vez

```bash
cd scripts-cluster
chmod +x run-all.sh collect-metrics.sh
./run-all.sh
```

Esto generara el archivo `logs-cluster/jobs.submitted` con los JOBIDs.

### 2. Enviar un job individual

```bash
sbatch ej4-mpi-matmul.slurm
```

**IMPORTANTE:** Nunca ejecute los scripts `.slurm` directamente (`./script.slurm`). Siempre use `sbatch`.

### 3. Verificar estado de los jobs

```bash
squeue -u $USER
```

### 4. Recolectar metricas y generar tablas

Una vez que **todos** los jobs hayan finalizado (estado `COMPLETED` o similar), ejecute:

```bash
./collect-metrics.sh
```

Esto creara el directorio `tablas-cluster/` con:

- `resultados-consolidados.csv` — CSV con todas las metricas.
- `tabla-ej3-blocking-ring.md` — Tabla de tiempos de comunicacion (blocking).
- `tabla-ej3-non-blocking-ring.md` — Tabla de tiempos de comunicacion (non-blocking).
- `tabla-ej4-mpi-matmul.md` — Tabla con Speedup, Eficiencia, tiempos.
- `tabla-ej1-mpi-simple.md` — Listado de logs del ejercicio 1.
- `tabla-ej2-comparativa.md` — Listado de logs comparativos ejercicio 2.

## Metricas de interes

Segun la guia de la practica, se miden:

- **Tiempo total de ejecucion** (`Tiempo total=`)
- **Tiempo de comunicacion** (`Tiempo comunicacion=` o `Tiempo de comunicacion : ... seconds`)
- **Speedup** (S = Ts / Tp)
- **Eficiencia** (E = S / P)

Los scripts SLURM ya incluyen la logica para calcular Speedup y Eficiencia en los ejercicios donde aplica (ej. Ejercicio 4).

## Ajustes para el cluster real

Los scripts estan configurados con valores razonables para un cluster con **4 cores por nodo**. Si su cluster tiene una configuracion diferente, modifique:

- `--tasks-per-node=X` en los scripts `.slurm`.
- Las variables `CONFIGS` en `ej4-mpi-matmul.slurm` para reflejar la cantidad real de nodos/cores.

## Ejercicios 5 y 6

Los scripts `ej5-vector-stats.slurm` y `ej6-hybrid-matmul.slurm` son **plantillas**. Antes de usarlos debe:

1. Implementar los fuentes correspondientes en `../Fuentes/`.
2. Descomentar las lineas de compilacion y ejecucion dentro de cada `.slurm`.
3. Ajustar los parametros (tamano de vectores/matrices, cantidad de hilos, etc.).

### Script hibrido (MPI + OpenMP)

Para el ejercicio 6, recuerde que el script debe usar:

- `--tasks-per-node=1` (1 proceso MPI por nodo).
- `export OMP_NUM_THREADS=T` (hilos OpenMP por nodo).
- `mpirun --bind-to none` (evita conflictos de binding entre MPI y OpenMP).

## Notas

- Los jobs de ejercicios 1, 2 y 3 usan `#SBATCH -N 1` (1 nodo).
- Los jobs de ejercicios 4, 5 y 6 usan `#SBATCH -N 2` (2 nodos) para probar comunicacion entre nodos.
- Para P=16 se incluye `#SBATCH --overcommit` y `mpirun --oversubscribe` segun la nota de la practica.
- Si un job falla, revise el archivo de errores en `logs-cluster/*.err`.
