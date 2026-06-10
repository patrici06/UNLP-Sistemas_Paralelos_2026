# Guia de Uso - Scripts de Cluster (Interactivo)

## Estructura de Directorios

```
script-cluster/
├── bin/                    # Binarios compilados (frontend)
├── blade/
│   ├── run-all.sh          # Orquestador interactivo Blade
│   └── jobs/               # Scripts de job generados con #SBATCH
│   └── mpi-P*-N*/          # Directorios de salida (MPI)
│   └── hybrid-P*-T*-N*/    # Directorios de salida (Hibrido)
└── xeonphi/
    ├── run-all.sh          # Orquestador interactivo XeonPHI
    └── jobs/               # Scripts de job generados con #SBATCH
    └── mpi-P*-N*/          # Directorios de salida (MPI)
    └── hybrid-P*-T*-N*/    # Directorios de salida (Hibrido)
└── GUIA_DE_USO.md          # Guia de uso completa
```

## Como Ejecutar

### 1. Blade (Cluster Multicore)

```bash
cd script-cluster/blade
bash run-all.sh
```

**Menu interactivo:**
```
==============================================
  Orquestador: Blade (Cluster Multicore)
==============================================

  1) MPI  P=8   (N=1, 8 tasks/nodo)
  2) MPI  P=16  (N=2, 8 tasks/nodo)
  3) MPI  P=32  (N=4, 8 tasks/nodo)
  4) Hibrido P=16, T=1 (N=2, 8 tasks/nodo)
  5) Hibrido P=32, T=1 (N=4, 8 tasks/nodo)
  6) TODOS los escenarios MPI (1,2,3)
  7) TODOS los escenarios Hibrido (4,5)
  8) TODOS los escenarios
  9) Compilar binarios
  0) Salir

Seleccione una opcion: _
```

**Acciones:**
- Elegir una opcion envia el escenario correspondiente con los 4 tamanos de matriz (512, 1024, 2048, 4096).
- Opcion `9` compila `matrices-mpi.c` y `matrices-hybrid.c` en el frontend.
- Los scripts de job se generan dinamicamente con `#SBATCH` y se envian via `sbatch`.

### 2. XeonPHI (Intel Manycore)

```bash
cd script-cluster/xeonphi
bash run-all.sh
```

**Menu interactivo:**
```
==============================================
  Orquestador: XeonPHI (Intel Manycore)
==============================================

  1) MPI  P=16  (N=1, 16 tasks/nodo)
  2) MPI  P=32  (N=1, 32 tasks/nodo)
  3) Hibrido P=16, T=4 (N=1, 16 tasks/nodo)
  4) Hibrido P=32, T=2 (N=1, 32 tasks/nodo)
  5) TODOS los escenarios MPI (1,2)
  6) TODOS los escenarios Hibrido (3,4)
  7) TODOS los escenarios
  0) Salir

Seleccione una opcion: _
```

**Acciones:**
- Elegir una opcion envia el escenario correspondiente con los 4 tamanos de matriz.
- Cada job **compila internamente** en el nodo XeonPHI con `mpiicc` (OneAPI).
- Los scripts de job se generan dinamicamente con `#SBATCH` y se envian via `sbatch`.

## Casos de Prueba

| Particion | Tipo | P (procesos) | T (threads) | N (nodos) | Tamanos |
|-----------|------|--------------|-------------|-----------|---------|
| Blade | MPI | 8, 16, 32 | 1 | 1, 2, 4 | 512, 1024, 2048, 4096 |
| Blade | Hibrido | 16, 32 | 1 | 2, 4 | 512, 1024, 2048, 4096 |
| XeonPHI | MPI | 16, 32 | 1 | 1 | 512, 1024, 2048, 4096 |
| XeonPHI | Hibrido | 16, 32 | 4, 2 | 1 | 512, 1024, 2048, 4096 |

## Directorios de Salida

Los resultados se organizan bajo `blade/` o `xeonphi/` con la siguiente estructura:

```
blade/mpi-P8-N1/N512/
    output_12345.txt    <- Salida estandar del programa
    errors_12345.txt    <- Errores (si los hubo)

blade/hybrid-P16-T1-N2/N1024/
    output_12346.txt
    errors_12346.txt
```

Donde `12345` es el **JOBID** asignado por SLURM.

## Formato de Salida

### Archivo: `output_<JOBID>.txt`

Contiene la salida del programa `matrices-mpi.c` o `matrices-hybrid.c`.

**Linea principal (formato CSV interno):**
```
RESULT;<N>;<TiempoTotal>;<GFlops>;<OverheadComm%>;<Speedup>
```

Campos:
- `N`: Dimension de la matriz
- `TiempoTotal`: Tiempo de ejecucion en segundos
- `GFlops`: Rendimiento en GFLOPS
- `OverheadComm%`: Porcentaje de tiempo en comunicaciones MPI
- `Speedup`: Aceleracion respecto al secuencial (si esta disponible)

**Ejemplo:**
```
RESULT;512;0.123456;3.456789;12.34%;1.2345
```

**Linea de validacion (solo para N <= 128):**
```
VALIDATION;OK
```

### Archivo: `errors_<JOBID>.txt`

Contiene errores de runtime o del gestor de recursos. Si esta vacio, la ejecucion fue exitosa.

## Comandos Utiles

```bash
# Ver trabajos en ejecucion o cola
squeue

# Cancelar un trabajo
scancel <JOBID>

# Ver salida de un trabajo en tiempo real (si esta corriendo)
tail -f blade/mpi-P8-N1/N512/output_<JOBID>.txt

# Listar todos los resultados de Blade
find blade/ -name "output_*.txt" | sort

# Extraer todos los RESULT de Blade en un CSV
find blade/ -name "output_*.txt" -exec grep "^RESULT" {} + > blade_resultados.csv
```

## Notas Importantes

- **No se establece limite de tiempo** (`--time`). Monitoree manualmente los trabajos.
- Blade compila en el frontend (opcion 9 del menu); XeonPHI compila dentro de cada job.
- Los scripts de job se generan dinamicamente; puede inspeccionarlos en `blade/jobs/` o `xeonphi/jobs/`.
- Todos los scripts de job usan **paths absolutos** para binarios y archivos de salida, evitando problemas con el directorio de trabajo de SLURM.
- El parametro `--bind-to none` esta incluido en todos los casos hibridos para evitar que OpenMPI restrinja los hilos a un solo core.
- El menu interactivo permite ejecutar escenarios de forma selectiva sin monopolizar el cluster.
