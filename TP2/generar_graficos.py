#!/usr/bin/env python3
"""
Script para generar gráficos de comparación de métricas
Tiempo, Eficiencia, Speedup y Overhead por tamaño de matriz y número de hilos
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os

# Configuración de estilo
plt.style.use('default')
COLORS = {
    'pthread': '#1f77b4',   # Azul
    'openmp': '#ff7f0e',    # Naranja
    'matrices': '#2ca02c'   # Verde
}

def load_data():
    """Carga los datos de los CSV"""
    comparativo = pd.read_csv('/home/patricio/Escritorio/sistemas-paralelos/TP2/csv/resultados_comparativo.csv')
    return comparativo

def create_bar_chart(data, metric, ylabel, title, filename, scale_factor=1.0):
    """
    Crea un gráfico de barras tipo torres
    
    Args:
        data: DataFrame con los datos
        metric: Columna del dataframe a graficar
        ylabel: Etiqueta del eje Y
        title: Título del gráfico
        filename: Nombre del archivo de salida
        scale_factor: Factor de escala para el eje Y
    """
    # Obtener tamaños de matriz únicos
    sizes = sorted(data['N'].unique())
    threads = sorted(data['T'].unique())
    algorithms = sorted(data['Algorithm'].unique())
    
    # Crear figura
    fig, axes = plt.subplots(2, 2, figsize=(16, 12))
    axes = axes.flatten()
    
    for idx, size in enumerate(sizes):
        ax = axes[idx]
        
        # Filtrar datos por tamaño de matriz
        data_size = data[data['N'] == size]
        
        # Preparar datos para las barras
        x_pos = np.arange(len(threads))
        bar_width = 0.25
        
        n_algorithms = len(algorithms)
        if n_algorithms == 1:
            offset = 0
        else:
            offset = (n_algorithms - 1) / 2.0
        
        for algo_idx, algo in enumerate(algorithms):
            algo_data = data_size[data_size['Algorithm'] == algo]
            values = []
            
            for t in threads:
                row = algo_data[algo_data['T'] == t]
                if len(row) > 0:
                    val = row[metric].values[0]
                    if isinstance(val, str) and val == 'N/A':
                        val = 1.0 if metric == 'speedup' else 0.0 if metric in ['overhead', 'overhead%'] else 100.0
                    values.append(float(val) * scale_factor)
                else:
                    values.append(0)
            
            positions = x_pos + (algo_idx - offset) * bar_width
            
            bars = ax.bar(positions, values, bar_width, label=algo, color=COLORS.get(algo, '#999999'), alpha=0.8)
            
            for i, (pos, val) in enumerate(zip(positions, values)):
                if val > 0:
                    ax.text(pos, val, f'{val:.2f}', 
                           ha='center', va='bottom', fontsize=8, fontweight='bold')
        
        # Configurar eje X
        ax.set_xlabel('Número de Hilos', fontsize=11, fontweight='bold')
        ax.set_ylabel(ylabel, fontsize=11, fontweight='bold')
        ax.set_title(f'Matriz {size}x{size}', fontsize=12, fontweight='bold')
        ax.set_xticks(x_pos)
        ax.set_xticklabels(threads)
        ax.legend(loc='upper left', fontsize=9)
        ax.grid(axis='y', alpha=0.3, linestyle='--')
        
        # Ajustar límites del eje Y
        ax.set_ylim(bottom=0)
    
    # Título general
    fig.suptitle(title, fontsize=14, fontweight='bold', y=0.995)
    plt.tight_layout()
    
    # Guardar figura
    output_path = f'/home/patricio/Escritorio/sistemas-paralelos/TP2/graficos/{filename}'
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    print(f"Gráfico guardado: {output_path}")
    plt.close()

def main():
    """Función principal"""
    print("Cargando datos...")
    data = load_data()
    
    print("\nGenerando gráficos...")
    
    # Configuración: (metric_column, ylabel, prefix)
    graficos = [
        ('workTime', 'Tiempo (segundos)', '01_tiempo'),
        ('efficiency', 'Eficiencia (%)', '02_eficiencia'),
        ('speedup', 'Speedup', '03_speedup'),
        ('overhead%', 'Overhead (%)', '04_overhead'),
    ]
    
    for metric, ylabel, prefix in graficos:
        # Individuales: pthread y openmp
        for algo in ['pthread', 'openmp']:
            algo_data = data[data['Algorithm'] == algo]
            create_bar_chart(
                algo_data,
                metric=metric,
                ylabel=ylabel,
                title=f'{ylabel.split(" (")[0]} - {algo.upper()}',
                filename=f'{prefix}_{algo}.png'
            )
        
        # Comparativo: pthread vs openmp lado a lado
        comparativo = data[data['Algorithm'].isin(['pthread', 'openmp'])]
        create_bar_chart(
            comparativo,
            metric=metric,
            ylabel=ylabel,
            title=f'Comparación {ylabel.split(" (")[0]} - PTHREAD vs OPENMP',
            filename=f'comparativo_{prefix.split("_")[1]}.png'
        )
    
    print("\n✓ Todos los gráficos han sido generados exitosamente")

if __name__ == '__main__':
    main()
