import matplotlib.pyplot as plt
import numpy as np
from graficador import style
from graficador.graphs import graph


@graph("comparativa_hybrid", "Comparativa MPI vs Hybrid P=16,32 (barras lado a lado)")
def generate(df, output_dir):
    ns = sorted(df['N'].unique())
    fig, axes = plt.subplots(1, 2, figsize=(12, 4.5), sharex=True)

    metricas = [
        ('time_s', 'Tiempo (s)'),
        ('speedup', 'Speedup'),
    ]

    for ax_idx, (metrica, ylabel) in enumerate(metricas):
        ax = axes[ax_idx]
        n_groups = len(ns)
        width = 0.35

        for bar_idx, total in enumerate([16, 32]):
            mpi_sub = df[(df['tipo'] == 'mpi') & (df['total_workers'] == total)]
            hybrid_sub = df[(df['tipo'] == 'hybrid') & (df['total_workers'] == total)]

            mpi_vals = []
            hybrid_vals = []
            for n in ns:
                m_row = mpi_sub[mpi_sub['N'] == n]
                h_row = hybrid_sub[hybrid_sub['N'] == n]
                mpi_vals.append(m_row[metrica].values[0] if not m_row.empty else 0)
                hybrid_vals.append(h_row[metrica].values[0] if not h_row.empty else 0)

            offset = bar_idx * 3 * width
            ax.bar(np.arange(n_groups) + offset, mpi_vals, width,
                   label=f'MPI P={total}' if ax_idx == 0 else None,
                   color=style.COLORS['MPI'], alpha=0.8)
            ax.bar(np.arange(n_groups) + offset + width, hybrid_vals, width,
                   label=f'Hybrid P={total}' if ax_idx == 0 else None,
                   color=style.COLORS['Hybrid'], alpha=0.8)

        ax.set_xlabel('Tamaño de matriz N')
        ax.set_ylabel(ylabel)
        ax.set_title(f'{ylabel} — MPI vs Hybrid')
        ax.set_xticks(np.arange(n_groups) + width / 2)
        ax.set_xticklabels([str(n) for n in ns])

    handles, labels = axes[0].get_legend_handles_labels()
    fig.legend(handles, labels, loc='upper center', ncol=4, fontsize=9)
    fig.suptitle('Comparativa MPI puro vs Híbrido MPI+OpenMP', y=1.02)
    plt.tight_layout()

    style.save_plot(fig, f"{output_dir}/comparativa_hybrid")
