# Ejercicio 4 - Scripts para Cluster (PLANO)

Este directorio contiene todo lo necesario para ejecutar el **Ejercicio 4** en el cluster remoto.

**Todo esta en el mismo directorio**: fuente, binario, logs y tablas. Sin subdirectorios.

## Archivos

| Archivo | Descripcion |
|---------|-------------|
| `mpi_matmul.c` | Codigo fuente del ejercicio. |
| `ej4-mpi-matmul.slurm` | Script SLURM principal. Compila, ejecuta P={1,4,8} y N={512,1024,2048}, calcula Speedup y Eficiencia. |
| `run-ej4.sh` | Helper que envia el job al cluster con `sbatch`. |
| `collect-metrics-ej4.sh` | Procesa los logs y genera tablas + CSV con las metricas del ejercicio. |

## Salidas que se generan en el mismo directorio

- `ej4-N*-P*.log` — Logs de cada ejecucion.
- `ej4-mpi-matmul-%j.out` / `.err` — Salida de SLURM.
- `jobs.submitted` — Registro del JOBID.
- `resultados-ej4.csv` — CSV consolidado.
- `tabla-ej4-mpi-matmul.md` — Tabla resumen.
- `tabla-ej4-comparativa-por-N.md` — Tablas por N.

## Uso rapido

```bash
cd scripts-cluster-ej4
chmod +x *.sh
./run-ej4.sh
```

Esperar a que termine (`squeue -u $USER`), luego:

```bash
./collect-metrics-ej4.sh
```

## Configuracion del cluster

Se asumen **4 cores por nodo**. Si tu cluster tiene otra config, edita el arreglo `CONFIGS` dentro de `ej4-mpi-matmul.slurm`:

```bash
CONFIGS=(
    "1|1"   # Secuencial
    "1|4"   # 1 nodo, X procesos
    "2|4"   # 2 nodos, X proc/nodo
)
```

Y ajusta `#SBATCH --tasks-per-node=4`.

## Metricas de interes

| Metrica | Formula | Fuente |
|---------|---------|--------|
| Tiempo total | - | Log del programa (`Tiempo total=`) |
| Tiempo comunicacion | - | Log del programa (`Tiempo comunicacion=`) |
| Speedup (S) | Ts / Tp | Calculado por el script SLURM |
| Eficiencia (E) | S / P | Calculado por el script SLURM |

## Notas

- No usar `./ej4-mpi-matmul.slurm` directamente. Siempre `sbatch`.
- No se crean directorios dinamicamente; todo se escribe en el directorio actual.
- Si necesitas limpiar entre corridas, elimina `ej4-*.log`, `ej4-mpi-matmul-*.out`, `.err` y el binario `mpi_matmul`.
