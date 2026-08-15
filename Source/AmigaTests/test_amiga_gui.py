# Optional GUI smoke (ASL MessageBox). Not in the default suite.

from __future__ import print_function

from AmigaTests.support import check, skip, require_import


def test_messagebox_import():
    amiga = require_import("amiga")
    if not amiga:
        return
    if not hasattr(amiga, "MessageBox"):
        skip("MessageBox", "not built")
        return
    # Do not actually open a requester in automated runs unless asked.
    skip("MessageBox popup", "optional GUI; call amiga.MessageBox manually")


def test_filerequest_import():
    try:
        import asl
        check("asl.FileRequest", hasattr(asl, "FileRequest"))
        check("asl.MessageBox", hasattr(asl, "MessageBox"))
    except Exception, e:
        skip("import asl", str(e))
