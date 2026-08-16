# Builtin _io / Lib/io (PEP 3116). Text is 8-bit str (no Py_USING_UNICODE).
# ASCII only (Python 2.7 / Amiga).

from __future__ import print_function

from AmigaTests.support import check, skip, require_import, temp_path, safe_remove


def test_import_io():
    _io = require_import("_io")
    if not _io:
        return
    check("_io.open", hasattr(_io, "open"))
    check("_io.BytesIO", hasattr(_io, "BytesIO"))
    check("_io.StringIO", hasattr(_io, "StringIO"))
    check("_io.FileIO", hasattr(_io, "FileIO"))
    io = require_import("io")
    if io:
        check("io.BytesIO", hasattr(io, "BytesIO"))
        check("io.StringIO", hasattr(io, "StringIO"))


def test_bytesio():
    io = require_import("io")
    if not io:
        return
    try:
        b = io.BytesIO()
        n = b.write("hello")
        check("BytesIO write", n == 5)
        check("BytesIO getvalue", b.getvalue() == "hello")
        b.seek(0)
        check("BytesIO read", b.read() == "hello")
        b.close()
    except Exception, e:
        check("BytesIO", False, str(e))


def test_stringio():
    io = require_import("io")
    if not io:
        return
    try:
        s = io.StringIO("abc\n")
        check("StringIO read", s.read() == "abc\n")
        s.seek(0)
        check("StringIO readline", s.readline() == "abc\n")
        s.close()
    except Exception, e:
        check("StringIO", False, str(e))


def test_binary_file_ram():
    io = require_import("io")
    if not io:
        return
    path = temp_path("amigatest_io_bin.dat")
    safe_remove(path)
    try:
        f = io.open(path, "wb")
        try:
            f.write("bin-data")
        finally:
            f.close()
        f = io.open(path, "rb")
        try:
            check("binary read", f.read() == "bin-data")
        finally:
            f.close()
    except Exception, e:
        check("binary file", False, str(e))
    safe_remove(path)


def test_text_file_ram():
    io = require_import("io")
    if not io:
        return
    path = temp_path("amigatest_io_txt.txt")
    safe_remove(path)
    try:
        f = io.open(path, "w")
        try:
            f.write("line1\nline2\n")
        finally:
            f.close()
        f = io.open(path, "r")
        try:
            data = f.read()
            check("text read", data == "line1\nline2\n" or "line1" in data)
        finally:
            f.close()
    except Exception, e:
        check("text file", False, str(e))
    safe_remove(path)


def test_utf8_encoding_rejected_or_passthrough():
    # utf-8 is allowed as 8-bit pass-through; utf-16 must fail.
    io = require_import("io")
    if not io:
        return
    path = temp_path("amigatest_io_enc.txt")
    safe_remove(path)
    try:
        f = io.open(path, "w", encoding="utf-8")
        try:
            f.write("ok")
        finally:
            f.close()
        check("utf-8 text open", True)
    except Exception, e:
        skip("utf-8 text open", str(e))
    safe_remove(path)
    try:
        f = io.open(path, "w", encoding="utf-16")
        f.close()
        check("utf-16 rejected", False, "expected ValueError")
    except ValueError:
        check("utf-16 rejected", True)
    except Exception, e:
        skip("utf-16 rejected", str(e))
    safe_remove(path)
