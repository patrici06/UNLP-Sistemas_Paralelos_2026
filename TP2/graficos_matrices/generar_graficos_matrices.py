#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os

plt.style.use('default')

OUT_DIR = '/home/patricio/Escritorio/sistemas-paralelos/TP2/graficos_matrices'

def load_data():
    df = pd.read_csv('/home/patricio/Escritorio/sistemas-paralelos/TP2/csv/resultados_matrices.csv')
    df['N'] = df['N'].astype(int)
    return df

def plot_tiempo(data):
    fig, ax = plt.subplots(figsize=(10, 6))
    sizes = [f'{n}x{n}' for n in data['N']]
    x = np.arange(len(sizes))
    bars = ax.bar(x, data['workTime'], width=0.5, color='#2ca02c', alpha=0.85)
    for bar, val in zip(bars, data['workTime']):
        ax.text(bar.get_x() + bar.get_width()/2, bar.get_height(),
                f'{val:.3f}s', ha='center', va='bottom', fontsize=10, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(sizes)
    ax.set_xlabel('Tamaño de Matriz', fontsize=12, fontweight='bold')
    ax.set_ylabel('Tiempo (segundos)', fontsize=12, fontweight='bold')
    ax.set_title('Tiempo de Ejecución - matrices.c (secuencial)', fontsize=13, fontweight='bold')
    ax.grid(axis='y', alpha=0.3, linestyle='--')
    plt.tight_layout()
    out = os.path.join(OUT_DIR, 'matrices_tiempo.png')
    plt.savefig(out, dpi=300, bbox_inches='tight')
    print(f'Guardado: {out}')
    plt.close()

def plot_gflops(data):
    fig, ax = plt.subplots(figsize=(10, 6))
    sizes = [f'{n}x{n}' for n in data['N']]
    x = np.arange(len(sizes))
    bars = ax.bar(x, data['GFLOPS'], width=0.5, color='#1f77b4', alpha=0.85)
    for bar, val in zip(bars, data['GFLOPS']):
        ax.text(bar.get_x() + bar.get_width()/2, bar.get_height(),
                f'{val:.4f}', ha='center', va='bottom', fontsize=10, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(sizes)
    ax.set_xlabel('Tamaño de Matriz', fontsize=12, fontweight='bold')
    ax.set_ylabel('GFLOPS', fontsize=12, fontweight='bold')
    ax.set_title('Rendimiento (GFLOPS) - matrices.c (secuencial)', fontsize=13, fontweight='bold')
    ax.grid(axis='y', alpha=0.3, linestyle='--')
    ax.set_ylim(bottom=0)
    plt.tight_layout()
    out = os.path.join(OUT_DIR, 'matrices_gflops.png')
    plt.savefig(out, dpi=300, bbox_inches='tight')
    print(f'Guardado: {out}')
    plt.close()

def plot_tiempo_line(data):
    fig, ax = plt.subplots(figsize=(10, 6))
    ax.plot(data['N'], data['workTime'], 'o-', color='#2ca02c', linewidth=2.5, markersize=10)
    for _, row in data.iterrows():
        ax.text(row['N'], row['workTime'], f'{row["workTime"]:.3f}s',
                ha='center', va='bottom', fontsize=10, fontweight='bold')
    ax.set_xlabel('Tamaño de Matriz (N)', fontsize=12, fontweight='bold')
    ax.set_ylabel('Tiempo (segundos)', fontsize=12, fontweight='bold')
    ax.set_title('Escalabilidad del Tiempo - matrices.c (secuencial)', fontsize=13, fontweight='bold')
    ax.set_xticks(data['N'])
    ax.set_xticklabels([f'{n}x{n}' for n in data['N']])
    ax.grid(alpha=0.3, linestyle='--')
    plt.tight_layout()
    out = os.path.join(OUT_DIR, 'matrices_tiempo_escalabilidad.png')
    plt.savefig(out, dpi=300, bbox_inches='tight')
    print(f'Guardado: {out}')
    plt.close()

def plot_gflops_line(data):
    fig, ax = plt.subplots(figsize=(10, 6))
    ax.plot(data['N'], data['GFLOPS'], 's-', color='#1f77b4', linewidth=2.5, markersize=10)
    ax.axhline(y=0.55, color='red', linestyle='--', alpha=0.5, label='~0.55 GFLOPS (techo empírico)')
    for _, row in data.iterrows():
        ax.text(row['N'], row['GFLOPS'], f'{row["GFLOPS"]:.4f}',
                ha='center', va='bottom', fontsize=10, fontweight='bold')
    ax.set_xlabel('Tamaño de Matriz (N)', fontsize=12, fontweight='bold')
    ax.set_ylabel('GFLOPS', fontsize=12, fontweight='bold')
    ax.set_title('Rendimiento GFLOPS vs Tamaño - matrices.c (secuencial)', fontsize=13, fontweight='bold')
    ax.set_xticks(data['N'])
    ax.set_xticklabels([f'{n}x{n}' for n in data['N']])
    ax.legend(fontsize=10)
    ax.grid(alpha=0.3, linestyle='--')
    plt.tight_layout()
    out = os.path.join(OUT_DIR, 'matrices_gflops_escalabilidad.png')
    plt.savefig(out, dpi=300, bbox_inches='tight')
    print(f'Guardado: {out}')
    plt.close()

def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    data = load_data()
    print('Generando gráficos para matrices.c (secuencial)...')
    plot_tiempo(data)
    plot_gflops(data)
    plot_tiempo_line(data)
    plot_gflops_line(data)
    print('✓ Todos los gráficos de matrices.c generados')

if __name__ == '__main__':
    main()
