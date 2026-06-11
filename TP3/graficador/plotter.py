#!/usr/bin/env python3
"""
CLI interactivo para generación de gráficos de rendimiento.

Uso:
  cd graficador && python3 -m plotter
  # o desde TP3/:
  python3 -m graficador.plotter

Antes:
  1. Ejecutar benchmarks (sbatch runs/runP*.sh)
  2. Generar CSVs: script/generar_csv.sh
"""

import os
import sys
import subprocess
from pathlib import Path

SCRIPT_DIR = Path(__file__).parent.resolve()
BASE_DIR = SCRIPT_DIR.parent
sys.path.insert(0, str(BASE_DIR))

from graficador import graphs
from graficador.loader import load_resultados, check_data_exists

DATA_DIR = BASE_DIR / 'data'
PLOTS_DIR = SCRIPT_DIR / 'plots'
GENERAR_CSV = BASE_DIR / 'script' / 'generar_csv.sh'

PLOTS_DIR.mkdir(parents=True, exist_ok=True)


def clear_screen():
    os.system('clear' if os.name == 'posix' else 'cls')


def print_header():
    print("╔══════════════════════════════════════════════╗")
    print("║      GRAFICADOR DE RENDIMIENTO — TP3       ║")
    print("╠══════════════════════════════════════════════╣")
    print("║  Análisis de rendimiento MPI e Híbrido      ║")
    print("╚══════════════════════════════════════════════╝")


def run_generar_csv():
    if not GENERAR_CSV.exists():
        print(f"\n[ERROR] No se encuentra: {GENERAR_CSV}")
        return
    print("\n[Ejecutando generar_csv.sh...]")
    result = subprocess.run(['bash', str(GENERAR_CSV)], capture_output=True, text=True)
    print(result.stdout)
    if result.stderr:
        print(result.stderr)


def generate_all(df):
    all_graphs = graphs.get_all()
    for name, func in all_graphs.items():
        print(f"  [{name}] Generando...", end=' ')
        try:
            func(df, str(PLOTS_DIR))
            print("OK")
        except Exception as e:
            print(f"ERROR: {e}")


def main():
    graphs.discover()

    while True:
        clear_screen()
        print_header()
        print()

        choices = graphs.get_choices()
        print("  ┌──── MENÚ ──────────────────────────────────┐")
        print("  │  0) Salir                                  │")
        print("  ├─── Datos ──────────────────────────────────┤")
        print("  │  1) Generar CSVs (desde times.txt)         │")
        print("  ├─── Gráficos ───────────────────────────────┤")

        for idx, (name, desc) in enumerate(choices, start=2):
            print(f"  │  {idx:2d}) {desc}")

        print("  ├─── Todos ──────────────────────────────────┤")
        print(f"  │  {len(choices) + 2:2d}) Todos los gráficos                    │")
        print("  └────────────────────────────────────────────┘")
        print()

        try:
            opt = int(input("  Seleccione una opción: ").strip())
        except (ValueError, EOFError):
            break

        print()

        if opt == 0:
            print("  Saliendo.")
            break

        if opt == 1:
            run_generar_csv()
            input("\n  Presione Enter para continuar...")
            continue

        choices_list = choices
        if opt == len(choices_list) + 2:
            if not check_data_exists():
                input("  Presione Enter para continuar...")
                continue
            df = load_resultados()
            print("  Generando todos los gráficos...")
            generate_all(df)
            print(f"\n  [OK] Gráficos guardados en: {PLOTS_DIR}/")
            input("\n  Presione Enter para continuar...")
            continue

        if 2 <= opt < 2 + len(choices_list):
            name, desc = choices_list[opt - 2]
            if not check_data_exists():
                input("  Presione Enter para continuar...")
                continue
            df = load_resultados()
            print(f"  [{name}] Generando...")
            try:
                graphs.run(name, df, str(PLOTS_DIR))
                print(f"  [OK] Gráfico guardado en: {PLOTS_DIR}/{name}.png")
            except Exception as e:
                print(f"  [ERROR] {e}")
            input("\n  Presione Enter para continuar...")
            continue

        print("  Opción inválida.")
        input("  Presione Enter para continuar...")


if __name__ == '__main__':
    main()
