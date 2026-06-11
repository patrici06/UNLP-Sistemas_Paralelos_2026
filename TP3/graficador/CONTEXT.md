# CONTEXTO COMPLETO — Graficador TP3

> Archivo generado para recuperar sesión en otra instancia.
> Contiene TODO lo necesario: estructura del proyecto, flujo de trabajo,
> archivos fuente completos, datos sintéticos y pendientes.

---

## 1. Estructura del proyecto

```
TP3/
├── matrices-mpi.c         # MPI puro (multiplicación de matrices por bloques)
├── hibrido.c              # Híbrido MPI+OpenMP
├── matrices-mpi           # Binario compilado
├── hibrido                # Binario compilado
├── times.txt              # Resultados crudos de benchmarks
│
├── script/
│   ├── blade.sh              # Orquestador original (menú interactivo SLURM)
│   ├── generar_run.sh        # Genera scripts SLURM individuales
│   ├── generar_csv.sh        ← NUEVO: parsea times.txt → CSVs
│   ├── mostrar_salidas.sh    # Muestra resultados en consola
│   └── runs/                 # Scripts SLURM generados
│       ├── runMPIP1.sh       # MPI P=1 (secuencial)
│       ├── runP8.sh          # MPI P=8
│       ├── runP16.sh         # MPI P=16
│       ├── runP32.sh         # MPI P=32
│       ├── runHybridP1T1.sh  # Hybrid 1MPI×1OMP (secuencial)
│       ├── runP16T8.sh       # Hybrid 2MPI×8OMP = 16
│       ├── runP32T8.sh       # Hybrid 4MPI×8OMP = 32
│       └── runP64T8.sh       # Hybrid 8MPI×8OMP = 64
│
├── data/                    ← NUEVO: CSVs generados
│   ├── resultados.csv        # Consolidado (MPI + Hybrid)
│   ├── mpi_resultados.csv    # Solo MPI puro
│   ├── hybrid_resultados.csv # Solo Híbrido
│   └── tp2_resultados.csv   # Plantilla para TP2 (completar)
│
└── graficador/              ← NUEVO: sistema de gráficos
    ├── plotter.py            # Menú CLI principal
    ├── style.py              # Config matplotlib + LaTeX/PGF
    ├── loader.py             # Carga y procesamiento de CSVs
    ├── INSTRUCTIVO.md        # Mini guía de uso
    ├── CONTEXT.md            # ← ESTE ARCHIVO
    ├── graphs/               # Módulos de gráficos (auto-descubiertos)
    │   ├── __init__.py           # Registro vía @graph decorator
    │   ├── tiempo.py             # Tiempo vs N (líneas)
    │   ├── speedup.py            # Speedup vs N (líneas)
    │   ├── speedup_vs_procs.py   # Speedup vs MPI_procs (líneas + ideal)
    │   ├── gflops.py             # GFLOPS vs N (líneas)
    │   ├── eficiencia.py         # Eficiencia (barras agrupadas)
    │   ├── isoeficiencia.py      # Eff vs N (líneas iso-eficiencia)
    │   ├── overhead.py           # Overhead % (barras agrupadas)
    │   ├── overhead_abs.py       # Overhead absoluto (líneas)
    │   ├── comparativa_hybrid.py # MPI vs Hybrid P=16,32 (barras)
    │   └── comparativa_tp2.py    # MPI P=8 vs TP2 (barras)
    └── plots/               # Gráficos generados (PNG + PGF si hay LaTeX)
```

---

## 2. Formato de datos

### times.txt (salida de los binarios)

Cada binario imprime líneas `RESULT;...`. Dos formatos posibles:

**8 campos** (MPI puro e Hybrid nuevo):
```
RESULT;N;procs;time_s;gflops;comm%;speedup;eff
```

**9 campos** (formato original de matrices-hybrid):
```
RESULT;N;procs;t;time_s;gflops;comm%;speedup;eff
```

### resultados.csv (generado por generar_csv.sh)

```
N,tipo,procs_mpi,omp_threads,total_workers,time_s,gflops,comm_pct,speedup,eff
```

- `tipo`: `mpi` o `hybrid`
- `procs_mpi`: cantidad de procesos MPI
- `omp_threads`: hilos OpenMP por proceso (1 para MPI puro)
- `total_workers`: procs_mpi × omp_threads
- `comm_pct`: overhead de comunicación en % (ya sin símbolo)
- `eff`: eficiencia en % (ya sin símbolo)

### tp2_resultados.csv (plantilla)

```
N,tipo,procs,time_s,gflops,speedup,eff
2048,Pthreads,8,,,,
4096,Pthreads,8,,,,
```

Completar con datos del mejor rendimiento entre Pthreads/OpenMP del TP2.

---

## 3. Configuraciones actuales

### MPI puro
| Config | P | Nodos | PPN | Sizes |
|--------|---|-------|-----|-------|
| P=1 | 1 | 1 | 1 | 512, 1024, 2048, 4096 |
| P=8 | 8 | 1 | 8 | 2048, 4096 |
| P=16 | 16 | 2 | 8 | 2048, 4096 |
| P=32 | 32 | 4 | 8 | 2048, 4096 |

### Híbrido MPI+OpenMP
| Config | MPI | OMP | Total | Nodos | PPN | Cores/nodo |
|--------|-----|-----|-------|-------|-----|------------|
| P=16T=8 | 2 | 8 | 16 | 1 | 2 | 16/64 |
| P=32T=8 | 4 | 8 | 32 | 1 | 4 | 32/64 |
| P=64T=8 | 8 | 8 | 64 | 1 | 8 | 64/64 |

---

## 4. Sistema de registro de gráficos (extensibilidad)

Mecanismo en `graficador/graphs/__init__.py`:

- Decorador `@graph(nombre, descripción)` registra la función
- `discover()` usa `pkgutil.iter_modules` para importar todos los `.py` del directorio
- El menú en `plotter.py` llama `get_choices()` para listar

**Para agregar un gráfico:** crear `graficador/graphs/mi_grafico.py`:

```python
from graficador import style
from graficador.graphs import graph

@graph("nombre_corto", "Descripción visible en menú")
def generate(df, output_dir):
    fig, ax = plt.subplots()
    # ... lógica ...
    style.save_plot(fig, f"{output_dir}/nombre_corto")
```

---

## 5. Manejo de LaTeX/PGF

El módulo `style.py` detecta automáticamente si `pdflatex` está disponible:

- **Con LaTeX**: usa backend PGF, exporta `.png` + `.pgf` con fuentes Computer Modern
- **Sin LaTeX**: usa backend Agg, exporta solo `.png` con DejaVu Sans

---

## 6. Datos sintéticos de prueba

Se creó `times.txt` con datos ficticios para validar el pipeline completo.
Los 10 gráficos se generaron correctamente desde esos datos.

Para usar datos reales: ejecutar los benchmarks en cluster con SLURM,
luego `bash script/generar_csv.sh` y finalmente el plotter.

---

## 7. Pendientes / Próximos pasos

- [ ] Completar `data/tp2_resultados.csv` con datos reales del TP2
- [ ] Ejecutar benchmarks reales en cluster (SLURM) para reemplazar `times.txt`
- [ ] Verificar que los gráficos con datos reales tengan sentido
- [ ] Si se necesita LaTeX/PGF, instalar `texlive-publishers` o similar
- [ ] Posible mejora: agregar más configuraciones en `generar_run.sh`

---

## 8. Dependencias

```bash
pip install pandas matplotlib numpy
```

Opcional para PGF/LaTeX:
```bash
sudo dnf install texlive-scheme-medium   # Fedora
# o: sudo apt install texlive-publishers  # Debian/Ubuntu
```
