#!/usr/bin/env python3
"""
Limpia el CSV existente y genera uno comparativo ordenado
"""

import csv

# Leer el CSV mal formateado
datos = {}

print("📊 Limpiando y reorganizando datos...\n")

with open('resultados.csv', 'r') as f:
    lines = f.readlines()
    
    for line in lines[1:]:  # Skip header
        line = line.strip()
        if not line:
            continue
        
        # Extraer programa
        parts = line.split(',', 1)
        if len(parts) < 2:
            continue
        
        programa = parts[0]
        rest = parts[1]
        
        # Extraer RESULT
        if 'RESULT;' not in rest:
            continue
        
        result_part = rest.split('RESULT;')[1]
        result_values = result_part.split(';')
        
        # Parsear según el programa
        if programa == 'matrices':
            if len(result_values) >= 3:
                n = int(result_values[0])
                worktime = float(result_values[1])
                gflops = float(result_values[2])
                
                key = (n, 1, 'matrices')
                datos[key] = {
                    'workTime': worktime,
                    'GFLOPS': gflops,
                    'speedup': 'N/A',
                    'efficiency': 'N/A',
                    'overhead': 'N/A',
                    'overhead%': 'N/A'
                }
                print(f"✓ matrices N={n} T=1")
        
        else:  # pthread o openmp
            if len(result_values) >= 8:
                n = int(result_values[0])
                t = int(result_values[1])
                worktime = float(result_values[2])
                gflops = float(result_values[3])
                speedup = float(result_values[4])
                efficiency = float(result_values[5])
                overhead = float(result_values[6])
                overhead_pct = float(result_values[7])
                
                key = (n, t, programa)
                datos[key] = {
                    'workTime': worktime,
                    'GFLOPS': gflops,
                    'speedup': speedup,
                    'efficiency': efficiency,
                    'overhead': overhead,
                    'overhead%': overhead_pct
                }
                print(f"✓ {programa} N={n} T={t}")

print(f"\n✅ Datos parseados: {len(datos)} métricas\n")

# Crear CSV comparativo
csv_file = 'resultados_comparativo.csv'

with open(csv_file, 'w', newline='') as f:
    writer = csv.writer(f)
    
    # Headers
    writer.writerow([
        'N', 'T', 'Algorithm',
        'workTime', 'GFLOPS',
        'speedup', 'efficiency', 'overhead', 'overhead%'
    ])
    
    # Agrupar por N y T
    pares = {}
    for (n, t, algo), vals in datos.items():
        if (n, t) not in pares:
            pares[(n, t)] = {}
        pares[(n, t)][algo] = vals
    
    # Escribir ordenado
    for (n, t) in sorted(pares.keys()):
        for algo in ['matrices', 'pthread', 'openmp']:
            if algo in pares[(n, t)]:
                d = pares[(n, t)][algo]
                writer.writerow([
                    n, t, algo,
                    f"{d['workTime']:.6f}",
                    f"{d['GFLOPS']:.6f}",
                    d['speedup'] if d['speedup'] == 'N/A' else f"{d['speedup']:.6f}",
                    d['efficiency'] if d['efficiency'] == 'N/A' else f"{d['efficiency']:.6f}",
                    d['overhead'] if d['overhead'] == 'N/A' else f"{d['overhead']:.6f}",
                    d['overhead%'] if d['overhead%'] == 'N/A' else f"{d['overhead%']:.6f}"
                ])

print(f"💾 CSV generado: {csv_file}\n")
print("📄 Contenido:\n")

with open(csv_file, 'r') as f:
    reader = csv.reader(f)
    for i, row in enumerate(reader):
        if i < 30:
            print(f"   {','.join(row)}")

print(f"\n✨ Estructura:")
print(f"   - Cada fila es una combinación N, T, Algorithm")
print(f"   - Fácil comparar algoritmos para la misma N y T")
print(f"   - Ideal para gráficos: speedup vs T para cada N")
