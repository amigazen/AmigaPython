# Optional: import from the prebuilt python27.zip (zipimport).
# ASCII only (Python 2.7 / Amiga).
#
# Archives (same content):
#   Source/lib/python27.zip
#   lib/python27.zip   (getpath: <prefix>/lib/python27.zip)
#
# Manual check from Source/:
#   python27 -c "import sys; sys.path.insert(0,'lib/python27.zip'); import os,arexx; print os.__file__; print arexx.RC_OK"

from __future__ import print_function

import os
import sys
from AmigaTests.support import check, skip, require_import


def _find_python27_zip():
    candidates = []
    # Beside this Source tree
    here = os.path.dirname(os.path.abspath(__file__))
    source = os.path.dirname(here)
    candidates.append(os.path.join(source, "lib", "python27.zip"))
    # getpath-style next to prefix
    prefix = getattr(sys, "prefix", None)
    if prefix:
        candidates.append(os.path.join(prefix, "lib", "python27.zip"))
        candidates.append(os.path.join(prefix, "Lib", "python27.zip"))
    # Already on path?
    for p in sys.path:
        if p and p.endswith("python27.zip"):
            candidates.insert(0, p)
    seen = set()
    for c in candidates:
        if not c or c in seen:
            continue
        seen.add(c)
        if os.path.isfile(c):
            return c
    return None


def test_python27_zip_present():
    path = _find_python27_zip()
    if not path:
        skip("python27.zip", "not built (run tools or see Source/lib/python27.zip)")
        return
    check("python27.zip exists", True)
    check("python27.zip size", os.path.getsize(path) > 1000)


def test_zipimporter_opens_python27_zip():
    zi = require_import("zipimport")
    if not zi:
        return
    path = _find_python27_zip()
    if not path:
        skip("zipimporter python27.zip", "archive missing")
        return
    try:
        imp = zi.zipimporter(path)
    except Exception, e:
        check("zipimporter(python27.zip)", False, str(e))
        return
    check("zipimporter(python27.zip)", imp is not None)
    check("find os in zip", imp.find_module("os") is not None)
    check("find arexx in zip", imp.find_module("arexx") is not None)


def test_import_os_from_python27_zip():
    # Load a module via the zip on sys.path without replacing the live os.
    zi = require_import("zipimport")
    if not zi:
        return
    path = _find_python27_zip()
    if not path:
        skip("import from python27.zip", "archive missing")
        return
    try:
        imp = zi.zipimporter(path)
        mod = imp.load_module("stat")
        check("load_module stat from zip", mod is not None)
        check("stat has S_ISDIR", hasattr(mod, "S_ISDIR"))
    except Exception, e:
        check("load_module from python27.zip", False, str(e))
