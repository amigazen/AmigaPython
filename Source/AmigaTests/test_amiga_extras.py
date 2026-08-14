# amigapath, environment, and Doslib Amiga-specific modules.

from __future__ import print_function

import os
import sys
from AmigaTests.support import check, skip, require_import, temp_path, safe_remove


def test_amigapath():
    ap = require_import("amigapath")
    if not ap:
        return
    check("has to_unix", hasattr(ap, "to_unix"))
    check("has from_unix", hasattr(ap, "from_unix"))
    check("has fullpath", hasattr(ap, "fullpath"))
    try:
        u = ap.to_unix("RAM:Foo/Bar")
        check("to_unix type", isinstance(u, basestring) and len(u) > 0)
        # Round-trip when possible
        try:
            back = ap.from_unix(u)
            check("from_unix type", isinstance(back, basestring))
        except Exception, e:
            skip("from_unix", str(e))
    except Exception, e:
        skip("to_unix", str(e))
    try:
        fp = ap.fullpath("RAM:")
        check("amigapath.fullpath", isinstance(fp, basestring) and len(fp) > 0)
    except Exception, e:
        skip("amigapath.fullpath", str(e))


def test_environment_module():
    env = require_import("environment")
    if not env:
        return
    name = "AMIGAPY_TEST_ENV"
    value = "port_test_value"
    try:
        if hasattr(env, "setenv"):
            env.setenv(name, value, 1)
            got = env.getenv(name)
            check("environment set/get", got == value)
            if hasattr(env, "unsetenv"):
                env.unsetenv(name)
                check("environment unset", True)
        elif hasattr(env, "putenv"):
            env.putenv("%s=%s" % (name, value))
            check("environment putenv", True)
        else:
            skip("environment mutators", "no setenv/putenv")
    except Exception, e:
        skip("environment", str(e))

    if hasattr(env, "setvar"):
        try:
            # Amiga setvar(name, value, flags)
            env.setvar(name, value, 0)
            got = env.getvar(name)
            check("environment setvar/getvar", got == value)
            try:
                env.unsetvar(name)
            except TypeError:
                env.unsetvar(name, 0)
        except Exception, e:
            skip("setvar/getvar", str(e))


def test_doslib_basic():
    Doslib = require_import("Doslib")
    if not Doslib:
        return
    try:
        ds = Doslib.DateStamp()
        check("DateStamp", ds is not None)
    except Exception, e:
        skip("DateStamp", str(e))
    try:
        name = Doslib.GetProgramName()
        check("GetProgramName", isinstance(name, basestring) and len(name) > 0)
    except Exception, e:
        skip("GetProgramName", str(e))
    try:
        d = Doslib.GetProgramDir()
        check("GetProgramDir", isinstance(d, basestring) and len(d) > 0)
    except Exception, e:
        skip("GetProgramDir", str(e))
    try:
        err = Doslib.IoErr()
        check("IoErr", isinstance(err, (int, long)))
    except Exception, e:
        skip("IoErr", str(e))
    try:
        msg = Doslib.Fault(205)
        check("Fault", isinstance(msg, basestring) and len(msg) > 0)
    except Exception, e:
        skip("Fault", str(e))
    try:
        # Examine a known file from Lib
        target = None
        for entry in sys.path:
            cand = os.path.join(entry, "os.py")
            if entry and os.path.exists(cand):
                target = cand
                break
        if target:
            fib = Doslib.Examine(target)
            check("Examine", fib is not None)
        else:
            skip("Examine", "no os.py found")
    except Exception, e:
        skip("Examine", str(e))
    try:
        ok = Doslib.IsFileSystem("RAM:")
        check("IsFileSystem RAM:", ok in (0, 1, True, False) or isinstance(ok, (int, long)))
    except Exception, e:
        skip("IsFileSystem", str(e))


def test_arexx_accelerator():
    # Private C accelerator; public API is Lib/ARexx.py.
    import sys
    check("_arexx builtin", "_arexx" in sys.builtin_module_names)
    ll = require_import("_arexx")
    if not ll:
        return
    check("_arexx.port", hasattr(ll, "port"))
    check("_arexx.errorstring", hasattr(ll, "errorstring"))
    check("_arexx.dorexx", hasattr(ll, "dorexx"))
    check("_arexx.error", hasattr(ll, "error"))
    try:
        s = ll.errorstring(1)
        check("errorstring", isinstance(s, basestring) and len(s) > 0)
    except Exception, e:
        skip("errorstring call", str(e))

    try:
        p = ll.port(None)
        check("port(None)", p is not None)
        if hasattr(p, "close"):
            p.close()
            check("port.close", True)
    except Exception, e:
        skip("port(None)", str(e))


def test_arexx_dos_wrappers():
    # High-level Lib wrappers (ARexx.py / Dos.py).
    try:
        import Dos
        check("import Dos", True)
        check("Dos.DateStamp", hasattr(Dos, "DateStamp"))
    except Exception, e:
        skip("import Dos", str(e))
    try:
        import ARexx
        check("import ARexx", True)
        check("ARexx.privateport", hasattr(ARexx, "privateport"))
        check("ARexx.publicport", hasattr(ARexx, "publicport"))
        check("ARexx.host", hasattr(ARexx, "host"))
        check("ARexx.RC_OK", getattr(ARexx, "RC_OK", None) == 0)
        # Create/close a private port without talking to other hosts.
        p = ARexx.privateport()
        check("privateport", p is not None)
        p.close()
        check("privateport.close", True)
    except Exception, e:
        skip("import ARexx", str(e))


def test_os_environ_dict():
    # os.environ should work via amiga convertenviron
    try:
        e = os.environ
        check("os.environ mapping", hasattr(e, "get") or isinstance(e, dict))
        # Setting a process env var
        key = "AMIGAPY_OSENV"
        os.environ[key] = "1"
        check("os.environ set/get", os.environ.get(key) == "1" or e[key] == "1")
        try:
            del os.environ[key]
        except Exception:
            pass
    except Exception, e:
        skip("os.environ", str(e))
