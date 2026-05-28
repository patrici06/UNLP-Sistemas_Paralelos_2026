# Cómo Hardcodear ref_time_sequential

## Flujo Recomendado para Benchmark

### 1. Ejecutar T=1 para todas las matrices

```bash
for n in 512 1024 2048 4096; do
    echo "=== Ejecutando N=$n T=1 ===" 
    ./matrices-pthread $n 1
    ./matrices-open-mp $n 1
done
```

Guardar los outputs:
```
N=512:   pthread: 0.0234  openmp: 0.0312
N=1024:  pthread: 0.0891  openmp: 0.1123
N=2048:  pthread: 0.4562  openmp: 0.5234
N=4096:  pthread: 1.8234  openmp: 2.1456
```

### 2. Hardcodear ref_time_sequential

Esto es importante porque cada job (T=1, T=2, T=4, T=8) es un proceso independiente, y la variable estática se reinicia.

#### Opción A: Editar el código (para pthread)

En `matrices-pthread.c`, línea ~50:
```c
// ANTES (variable dinámica):
static double ref_time_sequential = -1.0;

// DESPUÉS (hardcodeado para N=4096):
static double ref_time_sequential = 1.8234;  // Reemplazar con tu valor T=1
```

#### Opción B: Editar el código (para openmp)

En `matrices-open-mp.c`, línea ~42:
```c
// ANTES:
static double ref_time_sequential = -1.0;

// DESPUÉS:
static double ref_time_sequential = 2.1456;  // Valor de T=1 para N=4096
```

### 3. Recompilar con -O2

```bash
gcc -O2 -pthread -o matrices-pthread matrices-pthread.c -lm
gcc -O2 -fopenmp -o matrices-open-mp matrices-open-mp.c -lm
```

### 4. Ejecutar T={2, 4, 8}

Ahora cada ejecución con T>1 usará la referencia hardcodeada:

```bash
./matrices-pthread 4096 2  # Usará ref=1.8234
./matrices-pthread 4096 4  # Usará ref=1.8234
./matrices-pthread 4096 8  # Usará ref=1.8234
```

## Ventajas de Hardcodear

1. **Consistencia**: Todos los jobs T={2,4,8} usan la MISMA referencia T=1
2. **Independencia**: Cada job es independiente, no depende de orden de ejecución
3. **Benchmark limpio**: Mides paralelismo sin contaminación de inicialización

## Salida Esperada (Con Hardcode)

```
N=4096, T=1 (ejecutado primero):
RESULT;4096;1;1.8234;2.439;1.000000;100.000000;0.000000;0.000000

N=4096, T=2 (ejecutado después con ref=1.8234):
RESULT;4096;2;0.9456;4.728;1.927;96.35;0.0061;0.65

N=4096, T=4 (ejecutado después con ref=1.8234):
RESULT;4096;4;0.4912;9.102;3.710;92.75;0.0156;3.18

N=4096, T=8 (ejecutado después con ref=1.8234):
RESULT;4096;8;0.2677;16.574;6.802;85.02;0.0341;12.74
```

## Automatizar Hardcode

Si quieres un script que lo haga automáticamente:

```bash
#!/bin/bash
# Extraer ref_time_sequential de T=1
REF_TIME=$(./matrices-pthread $1 1 | grep "^RESULT" | awk -F';' '{print $4}')
echo "ref_time_sequential = $REF_TIME"

# Hardcodear en ambos códigos
sed -i "s/static double ref_time_sequential = -1.0;/static double ref_time_sequential = $REF_TIME;/" matrices-pthread.c
sed -i "s/static double ref_time_sequential = -1.0;/static double ref_time_sequential = $REF_TIME;/" matrices-open-mp.c

# Recompilar
gcc -O2 -pthread -o matrices-pthread matrices-pthread.c -lm
gcc -O2 -fopenmp -o matrices-open-mp matrices-open-mp.c -lm

echo "Compilación completada con ref_time_sequential = $REF_TIME"
```

## Workflow Completo (Recomendado)

```bash
#!/bin/bash
# Para cada tamaño N
for N in 512 1024 2048 4096; do
    echo "========== N=$N =========="
    
    # 1. Ejecutar T=1 para obtener referencia
    echo "Obteniendo referencia (T=1)..."
    REF_PTHREAD=$(./matrices-pthread $N 1 | grep "^RESULT" | awk -F';' '{print $4}')
    REF_OPENMP=$(./matrices-open-mp $N 1 | grep "^RESULT" | awk -F';' '{print $4}')
    echo "pthread ref: $REF_PTHREAD, openmp ref: $REF_OPENMP"
    
    # 2. Hardcodear referencias
    sed -i "s/static double ref_time_sequential = [0-9.]*;/static double ref_time_sequential = $REF_PTHREAD;/" matrices-pthread.c
    sed -i "s/static double ref_time_sequential = [0-9.]*;/static double ref_time_sequential = $REF_OPENMP;/" matrices-open-mp.c
    
    # 3. Recompilar
    gcc -O2 -pthread -o matrices-pthread matrices-pthread.c -lm
    gcc -O2 -fopenmp -o matrices-open-mp matrices-open-mp.c -lm
    
    # 4. Ejecutar T={2,4,8} (ahora con referencia correcta)
    for T in 2 4 8; do
        echo "  T=$T..."
        ./matrices-pthread $N $T | grep "^RESULT"
        ./matrices-open-mp $N $T | grep "^RESULT"
    done
    
    echo ""
done
```

## Información Importante

- **Variable**: `static double ref_time_sequential`
- **Ubicación (pthread)**: `matrices-pthread.c` línea ~50
- **Ubicación (openmp)**: `matrices-open-mp.c` línea ~42
- **Valor inicial**: `-1.0` (indica "no calculado")
- **Cambiar a**: El tiempo exacto de T=1 que obtuviste
