#!/usr/bin/env python3
"""
Genera CSVs independientes para cada algoritmo
"""

import csv

# Leer CSV comparativo
csv_file = 'resultados_comparativo.csv'
data = {}

with open(csv_file, 'r') as f:
    reader = csv.DictReader(f)
    for row in reader:
        algo = row['Algorithm']
        if algo not in data:
            data[algo] = []
        data[algo].append(row)

print("📊 Generando CSVs por algoritmo...\n")

# Generar CSV para cada algoritmo
for algo in ['matrices', 'pthread', 'openmp']:
    if algo in data:
        filename = f'resultados_{algo}.csv'
        
        with open(filename, 'w', newline='') as f:
            if data[algo]:
                writer = csv.DictWriter(f, fieldnames=data[algo][0].keys())
                writer.writeheader()
                writer.writerows(data[algo])
        
        print(f"✅ {filename}: {len(data[algo])} filas")

print("\n📁 Archivos creados:")
print("   - resultados_matrices.csv")
print("   - resultados_pthread.csv")
print("   - resultados_openmp.csv")
print("\n✨ Estructura idéntica al CSV comparativo, pero solo datos de cada algoritmo")
