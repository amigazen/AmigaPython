# Secondary amigalibs / intuitionlib smoke tests. Optional group.

from __future__ import print_function

from AmigaTests.support import check, skip, require_import


def test_amigalibs_open_exec():
    al = require_import("amigalibs")
    if not al:
        return
    try:
        lib = al.openlib("exec.library", 0)
        check("exec open", lib is not None)
        check("exec base", getattr(lib, "base", 0) != 0)
        ver = getattr(lib, "version", None)
        check("exec version", isinstance(ver, tuple) and len(ver) == 2)
    except Exception, e:
        check("amigalibs exec", False, str(e))


def test_amigalibs_bad_lvo():
    al = require_import("amigalibs")
    if not al:
        return
    try:
        lib = al.openlib("exec.library", 0)
        try:
            lib.call((0, 1), {0: 0})
            check("bad LVO rejected", False, "expected ValueError")
        except ValueError:
            check("bad LVO rejected", True)
    except Exception, e:
        skip("bad LVO", str(e))


def test_pack_tags():
    al = require_import("amigalibs")
    if not al:
        return
    try:
        s = al.pack_tags(1, 2, 3, 4)
        check("pack_tags len", isinstance(s, str) and len(s) == 24)
    except Exception, e:
        check("pack_tags", False, str(e))


def test_intuitionlib_import():
    try:
        import intuitionlib
        check("intuitionlib LVO", hasattr(intuitionlib, "LVO") and
              "DisplayBeep" in intuitionlib.LVO)
        check("intuitionlib call", hasattr(intuitionlib, "call"))
    except Exception, e:
        skip("intuitionlib", str(e))
