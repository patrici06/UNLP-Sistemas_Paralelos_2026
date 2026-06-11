import matplotlib.pyplot as plt
import numpy as np
from graficador import style
from graficador.loader import add_config_label
from graficador.graphs import graph


@graph("tiempo_vs_n", "Tiempo de ejecución vs N (líneas)")
def generate(df, output_dir):
    df = add_config_label(df)
    setups = df['config'].unique()
    fig, ax = plt.subplots(figsize=(7, 4.5))

    for i, cfg in enumerate(setups):
        sub = df[df['config'] == cfg].sort_values('N')
        ax.plot(sub['N'], sub['time_s'],
                color=style.SERIES_PALETTE[i % len(style.SERIES_PALETTE)],
                marker=style.MARKERS[i % len(style.MARKERS)],
                label=cfg)

    ax.set_xlabel('Tamaño de matriz N')
    ax.set_ylabel('Tiempo (s)')
    ax.set_title('Tiempo de ejecución vs Tamaño del problema')
    ax.legend()
    ax.set_xscale('log', base=2)
    ax.set_yscale('log')

    style.save_plot(fig, f"{output_dir}/tiempo_vs_n")
