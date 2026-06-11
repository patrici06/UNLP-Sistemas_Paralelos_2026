import pandas as pd
import os

DATA_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.realpath(__file__))), 'data')


def load_resultados(path=None):
    if path is None:
        path = os.path.join(DATA_DIR, 'resultados.csv')
    df = pd.read_csv(path)
    df.columns = df.columns.str.strip()
    df['comm_pct'] = pd.to_numeric(df['comm_pct'], errors='coerce')
    df['speedup'] = pd.to_numeric(df['speedup'], errors='coerce')
    df['eff'] = pd.to_numeric(df['eff'], errors='coerce')
    return df


def load_mpi(path=None):
    if path is None:
        path = os.path.join(DATA_DIR, 'mpi_resultados.csv')
    return load_resultados(path)


def load_hybrid(path=None):
    if path is None:
        path = os.path.join(DATA_DIR, 'hybrid_resultados.csv')
    return load_resultados(path)


def load_tp2(path=None):
    if path is None:
        path = os.path.join(DATA_DIR, 'tp2_resultados.csv')
    if os.path.exists(path):
        df = pd.read_csv(path)
        df['speedup'] = pd.to_numeric(df['speedup'], errors='coerce')
        df['eff'] = pd.to_numeric(df['eff'], errors='coerce')
        return df
    return None


def make_config_label(row):
    if row['tipo'] == 'mpi':
        return f"MPI P={int(row['procs_mpi'])}"
    else:
        return f"Hybrid {int(row['procs_mpi'])}MPI×{int(row['omp_threads'])}OMP"


def add_config_label(df):
    df = df.copy()
    df['config'] = df.apply(make_config_label, axis=1)
    return df


def check_data_exists():
    path = os.path.join(DATA_DIR, 'resultados.csv')
    if not os.path.exists(path):
        print(f"[ERROR] No existe {path}")
        print("  Ejecuta primero: script/generar_csv.sh")
        return False
    df = load_resultados()
    if df.empty:
        print("[ERROR] resultados.csv está vacío")
        return False
    return True
