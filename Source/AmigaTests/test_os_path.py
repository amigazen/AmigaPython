# os / posixpath / stat_result Amiga path tests.

from __future__ import print_function

import os
import sys
from AmigaTests.support import check, skip, temp_path, safe_remove


def test_os_basics():
    check("os.name", os.name == "amiga")
    check("os.getcwd", isinstance(os.getcwd(), basestring) and len(os.getcwd()) > 0)
    check("os.path is posixpath", os.path.__name__ == "posixpath")


def test_amiga_path_ops():
    p = os.path
    check("isabs volume", p.isabs("RAM:"))
    check("isabs volume:path", p.isabs("RAM:T"))
    check("not isabs relative", not p.isabs("relative/name"))
    check("join volume", p.join("RAM:", "t") in ("RAM:t", "RAM:t/"))
    check("join multi", p.basename(p.join("RAM:", "a", "x")) == "x")
    drive, tail = p.splitdrive("RAM:Foo/Bar")
    check("splitdrive", drive.endswith(":") and "Foo" in tail)
    check("basename", p.basename("RAM:Foo/Bar") == "Bar")
    check("dirname", "Foo" in p.dirname("RAM:Foo/Bar") or p.dirname("RAM:Foo/Bar").endswith("Foo"))
    check("normpath keeps volume", ":" in p.normpath("RAM:Foo/Bar"))


def test_lib_discovery():
    lib = None
    for entry in sys.path:
        if entry and os.path.isdir(entry) and os.path.exists(os.path.join(entry, "os.py")):
            lib = entry
            break
    check("Lib on sys.path", lib is not None, repr(sys.path[:4]))
    if not lib:
        return
    check("isdir(Lib)", os.path.isdir(lib))
    check("isfile(os.py)", os.path.isfile(os.path.join(lib, "os.py")))
    check("exists(os.py)", os.path.exists(os.path.join(lib, "os.py")))


def test_stat_result():
    lib = None
    for entry in sys.path:
        if entry and os.path.exists(os.path.join(entry, "os.py")):
            lib = entry
            break
    if not lib:
        skip("stat_result", "no Lib found")
        return
    path = os.path.join(lib, "os.py")
    st = os.stat(path)
    check("st_mode attr", hasattr(st, "st_mode"))
    check("st_size attr", hasattr(st, "st_size"))
    check("st_mtime attr", hasattr(st, "st_mtime"))
    check("indexable", st[0] == st.st_mode and st[6] == st.st_size)
    check("size > 0", st.st_size > 0)
    check("isinstance tuple", isinstance(st, tuple))
    lst = None
    try:
        lst = os.lstat(path)
    except OSError, e:
        skip("lstat", str(e))
    if lst is not None:
        check("lstat attrs", hasattr(lst, "st_size") and lst.st_size == st.st_size)


def test_listdir_mkdir_roundtrip():
    if getattr(sys, "platform", "") == "amiga":
        base = "RAM:amigatest_dir"
        child = "RAM:amigatest_dir/child"
        nested = "RAM:amigatest_dir/child/file.txt"
    else:
        base = os.path.join(".", "amigatest_dir")
        child = os.path.join(base, "child")
        nested = os.path.join(child, "file.txt")

    safe_remove(nested)
    safe_remove(child)
    safe_remove(base)

    try:
        os.mkdir(base)
        check("mkdir base", os.path.isdir(base))
        os.mkdir(child)
        check("mkdir child", os.path.isdir(child))
        names = os.listdir(base)
        check("listdir sees child", "child" in names)
        f = open(nested, "w")
        try:
            f.write("x\n")
        finally:
            f.close()
        check("file in nested", os.path.isfile(nested))
        st = os.stat(nested)
        check("nested st_size", st.st_size >= 2)
    finally:
        safe_remove(nested)
        safe_remove(child)
        safe_remove(base)


def test_file_io_ram():
    path = temp_path("amigatest_io.txt")
    safe_remove(path)
    data = "hello-amiga-port\n"
    try:
        f = open(path, "w")
        try:
            f.write(data)
        finally:
            f.close()
        f = open(path, "r")
        try:
            got = f.read()
        finally:
            f.close()
        check("write/read", got == data)
        f = open(path, "r")
        try:
            line = f.readline()
        finally:
            f.close()
        check("readline", line == data)
    finally:
        safe_remove(path)


def test_rename_unlink():
    a = temp_path("amigatest_a.txt")
    b = temp_path("amigatest_b.txt")
    safe_remove(a)
    safe_remove(b)
    try:
        f = open(a, "w")
        try:
            f.write("z")
        finally:
            f.close()
        os.rename(a, b)
        check("rename", os.path.exists(b) and not os.path.exists(a))
        os.unlink(b)
        check("unlink", not os.path.exists(b))
    finally:
        safe_remove(a)
        safe_remove(b)
