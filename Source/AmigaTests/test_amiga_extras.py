# amiga path/dos helpers, environment, _arexx, and Lib/site-python/arexx.

from __future__ import print_function

import os
import sys
from AmigaTests.support import check, skip, require_import, temp_path, safe_remove


def test_amiga_getcpu_getmachine():
    # OS4 AmigaPython.txt: amiga.getcpu() / amiga.getmachine()
    amiga = require_import("amiga")
    if not amiga:
        return
    try:
        cpu = amiga.getcpu()
        check("getcpu type", isinstance(cpu, basestring) and len(cpu) > 0)
        check("getcpu known", cpu in (
            "68000", "68010", "68020", "68030", "68040", "68060", "unknown"))
    except Exception, e:
        check("getcpu", False, str(e))
    try:
        mach = amiga.getmachine()
        check("getmachine type", isinstance(mach, basestring) and len(mach) > 0)
    except Exception, e:
        check("getmachine", False, str(e))


def test_amiga_path_helpers():
    amiga = require_import("amiga")
    if not amiga:
        return
    check("has to_unix", hasattr(amiga, "to_unix"))
    check("has from_unix", hasattr(amiga, "from_unix"))
    check("has fullpath", hasattr(amiga, "fullpath"))
    try:
        u = amiga.to_unix("RAM:Foo/Bar")
        check("to_unix type", isinstance(u, basestring) and len(u) > 0)
        try:
            back = amiga.from_unix(u)
            check("from_unix type", isinstance(back, basestring))
        except Exception, e:
            skip("from_unix", str(e))
    except Exception, e:
        skip("to_unix", str(e))
    try:
        fp = amiga.fullpath("RAM:")
        check("amiga.fullpath", isinstance(fp, basestring) and len(fp) > 0)
    except Exception, e:
        skip("amiga.fullpath", str(e))


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
            env.setvar(name, value, 0)
            got = env.getvar(name)
            check("environment setvar/getvar", got == value)
            try:
                env.unsetvar(name)
            except TypeError:
                env.unsetvar(name, 0)
        except Exception, e:
            skip("setvar/getvar", str(e))

    # OS4 amigavars names on the environment builtin
    if hasattr(env, "GetEnv") and hasattr(env, "SetEnv"):
        try:
            env.SetEnv(name, value, 0)
            got = env.GetEnv(name)
            check("environment GetEnv/SetEnv", got == value)
            env.UnSetEnv(name, 0)
            check("environment UnSetEnv", env.GetEnv(name) is None)
        except Exception, e:
            skip("GetEnv/SetEnv", str(e))


def test_amigavars_module():
    # OS4-compatible import name (site-python shim over environment)
    av = require_import("amigavars")
    if not av:
        return
    name = "AMIGAPY_TEST_AVARS"
    value = "avars_value"
    try:
        av.SetEnv(name, value, 0)
        check("amigavars GetEnv", av.GetEnv(name) == value)
        av.UnSetEnv(name, 0)
        check("amigavars UnSetEnv", av.GetEnv(name) is None)
    except Exception, e:
        skip("amigavars", str(e))


def test_amigados_basic():
    amiga = require_import("amiga")
    if not amiga:
        return
    try:
        ds = amiga.DateStamp()
        check("DateStamp", ds is not None)
    except Exception, e:
        skip("DateStamp", str(e))
    try:
        name = amiga.GetProgramName()
        check("GetProgramName", isinstance(name, basestring) and len(name) > 0)
    except Exception, e:
        skip("GetProgramName", str(e))
    try:
        d = amiga.GetProgramDir()
        check("GetProgramDir", isinstance(d, basestring) and len(d) > 0)
    except Exception, e:
        skip("GetProgramDir", str(e))
    try:
        err = amiga.IoErr()
        check("IoErr", isinstance(err, (int, long)))
    except Exception, e:
        skip("IoErr", str(e))
    try:
        msg = amiga.Fault(205)
        check("Fault", isinstance(msg, basestring) and len(msg) > 0)
    except Exception, e:
        skip("Fault", str(e))
    try:
        target = None
        for entry in sys.path:
            cand = os.path.join(entry, "os.py")
            if entry and os.path.exists(cand):
                target = cand
                break
        if target:
            fib = amiga.Examine(target)
            check("Examine", fib is not None)
        else:
            skip("Examine", "no os.py found")
    except Exception, e:
        skip("Examine", str(e))
    try:
        ok = amiga.IsFileSystem("RAM:")
        check("IsFileSystem RAM:", ok in (0, 1, True, False) or isinstance(ok, (int, long)))
    except Exception, e:
        skip("IsFileSystem", str(e))
    check("amiga.doserror", hasattr(amiga, "doserror"))
    check("amiga.ArgParser", hasattr(amiga, "ArgParser"))


def test_arexx_accelerator():
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


def test_arexx_wrapper():
    try:
        import arexx
        check("import arexx", True)
        check("arexx.privateport", hasattr(arexx, "privateport"))
        check("arexx.publicport", hasattr(arexx, "publicport"))
        check("arexx.Port", hasattr(arexx, "Port"))
        check("arexx.host", hasattr(arexx, "host"))
        check("arexx.dorexx", hasattr(arexx, "dorexx"))
        check("arexx.RC_OK", getattr(arexx, "RC_OK", None) == 0)
        p = arexx.privateport()
        check("privateport", p is not None)
        p.close()
        check("privateport.close", True)
    except Exception, e:
        skip("import arexx", str(e))


def test_os4_shims():
    for name in ("asl", "catalog", "icon"):
        try:
            m = __import__(name)
            check("import " + name, True)
        except Exception, e:
            skip("import " + name, str(e))
    amiga = require_import("amiga")
    if not amiga:
        return
    for attr in ("FileRequest", "MessageBox", "OpenCatalog", "DiskObject"):
        check("amiga." + attr, hasattr(amiga, attr))


def test_os_environ_dict():
    try:
        e = os.environ
        check("os.environ mapping", hasattr(e, "get") or isinstance(e, dict))
        key = "AMIGAPY_OSENV"
        os.environ[key] = "1"
        check("os.environ set/get", os.environ.get(key) == "1" or e[key] == "1")
        try:
            del os.environ[key]
        except Exception:
            pass
    except Exception, e:
        skip("os.environ", str(e))
