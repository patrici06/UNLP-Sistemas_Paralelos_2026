import matplotlib.pyplot as plt
from graficador import style
from graficador.loader import add_config_label, make_config_label
from graficador.graphs import graph


@graph("isoeficiencia", "Eficiencia vs N (iso-eficiencia, una línea por configuración)")
def generate(df, output_dir):
    df = add_config_label(df)
    setups = df['config'].unique()
    fig, ax = plt.subplots(figsize=(7, 4.5))

    for i, cfg in enumerate(setups):
        sub = df[df['config'] == cfg].sort_values('N')
        ax.plot(sub['N'], sub['eff'],
                color=style.SERIES_PALETTE[i % len(style.SERIES_PALETTE)],
                marker=style.MARKERS[i % len(style.MARKERS)],
                label=cfg)

    ax.set_xlabel('Tamaño de matriz N')
    ax.set_ylabel('Eficiencia (%)')
    ax.set_title('Eficiencia vs Tamaño del problema (Iso-eficiencia)')
    ax.legend()
    ax.set_xscale('log', base=2)

    style.save_plot(fig, f"{output_dir}/isoeficiencia")
