#!/usr/bin/env python3
"""
Actualiza CSV de matrices con speedup y eficiencia calculados
"""

import csv

# Leer resultados_comparativo.csv
datos = []
with open('resultados_comparativo.csv', 'r') as f:
    reader = csv.DictReader(f)
    for row in reader:
        if row['Algorithm'] == 'matrices':
            # Para matrices (secuencial): speedup = 1.0, efficiency = 100.0
            row['speedup'] = '1.000000'
            row['efficiency'] = '100.000000'
            row['overhead'] = '0.000000'
            row['overhead%'] = '0.000000'
        datos.append(row)

# Escribir resultados_matrices actualizado
with open('resultados_matrices.csv', 'w', newline='') as f:
    writer = csv.DictWriter(f, fieldnames=[
        'N', 'T', 'Algorithm', 'workTime', 'GFLOPS',
        'speedup', 'efficiency', 'overhead', 'overhead%'
    ])
    writer.writeheader()
    for row in datos:
        if row['Algorithm'] == 'matrices':
            writer.writerow(row)

print("✅ resultados_matrices.csv actualizado con:\n")
print("N,T,Algorithm,workTime,GFLOPS,speedup,efficiency,overhead,overhead%\n")

with open('resultados_matrices.csv', 'r') as f:
    for line in f:
        print(line.rstrip())

# Actualizar también resultados_comparativo.csv
with open('resultados_comparativo.csv', 'w', newline='') as f:
    writer = csv.DictWriter(f, fieldnames=[
        'N', 'T', 'Algorithm', 'workTime', 'GFLOPS',
        'speedup', 'efficiency', 'overhead', 'overhead%'
    ])
    writer.writeheader()
    writer.writerows(datos)

print("\n✅ resultados_comparativo.csv también actualizado")

print("\n📊 Resumen de matrices.c:")
print("   - Algoritmo: Secuencial (T=1)")
print("   - Speedup: 1.0 (baseline)")
print("   - Efficiency: 100% (por definición)")
print("   - Overhead: 0.0s (sin paralelización)")
