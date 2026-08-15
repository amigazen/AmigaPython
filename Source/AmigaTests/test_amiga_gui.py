#!/usr/bin/env python
"""Standalone / optional-suite Amiga GUI smoke test (Intuition / ASL / amigagui).

Default AmigaTests/run.py does NOT include this group (opens requesters).
Run alone from Source:
  python27 AmigaTests/test_amiga_gui.py
  python27 AmigaTests/test_amiga_gui.py --no-file
Or via suite:
  python27 AmigaTests/run.py gui

ASCII only (Python 2.7 / Amiga).
"""

from __future__ import print_function

import os
import sys

_HERE = os.path.dirname(os.path.abspath(__file__))
_ROOT = os.path.dirname(_HERE)
if _ROOT not in sys.path:
    sys.path.insert(0, _ROOT)

sys.dont_write_bytecode = True

from AmigaTests.support import check, skip, reset_counters, summary

# Set by main(); suite import uses True (interactive).
DO_FILEREQUEST = True
_AMIGA = None
_GUI = None


def test_01_import_amiga():
    global _AMIGA
    _AMIGA = None
    try:
        import amiga
    except ImportError, e:
        check("import amiga", False, str(e))
        return
    check("import amiga", True)
    _AMIGA = amiga


def test_02_currenttime_beep():
    amiga = _AMIGA
    if amiga is None:
        skip("CurrentTime", "no amiga")
        return
    if hasattr(amiga, "CurrentTime"):
        try:
            t = amiga.CurrentTime()
            check("CurrentTime tuple",
                  isinstance(t, tuple) and len(t) == 2)
            print("    CurrentTime:", t)
        except Exception, e:
            check("CurrentTime", False, str(e))
    else:
        skip("CurrentTime", "not built")
    if hasattr(amiga, "DisplayBeep"):
        try:
            amiga.DisplayBeep()
            check("DisplayBeep", True)
        except Exception, e:
            check("DisplayBeep", False, str(e))
    else:
        skip("DisplayBeep", "not built")


def test_03_easyrequest():
    amiga = _AMIGA
    if amiga is None:
        skip("EasyRequest", "no amiga")
        return
    if not hasattr(amiga, "EasyRequest"):
        skip("EasyRequest", "not built")
        return
    print("    Click a gadget in the EasyRequest...")
    try:
        n = amiga.EasyRequest(
            "AmigaPython GUI test\nEasyRequest OK?",
            "Yes|No",
            "AmigaTests")
        check("EasyRequest returned int", isinstance(n, (int, long)))
        print("    EasyRequest choice:", n)
    except Exception, e:
        check("EasyRequest", False, str(e))


def test_04_messagebox():
    amiga = _AMIGA
    if amiga is None:
        skip("MessageBox", "no amiga")
        return
    if not hasattr(amiga, "MessageBox"):
        skip("MessageBox", "not built")
        return
    print("    Click OK on the MessageBox...")
    try:
        n = amiga.MessageBox(
            "AmigaTests",
            "MessageBox from amiga module",
            "OK")
        check("MessageBox returned int", isinstance(n, (int, long)))
        print("    MessageBox choice:", n)
    except Exception, e:
        check("MessageBox", False, str(e))


def test_05_filerequest():
    if not DO_FILEREQUEST:
        skip("FileRequest", "disabled (--no-file)")
        return
    amiga = _AMIGA
    if amiga is None:
        skip("FileRequest", "no amiga")
        return
    fr = getattr(amiga, "FileRequest", None)
    if fr is None:
        try:
            import asl
            fr = asl.FileRequest
        except Exception, e:
            skip("FileRequest", str(e))
            return
    print("    Pick a file or cancel the ASL requester...")
    try:
        r = fr(title="AmigaTests FileRequest",
               drawer="RAM:", filename="", pattern="#?")
        if r is None:
            check("FileRequest cancel/None", True)
            print("    FileRequest: cancelled")
        else:
            check("FileRequest tuple",
                  isinstance(r, tuple) and len(r) == 2)
            print("    FileRequest:", r)
    except Exception, e:
        check("FileRequest", False, str(e))


def test_06_amigagui_window():
    global _GUI
    _GUI = None
    try:
        import amigagui
    except ImportError, e:
        check("import amigagui", False, str(e))
        return
    check("import amigagui", True)
    _GUI = amigagui

    print("    Drawing in amigagui window; close it or press a key...")
    win = None
    try:
        win = amigagui.window("AmigaPython amigagui test", 40, 40, 320, 180)
        check("window open", win is not None)
        check("window.signal", hasattr(win, "signal"))
        win.pen(1)
        win.clear(0)
        win.plot(10, 10)
        win.line(10, 10, 200, 80)
        win.lineto(280, 20)
        win.pen(2)
        win.line(20, 100, 280, 140)
        # Drain messages until CLOSEWINDOW or VANILLAKEY.
        done = False
        while not done:
            win.wait()
            while True:
                msg = win.getmsg()
                if msg is None:
                    break
                # (Class, Code, Qualifier, MouseX, MouseY, Seconds, Micros)
                cls = msg[0]
                # IDCMP_CLOSEWINDOW = 0x200, IDCMP_VANILLAKEY = 0x200000
                if cls == 0x200 or cls == 0x200000:
                    done = True
                    break
        check("window event loop", True)
    except KeyboardInterrupt:
        check("window Ctrl-C", True)
    except Exception, e:
        check("amigagui window", False, str(e))
    if win is not None:
        try:
            win.close()
            check("window close", True)
        except Exception, e:
            check("window close", False, str(e))


def test_07_intuition_shim():
    try:
        import intuition
        check("import intuition", True)
        check("intuition.EasyRequest", hasattr(intuition, "EasyRequest"))
        check("intuition.DisplayBeep", hasattr(intuition, "DisplayBeep"))
    except Exception, e:
        skip("intuition shim", str(e))


def main(argv=None):
    global DO_FILEREQUEST
    if argv is None:
        argv = sys.argv[1:]
    for a in argv:
        if a in ("-h", "--help"):
            print(__doc__)
            return 0
        if a == "--no-file":
            DO_FILEREQUEST = False

    reset_counters()
    print("AmigaTests GUI (interactive)")
    print("Source root:", _ROOT)
    mod = sys.modules[__name__]
    names = sorted(n for n in dir(mod) if n.startswith("test_"))
    for name in names:
        fn = getattr(mod, name)
        if not callable(fn):
            continue
        print("==", name, "==")
        try:
            fn()
        except Exception, e:
            from AmigaTests import support
            support.FAILED += 1
            print("  FAIL:", name, "- uncaught:", type(e).__name__ + ":", e)
    ok = summary()
    if ok:
        print("ALL GUI TESTS PASSED")
        return 0
    print("SOME GUI TESTS FAILED")
    return 1


if __name__ == "__main__":
    sys.exit(main())
