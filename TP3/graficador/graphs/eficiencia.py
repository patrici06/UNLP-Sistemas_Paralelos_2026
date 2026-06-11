import matplotlib.pyplot as plt
import numpy as np
from graficador import style
from graficador.loader import add_config_label
from graficador.graphs import graph


@graph("eficiencia", "Eficiencia por configuración (barras agrupadas por N)")
def generate(df, output_dir):
    df = add_config_label(df)
    ns = sorted(df['N'].unique())
    setups = df['config'].unique()

    fig, ax = plt.subplots(figsize=(8, 4.5))

    n_groups = len(ns)
    n_setups = len(setups)
    width = 0.8 / n_setups

    for i, cfg in enumerate(setups):
        values = []
        for n in ns:
            row = df[(df['config'] == cfg) & (df['N'] == n)]
            if not row.empty:
                values.append(row['eff'].values[0])
            else:
                values.append(0)
        offset = (i - n_setups / 2 + 0.5) * width
        ax.bar(np.arange(n_groups) + offset, values, width,
               label=cfg,
               color=style.SERIES_PALETTE[i % len(style.SERIES_PALETTE)],
               hatch=style.HATCHES[i % len(style.HATCHES)],
               edgecolor='white')

    ax.set_xlabel('Tamaño de matriz N')
    ax.set_ylabel('Eficiencia (%)')
    ax.set_title('Eficiencia por configuración')
    ax.set_xticks(np.arange(n_groups))
    ax.set_xticklabels([str(n) for n in ns])
    ax.legend(fontsize=8)

    style.save_plot(fig, f"{output_dir}/eficiencia")
