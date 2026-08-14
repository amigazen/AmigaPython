#!/usr/bin/env python
# Run the Amiga Python 2.7.18 port test suite.
#
# From the Source directory:
#   python27 AmigaTests/run.py
#   python27 AmigaTests/run.py runtime os_path
#   python27 AmigaTests/run.py -q

from __future__ import print_function

import os
import sys

# Ensure Source root (parent of AmigaTests) is on sys.path.
_HERE = os.path.dirname(os.path.abspath(__file__))
_ROOT = os.path.dirname(_HERE)
if _ROOT not in sys.path:
    sys.path.insert(0, _ROOT)

from AmigaTests import support
from AmigaTests.support import run_module_tests, summary, reset_counters

# Suite modules in run order.
SUITE = [
    ("runtime", "AmigaTests.test_runtime"),
    ("os_path", "AmigaTests.test_os_path"),
    ("amiga", "AmigaTests.test_amiga_module"),
    ("builtins", "AmigaTests.test_builtins_ext"),
    ("extras", "AmigaTests.test_amiga_extras"),
]


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
            print("Groups:", ", ".join(n for n, _ in SUITE))
            return 0
        else:
            names.append(a)

    if not names:
        selected = SUITE
    else:
        wanted = set(names)
        selected = [(n, m) for n, m in SUITE if n in wanted]
        unknown = wanted - set(n for n, _ in SUITE)
        if unknown:
            print("Unknown groups:", ", ".join(sorted(unknown)))
            print("Valid:", ", ".join(n for n, _ in SUITE))
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
