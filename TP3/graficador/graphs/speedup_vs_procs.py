import matplotlib.pyplot as plt
import numpy as np
from graficador import style
from graficador.loader import add_config_label
from graficador.graphs import graph


@graph("speedup_vs_procs", "Speedup vs procesos MPI (líneas, MPI y Hybrid separados)")
def generate(df, output_dir):
    df = add_config_label(df)
    fig, ax = plt.subplots(figsize=(7, 4.5))

    mpi = df[df['tipo'] == 'mpi'].sort_values('procs_mpi')
    hybrid = df[df['tipo'] == 'hybrid'].sort_values('procs_mpi')

    if not mpi.empty:
        ax.plot(mpi['procs_mpi'], mpi['speedup'],
                color=style.COLORS['MPI'], marker='o', linewidth=2,
                label='MPI puro')

    if not hybrid.empty:
        ax.plot(hybrid['procs_mpi'], hybrid['speedup'],
                color=style.COLORS['Hybrid'], marker='s', linewidth=2,
                label='Híbrido MPI+OpenMP')

        for _, row in hybrid.iterrows():
            ax.annotate(
                f"{int(row['omp_threads'])}OMP",
                (row['procs_mpi'], row['speedup']),
                textcoords="offset points",
                xytext=(0, 10),
                ha='center', fontsize=8,
                color=style.COLORS['Hybrid']
            )

    # Línea ideal (speedup = procs)
    max_procs = max(df['procs_mpi'].max(), df['total_workers'].max())
    procs_line = np.linspace(1, max_procs, 100)
    ax.plot(procs_line, procs_line, '--', color='gray', alpha=0.5,
            label='Speedup lineal ideal')

    ax.set_xlabel('Procesos MPI')
    ax.set_ylabel('Speedup')
    ax.set_title('Speedup vs Procesos MPI')
    ax.legend()

    style.save_plot(fig, f"{output_dir}/speedup_vs_procs")
