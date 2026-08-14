# Local socket / select smoke tests (no remote hosts, no SSL).
# ASCII only (Python 2.7 / Amiga).

from __future__ import print_function

import sys

from AmigaTests.support import check, skip, require_import


def test_dynload_socket():
    # Must NOT be a builtin; must LoadSeg from lib-dynload.
    check("_socket not builtin", "_socket" not in sys.builtin_module_names)
    sock = require_import("_socket")
    if sock is None:
        return
    check("_socket has socket", hasattr(sock, "socket"))
    check("_socket has AF_INET", hasattr(sock, "AF_INET"))
    check("_socket has SOCK_STREAM", hasattr(sock, "SOCK_STREAM"))


def test_import_socket_package():
    sock = require_import("socket")
    if not sock:
        return
    check("socket.socket", callable(sock.socket))
    check("socket.AF_INET", sock.AF_INET == 2 or isinstance(sock.AF_INET, (int, long)))
    check("socket.SOCK_STREAM", hasattr(sock, "SOCK_STREAM"))
    check("socket.error", hasattr(sock, "error"))


def test_create_close_tcp():
    socket = require_import("socket")
    if not socket:
        return
    s = None
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        check("create SOCK_STREAM", s is not None)
        if hasattr(s, "settimeout"):
            s.settimeout(2.0)
            check("settimeout", True)
        if hasattr(s, "fileno"):
            fd = s.fileno()
            check("fileno", isinstance(fd, (int, long)))
    except Exception, e:
        check("create SOCK_STREAM", False, str(e))
    if s is not None:
        try:
            s.close()
            check("close", True)
        except Exception, e:
            check("close", False, str(e))


def test_create_udp():
    socket = require_import("socket")
    if not socket:
        return
    s = None
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        check("create SOCK_DGRAM", s is not None)
    except Exception, e:
        check("create SOCK_DGRAM", False, str(e))
    if s is not None:
        try:
            s.close()
        except Exception:
            pass


def test_local_dns():
    socket = require_import("socket")
    if not socket:
        return
    try:
        name = socket.gethostname()
        check("gethostname", isinstance(name, basestring) and len(name) > 0)
    except Exception, e:
        check("gethostname", False, str(e))
        return
    try:
        ip = socket.gethostbyname(name)
        check("gethostbyname(local)",
              isinstance(ip, basestring) and ip.count(".") == 3,
              repr(ip))
    except Exception, e:
        skip("gethostbyname(local)", str(e))
    # Numeric dotted form must not need a nameserver.
    try:
        ip2 = socket.gethostbyname("127.0.0.1")
        check("gethostbyname(127.0.0.1)", ip2 == "127.0.0.1", repr(ip2))
    except Exception, e:
        skip("gethostbyname(127.0.0.1)", str(e))


def test_inet_aton_ntoa():
    socket = require_import("socket")
    if not socket:
        return
    if not hasattr(socket, "inet_aton"):
        skip("inet_aton", "missing")
        return
    try:
        packed = socket.inet_aton("192.168.1.1")
        check("inet_aton len", len(packed) == 4)
        if hasattr(socket, "inet_ntoa"):
            text = socket.inet_ntoa(packed)
            check("inet_ntoa", text == "192.168.1.1", repr(text))
    except Exception, e:
        check("inet_aton/ntoa", False, str(e))


def test_bind_ephemeral_localhost():
    # Bind only; do not connect to the public Internet.
    socket = require_import("socket")
    if not socket:
        return
    s = None
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        if hasattr(s, "settimeout"):
            s.settimeout(2.0)
        # Prefer loopback; some AmiTCP installs lack 127.0.0.1.
        bound = False
        last_err = None
        for host in ("127.0.0.1", "0.0.0.0"):
            try:
                s.bind((host, 0))
                bound = True
                addr = s.getsockname()
                check("bind ephemeral",
                      isinstance(addr, tuple) and addr[1] > 0,
                      repr(addr))
                break
            except Exception, e:
                last_err = e
        if not bound:
            skip("bind ephemeral", str(last_err))
    except Exception, e:
        check("bind ephemeral", False, str(e))
    if s is not None:
        try:
            s.close()
        except Exception:
            pass


def test_select_timeout():
    select = require_import("select")
    socket = require_import("socket")
    if not select:
        return
    # Zero-timeout select on empty sets is the safest portable call.
    try:
        r, w, x = select.select([], [], [], 0)
        check("select empty", r == [] and w == [] and x == [])
    except Exception, e:
        check("select empty", False, str(e))
        return
    if not socket:
        return
    s = None
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        if hasattr(s, "setblocking"):
            s.setblocking(0)
        r, w, x = select.select([], [s], [], 0)
        check("select writable fd", isinstance(w, list))
    except Exception, e:
        skip("select on socket", str(e))
    if s is not None:
        try:
            s.close()
        except Exception:
            pass
