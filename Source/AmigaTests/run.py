#!/usr/bin/env python
# Run the Amiga Python 2.7.18 port test suite.
#
# Covers every module enabled in Modules/config.c for this port.
# Socket / remote TCP stays separate (AmigaTests/test_socket_net.py).
#
# From the Source directory (do not pass python -v; it floods stderr):
#   python27 AmigaTests/run.py
#   python27 AmigaTests/run.py runtime os_path
#   python27 AmigaTests/run.py -q
#   python27 AmigaTests/test_socket_net.py

from __future__ import print_function

import os
import sys

# Avoid rewriting .pyc on shared Mac/Amiga volumes (bad mtime spam).
sys.dont_write_bytecode = True

# Mute -v import tracing for the rest of the suite (stderr was interleaving
# with PASS lines). Early site/import noise still needs omitting -v on CLI.
if getattr(sys.flags, "verbose", 0):
    try:
        import amiga
        if hasattr(amiga, "set_verbose"):
            amiga.set_verbose(0)
    except Exception:
        pass

# Ensure Source root (parent of AmigaTests) is on sys.path.
_HERE = os.path.dirname(os.path.abspath(__file__))
_ROOT = os.path.dirname(_HERE)
if _ROOT not in sys.path:
    sys.path.insert(0, _ROOT)

from AmigaTests import support
from AmigaTests.support import run_module_tests, summary, reset_counters

# Default suite: offline / local only (matches enabled Amiga build modules).
# Never call AmiTCP/usergroup/bsdsocket APIs here - they hard-crash without a stack.
# AmiTCP / socket tests stay optional (bsdsocket may be absent or gated).
SUITE = [
    ("inventory", "AmigaTests.test_inventory"),
    ("runtime", "AmigaTests.test_runtime"),
    ("os_path", "AmigaTests.test_os_path"),
    ("amiga", "AmigaTests.test_amiga_module"),
    ("extras", "AmigaTests.test_amiga_extras"),
    ("builtins", "AmigaTests.test_builtins_ext"),
    ("tier_a", "AmigaTests.test_tier_a"),
]

# Not run by default. Prefer: python27 AmigaTests/test_socket_net.py
# netmods = pwd/grp/crypt/syslog; socket* = _socket LoadSeg / remote TCP.
# Optional: GUI requesters (pops ASL/EasyRequest - skip in headless runs).
OPTIONAL = [
    ("netmods", "AmigaTests.test_net_modules"),
    ("socket", "AmigaTests.test_socket_local"),
    ("socket_net", "AmigaTests.test_socket_net"),
    ("gui", "AmigaTests.test_amiga_gui"),
    ("ziplib", "AmigaTests.test_ziplib"),
]

ALL_GROUPS = SUITE + OPTIONAL


def _load(modname):
    return __import__(modname, fromlist=["*"])


def main(argv=None):
    if argv is None:
        argv = sys.argv[1:]

    quiet_banner = False
    names = []
    for a in argv:
        if a in ("-q", "--quiet"):
            quiet_banner = True
        elif a in ("-h", "--help"):
            print("Usage: python27 AmigaTests/run.py [group ...]")
            print("Do not pass python -v (import tracing floods the log).")
            print("Default groups:", ", ".join(n for n, _ in SUITE))
            print("Optional groups:", ", ".join(n for n, _ in OPTIONAL))
            return 0
        else:
            names.append(a)

    if not names:
        selected = SUITE
    else:
        wanted = set(names)
        selected = [(n, m) for n, m in ALL_GROUPS if n in wanted]
        unknown = wanted - set(n for n, _ in ALL_GROUPS)
        if unknown:
            print("Unknown groups:", ", ".join(sorted(unknown)))
            print("Valid:", ", ".join(n for n, _ in ALL_GROUPS))
            return 2
        if not selected:
            print("No groups selected")
            return 2

    if not quiet_banner:
        print("Amiga Python port test suite")
        print("version:", sys.version.replace("\n", " "))
        print("platform:", getattr(sys, "platform", "?"))
        print("executable:", getattr(sys, "executable", "?"))
        print("prefix:", getattr(sys, "prefix", "?"))
        print("groups:", ", ".join(n for n, _ in selected))
        print("---")

    reset_counters()
    for name, modname in selected:
        print("#### group:", name)
        try:
            mod = _load(modname)
        except Exception, e:
            support.FAILED += 1
            print("  FAIL: load", modname, "-", e)
            continue
        run_module_tests(mod)

    ok = summary()
    if ok:
        print("ALL AMIGA PORT TESTS PASSED")
        return 0
    return 1


if __name__ == "__main__":
    sys.exit(main())
