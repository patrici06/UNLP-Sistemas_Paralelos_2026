import os
import importlib
import pkgutil

_graphs = {}
_choices = []


def graph(name, description):
    def decorator(func):
        _graphs[name] = func
        _choices.append((name, description))
        return func
    return decorator


def get_all():
    return dict(_graphs)


def get_choices():
    return list(_choices)


def run(name, *args, **kwargs):
    if name in _graphs:
        return _graphs[name](*args, **kwargs)
    raise KeyError(f"Gráfico '{name}' no encontrado")


def discover():
    pkg_dir = os.path.dirname(__file__)
    for _, module_name, _ in pkgutil.iter_modules([pkg_dir]):
        if module_name != '__init__':
            importlib.import_module(f'.{module_name}', __package__)
