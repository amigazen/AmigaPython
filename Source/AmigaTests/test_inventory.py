# Inventory: every module enabled in Modules/config.c.
# LoadSeg _socket is covered by optional group "socket" (test_socket_local).
# ASCII only (Python 2.7 / Amiga).

from __future__ import print_function

import sys
from AmigaTests.support import check, skip, try_import

# Always linked as builtins (see Modules/config.c).
ALWAYS_BUILTIN = (
    "amiga",
    "amigapath",
    "array",
    "math",
    "time",
    "binascii",
    "_codecs",
    "cStringIO",
    "cPickle",
    "errno",
    "operator",
    "_weakref",
    "_struct",
    "select",
    "zlib",
    "zipimport",
    "_sre",
    "md5",
    "cmath",
    "sha",
    "_hashlib",
    "_collections",
    "itertools",
    "_functools",
    "_random",
    "datetime",
    "_symtable",
    "_bisect",
    "_heapq",
    "_csv",
    "environment",
    "strop",
    "_arexx",
    "Doslib",
    "marshal",
    "imp",
    "_ast",
    "gc",
    "_warnings",
    "__main__",
    "__builtin__",
    "sys",
    "exceptions",
)

# Present in config.c when AMITCP is defined at build time.
AMITCP_BUILTIN = (
    "pwd",
    "grp",
    "crypt",
    "syslog",
)

def test_builtin_module_names():
    names = sys.builtin_module_names
    check("builtin_module_names tuple", isinstance(names, tuple) and len(names) > 0)
    for name in ALWAYS_BUILTIN:
        check("builtin " + name, name in names)


def test_amitcp_builtins():
    names = sys.builtin_module_names
    missing = [n for n in AMITCP_BUILTIN if n not in names]
    if missing:
        # Build without AMITCP: skip rather than fail.
        for n in missing:
            skip("builtin " + n, "not in this build")
        return
    for name in AMITCP_BUILTIN:
        check("builtin " + name, name in names)


def test_import_all_builtins():
    for name in ALWAYS_BUILTIN:
        if name in ("__main__", "__builtin__", "sys", "exceptions"):
            # Always present; import forms differ.
            continue
        mod = try_import(name)
        if name == "_hashlib" and mod is None:
            skip("import _hashlib", "needs crc.library 2+")
            continue
        check("import " + name, mod is not None)


def test_import_amitcp_modules():
    # Presence only. Importing pwd/grp/crypt/syslog opens usergroup/bsdsocket
    # and can hard-crash; runtime smoke is optional group "netmods".
    names = sys.builtin_module_names
    for name in AMITCP_BUILTIN:
        if name not in names:
            skip("builtin " + name, "not built")
        else:
            check("builtin listed " + name, True)
            skip("import " + name, "AmiTCP (optional netmods group)")


def test_socket_not_builtin():
    # Dynload exercise lives in optional "socket" group; inventory only
    # confirms gated _socket is not linked as a builtin.
    check("_socket not builtin", "_socket" not in sys.builtin_module_names)


def test_dynamic_loading_flag():
    # Compiling with HAVE_DYNAMIC_LOADING should expose dynload for .module.
    check("imp supports dynload hooks",
          hasattr(__import__("imp"), "load_module"))
    # Soft check: lib-dynload may be empty on a stripped install.
    import os
    found = False
    for entry in sys.path:
        if not entry:
            continue
        cand = os.path.join(entry, "lib-dynload")
        if os.path.isdir(cand):
            found = True
            break
        # Amiga: Lib may be .../lib with lib-dynload beside it
        parent = os.path.dirname(entry)
        cand2 = os.path.join(parent, "lib-dynload")
        if parent and os.path.isdir(cand2):
            found = True
            break
    if found:
        check("lib-dynload on tree", True)
    else:
        skip("lib-dynload dir", "not found beside Lib (ok if installed differently)")
