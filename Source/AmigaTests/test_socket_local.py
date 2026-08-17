# Local socket / select smoke tests (no remote hosts, no SSL).
# ASCII only (Python 2.7 / Amiga).
#
# test_NN_* names control run order (sorted). Dynload must run first so a
# hang on import _socket is obvious in the log before create/select tests.

from __future__ import print_function

import sys

from AmigaTests.support import check, skip, require_import


def test_00_dynload_socket():
    check("_socket not builtin", "_socket" not in sys.builtin_module_names)
    sock = require_import("_socket", report=True)
    if sock is None:
        return
    check("_socket has socket", hasattr(sock, "socket"))
    check("_socket has AF_INET", hasattr(sock, "AF_INET"))
    check("_socket has SOCK_STREAM", hasattr(sock, "SOCK_STREAM"))
    rev = getattr(sock, "amiga_plugin_rev", None)
    check("_socket.amiga_plugin_rev==6", rev == 6, "got %r (rebuild _socket.module)" % (rev,))
    # Import success means ensure_bsdsocket() returned OK (SocketBase live).
    print("  NOTE: _socket import OK => bsdsocket.library was opened")
    if rev is not None:
        print("  NOTE: amiga_plugin_rev =", rev)
    sys.stdout.flush()


def test_01_import_socket_package():
    sock = require_import("socket")
    if not sock:
        return
    check("socket.socket", callable(sock.socket))
    check("socket.AF_INET", sock.AF_INET == 2 or isinstance(sock.AF_INET, (int, long)))
    check("socket.SOCK_STREAM", hasattr(sock, "SOCK_STREAM"))
    check("socket.error", hasattr(sock, "error"))


def test_02_create_close_tcp():
    socket = require_import("socket")
    if not socket:
        return
    s = None
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        check("create SOCK_STREAM", s is not None)
        if hasattr(s, "settimeout"):
            s.settimeout(1.0)
            check("settimeout", True)
        if hasattr(s, "fileno"):
            fd = s.fileno()
            check("fileno", isinstance(fd, (int, long)))
        s.close()
        s = None
        check("close SOCK_STREAM", True)
    except Exception, e:
        skip("create/close TCP", str(e))
    finally:
        if s is not None:
            try:
                s.close()
            except Exception:
                pass


def test_03_create_close_udp():
    socket = require_import("socket")
    if not socket:
        return
    s = None
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        check("create SOCK_DGRAM", s is not None)
        s.close()
        s = None
        check("close SOCK_DGRAM", True)
    except Exception, e:
        skip("create/close UDP", str(e))
    finally:
        if s is not None:
            try:
                s.close()
            except Exception:
                pass


def test_04_gethostname_local():
    socket = require_import("socket")
    if not socket:
        return
    try:
        name = socket.gethostname()
        check("gethostname", isinstance(name, basestring) and len(name) > 0, repr(name))
    except Exception, e:
        skip("gethostname", str(e))
        return
    try:
        ip = socket.gethostbyname(name)
        check("gethostbyname(hostname)", isinstance(ip, basestring) and len(ip) > 0, repr(ip))
    except Exception, e:
        skip("gethostbyname(hostname)", str(e))
    try:
        ip2 = socket.gethostbyname("127.0.0.1")
        check("gethostbyname(127.0.0.1)",
              ip2 == "127.0.0.1" or isinstance(ip2, basestring), repr(ip2))
    except Exception, e:
        skip("gethostbyname(127.0.0.1)", str(e))


def test_05_inet_aton_ntoa():
    socket = require_import("socket")
    if not socket:
        return
    if not hasattr(socket, "inet_aton"):
        skip("inet_aton", "missing")
        return
    try:
        packed = socket.inet_aton("192.168.1.1")
        check("inet_aton", isinstance(packed, basestring) and len(packed) == 4)
        if hasattr(socket, "inet_ntoa"):
            text = socket.inet_ntoa(packed)
            check("inet_ntoa", text == "192.168.1.1", repr(text))
    except Exception, e:
        skip("inet_aton/ntoa", str(e))


def test_06_bind_ephemeral_localhost():
    socket = require_import("socket")
    if not socket:
        return
    s = None
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        if hasattr(s, "settimeout"):
            s.settimeout(2.0)
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
        skip("bind ephemeral", str(e))
    finally:
        if s is not None:
            try:
                s.close()
            except Exception:
                pass


def test_07_timeout_attribute():
    socket = require_import("socket")
    if not socket:
        return
    s = None
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        if not hasattr(s, "gettimeout"):
            skip("gettimeout", "missing")
            return
        check("default timeout is None or number",
              s.gettimeout() is None or isinstance(s.gettimeout(), float))
        s.settimeout(2.5)
        check("gettimeout after set",
              abs(s.gettimeout() - 2.5) < 0.01, repr(s.gettimeout()))
        s.settimeout(None)
        check("clear timeout", s.gettimeout() is None)
    except Exception, e:
        skip("timeout attribute", str(e))
    finally:
        if s is not None:
            try:
                s.close()
            except Exception:
                pass


def test_08_select_empty():
    select = require_import("select")
    if not select:
        return
    try:
        r, w, x = select.select([], [], [], 0)
        check("select empty", r == [] and w == [] and x == [])
    except Exception, e:
        skip("select empty", str(e))


def test_09_select_writable_fd():
    socket = require_import("socket")
    select = require_import("select")
    if not socket or not select:
        return
    s = None
    try:
        # AmiTCP does not report an unconnected SOCK_STREAM as writable.
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        r, w, x = select.select([], [s], [], 0.5)
        check("select writable fd", s in w, "w=%r" % (w,))
    except Exception, e:
        skip("select on socket", str(e))
    finally:
        if s is not None:
            try:
                s.close()
            except Exception:
                pass


def test_10_local_tcp_echo():
    """Loopback send/recv - carve-out proof without remote HTTP/MTU."""
    socket = require_import("socket")
    if not socket:
        return

    payload = "AmigaPython-echo-ping"
    srv = None
    cli = None
    acc = None
    try:
        srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        except Exception:
            pass
        srv.bind(("127.0.0.1", 0))
        srv.listen(1)
        if hasattr(srv, "settimeout"):
            srv.settimeout(5.0)
        host, port = srv.getsockname()
        check("local listen", isinstance(port, (int, long)) and port > 0,
              "port=%r" % (port,))

        cli = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        if hasattr(cli, "settimeout"):
            cli.settimeout(5.0)
        cli.connect((host, port))

        acc, peer = srv.accept()
        if hasattr(acc, "settimeout"):
            acc.settimeout(5.0)
        check("local accept", True, "peer=%r" % (peer,))

        n = cli.send(payload)
        check("local client send", n == len(payload), "n=%r" % (n,))
        got = acc.recv(64)
        check("local server recv", got == payload, repr(got))
        acc.sendall(got)
        back = cli.recv(64)
        check("local echo roundtrip", back == payload, repr(back))
    except Exception, e:
        check("local tcp echo", False, str(e))
    finally:
        for s in (acc, cli, srv):
            if s is not None:
                try:
                    s.close()
                except Exception:
                    pass
