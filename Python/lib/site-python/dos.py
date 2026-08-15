"""
Legacy Dos module name for classic scripts.

Prefer:  import amiga

Dos.library helpers, ArgParser, and FIBF_* constants live on the builtin
amiga module (see site-python/_amigados.py, loaded by site.py).
"""
import amiga as _amiga

for _name in dir(_amiga):
    if not _name.startswith('__'):
        globals()[_name] = getattr(_amiga, _name)

del _amiga, _name
