# GRAFICADOR DE RENDIMIENTO — TP3

## Flujo completo

```bash
# 1. Compilar (si no hay binarios)
cd TP3
mpicc -O2 matrices-mpi.c -o matrices-mpi -lm
mpicc -O2 hibrido.c -o hibrido -fopenmp -lm

# 2. Generar scripts SLURM y enviar
cd TP3/script
./generar_run.sh
sbatch runs/runP8.sh    # MPI P=8
sbatch runs/runP16.sh   # MPI P=16
sbatch runs/runP32.sh   # MPI P=32
sbatch runs/runP16T8.sh # Hybrid 2MPI×8OMP
sbatch runs/runP32T8.sh # Hybrid 4MPI×8OMP
sbatch runs/runP64T8.sh # Hybrid 8MPI×8OMP

# 3. Generar CSVs
cd TP3
bash script/generar_csv.sh

# 4. Graficar (menú interactivo)
python3 -m graficador.plotter
```

## Menú del plotter

| Opción | Gráfico | Tipo |
|--------|---------|------|
| 2 | Comparativa MPI vs Hybrid P=16,32 | Barras |
| 3 | Comparativa TP3 MPI P=8 vs TP2 | Barras |
| 4 | Eficiencia por configuración | Barras |
| 5 | GFLOPS vs N | Líneas |
| 6 | Iso-eficiencia (Eff vs N) | Líneas |
| 7 | Overhead de comunicación | Barras |
| 8 | Overhead absoluto vs N | Líneas |
| 9 | Speedup vs N | Líneas |
| 10 | Speedup vs procesos MPI | Líneas |
| 11 | Tiempo de ejecución vs N | Líneas |
| 12 | Todos los gráficos | - |

## Agregar un nuevo gráfico

Crear `graficador/graphs/mi_grafico.py`:

```python
from graficador import style
from graficador.graphs import graph

@graph("nombre_corto", "Descripción para el menú")
def generate(df, output_dir):
    fig, ax = plt.subplots()
    # ... lógica del gráfico ...
    style.save_plot(fig, f"{output_dir}/nombre_corto")
```

El auto-descubrimiento lo registra automáticamente.

## Datos de TP2

Completar `data/tp2_resultados.csv` con columnas:
`N,tipo,procs,time_s,gflops,speedup,eff`

El gráfico 3 lo lee y compara con MPI P=8.
