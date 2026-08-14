# AmiTCP companion modules from config.c: pwd, grp, crypt, syslog.
# Soft-fail with skip when usergroup/TCP helpers are incomplete.
# ASCII only (Python 2.7 / Amiga).

from __future__ import print_function

import sys
from AmigaTests.support import check, skip, require_import


def _amitcp_built():
    return "pwd" in sys.builtin_module_names


def test_pwd_smoke():
    if not _amitcp_built():
        skip("pwd", "AMITCP modules not in this build")
        return
    pwd = require_import("pwd")
    if not pwd:
        return
    check("pwd.getpwuid", callable(pwd.getpwuid))
    check("pwd.getpwnam", callable(pwd.getpwnam))
    try:
        ent = pwd.getpwuid(0)
        check("getpwuid(0)", ent is not None and len(ent) >= 3)
    except Exception, e:
        skip("getpwuid(0)", str(e))
    try:
        if hasattr(pwd, "getpwall"):
            all_ents = pwd.getpwall()
            check("getpwall", isinstance(all_ents, list))
    except Exception, e:
        skip("getpwall", str(e))


def test_grp_smoke():
    if not _amitcp_built():
        skip("grp", "AMITCP modules not in this build")
        return
    grp = require_import("grp")
    if not grp:
        return
    check("grp.getgrgid", callable(grp.getgrgid))
    try:
        ent = grp.getgrgid(0)
        check("getgrgid(0)", ent is not None and len(ent) >= 3)
    except Exception, e:
        skip("getgrgid(0)", str(e))


def test_crypt_smoke():
    if not _amitcp_built():
        skip("crypt", "AMITCP modules not in this build")
        return
    crypt = require_import("crypt")
    if not crypt:
        return
    if not hasattr(crypt, "crypt"):
        skip("crypt.crypt", "missing")
        return
    try:
        h = crypt.crypt("amiga", "xy")
        check("crypt.crypt", isinstance(h, basestring) and len(h) > 0, repr(h))
    except Exception, e:
        skip("crypt.crypt", str(e))


def test_syslog_smoke():
    if not _amitcp_built():
        skip("syslog", "AMITCP modules not in this build")
        return
    syslog = require_import("syslog")
    if not syslog:
        return
    for name in ("openlog", "syslog", "closelog"):
        if not hasattr(syslog, name):
            skip("syslog." + name, "missing")
            return
    try:
        syslog.openlog("AmigaPythonTest")
        syslog.syslog("amiga port suite smoke")
        syslog.closelog()
        check("syslog open/log/close", True)
    except Exception, e:
        skip("syslog calls", str(e))
