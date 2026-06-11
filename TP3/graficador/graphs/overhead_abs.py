import matplotlib.pyplot as plt
from graficador import style
from graficador.loader import add_config_label
from graficador.graphs import graph


@graph("overhead_abs", "Tiempo absoluto de comunicación vs N (líneas)")
def generate(df, output_dir):
    df = add_config_label(df)
    df['comm_time_s'] = df['time_s'] * df['comm_pct'] / 100.0
    setups = df['config'].unique()
    fig, ax = plt.subplots(figsize=(7, 4.5))

    for i, cfg in enumerate(setups):
        sub = df[df['config'] == cfg].sort_values('N')
        ax.plot(sub['N'], sub['comm_time_s'],
                color=style.SERIES_PALETTE[i % len(style.SERIES_PALETTE)],
                marker=style.MARKERS[i % len(style.MARKERS)],
                label=cfg)

    ax.set_xlabel('Tamaño de matriz N')
    ax.set_ylabel('Tiempo de comunicación (s)')
    ax.set_title('Tiempo absoluto de comunicación vs Tamaño del problema')
    ax.legend()
    ax.set_xscale('log', base=2)
    ax.set_yscale('log')

    style.save_plot(fig, f"{output_dir}/overhead_abs")
