#!/bin/bash

echo "============================================"
echo "TEST FINAL: Validación Completa"
echo "============================================"
echo ""

# Array de test cases: n:np:variant:omp
tests=(
    "128:2:mpi"        # Con validación
    "128:2:hybrid:2"   # Con validación
    "256:4:mpi"        # Solo compilación/ejecución
    "256:4:hybrid:2"   # Solo compilación/ejecución
    "512:4:mpi"        # Solo compilación/ejecución
    "512:4:hybrid:2"   # Solo compilación/ejecución
)

passed=0
failed=0

for test in "${tests[@]}"; do
    IFS=':' read -r n np variant omp <<< "$test"
    
    if [ "$variant" = "mpi" ]; then
        result=$(mpirun -np $np ./matrices-mpi $n 2>&1)
        label="MPI(n=$n,np=$np)"
    else
        export OMP_NUM_THREADS=$omp
        result=$(mpirun -np $np ./matrices-hybrid $n 2>&1)
        label="Hybrid(n=$n,np=$np,omp=$omp)"
    fi
    
    # Verificar que hay RESULT
    if echo "$result" | grep -q "^RESULT;"; then
        # Para n<=128, también debe haber VALIDATION;OK
        if [ $n -le 128 ]; then
            if echo "$result" | grep -q "^VALIDATION;OK"; then
                echo "✓ $label (con validación)"
                ((passed++))
            else
                echo "✗ $label (falta validación)"
                echo "  Output: $result"
                ((failed++))
            fi
        else
            # Para n>128, solo requerimos RESULT
            echo "✓ $label"
            ((passed++))
        fi
    else
        echo "✗ $label (sin RESULT)"
        echo "  Output: $result"
        ((failed++))
    fi
done

echo ""
echo "============================================"
echo "Resultados: $passed pasados, $failed fallidos"
echo "============================================"

if [ $failed -eq 0 ]; then
    echo "✅ TODOS LOS TESTS PASARON"
    exit 0
else
    echo "❌ ALGUNOS TESTS FALLARON"
    exit 1
fi
