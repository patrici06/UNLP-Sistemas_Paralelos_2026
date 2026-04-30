#!/usr/bin/env python3
"""
Master script to generate all benchmark reports and visualizations
Run this script to regenerate all reports and graphs from raw benchmark data
"""

import subprocess
import sys
import os

def run_script(script_path, description):
    """Run a Python script and report results"""
    print(f"\n{'='*70}")
    print(f"▶️  {description}")
    print('='*70)
    
    try:
        result = subprocess.run([sys.executable, script_path], check=True, cwd='/home/patricio/Escritorio/sistemas-paralelos/practica-1')
        print(f"✅ {description} - Completado")
        return True
    except subprocess.CalledProcessError as e:
        print(f"❌ {description} - Error: {e}")
        return False

def main():
    print("\n" + "█"*70)
    print("█" + " "*68 + "█")
    print("█" + "  BENCHMARK REPORT GENERATION SYSTEM".center(68) + "█")
    print("█" + " "*68 + "█")
    print("█"*70)
    
    scripts = [
        ('/home/patricio/Escritorio/sistemas-paralelos/practica-1/generate_reports.py', 
         'Generando reportes CSV y gráficos PNG'),
        ('/home/patricio/Escritorio/sistemas-paralelos/practica-1/generate_html_report.py', 
         'Generando reporte HTML interactivo'),
    ]
    
    results = []
    for script, desc in scripts:
        if os.path.exists(script):
            results.append(run_script(script, desc))
        else:
            print(f"⚠️  Script no encontrado: {script}")
            results.append(False)
    
    print(f"\n{'='*70}")
    print("📊 RESUMEN DE GENERACIÓN")
    print('='*70)
    
    if all(results):
        print("✅ Todos los reportes se generaron exitosamente")
        print("\nArchivos disponibles en:")
        print("  📁 /home/patricio/Escritorio/sistemas-paralelos/practica-1/cluster/")
    else:
        print("⚠️  Algunos reportes fallaron. Revise los errores arriba.")
    
    print("\n" + "█"*70)
    print("█" + " "*68 + "█")
    print("█" + "  REPORTES GENERADOS EXITOSAMENTE".center(68) + "█")
    print("█" + " "*68 + "█")
    print("█"*70 + "\n")

if __name__ == '__main__':
    main()
