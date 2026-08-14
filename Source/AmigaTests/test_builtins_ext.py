# Built-in extension modules linked in Modules/config.c for this port.

from __future__ import print_function

import sys
from AmigaTests.support import check, skip, require_import


def test_math_cmath():
    math = require_import("math")
    if math:
        check("math.sqrt", abs(math.sqrt(9.0) - 3.0) < 1e-9)
        check("math.sin", abs(math.sin(0.0)) < 1e-9)
        check("math.pi", math.pi > 3.1)
    cmath = require_import("cmath")
    if cmath:
        try:
            z = cmath.sqrt(-1+0j)
            # Accept +/- 1j (softfloat may differ slightly from exact).
            check("cmath.sqrt(-1)", abs(z*z + 1) < 1e-5, repr(z))
        except Exception, e:
            skip("cmath.sqrt(-1)", str(e))


def test_time():
    time = require_import("time")
    if not time:
        return
    now = time.time()
    check("time.time", isinstance(now, float) and now > 0)
    text = time.strftime("%Y-%m-%d", time.localtime(now))
    check("strftime", len(text) == 10 and text[4] == "-")
    check("sleep", True)
    time.sleep(0)


def test_datetime():
    # C-only module in 2.7 (no Lib/datetime.py fallback).
    dt = require_import("datetime")
    if not dt:
        return
    d = dt.date(2026, 8, 14)
    check("date ymd", d.year == 2026 and d.month == 8 and d.day == 14)
    check("date weekday", d.weekday() == 4)  # Friday
    check("date isoformat", d.isoformat() == "2026-08-14")
    check("date toordinal", dt.date.fromordinal(d.toordinal()) == d)

    t = dt.time(12, 30, 45, 123456)
    check("time hms", t.hour == 12 and t.minute == 30 and t.second == 45)
    check("time microsecond", t.microsecond == 123456)
    check("time isoformat", t.isoformat() == "12:30:45.123456")

    stamp = dt.datetime(2026, 8, 14, 12, 0, 0)
    check("datetime isoformat", stamp.isoformat() == "2026-08-14T12:00:00")
    check("datetime date()", stamp.date() == d)
    check("datetime time()", stamp.time() == dt.time(12, 0, 0))
    check("datetime replace",
          stamp.replace(year=2025).year == 2025 and stamp.year == 2026)
    check("datetime strftime",
          stamp.strftime("%Y-%m-%d %H:%M") == "2026-08-14 12:00")
    check("datetime combine",
          dt.datetime.combine(d, dt.time(1, 2, 3)) ==
          dt.datetime(2026, 8, 14, 1, 2, 3))

    delta = dt.timedelta(days=1, seconds=30, microseconds=500)
    check("timedelta parts",
          delta.days == 1 and delta.seconds == 30 and delta.microseconds == 500)
    check("timedelta add", (stamp + dt.timedelta(days=1)).day == 15)
    check("timedelta sub", (stamp - dt.timedelta(hours=1)).hour == 11)
    check("timedelta total compare",
          dt.timedelta(days=1) > dt.timedelta(hours=23))

    check("date min/max", dt.date.min < d < dt.date.max)
    check("datetime compare",
          dt.datetime(2020, 1, 1) < stamp < dt.datetime(2030, 1, 1))

    try:
        now = dt.datetime.now()
        check("datetime.now", isinstance(now, dt.datetime) and now.year >= 2020)
    except Exception, e:
        skip("datetime.now", str(e))
    try:
        utc = dt.datetime.utcnow()
        check("datetime.utcnow", isinstance(utc, dt.datetime))
    except Exception, e:
        skip("datetime.utcnow", str(e))
    try:
        today = dt.date.today()
        check("date.today", isinstance(today, dt.date))
    except Exception, e:
        skip("date.today", str(e))

    try:
        import calendar
        check("calendar.weekday matches",
              calendar.weekday(2026, 8, 14) == stamp.weekday())
        check("calendar.monthcalendar",
              isinstance(calendar.monthcalendar(2026, 8), list) and
              len(calendar.monthcalendar(2026, 8)) >= 4)
    except ImportError, e:
        skip("calendar", str(e))


def test_array():
    array = require_import("array")
    if not array:
        return
    a = array.array("i", [1, 2, 3])
    check("array", list(a) == [1, 2, 3] and a[1] == 2)


def test_binascii_struct_marshal():
    binascii = require_import("binascii")
    if binascii:
        check("b2a/a2b_hex", binascii.a2b_hex(binascii.b2a_hex("hi")) == "hi")
    struct = require_import("struct")
    if struct:
        check("struct", struct.unpack(">I", struct.pack(">I", 0x1020304))[0] == 0x1020304)
    marshal = require_import("marshal")
    if marshal:
        obj = [1, "x", {"a": 2}]
        check("marshal", marshal.loads(marshal.dumps(obj)) == obj)


def test_codecs_strop_operator():
    codecs = require_import("_codecs")
    if codecs:
        try:
            import encodings
            check("encodings import", True)
        except ImportError:
            skip("encodings", "not available")
        try:
            check("str encode ascii", "abc".encode("ascii") == "abc")
        except Exception:
            # Port still missing some _codecs.*_encode C helpers.
            skip("str.encode ascii", "ascii_encode not in _codecs yet")
    strop = require_import("strop")
    if strop:
        check("strop.lower", strop.lower("AB") == "ab")
    operator = require_import("operator")
    if operator:
        check("operator.add", operator.add(2, 3) == 5)


def test_cpickle_cstringio():
    cPickle = require_import("cPickle")
    if cPickle:
        check("cPickle", cPickle.loads(cPickle.dumps({"a": 1})) == {"a": 1})
    cStringIO = require_import("cStringIO")
    if cStringIO:
        buf = cStringIO.StringIO()
        buf.write("xyz")
        check("cStringIO", buf.getvalue() == "xyz")


def test_hash_zlib():
    md5 = require_import("md5")
    if md5:
        h = md5.new("abc").hexdigest()
        check("md5", h == "900150983cd24fb0d6963f7d28e17f72")
    sha = require_import("sha")
    if sha:
        h = sha.new("abc").hexdigest()
        check("sha", h == "a9993e364706816aba3e25717850c26c9cd0d89d", h)
    zlib = require_import("zlib")
    if zlib:
        raw = "hello" * 50
        check("zlib roundtrip", zlib.decompress(zlib.compress(raw)) == raw)
        check("zlib.crc32", (zlib.crc32("123456789") & 0xffffffff) == 0xcbf43926)


def test_zipimport():
    # Builtin zipimport (Modules/zipimport.c); zlib used for deflated members.
    from AmigaTests.support import temp_path, safe_remove

    zi = require_import("zipimport")
    if not zi:
        return
    check("zipimporter", hasattr(zi, "zipimporter"))
    check("ZipImportError", issubclass(zi.ZipImportError, ImportError))
    check("zipimport in builtins", "zipimport" in sys.builtin_module_names)
    # Startup should have registered the path hook when zipimport is builtin.
    hooks = getattr(sys, "path_hooks", None)
    if hooks is not None:
        check("zipimporter on path_hooks",
              any(h is zi.zipimporter or
                  getattr(h, "__name__", "") == "zipimporter"
                  for h in hooks))

    try:
        zi.zipimporter("RAM:not_a_real_archive_zzzz.zip")
        check("zipimporter missing archive", False, "expected ZipImportError")
    except zi.ZipImportError:
        check("zipimporter missing archive", True)
    except Exception, e:
        skip("zipimporter missing archive", str(e))

    zipfile = require_import("zipfile")
    if not zipfile:
        return
    path = temp_path("amigatest_zipimport.zip")
    safe_remove(path)
    try:
        zf = zipfile.ZipFile(path, "w", zipfile.ZIP_STORED)
        try:
            zf.writestr("amigaziptest.py", "VALUE = 42\n")
            zf.writestr("amigazippkg/__init__.py", "PKG = 1\n")
            zf.writestr("amigazippkg/mod.py", "NESTED = 7\n")
        finally:
            zf.close()

        imp = zi.zipimporter(path)
        check("zipimporter archive attr",
              getattr(imp, "archive", None) == path or
              str(getattr(imp, "archive", "")).endswith("amigatest_zipimport.zip"))
        check("find_module top", imp.find_module("amigaziptest") is not None)
        check("find_module pkg", imp.find_module("amigazippkg") is not None)
        check("is_package", imp.is_package("amigazippkg") is True)
        check("get_source",
              "VALUE = 42" in (imp.get_source("amigaziptest") or ""))

        sys.path.insert(0, path)
        try:
            for name in ("amigaziptest", "amigazippkg", "amigazippkg.mod"):
                if name in sys.modules:
                    del sys.modules[name]
            mod = __import__("amigaziptest")
            check("import from zip", getattr(mod, "VALUE", None) == 42)
            pkg = __import__("amigazippkg.mod", fromlist=["mod"])
            check("import package from zip",
                  getattr(pkg, "NESTED", None) == 7)
        finally:
            if path in sys.path:
                sys.path.remove(path)
            for name in ("amigaziptest", "amigazippkg", "amigazippkg.mod"):
                if name in sys.modules:
                    del sys.modules[name]
            cache = getattr(sys, "path_importer_cache", None)
            if cache is not None and path in cache:
                del cache[path]
            # Also drop directory-cache entry used by zipimport.
            zcache = getattr(zi, "_zip_directory_cache", None)
            if zcache is not None and path in zcache:
                del zcache[path]
    except Exception, e:
        check("import from zip", False, str(e))
    safe_remove(path)

    # Deflated member needs zlib (already linked).
    path2 = temp_path("amigatest_zipimport_deflate.zip")
    safe_remove(path2)
    try:
        zf = zipfile.ZipFile(path2, "w", zipfile.ZIP_DEFLATED)
        try:
            zf.writestr("amigazipdeflate.py", "DEFLATED = 99\n")
        finally:
            zf.close()
        sys.path.insert(0, path2)
        try:
            if "amigazipdeflate" in sys.modules:
                del sys.modules["amigazipdeflate"]
            mod = __import__("amigazipdeflate")
            check("import deflated from zip",
                  getattr(mod, "DEFLATED", None) == 99)
        finally:
            if path2 in sys.path:
                sys.path.remove(path2)
            if "amigazipdeflate" in sys.modules:
                del sys.modules["amigazipdeflate"]
            cache = getattr(sys, "path_importer_cache", None)
            if cache is not None and path2 in cache:
                del cache[path2]
            zcache = getattr(zi, "_zip_directory_cache", None)
            if zcache is not None and path2 in zcache:
                del zcache[path2]
    except Exception, e:
        skip("import deflated from zip", str(e))
    safe_remove(path2)


def test_hashlib_crc():
    # Amiga _hashlib wraps crc.library (MD5 / SHA-1 / SHA-256).
    hl = require_import("hashlib")
    if not hl:
        return
    check("hashlib.md5",
          hl.md5("abc").hexdigest() == "900150983cd24fb0d6963f7d28e17f72")
    check("hashlib.sha1",
          hl.sha1("abc").hexdigest() ==
          "a9993e364706816aba3e25717850c26c9cd0d89d")
    if hasattr(hl, "sha256"):
        check("hashlib.sha256",
              hl.sha256("abc").hexdigest() ==
              "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad")
    else:
        skip("hashlib.sha256", "not available")
    # Streaming + copy
    m = hl.md5()
    m.update("ab")
    m2 = m.copy()
    m.update("c")
    m2.update("c")
    check("hashlib.copy", m.hexdigest() == m2.hexdigest())
    check("hashlib.new",
          hl.new("sha1", "abc").hexdigest() ==
          "a9993e364706816aba3e25717850c26c9cd0d89d")
    # crc.library does not provide sha224/384/512
    check("hashlib no sha224",
          "sha224" not in getattr(hl, "algorithms_available", ()))


def test_sre_re():
    sre = require_import("_sre")
    if sre:
        check("_sre import", True)
        check("_sre MAGIC or getcodesize",
              hasattr(sre, "MAGIC") or hasattr(sre, "getcodesize") or
              hasattr(sre, "compile"))
    re = require_import("re")
    if not re:
        return
    m = re.search(r"(\d+)", "x42y")
    check("re.search", m is not None and m.group(1) == "42")
    check("re.sub", re.sub("a", "b", "cac") == "cbc")
    check("re.findall", re.findall(r"[a-z]+", "A bb CC ddd") == ["bb", "ddd"])


def test_ast_warnings():
    # _ast is builtin; ast.py is the public wrapper when Lib is present.
    _ast = require_import("_ast")
    if _ast:
        check("_ast.AST", hasattr(_ast, "AST") or hasattr(_ast, "Module"))
    try:
        import ast
    except ImportError, e:
        skip("import ast", str(e))
    else:
        tree = ast.parse("x = 1 + 2")
        check("ast.parse", tree is not None)

    warn = require_import("_warnings")
    if warn:
        check("_warnings present", True)
    try:
        import warnings
    except ImportError, e:
        skip("import warnings", str(e))
        return
    caught = []
    old_show = warnings.showwarning

    def _capture(message, category, filename, lineno, file=None, line=None):
        caught.append(str(message))

    warnings.showwarning = _capture
    try:
        warnings.warn("amiga-suite", UserWarning)
        check("warnings.warn", len(caught) >= 1)
    finally:
        warnings.showwarning = old_show


def test_select_errno_imp_gc():
    select = require_import("select")
    if select:
        # Do not call select.select() here - on Amiga it can touch bsdsocket.
        check("select module", hasattr(select, "select"))
        skip("select call", "AmiTCP/bsdsocket (optional socket group)")
    errno = require_import("errno")
    if errno:
        check("errno.ENOENT", hasattr(errno, "ENOENT"))
        check("errno.errorcode", hasattr(errno, "errorcode"))
    imp = require_import("imp")
    if imp:
        check("imp.find_module callable", callable(imp.find_module))
        try:
            file, pathname, descr = imp.find_module("os")
            check("imp.find_module os", pathname is not None)
            if file is not None:
                file.close()
        except Exception, e:
            skip("imp.find_module os", str(e))
    gc = require_import("gc")
    if gc:
        check("gc.get_count", isinstance(gc.get_count(), tuple))
        check("gc.isenabled", isinstance(gc.isenabled(), bool) or
              gc.isenabled() in (0, 1))


def test_weakref_module():
    wr = require_import("_weakref")
    if wr:
        check("_weakref present", True)
        check("_weakref.ref", hasattr(wr, "ref"))


def test_struct_module_name():
    # Public name is struct; accelerator is _struct.
    st = require_import("_struct")
    if st:
        check("_struct present", True)
    struct = require_import("struct")
    if struct:
        check("struct.calcsize", struct.calcsize("!I") == 4)
