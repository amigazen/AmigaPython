#!/usr/bin/env python
# Amiga HTTPS smoke test - public ssl/socket API, not only _wrap_socket.
#
# Exercises the same path users type at the REPL:
#   connect((hostname, 443)) + ssl.wrap_socket(sock) + send/recv
# Also one SSLContext.wrap_socket(..., server_hostname=) path (SNI).
#
# AmiTLS rev 12+ maps peer-close READ_FAILED to recv '' (not 8808).
# AmiSSL rev 10. Do not settimeout (FIONBIO).
#
#   python27 AmigaTests/run.py ssl ssl_net
#   python27 AmigaTests/test_ssl_net.py amiga.com
#
# ASCII only (Python 2.7 / Amiga).

from __future__ import print_function

import os
import sys
import time

_HERE = os.path.dirname(os.path.abspath(__file__))
_ROOT = os.path.dirname(_HERE)
if _ROOT not in sys.path:
    sys.path.insert(0, _ROOT)

from AmigaTests.support import check, skip, reset_counters, summary

DEFAULT_HOSTS = ("www.google.com",)
DEFAULT_TIMEOUT = 20.0

HOSTS = list(DEFAULT_HOSTS)
TIMEOUT = DEFAULT_TIMEOUT
SOFT_EXTRA = True


def test_00_import_ssl():
    check("_ssl not builtin", "_ssl" not in sys.builtin_module_names)
    try:
        import _ssl
    except Exception, e:
        skip("import _ssl", str(e))
        return None
    rev = getattr(_ssl, "amiga_plugin_rev", None)
    check("_ssl.amiga_plugin_rev in (10,12)", rev in (10, 12),
          "got %r (10=AmiSSL 12=AmiTLS; rebuild _ssl.module)" % (rev,))
    print("    NOTE: OPENSSL_VERSION =",
          getattr(_ssl, "OPENSSL_VERSION", "?"))
    try:
        import ssl
        import socket
    except Exception, e:
        skip("import ssl/socket", str(e))
        return None
    return ssl, socket


def _http_ok(first):
    if not first.startswith("HTTP/1."):
        return False
    return (
        " 200 " in first[:32]
        or " 301 " in first[:32]
        or " 302 " in first[:32]
        or " 303 " in first[:32]
        or " 307 " in first[:32]
        or " 308 " in first[:32]
    )


def _read_http_status(sslobj, socket, t0, timeout, drain_eof=0):
    chunks = []
    n = 0
    deadline = t0 + timeout
    saw_status = 0
    while n < 8192:
        if time.time() > deadline:
            raise socket.timeout("HTTPS read exceeded %.1fs" % timeout)
        data = sslobj.recv(1024)
        if not data:
            break
        chunks.append(data)
        n += len(data)
        body_so_far = "".join(chunks)
        if not saw_status:
            if "\r\n" in body_so_far or "\n" in body_so_far:
                if body_so_far.startswith("HTTP/1."):
                    saw_status = 1
                    if not drain_eof:
                        break
    body = "".join(chunks)
    first = body.split("\r\n", 1)[0] if body else ""
    if not first and body:
        first = body.split("\n", 1)[0]
    return body, first


def _https_wrap_socket_api(ssl, socket, host, timeout, hard):
    """Classic ssl.wrap_socket after connect((hostname, port))."""
    sock = None
    sslobj = None
    t0 = time.time()
    label = "ssl.wrap_socket GET https://%s/" % host
    try:
        print("    ---- %s (wrap_socket) ----" % host)
        sys.stdout.flush()
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((host, 443))
        print("    tcp connect((%r, 443))" % host)
        sys.stdout.flush()
        # Public API: no server_hostname (AmiTLS must not pass NULL to
        # TlsAttachSocket -- that was 8813 Invalid handle).
        sslobj = ssl.wrap_socket(sock)
        print("    ssl.wrap_socket ok")
        sys.stdout.flush()
        req = (
            "GET / HTTP/1.0\r\n"
            "Host: %s\r\n"
            "\r\n"
        ) % host
        nsent = sslobj.send(req)
        print("    sent %d bytes" % nsent)
        sys.stdout.flush()
        print("    reading...")
        sys.stdout.flush()
        # drain_eof=1: keep recv until '' so peer-close is not 8808.
        body, first = _read_http_status(sslobj, socket, t0, timeout, 1)
        dt = time.time() - t0
        print("    recv %d bytes in %.2fs" % (len(body), dt))
        print("    status:", first)
        sys.stdout.flush()
        ok = _http_ok(first)
        if hard:
            check(label, ok, "prefix=%r" % (body[:48],))
        elif ok:
            print("    NOTE: optional host OK:", label)
        else:
            print("    NOTE: optional host weak response:", repr(first[:48]))
        sys.stdout.flush()
    except Exception, e:
        dt = time.time() - t0
        if hard:
            check(label, False, "%s after %.2fs" % (str(e), dt))
        else:
            print("    NOTE: optional host failed: %s after %.2fs" % (
                str(e), dt))
        sys.stdout.flush()
    sslobj = None
    sock = None


def _https_context_sni_api(ssl, socket, host, timeout, hard):
    """SSLContext.wrap_socket with server_hostname (SNI)."""
    sock = None
    sslobj = None
    t0 = time.time()
    label = "SSLContext.wrap_socket SNI GET https://%s/" % host
    try:
        print("    ---- %s (context+SNI) ----" % host)
        sys.stdout.flush()
        ctx = ssl._create_unverified_context()
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((host, 443))
        print("    tcp connect((%r, 443))" % host)
        sys.stdout.flush()
        sslobj = ctx.wrap_socket(sock, server_hostname=host)
        print("    context.wrap_socket(server_hostname=%r) ok" % host)
        sys.stdout.flush()
        req = (
            "GET / HTTP/1.0\r\n"
            "Host: %s\r\n"
            "\r\n"
        ) % host
        nsent = sslobj.send(req)
        print("    sent %d bytes" % nsent)
        sys.stdout.flush()
        body, first = _read_http_status(sslobj, socket, t0, timeout)
        dt = time.time() - t0
        print("    recv %d bytes in %.2fs" % (len(body), dt))
        print("    status:", first)
        if hasattr(sslobj, "version"):
            print("    TLS version:", sslobj.version())
        if hasattr(sslobj, "cipher"):
            print("    cipher:", sslobj.cipher())
        sys.stdout.flush()
        ok = _http_ok(first)
        if hard:
            check(label, ok, "prefix=%r" % (body[:48],))
        elif ok:
            print("    NOTE: optional host OK:", label)
        else:
            print("    NOTE: optional host weak response:", repr(first[:48]))
        sys.stdout.flush()
    except Exception, e:
        dt = time.time() - t0
        if hard:
            check(label, False, "%s after %.2fs" % (str(e), dt))
        else:
            print("    NOTE: optional host failed: %s after %.2fs" % (
                str(e), dt))
        sys.stdout.flush()
    sslobj = None
    sock = None


def test_01_https_get():
    try:
        import ssl as sslmod
        import socket as socketmod
    except Exception, e:
        skip("HTTPS GET", str(e))
        return
    if not HOSTS:
        skip("HTTPS GET", "no hosts")
        return
    i = 0
    while i < len(HOSTS):
        hard = True
        if SOFT_EXTRA and i > 0:
            hard = False
        _https_wrap_socket_api(sslmod, socketmod, HOSTS[i], TIMEOUT, hard)
        # SNI path: hard only for first host (same suite budget).
        _https_context_sni_api(sslmod, socketmod, HOSTS[i], TIMEOUT, hard)
        i = i + 1


def main(argv=None):
    global HOSTS, TIMEOUT, SOFT_EXTRA
    if argv is None:
        argv = sys.argv[1:]
    if argv and argv[0] in ("-h", "--help"):
        print("Usage: python27 AmigaTests/test_ssl_net.py [host ...] [timeout]")
        print("  Default: www.google.com (hard). Extra hosts are soft.")
        return 0
    hosts = []
    timeout = DEFAULT_TIMEOUT
    i = 0
    while i < len(argv):
        a = argv[i]
        try:
            timeout = float(a)
        except ValueError:
            hosts.append(a)
        i = i + 1
    if hosts:
        HOSTS = hosts
        SOFT_EXTRA = len(hosts) > 1
    TIMEOUT = timeout
    reset_counters()
    test_00_import_ssl()
    test_01_https_get()
    ok = summary()
    sys.stdout.flush()
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
