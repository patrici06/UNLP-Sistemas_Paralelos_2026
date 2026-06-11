import matplotlib.pyplot as plt
import numpy as np
from graficador import style
from graficador.loader import load_tp2
from graficador.graphs import graph


@graph("comparativa_tp2", "Comparativa TP3 MPI P=8 vs TP2 (barras lado a lado)")
def generate(df, output_dir):
    tp2_df = load_tp2()
    if tp2_df is None or tp2_df.empty:
        print("  [SKIP] No hay datos de TP2 (data/tp2_resultados.csv)")
        return

    ns = sorted(df['N'].unique())
    mpi_p8 = df[(df['tipo'] == 'mpi') & (df['procs_mpi'] == 8)]

    fig, axes = plt.subplots(1, 2, figsize=(12, 4.5), sharex=True)

    metricas = [
        ('time_s', 'Tiempo (s)'),
        ('speedup', 'Speedup'),
    ]

    for ax_idx, (metrica, ylabel) in enumerate(metricas):
        ax = axes[ax_idx]
        n_groups = len(ns)
        width = 0.25

        mpi_vals = []
        for n in ns:
            row = mpi_p8[mpi_p8['N'] == n]
            mpi_vals.append(row[metrica].values[0] if not row.empty else 0)

        tp2_types = tp2_df['tipo'].unique()
        tp2_data = {}
        for t in tp2_types:
            tp2_data[t] = []
            for n in ns:
                row = tp2_df[(tp2_df['tipo'] == t) & (tp2_df['N'] == n)]
                tp2_data[t].append(row[metrica].values[0] if not row.empty else 0)

        colors = [style.COLORS['MPI']]
        labels = ['MPI P=8']
        all_vals = [mpi_vals]

        for t in tp2_types:
            colors.append(style.SERIES_PALETTE[len(all_vals) % len(style.SERIES_PALETTE)])
            labels.append(t)
            all_vals.append(tp2_data[t])

        for i, (vals, color, label) in enumerate(zip(all_vals, colors, labels)):
            offset = (i - len(all_vals) / 2 + 0.5) * width
            ax.bar(np.arange(n_groups) + offset, vals, width,
                   label=label if ax_idx == 0 else None,
                   color=color, alpha=0.8,
                   hatch=style.HATCHES[i % len(style.HATCHES)])

        ax.set_xlabel('Tamaño de matriz N')
        ax.set_ylabel(ylabel)
        ax.set_title(f'{ylabel} — TP3 (MPI) vs TP2')
        ax.set_xticks(np.arange(n_groups))
        ax.set_xticklabels([str(n) for n in ns])

    handles, labels = axes[0].get_legend_handles_labels()
    fig.legend(handles, labels, loc='upper center', ncol=4, fontsize=9)
    fig.suptitle('Comparativa MPI P=8 (TP3) vs TP2', y=1.02)
    plt.tight_layout()

    style.save_plot(fig, f"{output_dir}/comparativa_tp2")
