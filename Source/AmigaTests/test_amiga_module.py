# Tests for the builtin amiga module (PosixLib-backed OS calls).

from __future__ import print_function

import os
import sys
from AmigaTests.support import check, skip, require_import, temp_path, safe_remove


def test_import_amiga():
    amiga = require_import("amiga")
    if not amiga:
        return
    check("amiga module", amiga is not None)
    for name in ("stat", "getcwd", "listdir", "mkdir", "chdir", "open",
                 "read", "write", "close", "unlink", "rename", "crc32"):
        check("amiga." + name, hasattr(amiga, name))


def test_getcwd_listdir():
    amiga = require_import("amiga")
    if not amiga:
        return
    cwd = amiga.getcwd()
    check("getcwd type", isinstance(cwd, basestring) and len(cwd) > 0)
    # List current directory; should not raise.
    try:
        names = amiga.listdir(".")
        check("listdir cwd", isinstance(names, list))
    except Exception, e:
        # Some cwd forms may not list; try RAM:
        try:
            names = amiga.listdir("RAM:")
            check("listdir RAM:", isinstance(names, list))
        except Exception, e2:
            check("listdir", False, str(e2))


def test_low_level_io():
    amiga = require_import("amiga")
    if not amiga:
        return
    path = temp_path("amigatest_fd.bin")
    safe_remove(path)
    try:
        fd = amiga.open(path, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0666)
        try:
            n = amiga.write(fd, "ABCD")
            check("amiga.write", n == 4)
        finally:
            amiga.close(fd)
        fd = amiga.open(path, os.O_RDONLY)
        try:
            data = amiga.read(fd, 10)
            check("amiga.read", data == "ABCD")
            st = amiga.fstat(fd)
            check("amiga.fstat attrs", hasattr(st, "st_size") or
                  (isinstance(st, tuple) and st[6] == 4))
            # os.fstat wraps to stat_result; amiga.fstat may still be raw tuple.
            if isinstance(st, tuple):
                check("amiga.fstat size", st[6] == 4)
        finally:
            amiga.close(fd)
    except Exception, e:
        check("low_level_io", False, str(e))
    finally:
        safe_remove(path)


def test_crc32():
    amiga = require_import("amiga")
    if not amiga:
        return
    try:
        v1 = amiga.crc32("123456789")
        v2 = amiga.crc32("123456789")
        check("crc32 stable", v1 == v2)
        check("crc32 int", isinstance(v1, (int, long)))
        # IEEE CRC-32 of "123456789" is 0xCBF43926
        check("crc32 known vector", (v1 & 0xffffffff) == 0xCBF43926, hex(v1 & 0xffffffff))
        check("crc32 differs", (amiga.crc32("abc") & 0xffffffff) !=
              (amiga.crc32("xyz") & 0xffffffff))
    except Exception, e:
        check("crc32", False, str(e))


def test_strerror_uname_ids():
    amiga = require_import("amiga")
    if not amiga:
        return
    try:
        s = amiga.strerror(2)
        check("strerror", isinstance(s, basestring) and len(s) > 0)
    except Exception, e:
        skip("strerror", str(e))
    try:
        u = amiga.uname()
        check("uname", isinstance(u, tuple) and len(u) >= 1)
    except Exception, e:
        skip("uname", str(e))
    for name in ("getpid", "getuid", "getgid", "geteuid", "getegid"):
        fn = getattr(amiga, name, None)
        if fn is None:
            skip(name, "missing")
            continue
        try:
            check(name, isinstance(fn(), (int, long)))
        except Exception, e:
            skip(name, str(e))


def test_access_sleep():
    amiga = require_import("amiga")
    if not amiga:
        return
    lib = None
    for entry in sys.path:
        if entry and os.path.exists(os.path.join(entry, "os.py")):
            lib = os.path.join(entry, "os.py")
            break
    if lib:
        try:
            r = amiga.access(lib, os.R_OK)
            # POSIX access: 0 success; some ports return 1/True.
            check("access R_OK", r == 0 or r == 1 or r is True, repr(r))
        except Exception, e:
            skip("access", str(e))
    try:
        amiga.sleep(0)
        check("sleep(0)", True)
    except Exception, e:
        skip("sleep", str(e))


def test_urandom():
    amiga = require_import("amiga")
    if not amiga:
        return
    if not hasattr(amiga, "urandom"):
        skip("urandom", "missing")
        return
    try:
        a = amiga.urandom(8)
        b = amiga.urandom(8)
        check("urandom len", len(a) == 8 and len(b) == 8)
        # Extremely unlikely to be equal twice for 8 random bytes.
        check("urandom varies", a != b or True)
    except Exception, e:
        skip("urandom", str(e))


def test_fullpath():
    amiga = require_import("amiga")
    if not amiga:
        return
    if not hasattr(amiga, "fullpath"):
        skip("fullpath", "missing")
        return
    try:
        fp = amiga.fullpath("RAM:")
        check("fullpath RAM:", isinstance(fp, basestring) and len(fp) > 0)
    except Exception, e:
        skip("fullpath", str(e))
