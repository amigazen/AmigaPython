# OS4 amigavars API, backed by the builtin environment module.
# ASCII only (Python 2.7 / Amiga).
#
# GetEnv(name) -> value or None
# SetEnv(name, value, save=0)  -- save True writes ENVARC:
# UnSetEnv(name, delete=0)     -- delete True removes ENVARC:

from environment import GetEnv, SetEnv, UnSetEnv

__all__ = ["GetEnv", "SetEnv", "UnSetEnv"]
