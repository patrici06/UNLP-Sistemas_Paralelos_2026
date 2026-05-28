# Scripts T=1 - Guía de Referencia Secuencial

## Propósito

Los scripts **-t1** sirven **ÚNICAMENTE** para calcular la referencia de tiempo secuencial (`ref_time_sequential`) que se usará en los cálculos de speedup y eficiencia en los scripts principales.

## Scripts T=1

### 1️⃣ matrices.c (Baseline Secuencial)
```bash
./exec-cluster-matrices-t1.sh
```
- **Job script:** `job-matrices-t1.sh`
- **Salida:** `salidas/matrices-t1/N{512,1024,2048,4096}/outputs/exec_*.txt`
- **T:** 1 (sin paralelismo)
- **Propósito:** Referencia pura (sin pthread ni OpenMP)

### 2️⃣ matrices-pthread.c (T=1 Referencia)
```bash
./exec-cluster-pthreads-t1.sh
```
- **Job script:** `job-pthread-t1.sh`
- **Salida:** `salidas/pthread-t1/N{512,1024,2048,4096}/outputs/exec_*.txt`
- **T:** 1 (un único thread de pthread)
- **Propósito:** Extraer ref_time para hardcodear en `matrices-pthread.c`

### 3️⃣ matrices-open-mp.c (T=1 Referencia)
```bash
./exec-cluster-openmp-t1.sh
```
- **Job script:** `job-openmp-t1.sh`
- **Salida:** `salidas/openmp-t1/N{512,1024,2048,4096}/outputs/exec_*.txt`
- **T:** 1 (un único thread de OpenMP)
- **Propósito:** Extraer ref_time para hardcodear en `matrices-open-mp.c`

---

## Flujo Completo Recomendado

### FASE 1: Obtener Baseline (matrices.c)
```bash
# Ejecutar T=1 para matrices.c
./exec-cluster-matrices-t1.sh

# Monitorear
squeue -u $USER

# Ver salidas cuando terminen
grep "^RESULT" salidas/matrices-t1/N*/outputs/exec_*.txt
```

**Salida esperada:**
```
RESULT;512;1;0.0123;...
RESULT;1024;1;0.0987;...
RESULT;2048;1;0.7654;...
RESULT;4096;1;6.1234;...
```

**IMPORTANTE:** Guarda estos tiempos como referencia general.

---

### FASE 2: Calcular ref_time para pthread

#### Paso 1: Ejecutar T=1
```bash
./exec-cluster-pthreads-t1.sh

# Esperar a que terminen
squeue -u $USER
```

#### Paso 2: Extraer los tiempos
```bash
# Ver todos los resultados T=1
grep "^RESULT" salidas/pthread-t1/N*/outputs/exec_*.txt

# Ejemplo de salida:
# RESULT;512;1;0.0112;...
# RESULT;1024;1;0.0876;...
# RESULT;2048;1;0.6543;...
# RESULT;4096;1;5.2345;...
```

#### Paso 3: Extraer un valor representativo (N=4096)
```bash
# Para usar como referencia única
grep "^RESULT;4096;1" salidas/pthread-t1/N4096/outputs/exec_*.txt | awk -F';' '{print $4}'

# Resultado: 5.2345 (ejemplo)
```

#### Paso 4: Hardcodear en matrices-pthread.c
```bash
# Editar línea ~50 en matrices-pthread.c
# Cambiar:
# static double ref_time_sequential = -1.0;
# A:
# static double ref_time_sequential = 5.2345;

sed -i 's/static double ref_time_sequential = -1.0;/static double ref_time_sequential = 5.2345;/' matrices-pthread.c
```

#### Paso 5: Recompilar
```bash
gcc -O2 -pthread -o matrices-pthread matrices-pthread.c -lm
```

#### Paso 6: Ejecutar con T={2,4,8}
```bash
./exec-cluster-pthreads.sh
# Ahora T=2, T=4, T=8 usarán ref_time_sequential = 5.2345
```

---

### FASE 3: Calcular ref_time para OpenMP

#### Paso 1: Ejecutar T=1
```bash
./exec-cluster-openmp-t1.sh

# Esperar a que terminen
squeue -u $USER
```

#### Paso 2: Extraer los tiempos
```bash
# Ver todos los resultados T=1
grep "^RESULT" salidas/openmp-t1/N*/outputs/exec_*.txt

# Ejemplo de salida:
# RESULT;512;1;0.0145;...
# RESULT;1024;1;0.1123;...
# RESULT;2048;1;0.8765;...
# RESULT;4096;1;7.0123;...
```

#### Paso 3: Extraer un valor representativo (N=4096)
```bash
# Para usar como referencia única
grep "^RESULT;4096;1" salidas/openmp-t1/N4096/outputs/exec_*.txt | awk -F';' '{print $4}'

# Resultado: 7.0123 (ejemplo)
```

#### Paso 4: Hardcodear en matrices-open-mp.c
```bash
# Editar línea ~42 en matrices-open-mp.c
# Cambiar:
# static double ref_time_sequential = -1.0;
# A:
# static double ref_time_sequential = 7.0123;

sed -i 's/static double ref_time_sequential = -1.0;/static double ref_time_sequential = 7.0123;/' matrices-open-mp.c
```

#### Paso 5: Recompilar
```bash
gcc -O2 -fopenmp -o matrices-open-mp matrices-open-mp.c -lm
```

#### Paso 6: Ejecutar con T={2,4,8}
```bash
./exec-cluster-openmp.sh
# Ahora T=2, T=4, T=8 usarán ref_time_sequential = 7.0123
```

---

## Estrategia de Referencia

### Opción A: Una referencia por programa (Recomendado)
```c
// matrices-pthread.c
static double ref_time_sequential = 5.2345;  // Tiempo T=1 de pthread

// matrices-open-mp.c
static double ref_time_sequential = 7.0123;  // Tiempo T=1 de OpenMP
```

**Ventaja:** Cada programa compara contra su propia referencia T=1
**Resultado:** Speedup y eficiencia independientes para cada implementación

### Opción B: Una referencia única (matrices.c baseline)
```c
// matrices-pthread.c
static double ref_time_sequential = 6.1234;  // Baseline matrices.c N=4096

// matrices-open-mp.c
static double ref_time_sequential = 6.1234;  // Baseline matrices.c N=4096
```

**Ventaja:** Comparar pthread vs OpenMP contra una referencia común
**Resultado:** Medir ambos paralelos contra el secuencial puro

---

## Flujo Automatizado (Script)

Copia y guarda como `benchmark_t1.sh`:

```bash
#!/bin/bash

echo "======================================"
echo "FASE 1: matrices.c Baseline"
echo "======================================"
./exec-cluster-matrices-t1.sh
echo "Esperando... presiona Enter cuando terminen los jobs T=1"
read

echo ""
echo "======================================"
echo "FASE 2: pthread T=1"
echo "======================================"
./exec-cluster-pthreads-t1.sh
echo "Esperando... presiona Enter cuando terminen los jobs T=1"
read

# Extraer ref_time para pthread
PTHREAD_REF=$(grep "^RESULT;4096;1" salidas/pthread-t1/N4096/outputs/exec_*.txt | awk -F';' '{print $4}' | head -1)
echo "✓ ref_time pthread (N=4096): $PTHREAD_REF"
sed -i "s/static double ref_time_sequential = -1.0;/static double ref_time_sequential = $PTHREAD_REF;/" matrices-pthread.c
gcc -O2 -pthread -o matrices-pthread matrices-pthread.c -lm
echo "✓ matrices-pthread compilado con ref_time = $PTHREAD_REF"

echo ""
echo "======================================"
echo "FASE 3: openmp T=1"
echo "======================================"
./exec-cluster-openmp-t1.sh
echo "Esperando... presiona Enter cuando terminen los jobs T=1"
read

# Extraer ref_time para openmp
OPENMP_REF=$(grep "^RESULT;4096;1" salidas/openmp-t1/N4096/outputs/exec_*.txt | awk -F';' '{print $4}' | head -1)
echo "✓ ref_time openmp (N=4096): $OPENMP_REF"
sed -i "s/static double ref_time_sequential = -1.0;/static double ref_time_sequential = $OPENMP_REF;/" matrices-open-mp.c
gcc -O2 -fopenmp -o matrices-open-mp matrices-open-mp.c -lm
echo "✓ matrices-open-mp compilado con ref_time = $OPENMP_REF"

echo ""
echo "======================================"
echo "LISTO PARA BENCHMARK FINAL"
echo "======================================"
echo ""
echo "Ahora ejecuta:"
echo "  ./exec-cluster-pthreads.sh  (T={2,4,8})"
echo "  ./exec-cluster-openmp.sh    (T={2,4,8})"
```

Ejecutar:
```bash
chmod +x benchmark_t1.sh
./benchmark_t1.sh
```

---

## Estructura de Directorios

Después de ejecutar todos los -t1:

```
salidas/
├── matrices-t1/
│   ├── N512/outputs/exec_*.txt
│   ├── N1024/outputs/exec_*.txt
│   ├── N2048/outputs/exec_*.txt
│   └── N4096/outputs/exec_*.txt
├── pthread-t1/
│   ├── N512/outputs/exec_*.txt
│   ├── N1024/outputs/exec_*.txt
│   ├── N2048/outputs/exec_*.txt
│   └── N4096/outputs/exec_*.txt
└── openmp-t1/
    ├── N512/outputs/exec_*.txt
    ├── N1024/outputs/exec_*.txt
    ├── N2048/outputs/exec_*.txt
    └── N4096/outputs/exec_*.txt
```

---

## Monitoreo

### Ver estado de jobs
```bash
squeue -u $USER
```

### Ver específico job
```bash
squeue -j JOBID
```

### Ver output en tiempo real
```bash
tail -f salidas/pthread-t1/N4096/outputs/exec_*.txt
```

### Esperar a que todos terminen
```bash
while squeue -u $USER | grep -q "RUNNING\|PENDING"; do
    sleep 10
    squeue -u $USER
done
echo "¡Todos terminaron!"
```

---

## Verificación

Una vez que termina un -t1, verifica:

```bash
# Ver que el archivo no esté vacío
wc -l salidas/pthread-t1/N4096/outputs/exec_*.txt

# Ver el RESULT
grep "^RESULT" salidas/pthread-t1/N4096/outputs/exec_*.txt

# Ver que sea T=1 y esté bien
grep "^RESULT;4096;1" salidas/pthread-t1/N4096/outputs/exec_*.txt
```

---

## Resumen: Orden de Ejecución

1. **`./exec-cluster-matrices-t1.sh`** → Obtener baseline
2. **`./exec-cluster-pthreads-t1.sh`** → Obtener ref para pthread
3. **Hardcodear ref_time en matrices-pthread.c**
4. **Recompilar matrices-pthread**
5. **`./exec-cluster-openmp-t1.sh`** → Obtener ref para openmp
6. **Hardcodear ref_time en matrices-open-mp.c**
7. **Recompilar matrices-open-mp**
8. **`./exec-cluster-pthreads.sh`** → Ejecutar T={2,4,8} con ref
9. **`./exec-cluster-openmp.sh`** → Ejecutar T={2,4,8} con ref
10. **Recopilar y analizar todos los resultados**
