#!/usr/bin/env python3
"""
Script para extraer resultados y consolidar en CSVs.
Detecta y elimina duplicados, manteniendo el registro con MAYOR TIEMPO (peor rendimiento).
Genera CSVs por: ALGORITMO × ENTORNO
"""

import os
import re
import sys
from pathlib import Path
import csv
from collections import defaultdict
import pandas as pd

# Configuración
BASE_DIR = Path(__file__).parent
SALIDAS_LOCAL = BASE_DIR / "salidas-local"
SALIDAS_CLUSTER = BASE_DIR / "salidas-cluster" / "salidas"
OUTPUT_DIR = BASE_DIR / "datos_consolidados"

# Crear directorio de salida
OUTPUT_DIR.mkdir(exist_ok=True)

def parse_result_file(filepath):
    """Extrae datos de un archivo de salida."""
    try:
        with open(filepath, 'r') as f:
            content = f.read().strip()
        
        result_match = re.search(r'RESULT;(\d+);([0-9.]+);([0-9.]+)', content)
        validation_match = re.search(r'VALIDATION;(\w+)', content)
        
        if result_match:
            size = int(result_match.group(1))
            time = float(result_match.group(2))
            gflops = float(result_match.group(3))
            validation = validation_match.group(1) if validation_match else "UNKNOWN"
            return {
                'size': size,
                'time': time,
                'gflops': gflops,
                'validation': validation
            }
    except Exception as e:
        print(f"Error procesando {filepath}: {e}", file=sys.stderr)
    
    return None

def extract_results_from_directory(root_dir):
    """Extrae resultados de un directorio raíz (local o cluster)."""
    results = defaultdict(list)
    
    if not root_dir.exists():
        return results
    
    # Iterar sobre cada algoritmo
    for impl_dir in root_dir.iterdir():
        if not impl_dir.is_dir():
            continue
        
        impl_name = impl_dir.name
        
        if impl_name == "matrices":
            # Estructura: N{size}/O{opt}/{outputs,reports}
            for size_dir in impl_dir.iterdir():
                if not size_dir.is_dir() or not size_dir.name.startswith('N'):
                    continue
                
                size = int(size_dir.name[1:])
                
                for opt_dir in size_dir.iterdir():
                    if not opt_dir.is_dir() or not opt_dir.name.startswith('O'):
                        continue
                    
                    opt_level = int(opt_dir.name[1:])
                    outputs_dir = opt_dir / "outputs"
                    
                    if outputs_dir.exists():
                        for output_file in outputs_dir.glob("*.txt"):
                            data = parse_result_file(output_file)
                            if data:
                                results[impl_name].append({
                                    'size': data['size'],
                                    'opt_level': opt_level,
                                    'time': data['time'],
                                    'gflops': data['gflops'],
                                    'validation': data['validation']
                                })
        
        elif impl_name in ["openmp", "pthreads"]:
            # Estructura: N{size}/T{threads}/O{opt}/{outputs,reports}
            for size_dir in impl_dir.iterdir():
                if not size_dir.is_dir() or not size_dir.name.startswith('N'):
                    continue
                
                size = int(size_dir.name[1:])
                
                for threads_dir in size_dir.iterdir():
                    if not threads_dir.is_dir() or not threads_dir.name.startswith('T'):
                        continue
                    
                    threads = int(threads_dir.name[1:])
                    
                    for opt_dir in threads_dir.iterdir():
                        if not opt_dir.is_dir() or not opt_dir.name.startswith('O'):
                            continue
                        
                        opt_level = int(opt_dir.name[1:])
                        outputs_dir = opt_dir / "outputs"
                        
                        if outputs_dir.exists():
                            for output_file in outputs_dir.glob("*.txt"):
                                data = parse_result_file(output_file)
                                if data:
                                    results[impl_name].append({
                                        'size': data['size'],
                                        'threads': threads,
                                        'opt_level': opt_level,
                                        'time': data['time'],
                                        'gflops': data['gflops'],
                                        'validation': data['validation']
                                    })
    
    return results

def remove_duplicates_keep_worst(records, impl_name):
    """
    Elimina duplicados, manteniendo el registro con MAYOR TIEMPO (peor rendimiento).
    """
    if impl_name == "matrices":
        group_keys = ['size', 'opt_level']
    else:  # openmp, pthreads
        group_keys = ['size', 'threads', 'opt_level']
    
    df = pd.DataFrame(records)
    
    # Agrupar por configuración
    grouped = df.groupby(group_keys, as_index=False)
    
    # Mantener el registro con mayor tiempo (peor rendimiento)
    deduplicated = grouped.apply(lambda g: g.loc[g['time'].idxmax()]).reset_index(drop=True)
    
    # Contar duplicados eliminados
    duplicates_removed = len(df) - len(deduplicated)
    
    return deduplicated.to_dict('records'), duplicates_removed

def save_csv(records, output_file, impl_name):
    """Guarda registros en CSV."""
    if not records:
        return 0
    
    with open(output_file, 'w', newline='') as f:
        if impl_name == "matrices":
            fieldnames = ['size', 'opt_level', 'time', 'gflops', 'validation']
            sort_key = lambda x: (x['size'], x['opt_level'])
        else:
            fieldnames = ['size', 'threads', 'opt_level', 'time', 'gflops', 'validation']
            sort_key = lambda x: (x['size'], x['threads'], x['opt_level'])
        
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        
        for record in sorted(records, key=sort_key):
            writer.writerow(record)
    
    return len(records)

def main():
    print("╔" + "=" * 78 + "╗")
    print("║" + "EXTRACCIÓN Y CONSOLIDACIÓN DE RESULTADOS".center(78) + "║")
    print("║" + "Limpieza de duplicados: MANTENER PEOR (mayor tiempo)".center(78) + "║")
    print("╚" + "=" * 78 + "╝")
    print()
    
    all_data = {}
    total_duplicates = 0
    
    # Procesar salidas-local
    print("Procesando salidas-local...")
    local_results = extract_results_from_directory(SALIDAS_LOCAL)
    
    if local_results:
        print(f"  Registros encontrados: {sum(len(v) for v in local_results.values())}")
        for impl_name in sorted(local_results.keys()):
            cleaned, dups = remove_duplicates_keep_worst(local_results[impl_name], impl_name)
            total_duplicates += dups
            
            filename = f"{impl_name}_local.csv"
            filepath = OUTPUT_DIR / filename
            count = save_csv(cleaned, filepath, impl_name)
            
            print(f"  ✓ {impl_name:<12} {count:3d} registros", end="")
            if dups > 0:
                print(f"  ({dups} duplicados eliminados)")
            else:
                print()
            
            if impl_name not in all_data:
                all_data[impl_name] = {}
            all_data[impl_name]['local_file'] = filepath
            all_data[impl_name]['local_count'] = count
    
    print()
    
    # Procesar salidas-cluster
    print("Procesando salidas-cluster...")
    cluster_results = extract_results_from_directory(SALIDAS_CLUSTER)
    
    if cluster_results:
        print(f"  Registros encontrados: {sum(len(v) for v in cluster_results.values())}")
        for impl_name in sorted(cluster_results.keys()):
            cleaned, dups = remove_duplicates_keep_worst(cluster_results[impl_name], impl_name)
            total_duplicates += dups
            
            filename = f"{impl_name}_cluster.csv"
            filepath = OUTPUT_DIR / filename
            count = save_csv(cleaned, filepath, impl_name)
            
            print(f"  ✓ {impl_name:<12} {count:3d} registros", end="")
            if dups > 0:
                print(f"  ({dups} duplicados eliminados)")
            else:
                print()
            
            if impl_name not in all_data:
                all_data[impl_name] = {}
            all_data[impl_name]['cluster_file'] = filepath
            all_data[impl_name]['cluster_count'] = count
    
    # Resumen
    print()
    print("=" * 80)
    print("RESUMEN DE CONSOLIDACIÓN")
    print("=" * 80)
    print()
    
    print(f"{'Algoritmo':<15} {'Local':<20} {'Cluster':<20}")
    print("-" * 80)
    
    total_records = 0
    for algo in sorted(all_data.keys()):
        local_count = all_data[algo].get('local_count', 0)
        cluster_count = all_data[algo].get('cluster_count', 0)
        total_records += local_count + cluster_count
        
        print(f"{algo:<15} {local_count:>3} registros        {cluster_count:>3} registros")
    
    print()
    print(f"Total de registros consolidados: {total_records}")
    print(f"Duplicados eliminados (mantener peor): {total_duplicates}")
    print()
    print(f"Directorio: {OUTPUT_DIR}")
    print("=" * 80)
    
    # Listar archivos
    print()
    print("Archivos CSV generados:")
    csv_files = sorted(OUTPUT_DIR.glob("*.csv"))
    for csv_file in csv_files:
        size_kb = csv_file.stat().st_size / 1024
        record_count = sum(1 for line in open(csv_file)) - 1
        print(f"  ✓ {csv_file.name:<35} {record_count:>3} registros ({size_kb:>6.2f} KB)")

if __name__ == "__main__":
    main()
