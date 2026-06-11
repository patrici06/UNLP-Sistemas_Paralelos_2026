import matplotlib.pyplot as plt
from graficador import style
from graficador.loader import add_config_label
from graficador.graphs import graph


@graph("speedup_vs_n", "Speedup vs N (líneas)")
def generate(df, output_dir):
    df = add_config_label(df)
    setups = df['config'].unique()
    fig, ax = plt.subplots(figsize=(7, 4.5))

    for i, cfg in enumerate(setups):
        sub = df[df['config'] == cfg].sort_values('N')
        ax.plot(sub['N'], sub['speedup'],
                color=style.SERIES_PALETTE[i % len(style.SERIES_PALETTE)],
                marker=style.MARKERS[i % len(style.MARKERS)],
                label=cfg)

    ax.set_xlabel('Tamaño de matriz N')
    ax.set_ylabel('Speedup')
    ax.set_title('Speedup vs Tamaño del problema')
    ax.legend()
    ax.set_xscale('log', base=2)

    style.save_plot(fig, f"{output_dir}/speedup_vs_n")
