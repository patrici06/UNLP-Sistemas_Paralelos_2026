import matplotlib.pyplot as plt
from graficador import style
from graficador.loader import add_config_label
from graficador.graphs import graph


@graph("gflops_vs_n", "GFLOPS vs N (líneas)")
def generate(df, output_dir):
    df = add_config_label(df)
    setups = df['config'].unique()
    fig, ax = plt.subplots(figsize=(7, 4.5))

    for i, cfg in enumerate(setups):
        sub = df[df['config'] == cfg].sort_values('N')
        ax.plot(sub['N'], sub['gflops'],
                color=style.SERIES_PALETTE[i % len(style.SERIES_PALETTE)],
                marker=style.MARKERS[i % len(style.MARKERS)],
                label=cfg)

    ax.set_xlabel('Tamaño de matriz N')
    ax.set_ylabel('GFLOPS')
    ax.set_title('Rendimiento (GFLOPS) vs Tamaño del problema')
    ax.legend()
    ax.set_xscale('log', base=2)

    style.save_plot(fig, f"{output_dir}/gflops_vs_n")
