# Scripts Independientes - Guía Completa

## Estructura General

Ahora tienes **3 conjuntos de scripts completamente independientes**, cada uno para:
1. **matrices.c** (Referencia secuencial)
2. **matrices-pthread.c** (Paralelo con pthreads)
3. **matrices-open-mp.c** (Paralelo con OpenMP)

## Scripts Disponibles

### MATRICES.C (Referencia Secuencial)
```
exec-cluster-matrices.sh  → Script principal
job-matrices.sh          → Job para cluster
Compilación: -O2 (único nivel)
Estructura: salidas/matrices/N{512,1024,2048,4096}/outputs/
Threads: T=1 (no paralelismo)
```

### MATRICES-PTHREAD.C (Paralelo - Pthreads)
```
exec-cluster-pthreads.sh → Script principal
job-pthread.sh           → Job para cluster
Compilación: -O2 (único nivel)
Estructura: salidas/pthread/N{512,1024,2048,4096}-T{1,2,4,8}/outputs/
Threads: T={1,2,4,8}
```

### MATRICES-OPEN-MP.C (Paralelo - OpenMP)
```
exec-cluster-openmp.sh  → Script principal
job-openmp.sh           → Job para cluster
Compilación: -O2 (único nivel)
Estructura: salidas/openmp/N{512,1024,2048,4096}-T{1,2,4,8}/outputs/
Threads: T={1,2,4,8}
```

## Flujo de Uso Recomendado

### PASO 1: Ejecutar matrices.c (REFERENCIA BASELINE)
```bash
./exec-cluster-matrices.sh
# Esperar a que terminen todos los jobs
squeue -u $USER
```

**Salida esperada:**
```
RESULT;4096;1;1.8234;2.439;1.000000;100.000000;0.000000;0.000000
```

Estos tiempos son tu **referencia baseline**.

---

### PASO 2: Ejecutar matrices-pthread.c (SOLO T=1 PRIMERO)
```bash
./exec-cluster-pthreads.sh
# Esto envía T={1,2,4,8}

# Pero primero espera solo T=1 de TODOS los N
squeue -u $USER
```

**Una vez que T=1 termina:**
1. Extrae el tiempo secuencial:
```bash
grep "^RESULT.*;1;" salidas/pthread/N*/outputs/exec_*.txt
# Por ejemplo: RESULT;4096;1;0.0412;...
```

2. Hardcodea este valor en `matrices-pthread.c` línea ~50:
```c
static double ref_time_sequential = 0.0412;  // Tu valor aquí
```

3. Recompila:
```bash
gcc -O2 -pthread -o matrices-pthread matrices-pthread.c -lm
```

4. Envía de nuevo (ahora con ref_time correcto):
```bash
./exec-cluster-pthreads.sh
```

---

### PASO 3: Ejecutar matrices-open-mp.c (SOLO T=1 PRIMERO)
```bash
./exec-cluster-openmp.sh
# Esto envía T={1,2,4,8}

# Pero primero espera solo T=1 de TODOS los N
squeue -u $USER
```

**Una vez que T=1 termina:**
1. Extrae el tiempo secuencial:
```bash
grep "^RESULT.*;1;" salidas/openmp/N*/outputs/exec_*.txt
# Por ejemplo: RESULT;4096;1;0.0562;...
```

2. Hardcodea en `matrices-open-mp.c` línea ~42:
```c
static double ref_time_sequential = 0.0562;  // Tu valor aquí
```

3. Recompila:
```bash
gcc -O2 -fopenmp -o matrices-open-mp matrices-open-mp.c -lm
```

4. Envía de nuevo:
```bash
./exec-cluster-openmp.sh
```

---

## Estructura de Directorios Resultante

```
salidas/
├── matrices/
│   ├── N512/outputs/
│   ├── N1024/outputs/
│   ├── N2048/outputs/
│   └── N4096/outputs/
├── pthread/
│   ├── N512-T1/outputs/
│   ├── N512-T2/outputs/
│   ├── N512-T4/outputs/
│   ├── N512-T8/outputs/
│   ├── N1024-T1/outputs/
│   ├── N1024-T2/outputs/
│   ... (N={1024,2048,4096} × T={1,2,4,8})
└── openmp/
    ├── N512-T1/outputs/
    ├── N512-T2/outputs/
    ├── N512-T4/outputs/
    ├── N512-T8/outputs/
    ├── N1024-T1/outputs/
    ├── N1024-T2/outputs/
    ... (N={1024,2048,4096} × T={1,2,4,8})
```

## Archivo de Salida

Cada job genera un archivo `exec_JOBID.txt` con:
```
========================================
Ejecutando matrices-pthread
==========================================
N = 4096
T = 8
Compilación: -O2
Host: node01
Date: 2026-05-22 10:15:30
Threads disponibles: 8
==========================================

RESULT;4096;8;0.2456;18.234;7.423;92.78;0.0312;12.71
CONSTANTE_K;7.500000
VALIDATION;OK
```

## Campos del RESULT

```
RESULT;N;T;workTime;GFLOPS;speedup;efficiency;overhead;overhead%

- N:          Tamaño de matriz
- T:          Cantidad de threads
- workTime:   Tiempo de ejecución (segundos)
- GFLOPS:     (2*N³) / (workTime*1e9)
- speedup:    ref_time_sequential / workTime
- efficiency: (speedup / T) * 100
- overhead:   workTime - (ref_time / T)
- overhead%:  (overhead / workTime) * 100
```

## Monitoreo en Cluster

### Ver jobs activos:
```bash
squeue -u $USER

# Ver más detalles:
squeue -u $USER -o "%.18i %.9P %.8j %.8u %.2t %.10M %.6D %R"

# Ver específico:
squeue -j JOBID
```

### Ver output en tiempo real:
```bash
tail -f salidas/pthread/N4096-T8/outputs/exec_*.txt
```

### Esperar a que terminen todos:
```bash
while squeue -u $USER | grep -q "RUNNING\|PENDING"; do
    sleep 5
done
echo "¡Todos terminaron!"
```

## Extracción y Análisis de Resultados

### Extraer todos los RESULT:
```bash
echo "=== MATRICES (Baseline) ==="
find salidas/matrices -name "exec_*.txt" -exec grep "^RESULT" {} +

echo ""
echo "=== PTHREAD ==="
find salidas/pthread -name "exec_*.txt" -exec grep "^RESULT" {} +

echo ""
echo "=== OPENMP ==="
find salidas/openmp -name "exec_*.txt" -exec grep "^RESULT" {} +
```

### Comparar speedup entre N y T:
```bash
# Para pthread N=4096
grep "^RESULT;4096" salidas/pthread/N4096-T*/outputs/exec_*.txt

# Salida esperada:
# RESULT;4096;1;1.8234;...
# RESULT;4096;2;0.9456;...
# RESULT;4096;4;0.4912;...
# RESULT;4096;8;0.2677;...
```

### Tabla de comparación (spreadsheet):
```bash
cat > resultados.csv << 'EOF'
Program,N,T,workTime,GFLOPS,speedup,efficiency,overhead,overhead%
EOF

# Agregar matriz.c
for file in salidas/matrices/*/outputs/exec_*.txt; do
    grep "^RESULT" "$file" | sed 's/^RESULT;//' | awk -v prog="matrices" '{print prog","$0}' >> resultados.csv
done

# Agregar pthread
for file in salidas/pthread/*/outputs/exec_*.txt; do
    grep "^RESULT" "$file" | sed 's/^RESULT;//' | awk -v prog="pthread" '{print prog","$0}' >> resultados.csv
done

# Agregar openmp
for file in salidas/openmp/*/outputs/exec_*.txt; do
    grep "^RESULT" "$file" | sed 's/^RESULT;//' | awk -v prog="openmp" '{print prog","$0}' >> resultados.csv
done

echo "Archivo generado: resultados.csv"
cat resultados.csv
```

## Ventajas de Scripts Independientes

✓ **Claridad**: Cada script es responsable de una implementación
✓ **Modularidad**: Ejecutar solo lo que necesitas
✓ **Debugging**: Si hay error, sabes exactamente dónde
✓ **Flexibilidad**: Modificar parámetros sin afectar otros scripts
✓ **Documentación**: Cada script es autoexplicativo

## Troubleshooting

### Error: "gcc: command not found"
```bash
module load gcc  # En el frontend del cluster
./exec-cluster-matrices.sh
```

### Error: "matrices-pthread: No such file or directory"
Los scripts asumen compilación en frontend:
```bash
# Verifica que los ejecutables existan
ls -la matrices-* 2>/dev/null
# Si no existen, los scripts compilan automáticamente
```

### Job quedan en PENDING demasiado tiempo
Aumenta el tiempo en el script:
```bash
# En exec-cluster-*.sh, cambia:
--time=00:15:00  # Aumentar a 00:30:00 si es necesario
```

### Resultados no tienen speedup/efficiency (todo 1.0/100.0)
Esto es normal si:
- Es T=1 (aún no has hardcodeado ref_time)
- Aún no recompilaste después de hardcodear

Solución:
```bash
# Hardcodea ref_time
# Recompila
# Envía de nuevo
```

## Flujo Completo (Script)

```bash
#!/bin/bash
# Ejecuta TODO secuencialmente

echo "1. Matrices.c (referencia)"
./exec-cluster-matrices.sh
sleep 5

echo "2. Pthread (T=1 primero)"
./exec-cluster-pthreads.sh
# Espera a que T=1 terminen
echo "Esperando T=1... presiona Enter"
read

echo "3. Extracta ref_time y hardcodea"
PTHREAD_REF=$(grep "^RESULT.*;1;" salidas/pthread/N4096*/outputs/exec_*.txt | head -1 | awk -F';' '{print $4}')
sed -i "s/static double ref_time_sequential = -1.0;/static double ref_time_sequential = $PTHREAD_REF;/" matrices-pthread.c
gcc -O2 -pthread -o matrices-pthread matrices-pthread.c -lm

echo "4. Pthread (con ref_time)"
./exec-cluster-pthreads.sh
sleep 5

echo "5. OpenMP (T=1 primero)"
./exec-cluster-openmp.sh
echo "Esperando T=1... presiona Enter"
read

echo "6. Extracta ref_time y hardcodea"
OPENMP_REF=$(grep "^RESULT.*;1;" salidas/openmp/N4096*/outputs/exec_*.txt | head -1 | awk -F';' '{print $4}')
sed -i "s/static double ref_time_sequential = -1.0;/static double ref_time_sequential = $OPENMP_REF;/" matrices-open-mp.c
gcc -O2 -fopenmp -o matrices-open-mp matrices-open-mp.c -lm

echo "7. OpenMP (con ref_time)"
./exec-cluster-openmp.sh

echo "¡LISTO! Todos los benchmarks completados"
```

Guarda esto en `benchmark_completo.sh` y ejecuta:
```bash
chmod +x benchmark_completo.sh
./benchmark_completo.sh
```
