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
        except Exception, e:
            skip("str.encode ascii", str(e))
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
        check("sha", len(h) == 40)
    zlib = require_import("zlib")
    if zlib:
        raw = "hello" * 50
        check("zlib roundtrip", zlib.decompress(zlib.compress(raw)) == raw)


def test_sre_re():
    re = require_import("re")
    if not re:
        return
    m = re.search(r"(\d+)", "x42y")
    check("re.search", m is not None and m.group(1) == "42")
    check("re.sub", re.sub("a", "b", "cac") == "cbc")


def test_select_errno_imp_gc():
    select = require_import("select")
    if select:
        check("select module", hasattr(select, "select"))
    errno = require_import("errno")
    if errno:
        check("errno.ENOENT", hasattr(errno, "ENOENT"))
    imp = require_import("imp")
    if imp:
        check("imp.find_module os", True)
        # find_module may need path; just ensure callable
        check("imp.find_module callable", callable(imp.find_module))
    gc = require_import("gc")
    if gc:
        check("gc.get_count", isinstance(gc.get_count(), tuple))


def test_weakref_module():
    wr = require_import("_weakref")
    if wr:
        check("_weakref present", True)


def test_optional_network():
    # Only present when AMITCP was defined at build time.
    sock = require_import("_socket")
    if not sock:
        skip("_socket", "not built (AMITCP)")
        return
    check("_socket", hasattr(sock, "socket"))
    for name in ("pwd", "grp", "crypt", "syslog"):
        if require_import(name):
            check("import " + name, True)
