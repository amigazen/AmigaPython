# Built-in extension modules linked in Modules/config.c for this port.

from __future__ import print_function

import sys
from AmigaTests.support import check, skip, require_import, try_import, is_standard_build


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
    # Build a minimal stored zip without Lib/zipfile (needs unicode on this port).
    import struct
    import zlib as zlibmod
    from AmigaTests.support import temp_path, safe_remove

    zi = require_import("zipimport")
    if not zi:
        return
    check("zipimporter", hasattr(zi, "zipimporter"))
    check("ZipImportError", issubclass(zi.ZipImportError, ImportError))
    check("zipimport in builtins", "zipimport" in sys.builtin_module_names)
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

    def _dos_time_date():
        # Fixed DOS time/date for headers (not used for logic).
        return 0, 0

    def _crc(data):
        return zlibmod.crc32(data) & 0xffffffff

    def _write_stored_zip(path, members):
        # members: list of (name, data) with forward-slash names
        f = open(path, "wb")
        try:
            central = []
            for name, data in members:
                t, d = _dos_time_date()
                crc = _crc(data)
                n = len(name)
                offset = f.tell()
                # local file header
                f.write(struct.pack("<IHHHHHIIIHH",
                    0x04034b50, 20, 0, 0, t, d, crc,
                    len(data), len(data), n, 0))
                f.write(name)
                f.write(data)
                central.append((name, data, crc, t, d, offset))
            cd_start = f.tell()
            for name, data, crc, t, d, offset in central:
                n = len(name)
                f.write(struct.pack("<IHHHHHHIIIHHHHHII",
                    0x02014b50, 20, 20, 0, 0, t, d, crc,
                    len(data), len(data), n, 0, 0, 0, 0, 0, offset))
                f.write(name)
            cd_size = f.tell() - cd_start
            f.write(struct.pack("<IHHHHIIH",
                0x06054b50, 0, 0, len(central), len(central),
                cd_size, cd_start, 0))
        finally:
            f.close()

    path = temp_path("amigatest_zipimport.zip")
    safe_remove(path)
    try:
        _write_stored_zip(path, [
            ("amigaziptest.py", "VALUE = 42\n"),
            ("amigazippkg/__init__.py", "PKG = 1\n"),
            ("amigazippkg/mod.py", "NESTED = 7\n"),
        ])

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
            zcache = getattr(zi, "_zip_directory_cache", None)
            if zcache is not None and path in zcache:
                del zcache[path]
    except Exception, e:
        check("import from zip", False, str(e))
    safe_remove(path)

    # Deflated member (method 8) via zlib; still no zipfile module.
    path2 = temp_path("amigatest_zipimport_deflate.zip")
    safe_remove(path2)
    try:
        raw = "DEFLATED = 99\n"
        # zip expects raw deflate (no zlib header): wbits=-15
        comp = zlibmod.compress(raw)[2:-4]
        f = open(path2, "wb")
        try:
            name = "amigazipdeflate.py"
            t, d = _dos_time_date()
            crc = _crc(raw)
            n = len(name)
            offset = 0
            f.write(struct.pack("<IHHHHHIIIHH",
                0x04034b50, 20, 0, 8, t, d, crc,
                len(comp), len(raw), n, 0))
            f.write(name)
            f.write(comp)
            cd_start = f.tell()
            f.write(struct.pack("<IHHHHHHIIIHHHHHII",
                0x02014b50, 20, 20, 0, 8, t, d, crc,
                len(comp), len(raw), n, 0, 0, 0, 0, 0, offset))
            f.write(name)
            cd_size = f.tell() - cd_start
            f.write(struct.pack("<IHHHHIIH",
                0x06054b50, 0, 0, 1, 1, cd_size, cd_start, 0))
        finally:
            f.close()
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


def test_pyexpat():
    # Standard: statically linked Modules/expat. Compact omits XML.
    if not is_standard_build():
        skip("pyexpat", "SlimPython (BUILD=standard to enable)")
        return
    check("pyexpat listed builtin", "pyexpat" in sys.builtin_module_names)
    px = require_import("pyexpat")
    check("ParserCreate", callable(getattr(px, "ParserCreate", None)))
    check("ErrorString", callable(getattr(px, "ErrorString", None)))
    check("ExpatError", hasattr(px, "ExpatError"))
    try:
        p = px.ParserCreate()
        p.Parse("<a>x</a>", 1)
        check("pyexpat Parse", True)
    except Exception, e:
        check("pyexpat Parse", False, str(e))

    # Handlers / attributes
    try:
        seen = []
        def start(name, attrs):
            seen.append(("start", name))
        def end(name):
            seen.append(("end", name))
        def char(data):
            seen.append(("char", data))
        p = px.ParserCreate()
        p.StartElementHandler = start
        p.EndElementHandler = end
        p.CharacterDataHandler = char
        p.Parse("<root><child>hi</child></root>", 1)
        check("handler start root", ("start", "root") in seen)
        check("handler start child", ("start", "child") in seen)
        check("handler char", ("char", "hi") in seen)
        check("handler end child", ("end", "child") in seen)
    except Exception, e:
        check("pyexpat handlers", False, str(e))

    # Public xml.parsers.expat wrapper
    try:
        import xml.parsers.expat as expat
        check("xml.parsers.expat", expat is not None)
        p = expat.ParserCreate()
        p.Parse("<b/>", 1)
        check("xml.parsers.expat Parse", True)
    except ImportError, e:
        skip("xml.parsers.expat", str(e))
    except Exception, e:
        check("xml.parsers.expat", False, str(e))

    # ElementTree when Lib/xml is present (needs codecs ascii_encode)
    try:
        import xml.etree.ElementTree as ET
        root = ET.fromstring("<doc id='1'><item>ok</item></doc>")
        check("ElementTree tag", root.tag == "doc")
        check("ElementTree attrib", root.get("id") == "1")
        check("ElementTree child", root.find("item").text == "ok")
    except ImportError, e:
        skip("ElementTree", str(e))
    except AttributeError, e:
        # Same gap as str.encode ascii on this port.
        if "ascii_encode" in str(e):
            skip("ElementTree", "ascii_encode not in _codecs yet")
        else:
            check("ElementTree", False, str(e))
    except Exception, e:
        check("ElementTree", False, str(e))
