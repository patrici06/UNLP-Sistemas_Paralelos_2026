#!/usr/bin/env python3
"""
Generate benchmark reports and comparative visualizations
Processes benchmark results from bs32 and bs64 directories
"""

import os
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from pathlib import Path
import glob

# Set style for better-looking plots
sns.set_style("whitegrid")
plt.rcParams['figure.figsize'] = (14, 8)

def extract_benchmark_data(base_path):
    """
    Extract benchmark data from directory structure
    Returns a list of dictionaries with benchmark results
    """
    data = []
    
    # Find all resultados-* directories
    results_dirs = glob.glob(os.path.join(base_path, '**/resultados-*'), recursive=True)
    
    for results_dir in sorted(results_dirs):
        # Extract bitness (32 or 64) and compilation method
        dir_parts = results_dir.split(os.sep)
        bitness = [p for p in dir_parts if p in ['bs32', 'bs64']][0]
        compilation_method = os.path.basename(results_dir).replace('resultados-', '')
        
        # Find all output_*.txt files
        output_files = glob.glob(os.path.join(results_dir, 'output_*.txt'))
        
        for output_file in output_files:
            with open(output_file, 'r') as f:
                lines = f.readlines()
                for line in lines:
                    if line.startswith('RESULT'):
                        parts = line.strip().split(';')
                        if len(parts) >= 4:
                            data.append({
                                'Bitness': bitness,
                                'Compilation': compilation_method,
                                'Entry_Size': int(parts[1]),
                                'Time_Output': float(parts[2]),
                                'GFLOPS': float(parts[3])
                            })
    
    return data

def main():
    base_path = '/home/patricio/Escritorio/sistemas-paralelos/practica-1/cluster'
    
    print("=" * 70)
    print("BENCHMARK ANALYSIS - REPORT GENERATION")
    print("=" * 70)
    
    # Extract data
    print("\n[1/3] Extracting benchmark data...")
    data = extract_benchmark_data(base_path)
    df = pd.DataFrame(data)
    
    if df.empty:
        print("ERROR: No data found!")
        return
    
    print(f"✓ Found {len(df)} benchmark results")
    
    # Create comprehensive CSV report
    print("\n[2/3] Generating CSV reports...")
    
    # Report 1: All data with summary statistics
    df_sorted = df.sort_values(['Bitness', 'Compilation', 'Entry_Size'])
    report_file = os.path.join(base_path, 'benchmark_report.csv')
    df_sorted.to_csv(report_file, index=False)
    print(f"✓ Detailed report: {report_file}")
    
    # Report 2: Summary by compilation method and entry size
    summary = df.groupby(['Bitness', 'Compilation', 'Entry_Size']).agg({
        'Time_Output': ['mean', 'std', 'min', 'max'],
        'GFLOPS': ['mean', 'std', 'min', 'max']
    }).round(6)
    
    summary_file = os.path.join(base_path, 'benchmark_summary.csv')
    summary.to_csv(summary_file)
    print(f"✓ Summary report: {summary_file}")
    
    # Print summary statistics
    print("\n" + "=" * 70)
    print("SUMMARY STATISTICS BY COMPILATION METHOD")
    print("=" * 70)
    for bitness in ['bs32', 'bs64']:
        df_bit = df[df['Bitness'] == bitness]
        if not df_bit.empty:
            print(f"\n{bitness.upper()}:")
            for comp in sorted(df_bit['Compilation'].unique()):
                df_comp = df_bit[df_bit['Compilation'] == comp]
                avg_time = df_comp['Time_Output'].mean()
                avg_gflops = df_comp['GFLOPS'].mean()
                print(f"  {comp:20s} - Avg Time: {avg_time:.6f}s | Avg GFLOPS: {avg_gflops:.6f}")
    
    # Generate visualizations
    print("\n[3/3] Generating comparative visualizations...")
    
    # Graph 1: Time comparison by entry size (BS32 and BS64 together)
    fig, axes = plt.subplots(1, 2, figsize=(16, 6))
    
    for idx, bitness in enumerate(['bs32', 'bs64']):
        df_bit = df[df['Bitness'] == bitness]
        ax = axes[idx]
        
        for comp in sorted(df_bit['Compilation'].unique()):
            df_comp = df_bit[df_bit['Compilation'] == comp]
            df_comp_sorted = df_comp.sort_values('Entry_Size')
            ax.plot(df_comp_sorted['Entry_Size'], df_comp_sorted['Time_Output'], 
                   marker='o', label=comp, linewidth=2, markersize=8)
        
        ax.set_xlabel('Entry Size (n)', fontsize=11, fontweight='bold')
        ax.set_ylabel('Execution Time (seconds)', fontsize=11, fontweight='bold')
        ax.set_title(f'Execution Time Comparison - {bitness.upper()}', fontsize=12, fontweight='bold')
        ax.legend(loc='best')
        ax.grid(True, alpha=0.3)
        ax.set_xscale('log')
    
    plt.tight_layout()
    graph1_file = os.path.join(base_path, 'graph_1_time_comparison.png')
    plt.savefig(graph1_file, dpi=300, bbox_inches='tight')
    print(f"✓ Graph 1 (Time comparison): {graph1_file}")
    plt.close()
    
    # Graph 2: GFLOPS comparison by entry size
    fig, axes = plt.subplots(1, 2, figsize=(16, 6))
    
    for idx, bitness in enumerate(['bs32', 'bs64']):
        df_bit = df[df['Bitness'] == bitness]
        ax = axes[idx]
        
        for comp in sorted(df_bit['Compilation'].unique()):
            df_comp = df_bit[df_bit['Compilation'] == comp]
            df_comp_sorted = df_comp.sort_values('Entry_Size')
            ax.plot(df_comp_sorted['Entry_Size'], df_comp_sorted['GFLOPS'], 
                   marker='s', label=comp, linewidth=2, markersize=8)
        
        ax.set_xlabel('Entry Size (n)', fontsize=11, fontweight='bold')
        ax.set_ylabel('GFLOPS', fontsize=11, fontweight='bold')
        ax.set_title(f'GFLOPS Comparison - {bitness.upper()}', fontsize=12, fontweight='bold')
        ax.legend(loc='best')
        ax.grid(True, alpha=0.3)
        ax.set_xscale('log')
    
    plt.tight_layout()
    graph2_file = os.path.join(base_path, 'graph_2_gflops_comparison.png')
    plt.savefig(graph2_file, dpi=300, bbox_inches='tight')
    print(f"✓ Graph 2 (GFLOPS comparison): {graph2_file}")
    plt.close()
    
    # Graph 3: Box plot of execution times by compilation method
    fig, axes = plt.subplots(1, 2, figsize=(16, 6))
    
    for idx, bitness in enumerate(['bs32', 'bs64']):
        df_bit = df[df['Bitness'] == bitness]
        ax = axes[idx]
        
        sns.boxplot(data=df_bit, x='Compilation', y='Time_Output', ax=ax, palette='Set2')
        ax.set_xlabel('Compilation Method', fontsize=11, fontweight='bold')
        ax.set_ylabel('Execution Time (seconds)', fontsize=11, fontweight='bold')
        ax.set_title(f'Time Distribution by Compilation Method - {bitness.upper()}', fontsize=12, fontweight='bold')
        ax.tick_params(axis='x', rotation=45)
    
    plt.tight_layout()
    graph3_file = os.path.join(base_path, 'graph_3_boxplot_times.png')
    plt.savefig(graph3_file, dpi=300, bbox_inches='tight')
    print(f"✓ Graph 3 (Time distribution): {graph3_file}")
    plt.close()
    
    # Graph 4: Heatmap of average times
    fig, axes = plt.subplots(1, 2, figsize=(16, 5))
    
    for idx, bitness in enumerate(['bs32', 'bs64']):
        df_bit = df[df['Bitness'] == bitness]
        pivot_time = df_bit.pivot_table(values='Time_Output', 
                                        index='Compilation', 
                                        columns='Entry_Size', 
                                        aggfunc='mean')
        
        ax = axes[idx]
        sns.heatmap(pivot_time, annot=True, fmt='.4f', cmap='YlOrRd', ax=ax, cbar_kws={'label': 'Time (s)'})
        ax.set_title(f'Average Execution Time Heatmap - {bitness.upper()}', fontsize=12, fontweight='bold')
        ax.set_xlabel('Entry Size (n)', fontsize=11, fontweight='bold')
        ax.set_ylabel('Compilation Method', fontsize=11, fontweight='bold')
    
    plt.tight_layout()
    graph4_file = os.path.join(base_path, 'graph_4_heatmap_times.png')
    plt.savefig(graph4_file, dpi=300, bbox_inches='tight')
    print(f"✓ Graph 4 (Time heatmap): {graph4_file}")
    plt.close()
    
    # Graph 5: Performance speedup relative to baseline
    fig, axes = plt.subplots(1, 2, figsize=(16, 6))
    
    for idx, bitness in enumerate(['bs32', 'bs64']):
        df_bit = df[df['Bitness'] == bitness]
        ax = axes[idx]
        
        # Use first compilation method as baseline
        baseline_comp = sorted(df_bit['Compilation'].unique())[0]
        df_baseline = df_bit[df_bit['Compilation'] == baseline_comp].set_index('Entry_Size')['Time_Output']
        
        for comp in sorted(df_bit['Compilation'].unique()):
            df_comp = df_bit[df_bit['Compilation'] == comp]
            df_comp_sorted = df_comp.sort_values('Entry_Size')
            
            speedup = []
            sizes = []
            for _, row in df_comp_sorted.iterrows():
                if row['Entry_Size'] in df_baseline.index:
                    speedup.append(df_baseline[row['Entry_Size']] / row['Time_Output'])
                    sizes.append(row['Entry_Size'])
            
            ax.plot(sizes, speedup, marker='o', label=comp, linewidth=2, markersize=8)
        
        ax.axhline(y=1, color='red', linestyle='--', linewidth=1, alpha=0.5, label='Baseline')
        ax.set_xlabel('Entry Size (n)', fontsize=11, fontweight='bold')
        ax.set_ylabel('Speedup (vs baseline)', fontsize=11, fontweight='bold')
        ax.set_title(f'Speedup Comparison - {bitness.upper()}', fontsize=12, fontweight='bold')
        ax.legend(loc='best')
        ax.grid(True, alpha=0.3)
        ax.set_xscale('log')
    
    plt.tight_layout()
    graph5_file = os.path.join(base_path, 'graph_5_speedup_comparison.png')
    plt.savefig(graph5_file, dpi=300, bbox_inches='tight')
    print(f"✓ Graph 5 (Speedup comparison): {graph5_file}")
    plt.close()
    
    print("\n" + "=" * 70)
    print("✓ REPORT GENERATION COMPLETE")
    print("=" * 70)
    print(f"\nGenerated files in: {base_path}")
    print("  - benchmark_report.csv (detailed results)")
    print("  - benchmark_summary.csv (statistical summary)")
    print("  - graph_1_time_comparison.png")
    print("  - graph_2_gflops_comparison.png")
    print("  - graph_3_boxplot_times.png")
    print("  - graph_4_heatmap_times.png")
    print("  - graph_5_speedup_comparison.png")

if __name__ == '__main__':
    main()
