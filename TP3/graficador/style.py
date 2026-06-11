import os
import shutil
import matplotlib as mpl
import matplotlib.pyplot as plt

COLORS = {
    'MPI': '#1f77b4',
    'Hybrid': '#d62728',
    'Pthreads': '#2ca02c',
    'OpenMP': '#ff7f0e',
    'sequential': '#7f7f7f',
}

SERIES_PALETTE = [
    '#1f77b4', '#ff7f0e', '#2ca02c', '#d62728',
    '#9467bd', '#8c564b', '#e377c2', '#7f7f7f',
    '#bcbd22', '#17becf',
]

MARKERS = ['o', 's', 'D', '^', 'v', '<', '>', 'p', '*', 'h']

HATCHES = ['/', '\\', 'x', '.', '+', '-', '|', 'o', 'O', '*']

_has_latex = shutil.which('pdflatex') is not None


def setup_style():
    base_style = {
        'font.family': 'sans-serif',
        'font.sans-serif': ['DejaVu Sans'],
        'font.size': 10,
        'axes.labelsize': 11,
        'axes.titlesize': 12,
        'legend.fontsize': 9,
        'xtick.labelsize': 9,
        'ytick.labelsize': 9,
        'figure.dpi': 150,
        'savefig.dpi': 300,
        'savefig.bbox': 'tight',
        'lines.linewidth': 1.5,
        'lines.markersize': 6,
        'axes.grid': True,
        'grid.alpha': 0.3,
        'grid.linestyle': '--',
    }

    if _has_latex:
        mpl.use('pgf')
        base_style.update({
            'pgf.texsystem': 'pdflatex',
            'pgf.rcfonts': False,
            'text.usetex': True,
        })
    else:
        mpl.use('Agg')

    mpl.rcParams.update(base_style)


def has_latex():
    return _has_latex


def save_plot(fig, filepath):
    fig.savefig(f"{filepath}.png", dpi=300)
    if _has_latex:
        fig.savefig(f"{filepath}.pgf")
    plt.close(fig)
