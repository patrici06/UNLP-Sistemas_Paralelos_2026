# Guia Rapida de Scripts

## Ejecucion Local

| Script | Descripcion |
|--------|-------------|
| `./exec-local-matrices.sh` | Ejecuta version secuencial (N=512,1024,2048,4096; O=1,2,3) |
| `./exec-local-pthreads.sh` | Ejecuta version pthreads (T=2,4,8 como parametro) |
| `./exec-local-openmp.sh` | Ejecuta version OpenMP (T=2,4,8 por OMP_NUM_THREADS) |

## Ejecucion Cluster (via sbatch)

| Script | Descripcion |
|--------|-------------|
| `./exec-cluster-matrices.sh` | Envia jobs secuenciales al cluster |
| `./exec-cluster-pthreads.sh` | Envia jobs pthreads al cluster |
| `./exec-cluster-openmp.sh` | Envia jobs OpenMP al cluster |
| `./job-matrices.sh N O` | Job SLURM para secuencial |
| `./job-pthreads.sh N O T` | Job SLURM para pthreads (T como parametro) |
| `./job-openmp.sh N O T` | Job SLURM para OpenMP (T via OMP_NUM_THREADS) |

## Analisis

| Script | Descripcion |
|--------|-------------|
| `./compare-results.sh [N] [O]` | Compara speedup y genera analisis (opcional: N y/O) |
| `./generate-reports.sh [tipo] [formato]` | Genera reportes en csv/json/txt |
| `./extract_results.py` | Extrae y consolida resultados en CSVs |

## Estructura de Salidas

```
salidas/
  matrices/N{N}/O{O}/{outputs,reports}
  pthreads/N{N}/T{T}/O{O}/{outputs,reports}
  openmp/N{N}/T{T}/O{O}/{outputs,reports}
```

## Ejemplos

```bash
# Ejecutar todo localmente
./exec-local-matrices.sh

# Comparar solo N=1024 con O=3
./compare-results.sh 1024 3

# Generar reportes JSON solo de pthreads
./generate-reports.sh pthreads json
```
