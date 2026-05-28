# Guía Rápida de Scripts - Cluster SOLO

## Ejecución en Cluster (ÚNICO MÉTODO)

Todos los scripts se ejecutan en el cluster mediante `sbatch`. No hay ejecución local.

### Paso 1: Obtener Referencia T=1

```bash
./exec-cluster-matrices-t1.sh   # Baseline matrices.c
./exec-cluster-pthreads-t1.sh   # Referencia pthread
./exec-cluster-openmp-t1.sh     # Referencia openmp
```

**Estructura de salida:**
```
salidas/matrices-t1/N{512,1024,2048,4096}/outputs/exec_*.txt
salidas/pthread-t1/N{512,1024,2048,4096}/outputs/exec_*.txt
salidas/openmp-t1/N{512,1024,2048,4096}/outputs/exec_*.txt
```

### Paso 2: Hardcodear ref_time_sequential

```bash
# Para pthread
PTHREAD_REF=$(grep "^RESULT;4096;1" salidas/pthread-t1/N4096/outputs/exec_*.txt | awk -F';' '{print $4}')
sed -i "s/static double ref_time_sequential = -1.0;/static double ref_time_sequential = $PTHREAD_REF;/" matrices-pthread.c
gcc -O2 -pthread -o matrices-pthread matrices-pthread.c -lm

# Para openmp
OPENMP_REF=$(grep "^RESULT;4096;1" salidas/openmp-t1/N4096/outputs/exec_*.txt | awk -F';' '{print $4}')
sed -i "s/static double ref_time_sequential = -1.0;/static double ref_time_sequential = $OPENMP_REF;/" matrices-open-mp.c
gcc -O2 -fopenmp -o matrices-open-mp matrices-open-mp.c -lm
```

### Paso 3: Ejecutar Benchmark con T={2,4,8}

```bash
./exec-cluster-pthreads.sh   # Ejecuta T={1,2,4,8}
./exec-cluster-openmp.sh     # Ejecuta T={1,2,4,8}
```

**Estructura de salida:**
```
salidas/pthread/N{512,1024,2048,4096}-T{1,2,4,8}/outputs/exec_*.txt
salidas/openmp/N{512,1024,2048,4096}-T{1,2,4,8}/outputs/exec_*.txt
```

---

## Monitoreo

```bash
# Ver jobs activos
squeue -u $USER

# Ver output en tiempo real
tail -f salidas/pthread-t1/N4096/outputs/exec_*.txt

# Esperar a que terminen todos
while squeue -u $USER | grep -q "RUNNING\|PENDING"; do sleep 10; done
```

---

## Extracción de Resultados

```bash
# Todos los RESULT
find salidas -name "exec_*.txt" -exec grep "^RESULT" {} +

# Solo pthread
grep "^RESULT" salidas/pthread-t1/N*/outputs/exec_*.txt
grep "^RESULT" salidas/pthread/N*-T*/outputs/exec_*.txt

# Solo openmp
grep "^RESULT" salidas/openmp-t1/N*/outputs/exec_*.txt
grep "^RESULT" salidas/openmp/N*-T*/outputs/exec_*.txt

# Generar CSV
cat > resultados.csv << 'EOF'
Program,N,T,workTime,GFLOPS,speedup,efficiency,overhead,overhead%
EOF
find salidas -name "exec_*.txt" -exec grep "^RESULT" {} + | sed 's/^RESULT;//' | while read line; do
    prog=$(echo $line | awk '{print substr($0,1,3)}')
    echo "$prog;$line" >> resultados.csv
done
```

---

## Archivos de Documentación

| Archivo | Descripción |
|---------|-------------|
| `GUIDE_T1_REFERENCE.md` | Guía completa para calcular referencias |
| `README_SCRIPTS_INDEPENDIENTES.md` | Explicación de estructura de scripts |
| `INSTRUCCIONES_TEST.md` | Formato de salida y campos |
| `HARDCODE_REFERENCE.md` | Cómo hardcodear ref_time |

---

## Resumen Rápido

```bash
# 1. Baselines (5-20 min cada uno)
./exec-cluster-matrices-t1.sh
./exec-cluster-pthreads-t1.sh
./exec-cluster-openmp-t1.sh

# 2. Hardcodear refs (manual)
PTHREAD_REF=...
OPENMP_REF=...

# 3. Benchmark (15-30 min cada uno)
./exec-cluster-pthreads.sh
./exec-cluster-openmp.sh

# 4. Analizar
find salidas -name "exec_*.txt" -exec grep "^RESULT" {} +
```
