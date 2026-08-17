#!/usr/bin/env python
# Amiga socket TCP smoke test (no SSL) - carve-out proof for _socket.module.
#
# Includes DNS + HTTP/1.0 GET (connect/send/recv). That is the real end-to-end
# check; local bind/select alone does not exercise the host PosixLib path fully.
#
# Default AmigaTests/run.py does NOT include this group (needs live DNS/HTTP).
# Run:
#   python27 AmigaTests/run.py socket_net
#   python27 AmigaTests/test_socket_net.py
#   python27 AmigaTests/test_socket_net.py example.com 30
#
# ASCII only (Python 2.7 / Amiga).

from __future__ import print_function

import os
import sys
import struct
import time

_HERE = os.path.dirname(os.path.abspath(__file__))
_ROOT = os.path.dirname(_HERE)
if _ROOT not in sys.path:
    sys.path.insert(0, _ROOT)

from AmigaTests.support import check, skip, reset_counters, summary

# info.cern.ch / neverssl.com have timed out after connect from this stack.
DEFAULT_HOST = "www.google.com"
DEFAULT_TIMEOUT = 15.0

# Overridden by main() when run as a script; suite uses defaults.
HOST = DEFAULT_HOST
TIMEOUT = DEFAULT_TIMEOUT
_SOCKET = None
_IP = None


def _icmp_checksum(data):
    if len(data) & 1:
        data = data + "\0"
    s = 0
    for i in xrange(0, len(data), 2):
        w = (ord(data[i]) << 8) + ord(data[i + 1])
        s = s + w
    s = (s >> 16) + (s & 0xffff)
    s = s + (s >> 16)
    return (~s) & 0xffff


def test_01_import_socket():
    global _SOCKET
    _SOCKET = None
    try:
        import socket
    except ImportError, e:
        check("import socket", False, str(e))
        return
    check("import socket", True)
    _SOCKET = socket


def test_02_dns():
    global _IP
    _IP = None
    socket = _SOCKET
    if socket is None:
        skip("dns", "no socket module")
        return
    host = HOST
    try:
        name = socket.gethostname()
        check("gethostname", isinstance(name, basestring) and len(name) > 0,
              repr(name))
        print("    hostname:", name)
    except Exception, e:
        check("gethostname", False, str(e))
        name = None

    if name:
        try:
            lip = socket.gethostbyname(name)
            check("gethostbyname(local)", isinstance(lip, basestring),
                  repr(lip))
            print("    local ip:", lip)
        except Exception, e:
            skip("gethostbyname(local)", str(e))

    try:
        ip = socket.gethostbyname(host)
        check("gethostbyname(%s)" % host,
              isinstance(ip, basestring) and ip.count(".") == 3,
              repr(ip))
        print("    %s -> %s" % (host, ip))
        _IP = ip
    except Exception, e:
        check("gethostbyname(%s)" % host, False, str(e))


def test_03_http_get():
    """Plain HTTP/1.0 GET - end-to-end proof of _socket.module (DNS+TCP+I/O)."""
    global _IP
    socket = _SOCKET
    host = HOST
    timeout = TIMEOUT
    if socket is None:
        skip("http GET", "no socket module")
        return

    # Resolve here so a prior DNS FAIL/skip does not skip the real smoke test.
    ip = _IP
    if ip is None:
        try:
            ip = socket.gethostbyname(host)
            check("http DNS %s" % host,
                  isinstance(ip, basestring) and ip.count(".") == 3,
                  repr(ip))
            _IP = ip
            print("    %s -> %s" % (host, ip))
        except Exception, e:
            check("http DNS %s" % host, False, str(e))
            return

    req = (
        "GET / HTTP/1.0\r\n"
        "Host: %s\r\n"
        "User-Agent: AmigaPython-socket-test/1.0\r\n"
        "Connection: close\r\n"
        "\r\n"
    ) % host

    sock = None
    t0 = time.time()
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        if hasattr(sock, "settimeout"):
            sock.settimeout(timeout)

        # Connect by dotted IP (gethostbyname already proved DNS). Avoids
        # setipaddr/getaddrinfo on the connect path for this smoke test.
        peer = (ip, 80)
        try:
            sock.connect(peer)
            check("tcp connect :80", True,
                  "peer=%r elapsed=%.2fs" % (peer, time.time() - t0))
        except socket.timeout:
            check("http GET", False,
                  "connect timed out after %.1fs (peer=%r)" % (timeout, peer))
            return
        except Exception, e:
            check("http GET", False, "%s (peer=%r)" % (e, peer))
            return

        sock.sendall(req)
        print("    sent", len(req), "bytes to", peer[0])
        try:
            who = sock.getpeername()
            print("    getpeername:", who)
        except Exception, e:
            print("    getpeername failed:", e)
        sys.stdout.flush()

        chunks = []
        total = 0
        while total < 8192:
            try:
                data = sock.recv(1024)
            except socket.timeout:
                if chunks:
                    break
                check("http GET", False,
                      "recv timed out after %.1fs (connected ok, sent %d)" % (
                          timeout, len(req)))
                return
            except Exception, e:
                if getattr(e, "errno", None) == 4 or "Interrupted" in str(e):
                    if time.time() - t0 < timeout:
                        continue
                if chunks:
                    break
                check("http recv", False, str(e))
                return
            if not data:
                break
            chunks.append(data)
            total = total + len(data)
        elapsed = time.time() - t0
        body = "".join(chunks)
        # Hex only - raw binary on Amiga console has corrupted follow-on GC.
        head = body[:16]
        hexhead = " ".join(["%02x" % ord(c) for c in head])
        print("    recv %d bytes in %.2fs  head: %s" % (
            len(body), elapsed, hexhead))
        sys.stdout.flush()
        is_http = body.startswith("HTTP/1.") and (
            "\r\n" in body or "\n" in body)
        if is_http:
            first = body.split("\r\n", 1)[0].split("\n", 1)[0]
            safe = "".join([c if 32 <= ord(c) < 127 else "." for c in first])
            low = body.lower()
            has_html = ("text/html" in low or "<!doctype" in low
                        or "<html" in low)
            # Same request from a modern host yields HTTP/1.0 200 + HTML.
            check("http GET response", "200" in first and has_html,
                  "bytes=%d elapsed=%.2fs status=%r html=%r" % (
                      len(body), elapsed, safe, has_html))
            print("    status:", safe)
            if "text/html" in low:
                print("    NOTE: Content-Type text/html present")
            if "<!doctype" in low or "<html" in low:
                print("    NOTE: HTML body marker present")
        elif len(body) > 0:
            check("http GET tcp i/o", True,
                  "non-HTTP payload bytes=%d (head %s)" % (
                      len(body), hexhead))
        else:
            check("http GET response", False, "empty recv")
        sys.stdout.flush()
    except socket.timeout:
        check("http GET", False, "timed out after %.1fs" % timeout)
    except Exception, e:
        check("http GET", False, str(e))
    finally:
        # Always close: early return used to skip close, then SOCK_RAW ran
        # on a live half-open TCP session (Amiga stacks often hard-crash).
        if sock is not None:
            try:
                if hasattr(sock, "settimeout"):
                    sock.settimeout(None)
            except Exception:
                pass
            try:
                sock.close()
            except Exception:
                pass


def test_04_icmp_ping():
    """ICMP echo via SOCK_RAW. Opt-in only - raw sockets can crash AmiTCP."""
    # Default skip: opening SOCK_RAW has hard-crashed stacks after HTTP tests.
    if os.environ.get("AMIGA_TEST_ICMP") != "1":
        skip("icmp ping", "set AMIGA_TEST_ICMP=1 to enable (SOCK_RAW risk)")
        return

    socket = _SOCKET
    ip = _IP
    timeout = TIMEOUT
    if socket is None:
        skip("icmp ping", "no socket module")
        return
    if ip is None:
        skip("icmp ping", "no resolved address")
        return

    if not hasattr(socket, "SOCK_RAW"):
        skip("icmp ping", "no SOCK_RAW")
        return

    proto = getattr(socket, "IPPROTO_ICMP", 1)
    sock = None
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_RAW, proto)
    except Exception, e:
        skip("icmp ping", "SOCK_RAW unavailable: %s" % e)
        return

    try:
        if hasattr(sock, "settimeout"):
            sock.settimeout(timeout)
        ident = os.getpid() & 0xffff
        seq = 1
        header = struct.pack("!BBHHH", 8, 0, 0, ident, seq)
        payload = "AmigaPython" + struct.pack("!d", time.time())
        chk = _icmp_checksum(header + payload)
        packet = struct.pack("!BBHHH", 8, 0, chk, ident, seq) + payload

        t0 = time.time()
        sock.sendto(packet, (ip, 0))
        deadline = t0 + timeout
        got = False
        while time.time() < deadline:
            try:
                data, addr = sock.recvfrom(1024)
            except Exception, e:
                check("icmp recv", False, str(e))
                return
            if len(data) < 28:
                continue
            icmp_off = (ord(data[0]) & 0x0f) * 4
            if len(data) < icmp_off + 8:
                continue
            icmp = data[icmp_off:]
            itype, icode, _c, rid, rseq = struct.unpack("!BBHHH", icmp[:8])
            if itype == 0 and icode == 0 and rid == ident and rseq == seq:
                rtt = (time.time() - t0) * 1000.0
                check("icmp echo reply", True,
                      "from=%s rtt=%.1fms" % (addr[0], rtt))
                print("    ping %s: %.1f ms" % (addr[0], rtt))
                got = True
                break
        if not got:
            check("icmp echo reply", False, "timeout after %.1fs" % timeout)
    except Exception, e:
        check("icmp ping", False, str(e))
    finally:
        if sock is not None:
            try:
                sock.close()
            except Exception:
                pass


def main(argv=None):
    global HOST, TIMEOUT
    if argv is None:
        argv = sys.argv[1:]

    host = DEFAULT_HOST
    timeout = DEFAULT_TIMEOUT
    if len(argv) >= 1 and argv[0] not in ("-h", "--help"):
        host = argv[0]
    if len(argv) >= 2:
        try:
            timeout = float(argv[1])
        except ValueError:
            print("bad timeout:", argv[1])
            return 2
    if argv and argv[0] in ("-h", "--help"):
        print("Usage: python27 AmigaTests/test_socket_net.py [host] [timeout]")
        print("Default host:", DEFAULT_HOST)
        print("Tests: DNS, HTTP/1.0 GET :80, ICMP echo (if SOCK_RAW allowed)")
        return 0

    HOST = host
    TIMEOUT = timeout

    print("Amiga Python socket net test (no SSL)")
    print("version:", sys.version.replace("\n", " "))
    print("host:", HOST, "timeout:", TIMEOUT)
    print("---")

    reset_counters()
    test_01_import_socket()
    test_02_dns()
    test_03_http_get()
    test_04_icmp_ping()

    ok = summary()
    if ok:
        print("SOCKET NET TESTS PASSED")
        return 0
    return 1


if __name__ == "__main__":
    sys.exit(main())
