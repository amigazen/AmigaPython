# Port of Amiga_Misc/testset scripts into AmigaTests.
# Sources: test_argparser.py, test_misc.py (crc32 / dates).
# test_links.py needs root + hard links (optional group).
# as.py is an ARexx+socket host demo, not a unit test.
# ASCII only (Python 2.7 / Amiga).

from __future__ import print_function

import os
import time
from AmigaTests.support import check, skip, require_import, temp_path, safe_remove


def test_crc32_vectors():
    # From Amiga_Misc/testset/test_misc.py.
    # Old SAS/C ASM vectors differed; Amiga/crc32.c is IEEE (zlib-compatible).
    amiga = require_import("amiga")
    if not amiga or not hasattr(amiga, "crc32"):
        return
    vectors = (
        ("", 0x00000000),
        ("a", 0xE8B7BE43),
        ("abc", 0x352441C2),
        ("abcdefghijklmnopqrstuvwxyz", 0x4C2750BD),
        ("ABCDEFGHIJKLMNOPQRSTUVWXYZ", 0xABF77822),
        ("123456789", 0xCBF43926),
        ("123456789012345678901234567890123456789012345678901234567890"
         "12345678901234567890  The quick brown Fox jumps over the lazy Dog",
         0xFC7C2A29),
    )
    try:
        for s, expect in vectors:
            got = amiga.crc32(s) & 0xffffffff
            label = s[:12] + ("..." if len(s) > 12 else "")
            check("crc32 %r" % (label,),
                  got == (expect & 0xffffffff),
                  "got 0x%08X want 0x%08X" % (got, expect & 0xffffffff))
    except Exception, e:
        check("crc32 vectors", False, str(e))


def test_argparser_setup():
    # From Amiga_Misc/testset/test_argparser.py
    amiga = require_import("amiga")
    if not amiga:
        return
    if not hasattr(amiga, "ArgParser"):
        skip("ArgParser", "not on amiga module")
        return
    try:
        ap = amiga.ArgParser("ONE")
        check("defaults ONE", ap.defaults == {"ONE": None})
        check("types ONE", ap.types == (("ONE", "X"),))
        ap.new("ONE,TWO")
        check("defaults TWO", ap.defaults == {"TWO": None, "ONE": None})
        check("types TWO", ap.types == (("ONE", "X"), ("TWO", "X")))
        ap.new("STR,INT/N")
        check("types STR INT", ap.types == (("STR", "X"), ("INT", "N")))
        ap.new("REQ/A,INTLIST/M/N")
        check("defaults INTLIST", ap.defaults == {"INTLIST": []})
        check("types INTLIST", ap.types == (("REQ", "X"), ("INTLIST", "I")))
        ap.new("REQ/A/K,REST/F,FLAG/S")
        check("defaults FLAG REST",
              ap.defaults == {"FLAG": 0, "REST": None})
        ap.new("BLA/M,REST/F,FLAG/S/A")
        check("defaults BLA REST",
              ap.defaults == {"BLA": [], "REST": None})
        check("types BLA",
              ap.types == (("BLA", "A"), ("REST", "X"), ("FLAG", "S")))
    except Exception, e:
        check("ArgParser setup", False, str(e))


def test_argparser_parse():
    amiga = require_import("amiga")
    if not amiga or not hasattr(amiga, "ArgParser"):
        skip("ArgParser parse", "not available")
        return
    try:
        ap = amiga.ArgParser("STR")
        check("parse string", ap.parse("string") == {"STR": "string"})
        check("parse STR string", ap.parse("STR string") == {"STR": "string"})
        check("parse empty STR", ap.parse("") == {"STR": None})

        ap.new("NUM/N")
        check("parse number", ap.parse("1234") == {"NUM": 1234})
        check("parse NUM=", ap.parse("NUM=1234") == {"NUM": 1234})

        ap.new("SW/S")
        check("parse switch", ap.parse("SW") == {"SW": -1})
        ap.defaults["SW"] = 999
        check("parse switch keep", ap.parse("SW") == {"SW": -1})
        check("parse switch default", ap.parse("") == {"SW": 999})

        ap.new("STRL/M")
        check("parse str list",
              ap.parse("foo bar foobar") ==
              {"STRL": ["foo", "bar", "foobar"]})

        ap.new("NL/M/N")
        check("parse num list", ap.parse("1 2 3") == {"NL": [1, 2, 3]})

        ap.new("STR/A")
        check("parse /A", ap.parse("string") == {"STR": "string"})

        ap.new("STR/F,SW/S")
        check("parse /F",
              ap.parse("foo bar   foobar 42") ==
              {"STR": "foo bar   foobar 42", "SW": 0})

        ap.new("STR/K")
        check("parse /K", ap.parse("STR string") == {"STR": "string"})

        ap.new("FROM/A/M,TO/A,ALL/S,QUIET/S,BUF=BUFFER/K/N")
        ap.defaults["BUF=BUFFER"] = 999
        ap.defaults["QUIET"] = 888
        check("parse multi FROM",
              ap.parse("f1 f2 f3 dest") ==
              {"BUF=BUFFER": 999, "TO": "dest", "ALL": 0,
               "QUIET": 888, "FROM": ["f1", "f2", "f3"]})
        check("parse combined",
              ap.parse("src dest ALL") ==
              {"BUF=BUFFER": 999, "TO": "dest", "ALL": -1,
               "QUIET": 888, "FROM": ["src"]})
        check("parse BUF=",
              ap.parse("src dest BUF=10") ==
              {"BUF=BUFFER": 10, "TO": "dest", "ALL": 0,
               "QUIET": 888, "FROM": ["src"]})
    except Exception, e:
        check("ArgParser parse", False, str(e))


def test_argparser_errors():
    amiga = require_import("amiga")
    if not amiga or not hasattr(amiga, "ArgParser"):
        skip("ArgParser errors", "not available")
        return
    # Dos ReadArgs failures raise amiga.doserror; amiga.error is OSError.
    err_types = (ValueError, TypeError, SystemError, IndexError)
    if hasattr(amiga, "error"):
        err_types = err_types + (amiga.error,)
    if hasattr(amiga, "doserror"):
        err_types = err_types + (amiga.doserror,)

    def expect_err(label, fn):
        try:
            fn()
            check(label, False, "expected error")
        except err_types:
            check(label, True)
        except Exception, e:
            check(label, False, "wrong error: " + str(e))

    try:
        ap = amiga.ArgParser("A")
        expect_err("bad template /", lambda: ap.new("/"))
        expect_err("bad template A/", lambda: ap.new("A/"))
        expect_err("bad template A/A/A", lambda: ap.new("A/A/A"))
        expect_err("bad template A/M/F", lambda: ap.new("A/M/F"))
        expect_err("bad template A/F/M", lambda: ap.new("A/F/M"))
        expect_err("dup name", lambda: ap.new("A,A"))
        expect_err("dup name mid", lambda: ap.new("A,B,A"))

        ap.new("A,B")
        ap.types = (1, 2)
        expect_err("bad types tuple", lambda: ap.parse("A B"))
        ap.reset()
        check("reset types", ap.types == (("A", "X"), ("B", "X")))

        ap.new("A,B")
        ap.types = (("A", "Z"), ("B", "Z"))
        expect_err("bad type letter", lambda: ap.parse("A B"))

        ap.new("")
        expect_err("empty template parse", lambda: ap.parse("foo"))
        check("empty parse ok", ap.parse("") == {})
    except Exception, e:
        check("ArgParser errors", False, str(e))


def test_touch_and_dates():
    # From Amiga_Misc/testset/test_misc.py (needs Dos date helpers).
    amiga = require_import("amiga")
    if not amiga:
        return
    need = ("touch", "Examine", "fib_Date", "DateStamp", "SetFileDate")
    missing = [n for n in need if not hasattr(amiga, n)]
    if missing:
        skip("touch/dates", "missing " + ",".join(missing))
        return
    path = temp_path("amigatest_touch")
    safe_remove(path)
    try:
        amiga.touch(path)
        check("touch create", os.path.exists(path))
        if hasattr(amiga, "StrToDate") and hasattr(amiga, "DS2time"):
            datestr = ("18-Jul-98", "13:33:33")
            datestamp = amiga.StrToDate(datestr[0], datestr[1])
            unixtime = amiga.DS2time(datestamp)
            check("DS2time", abs(unixtime - 900765213.0) < 1.0,
                  repr(unixtime))
            amiga.touch(path, unixtime)
            ds = amiga.Examine(path)[amiga.fib_Date]
            check("touch set date", ds == datestamp)
        else:
            skip("StrToDate/DS2time", "not available")
        datestamp = amiga.DateStamp()
        amiga.SetFileDate(path, datestamp)
        ds = amiga.Examine(path)[amiga.fib_Date]
        check("SetFileDate", ds == datestamp)
    except Exception, e:
        check("touch/dates", False, str(e))
    safe_remove(path)


def test_links_optional():
    # From Amiga_Misc/testset/test_links.py -- needs soft/hard links + often root.
    # Hard links to dirs need root; soft-fail without usergroup.
    skip("links", "needs usergroup + root; use optional netmods manually")
