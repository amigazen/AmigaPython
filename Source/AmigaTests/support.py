# Shared helpers for AmigaTests. ASCII only (Python 2.7 / Amiga).

from __future__ import print_function

import os
import sys
import traceback

PASSED = 0
FAILED = 0
SKIPPED = 0


def reset_counters():
    global PASSED, FAILED, SKIPPED
    PASSED = 0
    FAILED = 0
    SKIPPED = 0


def check(name, cond, detail=None):
    global PASSED, FAILED
    if cond:
        PASSED += 1
        print("  PASS:", name)
        sys.stdout.flush()
        return True
    FAILED += 1
    if detail is None:
        print("  FAIL:", name)
    else:
        print("  FAIL:", name, "-", detail)
    sys.stdout.flush()
    return False


def skip(name, reason):
    global SKIPPED
    SKIPPED += 1
    print("  SKIP:", name, "-", reason)
    sys.stdout.flush()


def expect_raises(name, exc_type, fn):
    try:
        fn()
    except exc_type:
        return check(name, True)
    except Exception:
        et, ev = sys.exc_info()[:2]
        return check(name, False, "wrong exception %s: %s" % (et.__name__, ev))
    return check(name, False, "no exception raised")


def try_import(name, report=False):
    # Soft-fail any init error so one broken builtin (e.g. pyexpat) does not
    # abort inventory; drop a half-inited entry from sys.modules if present.
    try:
        return __import__(name)
    except ImportError:
        if report:
            et, ev = sys.exc_info()[:2]
            print("  NOTE: ImportError %s: %s" % (name, ev))
            sys.stdout.flush()
        return None
    except Exception:
        if report:
            et, ev = sys.exc_info()[:2]
            print("  NOTE: import %s failed: %s: %s" % (name, et.__name__, ev))
            sys.stdout.flush()
        if name in sys.modules:
            try:
                del sys.modules[name]
            except Exception:
                pass
        return None


def require_import(name, report=False):
    mod = try_import(name, report=report)
    if mod is None:
        skip("import " + name, "not available")
    return mod


def temp_path(name):
    # Prefer RAM: on Amiga; otherwise local cwd.
    if getattr(sys, "platform", "") == "amiga":
        return "RAM:" + name
    return os.path.join(".", name)


def safe_remove(path):
    try:
        if os.path.isdir(path):
            for name in os.listdir(path):
                safe_remove(os.path.join(path, name))
            os.rmdir(path)
        elif os.path.exists(path):
            os.remove(path)
    except Exception:
        pass


def run_module_tests(mod):
    """Call every test_* function in mod; return True if no uncaught errors."""
    ok = True
    names = sorted(n for n in dir(mod) if n.startswith("test_"))
    for name in names:
        fn = getattr(mod, name)
        if not callable(fn):
            continue
        print("==", mod.__name__ + "." + name, "==")
        sys.stdout.flush()
        try:
            fn()
        except Exception, e:
            global FAILED
            FAILED += 1
            ok = False
            print("  FAIL:", name, "- uncaught:", type(e).__name__ + ":", e)
            sys.stdout.flush()
    return ok


def summary():
    print("---")
    print("Passed: %d  Failed: %d  Skipped: %d" % (PASSED, FAILED, SKIPPED))
    return FAILED == 0
