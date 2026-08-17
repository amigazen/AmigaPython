/*
 * Amiga _ssl: CPython 2.7 ssl.py API on amitls.library (BearSSL) for OS3.
 *
 * Selected with -DPYAMIGA_USE_AMITLS. TlsAttachSocket gets the AmiTCP
 * native id via PyHost->fn_socket_native_fd (posix sock_fd is a psocket
 * slot, not valid for AmiTLS send/recv).
 *
 * Verified ATlsTest / amitls.c (amihttp ht_ssl_*) model:
 *   - Native AmiTCP socket + connect; leave fd blocking (no FIONBIO,
 *     no ATTA_NON_BLOCKING / ATTA_EXTERNAL_WAIT).
 *   - Clear FIONBIO on the AmiTCP id with D7-safe IoctlSocket (same LVO
 *     as SAS/C proto/bsdsocket.h). Host fn_ioctl trampolines are not enough.
 *   - TlsAttachSocket: never pass NULL hostname (ssl.wrap_socket omits
 *     server_hostname; AmiTLS returns 8813 Invalid handle for NULL).
 *     Use "" when SNI is absent.
 *   - ht_ssl_recv: if TlsPending==0, blocking WaitSelect on the AmiTCP fd
 *     (full timeout) then TlsRead(..., 30).
 *   - TlsTaskAttach(this task SocketBase, errno) before every TlsWrite/TlsRead.
 *   - Do not CloseLibrary(amitls) at atexit while connections may dispose.
 *
 * C89 / ANSI; plugin FAR data. Assign amitlsinclude: to AmiTLS SDK Include_H.
 */
#include "Python.h"

#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/execbase.h>
#include <dos/dosextens.h>
#include <proto/exec.h>
#include <utility/tagitem.h>
#include <libraries/amitls.h>
#include <proto/amitls.h>
#include <string.h>

#include "socketmodule.h"

#ifdef PYAMIGA_PLUGIN_BUILD
#include "pyamiga_plugin.h"
#include "pyamiga_redir.h"
#endif

#ifndef EWOULDBLOCK
#define EWOULDBLOCK 35
#endif
#ifndef EAGAIN
#define EAGAIN EWOULDBLOCK
#endif
#ifndef EINTR
#define EINTR 4
#endif

/* VBCC proto/amitls.h inlines jsr through this A6 base. */
struct Library *TlsBase = NULL;

enum py_ssl_error {
    PY_SSL_ERROR_NONE,
    PY_SSL_ERROR_SSL,
    PY_SSL_ERROR_WANT_READ,
    PY_SSL_ERROR_WANT_WRITE,
    PY_SSL_ERROR_WANT_X509_LOOKUP,
    PY_SSL_ERROR_SYSCALL,
    PY_SSL_ERROR_ZERO_RETURN,
    PY_SSL_ERROR_WANT_CONNECT,
    PY_SSL_ERROR_EOF,
    PY_SSL_ERROR_INVALID_ERROR_CODE
};

enum py_ssl_server_or_client {
    PY_SSL_CLIENT,
    PY_SSL_SERVER
};

enum py_ssl_cert_requirements {
    PY_SSL_CERT_NONE,
    PY_SSL_CERT_OPTIONAL,
    PY_SSL_CERT_REQUIRED
};

/* Match CPython 2.7 numbering (ssl.py PROTOCOL_*). */
enum py_ssl_version {
    PY_SSL_VERSION_SSL2 = 0,
    PY_SSL_VERSION_SSL3 = 1,
    PY_SSL_VERSION_TLS = 2,
    PY_SSL_VERSION_TLS1 = 3,
    PY_SSL_VERSION_TLS1_1 = 4,
    PY_SSL_VERSION_TLS1_2 = 5
};

enum {
    SOCK_IS_BLOCKING,
    SOCK_IS_NONBLOCKING,
    SOCK_HAS_TIMED_OUT,
    SOCK_HAS_BEEN_CLOSED,
    SOCK_OPERATION_OK
};

#define PYSSL_OP_BIT(n) (1UL << (n))
#define PYSSL_OP_TLSEXT_PADDING              PYSSL_OP_BIT(4)
#define PYSSL_OP_SAFARI_ECDHE_ECDSA_BUG      PYSSL_OP_BIT(6)
#define PYSSL_OP_DONT_INSERT_EMPTY_FRAGMENTS PYSSL_OP_BIT(11)
#define PYSSL_OP_NO_COMPRESSION              PYSSL_OP_BIT(17)
#define PYSSL_OP_ENABLE_MIDDLEBOX_COMPAT     PYSSL_OP_BIT(20)
#define PYSSL_OP_CIPHER_SERVER_PREFERENCE    PYSSL_OP_BIT(22)
#define PYSSL_OP_NO_SSLv3                    PYSSL_OP_BIT(25)
#define PYSSL_OP_NO_TLSv1                    PYSSL_OP_BIT(26)
#define PYSSL_OP_NO_TLSv1_2                  PYSSL_OP_BIT(27)
#define PYSSL_OP_NO_TLSv1_1                  PYSSL_OP_BIT(28)
#define PYSSL_OP_NO_TLSv1_3                  PYSSL_OP_BIT(29)
#define PYSSL_OP_CRYPTOPRO_TLSEXT_BUG        PYSSL_OP_BIT(31)
#define PYSSL_OP_ALL (PYSSL_OP_CRYPTOPRO_TLSEXT_BUG | \
    PYSSL_OP_DONT_INSERT_EMPTY_FRAGMENTS | \
    PYSSL_OP_TLSEXT_PADDING | PYSSL_OP_SAFARI_ECDHE_ECDSA_BUG)

#define PYSSL_OP_DEFAULT ( \
    (PYSSL_OP_ALL & ~PYSSL_OP_DONT_INSERT_EMPTY_FRAGMENTS) | \
    PYSSL_OP_NO_SSLv3 | PYSSL_OP_NO_COMPRESSION | \
    PYSSL_OP_CIPHER_SERVER_PREFERENCE)

static PyObject *PySSLErrorObject;
static PyObject *PySSLZeroReturnErrorObject;
static PyObject *PySSLWantReadErrorObject;
static PyObject *PySSLWantWriteErrorObject;
static PyObject *PySSLSyscallErrorObject;
static PyObject *PySSLEOFErrorObject;

typedef struct {
    PyObject_HEAD
    struct TlsContext *ctx;
    unsigned char *alpn_protocols;
    unsigned int alpn_protocols_len;
    int check_hostname;
    int verify_mode;
    unsigned long options;
    unsigned long verify_flags;
    char *ca_path;
    int proto_version;
} PySSLContext;

typedef struct {
    PyObject_HEAD
    PySSLContext *ctx;
    PySocketSockObject *Socket;
    struct TlsConnection *conn;
    int native_fd;
    int handshake_done;
    int attached;
    enum py_ssl_server_or_client socket_type;
    char sni[256];
} PySSLSocket;

static PyTypeObject PySSLContext_Type;
static PyTypeObject PySSLSocket_Type;

static int amiga_amitls_ready = 0;
static int amiga_amitls_task_attached = 0;

/* Default AmiTLS trust store (AWeb CA bundle). Pointer must stay valid
 * for TlsBaseTags / NewTlsContextA (library strdup's it). */
static const char pyssl_default_ca[] = "AWeb:Certs/cacert.pem";

typedef struct {
    int nid;
    const char *sn;
    const char *ln;
    const char *oid;
} pyssl_asn1_obj;

/* CPython NIDs used by ssl.py Purpose.* at import. */
static const pyssl_asn1_obj pyssl_asn1_table[] = {
    {129, "serverAuth", "TLS Web Server Authentication",
     "1.3.6.1.5.5.7.3.1"},
    {130, "clientAuth", "TLS Web Client Authentication",
     "1.3.6.1.5.5.7.3.2"},
    {13, "CN", "commonName", "2.5.4.3"},
    {0, NULL, NULL, NULL}
};

static PyObject *
ssl_nomem(void)
{
    PyErr_SetString(PyExc_MemoryError, "out of memory");
    return NULL;
}

static ULONG
pyssl_verify_tag(int mode)
{
    if (mode == PY_SSL_CERT_REQUIRED)
        return ATSSL_VERIFY_PEER_STRICT;
    if (mode == PY_SSL_CERT_OPTIONAL)
        return ATSSL_VERIFY_PEER;
    return ATSSL_VERIFY_NONE;
}

static int
tls_is_want(LONG rc)
{
    return (rc == ERROR_TLS_WANT_READ || rc == ERROR_TLS_WANT_WRITE);
}

static int
tls_write_ok(LONG rc)
{
    return (rc > 0 && rc < ERROR_TLS_NOT_IMPLEMENTED);
}

/*
 * AmiTLS atls_br_sock_read: recv()==0 (peer close) becomes
 * ERROR_TLS_READ_FAILED with errno often EWOULDBLOCK/0, not rc==0.
 * Treat as TLS EOF so SSLSocket.recv loops get "".
 */
static int
tls_read_is_eof(LONG rc)
{
    int errn;

    if (rc != ERROR_TLS_READ_FAILED)
        return 0;
    errn = 0;
#ifdef PYAMIGA_PLUGIN_BUILD
    if (PyAmiga_Host != NULL && PyAmiga_Host->ptr_errno != NULL)
        errn = *PyAmiga_Host->ptr_errno;
#endif
    return (errn == 0 || errn == EWOULDBLOCK || errn == EAGAIN);
}

static int
ssl_tls_bind(void)
{
    struct Library *sockbase;
    APTR errptr;
    LONG rc;

#ifdef PYAMIGA_PLUGIN_BUILD
    if (PyAmiga_Host == NULL || TlsBase == NULL)
        return -1;
    sockbase = (struct Library *)PyAmiga_Host->ptr_SocketBase;
    errptr = (APTR)PyAmiga_Host->ptr_errno;
    if (sockbase == NULL)
        return -1;
    rc = TlsTaskAttach(sockbase, errptr);
    if (rc != 0)
        return -1;
    amiga_amitls_task_attached = 1;
    return 0;
#else
    (void)sockbase;
    (void)errptr;
    (void)rc;
    return -1;
#endif
}

#ifdef PYAMIGA_PLUGIN_BUILD
/* Roadshow netinclude/sys/filio.h FIONBIO (_IOW('f', 126, __LONG)). */
#define SSL_AMITCP_FIONBIO 0x8004667eUL

/*
 * Same register convention as host AmiTCP stubs: base in D7, not A6.
 * VBCC uses A6 as the frame pointer; jsr -114(a6) with A6=SocketBase deadlocks.
 */
static LONG ssl_IoctlSocket(__reg("d7") void *base,
                            __reg("d0") LONG sock,
                            __reg("d1") ULONG req,
                            __reg("a0") APTR argp)
    = "\tmove.l\ta6,-(sp)\n\tmove.l\td7,a6\n\tjsr\t-114(a6)\n\tmove.l\t(sp)+,a6";

static void
ssl_force_blocking_native(int nfd)
{
    LONG nb;
    struct Library *sbase;

    if (nfd < 0 || PyAmiga_Host == NULL)
        return;
    sbase = (struct Library *)PyAmiga_Host->ptr_SocketBase;
    if (sbase == NULL)
        return;
    /* amitls.c / ht_tcp_connect: blocking fd for BearSSL low_read. */
    nb = 0L;
    (void)ssl_IoctlSocket(sbase, (LONG)nfd, SSL_AMITCP_FIONBIO, (APTR)&nb);
}

static int
ssl_tls_prep(PySSLSocket *self)
{
    int nfd;

    nfd = (self != NULL) ? self->native_fd : -1;
    if (nfd >= 0)
        ssl_force_blocking_native(nfd);
    return ssl_tls_bind();
}

static void
ssl_tls_restore(PySSLSocket *self)
{
    (void)self;
}
#else
static int
ssl_tls_prep(PySSLSocket *self)
{
    (void)self;
    return ssl_tls_bind();
}

static void
ssl_tls_restore(PySSLSocket *self)
{
    (void)self;
}
#endif

static int
amiga_open_amitls(void)
{
    struct Process *pr;
    APTR oldwin;
    struct Library *sockbase;
    APTR errptr;
    LONG rc;
    struct TagItem tags[5];
    int n;

    if (amiga_amitls_ready && TlsBase != NULL)
        return 0;

#ifdef PYAMIGA_PLUGIN_BUILD
    if (PyAmiga_Host == NULL || PyAmiga_Host->ptr_SocketBase == NULL) {
        PyErr_SetString(PyExc_ImportError,
                        "amitls.library needs host SocketBase "
                        "(import _socket first)");
        return -1;
    }
    sockbase = (struct Library *)PyAmiga_Host->ptr_SocketBase;
    errptr = (APTR)PyAmiga_Host->ptr_errno;
#else
    sockbase = NULL;
    errptr = NULL;
#endif

    pr = (struct Process *)FindTask(NULL);
    oldwin = pr->pr_WindowPtr;
    pr->pr_WindowPtr = (APTR)-1L;

    TlsBase = OpenLibrary(AMITLSNAME, AMITLSVERSION);
    pr->pr_WindowPtr = oldwin;

    if (TlsBase == NULL) {
        PyErr_SetString(PyExc_ImportError,
                        "amitls.library required for _ssl "
                        "(AmiTLS not installed)");
        return -1;
    }

    rc = TlsTaskAttach(sockbase, errptr);
    if (rc != 0) {
        CloseLibrary(TlsBase);
        TlsBase = NULL;
        PyErr_SetString(PyExc_ImportError,
                        "TlsTaskAttach failed (need host SocketBase)");
        return -1;
    }
    amiga_amitls_task_attached = 1;
    amiga_amitls_ready = 1;
    n = 0;
#ifdef PYAMIGA_PLUGIN_BUILD
    tags[n].ti_Tag = ATBT_ERRNOPTR;
    tags[n].ti_Data = (ULONG)errptr;
    n++;
#endif
    tags[n].ti_Tag = ATBT_CA_BUNDLE_PATH;
    tags[n].ti_Data = (ULONG)pyssl_default_ca;
    n++;
    tags[n].ti_Tag = ATBT_SSL_VERIFY;
    tags[n].ti_Data = ATSSL_VERIFY_NONE;
    n++;
    tags[n].ti_Tag = TAG_DONE;
    tags[n].ti_Data = 0;
    TlsBaseTagsA(tags);
    oldwin = pr->pr_WindowPtr;
    pr->pr_WindowPtr = (APTR)-1L;
    (void)TlsLoadCABundle((STRPTR)pyssl_default_ca);
    pr->pr_WindowPtr = oldwin;
    /* Leave amitls.library open; atexit CloseLibrary races socket dealloc. */
    return 0;
}

static PyObject *
tls_set_error(LONG rc, const char *where)
{
    const char *es;
    char buf[256];
    PyObject *v;
    PyObject *exc;
    enum py_ssl_error p;
    STRPTR libstr;
    int errn;

    exc = PySSLErrorObject;
    p = PY_SSL_ERROR_SSL;
    es = "amitls error";
    libstr = NULL;
    errn = 0;
#ifdef PYAMIGA_PLUGIN_BUILD
    if (PyAmiga_Host != NULL && PyAmiga_Host->ptr_errno != NULL)
        errn = *PyAmiga_Host->ptr_errno;
#endif
    if (rc == ERROR_TLS_WANT_READ) {
        p = PY_SSL_ERROR_WANT_READ;
        exc = PySSLWantReadErrorObject;
        es = "The operation did not complete (read)";
    } else if (rc == ERROR_TLS_WANT_WRITE) {
        p = PY_SSL_ERROR_WANT_WRITE;
        exc = PySSLWantWriteErrorObject;
        es = "The operation did not complete (write)";
    } else if (rc == 0) {
        p = PY_SSL_ERROR_ZERO_RETURN;
        exc = PySSLZeroReturnErrorObject;
        es = "TLS/SSL connection has been closed";
    } else if (rc == ERROR_TLS_READ_FAILED
               && (errn == 0 || errn == EWOULDBLOCK || errn == EAGAIN)) {
        /* atls_br_sock_read treats recv()==0 (peer FIN) as failure and
         * surfaces READ_FAILED; errno is often still EWOULDBLOCK. Map to
         * EOF so ssl.SSLSocket.recv / suppress_ragged_eofs return "". */
        p = PY_SSL_ERROR_EOF;
        exc = PySSLEOFErrorObject;
        es = "EOF occurred in violation of protocol";
    } else {
        if (TlsBase != NULL)
            libstr = TlsGetErrorString(rc);
        if (libstr != NULL && libstr[0] != '\0')
            es = (const char *)libstr;
        if (rc == ERROR_TLS_READ_FAILED || rc == ERROR_TLS_WRITE_FAILED
            || rc == ERROR_TLS_IO)
            exc = PySSLSyscallErrorObject;
        p = PY_SSL_ERROR_SSL;
        sprintf(buf, "%s: %s (rc=%ld errno=%d)", where, es, (long)rc, errn);
        es = buf;
    }
    v = Py_BuildValue("is", (int)p, es);
    if (v == NULL)
        return NULL;
    PyErr_SetObject(exc, v);
    Py_DECREF(v);
    return NULL;
}

#ifdef PYAMIGA_PLUGIN_BUILD
/* 1.0 and 30.0 as big-endian IEEE-754; plugin must not do soft-float. */
static const unsigned char ssl_slice_1s[8] = {
    0x3F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static const unsigned char ssl_slice_30s[8] = {
    0x40, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static int
ssl_wait(PySocketSockObject *s, int writing)
{
    unsigned char bits[8];
    int cmp;
    int rc;

    if (s == NULL || s->sock_fd < 0)
        return SOCK_HAS_BEEN_CLOSED;
    memcpy(bits, &s->sock_timeout, 8);
    cmp = PyAmiga_Host->fn_sock_timeout_cmp0(bits);
    if (cmp == 0)
        return SOCK_IS_NONBLOCKING;
    /* 1s WaitSelect slices so PyErr_CheckSignals can run (Ctrl-C). */
    rc = PyAmiga_Host->fn_sock_select1(s->sock_fd, writing, ssl_slice_1s);
    if (rc < 0)
        return SOCK_HAS_BEEN_CLOSED;
    if (rc == 1)
        return SOCK_HAS_TIMED_OUT;
    return SOCK_OPERATION_OK;
}

/* amitls.c ht_ssl_wait_socket: blocking WaitSelect, tv_sec = 30. */
static int
ssl_wait_appdata(PySocketSockObject *s)
{
    int rc;

    if (s == NULL || s->sock_fd < 0)
        return SOCK_HAS_BEEN_CLOSED;
    rc = PyAmiga_Host->fn_sock_select1(s->sock_fd, 0, ssl_slice_30s);
    if (rc < 0)
        return SOCK_HAS_BEEN_CLOSED;
    if (rc == 1)
        return SOCK_HAS_TIMED_OUT;
    return SOCK_OPERATION_OK;
}
#else
static int
ssl_wait(PySocketSockObject *s, int writing)
{
    (void)s;
    (void)writing;
    return SOCK_IS_BLOCKING;
}
#endif

static void
ssl_deadline_begin(PySocketSockObject *sock, unsigned char *deadline,
                   int *have_dl)
{
#ifdef PYAMIGA_PLUGIN_BUILD
    unsigned char tbits[8];

    memcpy(tbits, &sock->sock_timeout, 8);
    *have_dl = (PyAmiga_Host->fn_sock_timeout_cmp0(tbits) > 0);
    if (*have_dl)
        PyAmiga_Host->fn_sock_deadline_init(tbits, deadline);
#else
    (void)sock;
    (void)deadline;
    *have_dl = 0;
#endif
}

static int
ssl_deadline_hit(unsigned char *deadline, int have_dl)
{
#ifdef PYAMIGA_PLUGIN_BUILD
    unsigned char remain[8];

    if (!have_dl)
        return 0;
    return PyAmiga_Host->fn_sock_deadline_remaining(deadline, remain);
#else
    (void)deadline;
    (void)have_dl;
    return 1;
#endif
}

static int
ssl_native_fd(int posix_fd)
{
    int nfd;

#ifdef PYAMIGA_PLUGIN_BUILD
    if (PyAmiga_Host == NULL || PyAmiga_Host->fn_socket_native_fd == NULL)
        return -1;
    nfd = PyAmiga_Host->fn_socket_native_fd(posix_fd);
#else
    nfd = posix_fd;
#endif
    return nfd;
}

static int
ssl_apply_wait(PySocketSockObject *sock, LONG rc, int *st)
{
    int writing;

    writing = (rc == ERROR_TLS_WANT_WRITE) ? 1 : 0;
    *st = ssl_wait(sock, writing);
    return *st;
}

static PySSLSocket *
newPySSLSocket(PySSLContext *sslctx, PySocketSockObject *sock,
               enum py_ssl_server_or_client socket_type,
               char *server_hostname, PyObject *ssl_sock)
{
    PySSLSocket *self;
    PySSLSocket * volatile selfv;
    PySSLContext * volatile ctxsaved;
    PySocketSockObject * volatile socksaved;
    struct TlsContext *tlsctx;
    struct TlsConnection *conn;
    int posix_fd;
    int nfd;
    int verifymode;
    struct TagItem tags[3];
    LONG rc;
    char sni[256];
    STRPTR hostarg;

    (void)ssl_sock;
    /* Copy everything derived from pointer args BEFORE any host trampoline
     * or AmiTLS LVO. 68k A0/A1/D0/D1 do not survive PyObject_New. */
    ctxsaved = sslctx;
    tlsctx = (sslctx != NULL) ? sslctx->ctx : NULL;
    verifymode = (sslctx != NULL) ? sslctx->verify_mode : PY_SSL_CERT_NONE;
    socksaved = sock;
    posix_fd = (sock != NULL) ? sock->sock_fd : -1;
    sni[0] = '\0';
    if (server_hostname != NULL && server_hostname[0] != '\0') {
        strncpy(sni, server_hostname, sizeof(sni) - 1);
        sni[sizeof(sni) - 1] = '\0';
    }
    nfd = ssl_native_fd(posix_fd);
    if (nfd < 0) {
        PyErr_SetString(PySSLErrorObject, "invalid socket fd");
        return NULL;
    }
    if (socket_type == PY_SSL_SERVER) {
        PyErr_SetString(PyExc_NotImplementedError,
                        "AmiTLS is a TLS client library");
        return NULL;
    }
    if (ssl_tls_bind() != 0) {
        PyErr_SetString(PySSLErrorObject, "TlsTaskAttach failed");
        return NULL;
    }
#ifdef PYAMIGA_PLUGIN_BUILD
    /* amitls.c: blocking AmiTCP id before TlsAttachSocket. */
    ssl_force_blocking_native(nfd);
#endif
    self = PyObject_New(PySSLSocket, &PySSLSocket_Type);
    if (self == NULL)
        return NULL;
    selfv = self;
    selfv->conn = NULL;
    selfv->Socket = NULL;
    selfv->ctx = ctxsaved;
    selfv->native_fd = nfd;
    selfv->handshake_done = 0;
    selfv->attached = 0;
    selfv->socket_type = socket_type;
    selfv->sni[0] = '\0';
    strncpy(selfv->sni, sni, sizeof(selfv->sni) - 1);
    selfv->sni[sizeof(selfv->sni) - 1] = '\0';
    Py_INCREF(ctxsaved);

    conn = NewTlsConnection(tlsctx);
    self = selfv;
    self->conn = conn;
    if (conn == NULL) {
        PyErr_SetString(PySSLErrorObject, "NewTlsConnection failed");
        Py_DECREF(self);
        return NULL;
    }

    tags[0].ti_Tag = ATTA_SSL_VERIFY;
    tags[0].ti_Data = pyssl_verify_tag(verifymode);
    tags[1].ti_Tag = TAG_DONE;
    tags[1].ti_Data = 0;
    /* AmiTLS atls_bearssl_attach rejects NULL hostname with 8813
     * (ERROR_TLS_INVALID_HANDLE). ssl.wrap_socket() does not pass
     * server_hostname — use "" (no SNI) rather than NULL. */
    hostarg = (STRPTR)"";
    if (sni[0] != '\0')
        hostarg = (STRPTR)sni;
    rc = TlsAttachSocketA(conn, (LONG)nfd, hostarg, tags);
    self = selfv;
    if (rc != 0) {
        tls_set_error(rc, "TlsAttachSocket");
        Py_DECREF(self);
        return NULL;
    }
    self->attached = 1;
    self->Socket = socksaved;
    Py_INCREF(socksaved);
    return selfv;
}

static void
PySSL_dealloc(PySSLSocket *self)
{
    struct TlsConnection *conn;
    PySSLContext *ctx;
    PySocketSockObject *sock;

    conn = self->conn;
    ctx = self->ctx;
    sock = self->Socket;
    self->conn = NULL;
    self->ctx = NULL;
    self->Socket = NULL;
    /* Library never CloseSocket; Python owns the AmiTCP fd. */
    if (amiga_amitls_ready && conn != NULL)
        DisposeTlsConnection(conn);
    Py_XDECREF(ctx);
    Py_XDECREF(sock);
    PyObject_Del(self);
}

static PyObject *
PySSL_SSLdo_handshake(PySSLSocket *self)
{
    PySocketSockObject *sock;

    sock = self->Socket;
    if (sock == NULL) {
        PyErr_SetString(PySSLErrorObject, "underlying socket is None");
        return NULL;
    }
    if (self->conn == NULL || !self->attached) {
        PyErr_SetString(PySSLErrorObject, "TLS socket is not attached");
        return NULL;
    }
    /*
     * Handshake is deferred to the first TlsWrite (amihttp/ATlsTest).
     * Calling TlsHandshake here with NON_BLOCKING/EXTERNAL_WAIT returned
     * success without SENDAPP and then TlsWrite spun on WANT_WRITE.
     */
    Py_INCREF(sock);
    if (ssl_tls_prep(self) < 0) {
        Py_DECREF(sock);
        PyErr_SetString(PySSLErrorObject, "TlsTaskAttach failed");
        return NULL;
    }
    ssl_tls_restore(self);
    Py_DECREF(sock);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
PySSL_peercert(PySSLSocket *self, PyObject *args)
{
    int binary_mode;
    struct TlsPeerCert cert;
    LONG rc;
    PyObject *d;
    PyObject *sub;

    binary_mode = 0;
    if (!PyArg_ParseTuple(args, "|i:peer_certificate", &binary_mode))
        return NULL;
    if (!self->handshake_done || self->conn == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    if (binary_mode) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    memset(&cert, 0, sizeof(cert));
    rc = TlsGetPeerCert(self->conn, &cert);
    if (rc != 0) {
        return PyDict_New();
    }
    d = PyDict_New();
    if (d == NULL) {
        TlsPeerCertFree(&cert);
        return NULL;
    }
    if (cert.tpc_CommonName != NULL && cert.tpc_CommonName[0] != '\0') {
        sub = Py_BuildValue("((ss))", "commonName",
                            (char *)cert.tpc_CommonName);
        if (sub == NULL) {
            Py_DECREF(d);
            TlsPeerCertFree(&cert);
            return NULL;
        }
        PyDict_SetItemString(d, "subject", sub);
        Py_DECREF(sub);
    }
    TlsPeerCertFree(&cert);
    return d;
}

static PyObject *
PySSL_cipher(PySSLSocket *self)
{
    if (!self->handshake_done) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    /* AmiTLS client profile: TLS 1.2 ECDHE-RSA AES-128-GCM. */
    return Py_BuildValue("ssi",
                         "ECDHE-RSA-AES128-GCM-SHA256",
                         "TLSv1.2",
                         128);
}

static PyObject *
PySSL_version(PySSLSocket *self)
{
    if (!self->handshake_done) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return PyString_FromString("TLSv1.2");
}

static PyObject *
PySSL_selected_npn_protocol(PySSLSocket *self)
{
    (void)self;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
PySSL_selected_alpn_protocol(PySSLSocket *self)
{
    (void)self;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
PySSL_compression(PySSLSocket *self)
{
    (void)self;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
PySSL_get_context(PySSLSocket *self, void *closure)
{
    (void)closure;
    Py_INCREF(self->ctx);
    return (PyObject *)self->ctx;
}

static int
PySSL_set_context(PySSLSocket *self, PyObject *value, void *closure)
{
    (void)self;
    (void)value;
    (void)closure;
    PyErr_SetString(PyExc_NotImplementedError,
                    "cannot rebind AmiTLS context after wrap");
    return -1;
}

static PyObject *
PySSL_SSLwrite(PySSLSocket *self, PyObject *args)
{
    char *data;
    char *copy;
    int len;
    LONG rc;
    int st;
    int sent;
    ULONG attempts;
    PySocketSockObject *sock;
    struct TlsConnection *conn;
    PySSLSocket * volatile selfv;

    copy = NULL;
    if (!PyArg_ParseTuple(args, "s#:write", &data, &len))
        return NULL;
    if (len == 0)
        return PyInt_FromLong(0);
    /* Copy before any host trampoline / AmiTLS LVO (68k A0/A1). */
    copy = (char *)PyMem_Malloc((size_t)len);
    if (copy == NULL)
        return ssl_nomem();
    memcpy(copy, data, (size_t)len);
    selfv = self;
    conn = self->conn;
    sock = self->Socket;
    if (conn == NULL || sock == NULL) {
        PyMem_Free(copy);
        PyErr_SetString(PySSLErrorObject, "TLS socket is closed");
        return NULL;
    }
    Py_INCREF(sock);
    if (ssl_tls_prep(self) < 0) {
        PyMem_Free(copy);
        Py_DECREF(sock);
        PyErr_SetString(PySSLErrorObject, "TlsTaskAttach failed");
        return NULL;
    }
    sent = 0;
    attempts = 0;
    for (;;) {
        attempts++;
        if (attempts > 200) {
            ssl_tls_restore(self);
            PyMem_Free(copy);
            Py_DECREF(sock);
            PyErr_SetString(PySSLErrorObject,
                            "The write operation timed out");
            return NULL;
        }
        rc = TlsWrite(conn, (APTR)(copy + sent), (ULONG)(len - sent));
        if (PyErr_CheckSignals()) {
            ssl_tls_restore(self);
            PyMem_Free(copy);
            Py_DECREF(sock);
            return NULL;
        }
        if (tls_write_ok(rc)) {
            sent += (int)rc;
            if (sent >= len)
                break;
            continue;
        }
        if (!tls_is_want(rc)) {
            ssl_tls_restore(self);
            PyMem_Free(copy);
            Py_DECREF(sock);
            return tls_set_error(rc, "TlsWrite");
        }
        ssl_apply_wait(sock, rc, &st);
        if (st == SOCK_HAS_BEEN_CLOSED || st == SOCK_HAS_TIMED_OUT) {
            ssl_tls_restore(self);
            PyMem_Free(copy);
            Py_DECREF(sock);
            PyErr_SetString(PySSLErrorObject,
                            "The write operation timed out");
            return NULL;
        }
        if (st == SOCK_IS_NONBLOCKING) {
            ssl_tls_restore(self);
            PyMem_Free(copy);
            Py_DECREF(sock);
            return tls_set_error(rc, "TlsWrite");
        }
    }
    selfv->handshake_done = 1;
    ssl_tls_restore(self);
    PyMem_Free(copy);
    Py_DECREF(sock);
    return PyInt_FromLong(sent);
}

static PyObject *
PySSL_SSLpending(PySSLSocket *self)
{
    ULONG n;

    n = 0;
    if (self->conn != NULL)
        n = TlsPending(self->conn);
    return PyInt_FromLong((long)n);
}

static PyObject *
PySSL_SSLread(PySSLSocket *self, PyObject *args)
{
    PyObject *buffer = NULL;
    PyObject *dest = NULL;
    char *data;
    int len = 1024;
    LONG rc;
    int st;
    Py_ssize_t buflen;
    void *wbuf;
    PySocketSockObject *sock;
    struct TlsConnection *conn;
    ULONG attempts;
    ULONG pending;

    st = SOCK_OPERATION_OK;
    if (!PyArg_ParseTuple(args, "|iO:read", &len, &buffer))
        return NULL;
    if (len < 0) {
        PyErr_SetString(PyExc_ValueError, "size should not be negative");
        return NULL;
    }
    if (len == 0)
        return PyString_FromStringAndSize("", 0);
    /* AmiTLS success is a byte count below ERROR_TLS_NOT_IMPLEMENTED (8800). */
    if (len > 4096)
        len = 4096;
    conn = self->conn;
    sock = self->Socket;
    if (conn == NULL || sock == NULL) {
        PyErr_SetString(PySSLErrorObject, "TLS socket is closed");
        return NULL;
    }
    Py_INCREF(sock);
    if (buffer != NULL && buffer != Py_None) {
        if (PyObject_AsWriteBuffer(buffer, &wbuf, &buflen) < 0) {
            Py_DECREF(sock);
            return NULL;
        }
        if (buflen < len)
            len = (int)buflen;
        data = (char *)wbuf;
        dest = NULL;
    } else {
        dest = PyString_FromStringAndSize(NULL, len);
        if (dest == NULL) {
            Py_DECREF(sock);
            return NULL;
        }
        data = PyString_AsString(dest);
    }
    if (ssl_tls_prep(self) < 0) {
        Py_XDECREF(dest);
        Py_DECREF(sock);
        PyErr_SetString(PySSLErrorObject, "TlsTaskAttach failed");
        return NULL;
    }
#ifdef PYAMIGA_PLUGIN_BUILD
    /* amitls.c ht_ssl_recv: WaitSelect when no app data buffered yet. */
    pending = TlsPending(conn);
    if (pending == 0) {
        st = ssl_wait_appdata(sock);
        if (st == SOCK_HAS_BEEN_CLOSED || st == SOCK_HAS_TIMED_OUT) {
            ssl_tls_restore(self);
            Py_XDECREF(dest);
            Py_DECREF(sock);
            PyErr_SetString(PySSLErrorObject,
                            "The read operation timed out");
            return NULL;
        }
    }
#endif
    attempts = 0;
    for (;;) {
        attempts++;
        if (attempts > 200) {
            ssl_tls_restore(self);
            Py_XDECREF(dest);
            Py_DECREF(sock);
            PyErr_SetString(PySSLErrorObject,
                            "The read operation timed out");
            return NULL;
        }
        data = (dest != NULL) ? PyString_AsString(dest) : (char *)wbuf;
        rc = TlsRead(conn, (APTR)data, (ULONG)len, 30);
        if (PyErr_CheckSignals()) {
            ssl_tls_restore(self);
            Py_XDECREF(dest);
            Py_DECREF(sock);
            return NULL;
        }
        if (tls_write_ok(rc) || rc == 0)
            break;
        if (tls_read_is_eof(rc)) {
            rc = 0;
            break;
        }
        if (!tls_is_want(rc)) {
            ssl_tls_restore(self);
            Py_XDECREF(dest);
            Py_DECREF(sock);
            return tls_set_error(rc, "TlsRead");
        }
        ssl_apply_wait(sock, rc, &st);
        if (st == SOCK_HAS_BEEN_CLOSED || st == SOCK_HAS_TIMED_OUT) {
            ssl_tls_restore(self);
            Py_XDECREF(dest);
            Py_DECREF(sock);
            PyErr_SetString(PySSLErrorObject,
                            "The read operation timed out");
            return NULL;
        }
        if (st == SOCK_IS_NONBLOCKING) {
            ssl_tls_restore(self);
            Py_XDECREF(dest);
            Py_DECREF(sock);
            return tls_set_error(rc, "TlsRead");
        }
    }
    ssl_tls_restore(self);
    Py_DECREF(sock);
    if (rc < 0) {
        Py_XDECREF(dest);
        return tls_set_error(rc, "TlsRead");
    }
    if (dest != NULL) {
        if (_PyString_Resize(&dest, (Py_ssize_t)rc) < 0)
            return NULL;
        return dest;
    }
    return PyInt_FromLong(rc);
}

static PyObject *
PySSL_SSLshutdown(PySSLSocket *self)
{
    PySocketSockObject *sock;
    PyObject *sockobj;

    sock = self->Socket;
    if (sock == NULL) {
        PyErr_SetString(PySSLErrorObject, "underlying socket is None");
        return NULL;
    }
    Py_INCREF(sock);
    if (self->conn != NULL)
        (void)TlsShutdown(self->conn);
    sockobj = (PyObject *)sock;
    self->Socket = NULL;
    return sockobj;
}

static PyObject *
PySSL_tls_unique_cb(PySSLSocket *self)
{
    (void)self;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyGetSetDef ssl_getsetlist[] = {
    {"context", (getter)PySSL_get_context, (setter)PySSL_set_context, NULL},
    {NULL, NULL, NULL, NULL}
};

static PyMethodDef PySSLMethods[] = {
    {"do_handshake", (PyCFunction)PySSL_SSLdo_handshake, METH_NOARGS, NULL},
    {"write", (PyCFunction)PySSL_SSLwrite, METH_VARARGS, NULL},
    {"read", (PyCFunction)PySSL_SSLread, METH_VARARGS, NULL},
    {"pending", (PyCFunction)PySSL_SSLpending, METH_NOARGS, NULL},
    {"peer_certificate", (PyCFunction)PySSL_peercert, METH_VARARGS, NULL},
    {"cipher", (PyCFunction)PySSL_cipher, METH_NOARGS, NULL},
    {"version", (PyCFunction)PySSL_version, METH_NOARGS, NULL},
    {"selected_npn_protocol", (PyCFunction)PySSL_selected_npn_protocol,
     METH_NOARGS, NULL},
    {"selected_alpn_protocol", (PyCFunction)PySSL_selected_alpn_protocol,
     METH_NOARGS, NULL},
    {"compression", (PyCFunction)PySSL_compression, METH_NOARGS, NULL},
    {"shutdown", (PyCFunction)PySSL_SSLshutdown, METH_NOARGS, NULL},
    {"tls_unique_cb", (PyCFunction)PySSL_tls_unique_cb, METH_NOARGS, NULL},
    {NULL, NULL, 0, NULL}
};

static PyTypeObject PySSLSocket_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "_ssl._SSLSocket",                  /* tp_name */
    sizeof(PySSLSocket),                /* tp_basicsize */
    0,                                  /* tp_itemsize */
    (destructor)PySSL_dealloc,          /* tp_dealloc */
    0,                                  /* tp_print */
    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,                                  /* tp_compare */
    0,                                  /* tp_repr */
    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */
    0,                                  /* tp_hash */
    0,                                  /* tp_call */
    0,                                  /* tp_str */
    PyObject_GenericGetAttr,            /* tp_getattro */
    0,                                  /* tp_setattro */
    0,                                  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                 /* tp_flags */
    0,                                  /* tp_doc */
    0,                                  /* tp_traverse */
    0,                                  /* tp_clear */
    0,                                  /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */
    PySSLMethods,                       /* tp_methods */
    0,                                  /* tp_members */
    ssl_getsetlist,                     /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    0,                                  /* tp_init */
    PyType_GenericAlloc,                /* tp_alloc */
    0,                                  /* tp_new */
    PyObject_Del                        /* tp_free */
};

static int
context_apply_verify(PySSLContext *self)
{
    struct TagItem tags[2];
    LONG rc;

    if (self->ctx == NULL)
        return -1;
    if (self->ca_path == NULL || self->ca_path[0] == '\0')
        return 0;
    tags[0].ti_Tag = ATSA_CA_BUNDLE_PATH;
    tags[0].ti_Data = (ULONG)self->ca_path;
    tags[1].ti_Tag = TAG_DONE;
    tags[1].ti_Data = 0;
    rc = SetTlsContextAttrsA(self->ctx, tags);
    if (rc != 0)
        return -1;
    return 0;
}

static PyObject *
context_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    char *kwlist[2];
    PySSLContext *self;
    PySSLContext * volatile selfv;
    int proto_version;
    struct TagItem tags[2];
    struct TlsContext *tlsctx;

    kwlist[0] = "protocol";
    kwlist[1] = NULL;
    proto_version = PY_SSL_VERSION_TLS;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "i:_SSLContext", kwlist,
                                     &proto_version))
        return NULL;
    if (proto_version == PY_SSL_VERSION_SSL2
        || proto_version == PY_SSL_VERSION_SSL3) {
        PyErr_SetString(PyExc_ValueError,
                        "SSLv2/SSLv3 are not supported by AmiTLS");
        return NULL;
    }
    self = (PySSLContext *)type->tp_alloc(type, 0);
    if (self == NULL)
        return NULL;
    selfv = self;
    selfv->ctx = NULL;
    selfv->alpn_protocols = NULL;
    selfv->alpn_protocols_len = 0;
    selfv->check_hostname = 0;
    selfv->verify_mode = PY_SSL_CERT_NONE;
    selfv->options = PYSSL_OP_DEFAULT;
    selfv->verify_flags = 0;
    selfv->ca_path = NULL;
    selfv->proto_version = proto_version;

    /* Per-connection trust path (amihttp): CA on the TlsContext, verify
     * mode only at TlsAttachSocket via ATTA_SSL_VERIFY. */
    tags[0].ti_Tag = ATSA_CA_BUNDLE_PATH;
    tags[0].ti_Data = (ULONG)pyssl_default_ca;
    tags[1].ti_Tag = TAG_DONE;
    tags[1].ti_Data = 0;
    tlsctx = NewTlsContextA(tags);
    self = selfv;
    self->ctx = tlsctx;
    if (self->ctx == NULL) {
        PyErr_SetString(PySSLErrorObject, "NewTlsContext failed");
        Py_DECREF(self);
        return NULL;
    }
    return (PyObject *)selfv;
}

static void
context_dealloc(PySSLContext *self)
{
    struct TlsContext *tlsctx;

    tlsctx = self->ctx;
    self->ctx = NULL;
    if (amiga_amitls_ready && tlsctx != NULL)
        DisposeTlsContext(tlsctx);
    if (self->alpn_protocols != NULL)
        PyMem_Free(self->alpn_protocols);
    self->alpn_protocols = NULL;
    if (self->ca_path != NULL)
        PyMem_Free(self->ca_path);
    self->ca_path = NULL;
    /* ssl.SSLContext is a heap subclass: host subtype_dealloc calls this
     * plugin tp_dealloc. Free via PyObject_Del -> host fn_object_dealloc. */
    PyObject_Del(self);
}

static PyObject *
set_ciphers(PySSLContext *self, PyObject *args)
{
    const char *cipherlist;

    (void)self;
    if (!PyArg_ParseTuple(args, "s:set_ciphers", &cipherlist))
        return NULL;
    /* AmiTLS cipher set is fixed (TLS 1.2 ECDHE-RSA AES-GCM). */
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_set_npn_protocols(PySSLContext *self, PyObject *args)
{
    (void)self;
    (void)args;
    PyErr_SetString(PyExc_NotImplementedError,
                    "NPN is not available in AmiTLS");
    return NULL;
}

static PyObject *
_set_alpn_protocols(PySSLContext *self, PyObject *args)
{
    Py_buffer protos;

    (void)self;
    if (!PyArg_ParseTuple(args, "s*:set_alpn_protocols", &protos))
        return NULL;
    PyBuffer_Release(&protos);
    PyErr_SetString(PyExc_NotImplementedError,
                    "ALPN is not available in AmiTLS");
    return NULL;
}

static PyObject *
get_verify_mode(PySSLContext *self, void *c)
{
    (void)c;
    return PyLong_FromLong(self->verify_mode);
}

static int
set_verify_mode(PySSLContext *self, PyObject *arg, void *c)
{
    int n;

    (void)c;
    if (!PyArg_Parse(arg, "i", &n))
        return -1;
    if (n != PY_SSL_CERT_NONE && n != PY_SSL_CERT_OPTIONAL
        && n != PY_SSL_CERT_REQUIRED) {
        PyErr_SetString(PyExc_ValueError, "invalid value for verify_mode");
        return -1;
    }
    if (n == PY_SSL_CERT_NONE && self->check_hostname) {
        PyErr_SetString(PyExc_ValueError,
                        "Cannot set verify_mode to CERT_NONE when "
                        "check_hostname is enabled.");
        return -1;
    }
    self->verify_mode = n;
    if (context_apply_verify(self) < 0) {
        PyErr_SetString(PySSLErrorObject, "SetTlsContextAttrs failed");
        return -1;
    }
    return 0;
}

static PyObject *
get_verify_flags(PySSLContext *self, void *c)
{
    (void)c;
    return PyLong_FromUnsignedLong(self->verify_flags);
}

static int
set_verify_flags(PySSLContext *self, PyObject *arg, void *c)
{
    unsigned long new_flags;

    (void)c;
    if (!PyArg_Parse(arg, "k", &new_flags))
        return -1;
    self->verify_flags = new_flags;
    return 0;
}

static PyObject *
get_options(PySSLContext *self, void *c)
{
    (void)c;
    return PyLong_FromUnsignedLong(self->options);
}

static int
set_options(PySSLContext *self, PyObject *arg, void *c)
{
    unsigned long new_opts;

    (void)c;
    if (!PyArg_Parse(arg, "k", &new_opts))
        return -1;
    self->options = new_opts;
    return 0;
}

static PyObject *
get_check_hostname(PySSLContext *self, void *c)
{
    (void)c;
    return PyBool_FromLong(self->check_hostname);
}

static int
set_check_hostname(PySSLContext *self, PyObject *arg, void *c)
{
    int check_hostname;

    (void)c;
    check_hostname = PyObject_IsTrue(arg);
    if (check_hostname < 0)
        return -1;
    if (check_hostname && self->verify_mode == PY_SSL_CERT_NONE) {
        PyErr_SetString(PyExc_ValueError,
                        "check_hostname needs a SSL context with either "
                        "CERT_OPTIONAL or CERT_REQUIRED");
        return -1;
    }
    self->check_hostname = check_hostname;
    return 0;
}

static PyObject *
load_cert_chain(PySSLContext *self, PyObject *args, PyObject *kwds)
{
    (void)self;
    (void)args;
    (void)kwds;
    PyErr_SetString(PyExc_NotImplementedError,
                    "client certificates are not supported by AmiTLS");
    return NULL;
}

static PyObject *
load_verify_locations(PySSLContext *self, PyObject *args, PyObject *kwds)
{
    char *kwlist[4];
    PyObject *cafile;
    PyObject *capath;
    PyObject *cadata;
    const char *cafile_buf;
    LONG rc;
    size_t nlen;
    char *copy;

    kwlist[0] = "cafile";
    kwlist[1] = "capath";
    kwlist[2] = "cadata";
    kwlist[3] = NULL;
    cafile = NULL;
    capath = NULL;
    cadata = NULL;
    cafile_buf = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OOO:load_verify_locations",
                                     kwlist, &cafile, &capath, &cadata))
        return NULL;
    if (cafile == Py_None)
        cafile = NULL;
    if (capath == Py_None)
        capath = NULL;
    if (cadata == Py_None)
        cadata = NULL;
    if (cafile == NULL && capath == NULL && cadata == NULL) {
        PyErr_SetString(PyExc_TypeError,
                        "cafile, capath and cadata cannot be all omitted");
        return NULL;
    }
    if (cadata != NULL) {
        char *buf;
        Py_ssize_t n;
        if (!PyString_Check(cadata)) {
            PyErr_SetString(PyExc_TypeError,
                            "cadata should be a string of PEM or ASN1");
            return NULL;
        }
        buf = PyString_AsString(cadata);
        n = PyString_Size(cadata);
        rc = TlsAddTrustedCert((APTR)buf, (ULONG)n, ATCF_PEM);
        if (rc != 0)
            return tls_set_error(rc, "TlsAddTrustedCert");
    }
    if (cafile != NULL) {
        if (!PyString_Check(cafile)) {
            PyErr_SetString(PyExc_TypeError, "cafile should be a string");
            return NULL;
        }
        cafile_buf = PyString_AsString(cafile);
        nlen = strlen(cafile_buf);
        copy = (char *)PyMem_Malloc(nlen + 1);
        if (copy == NULL)
            return ssl_nomem();
        memcpy(copy, cafile_buf, nlen + 1);
        if (self->ca_path != NULL)
            PyMem_Free(self->ca_path);
        self->ca_path = copy;
        rc = TlsLoadCABundle((STRPTR)self->ca_path);
        if (rc != 0)
            return tls_set_error(rc, "TlsLoadCABundle");
        if (context_apply_verify(self) < 0)
            return tls_set_error(ERROR_TLS_INVALID_HANDLE,
                                 "SetTlsContextAttrs");
    }
    if (capath != NULL && cafile == NULL && cadata == NULL) {
        PyErr_SetString(PyExc_NotImplementedError,
                        "AmiTLS has no capath directory loader");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
load_dh_params(PySSLContext *self, PyObject *filepath)
{
    (void)self;
    (void)filepath;
    PyErr_SetString(PyExc_NotImplementedError,
                    "load_dh_params is not supported by AmiTLS");
    return NULL;
}

static PyObject *
context_wrap_socket(PySSLContext *self, PyObject *args, PyObject *kwds)
{
    char *kwlist[5];
    PySocketSockObject *sock;
    int server_side;
    PyObject *hostname_obj;
    PyObject *ssl_sock;
    char *hostname;
    PySSLSocket *res;

    kwlist[0] = "sock";
    kwlist[1] = "server_side";
    kwlist[2] = "server_hostname";
    kwlist[3] = "ssl_sock";
    kwlist[4] = NULL;
    server_side = 0;
    hostname_obj = Py_None;
    ssl_sock = Py_None;
    hostname = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!i|OO:_wrap_socket",
                                     kwlist,
                                     PySocketModule.Sock_Type, &sock,
                                     &server_side, &hostname_obj, &ssl_sock))
        return NULL;
    if (hostname_obj != NULL && hostname_obj != Py_None) {
        if (!PyString_Check(hostname_obj)) {
            PyErr_SetString(PyExc_TypeError,
                            "server_hostname must be a string or None");
            return NULL;
        }
        hostname = PyString_AsString(hostname_obj);
    }
    res = newPySSLSocket(self, sock,
                         server_side ? PY_SSL_SERVER : PY_SSL_CLIENT,
                         hostname, ssl_sock);
    return (PyObject *)res;
}

static PyObject *
session_stats(PySSLContext *self, PyObject *unused)
{
    (void)self;
    (void)unused;
    return Py_BuildValue("{s:i,s:i,s:i,s:i,s:i,s:i,s:i,s:i,s:i,s:i,s:i}",
                         "number", 0,
                         "connect", 0,
                         "connect_good", 0,
                         "connect_renegotiate", 0,
                         "accept", 0,
                         "accept_good", 0,
                         "accept_renegotiate", 0,
                         "hits", 0,
                         "misses", 0,
                         "timeouts", 0,
                         "cache_full", 0);
}

static PyObject *
set_default_verify_paths(PySSLContext *self, PyObject *unused)
{
    (void)unused;
    (void)TlsLoadCABundle((STRPTR)pyssl_default_ca);
    if (self != NULL && self->ctx != NULL) {
        if (self->ca_path == NULL)
            (void)context_apply_verify(self);
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
set_ecdh_curve(PySSLContext *self, PyObject *name)
{
    (void)self;
    if (!PyString_Check(name)) {
        PyErr_SetString(PyExc_TypeError, "curve name must be a string");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
set_servername_callback(PySSLContext *self, PyObject *args)
{
    (void)self;
    (void)args;
    PyErr_SetString(PyExc_NotImplementedError,
                    "set_servername_callback is not implemented on Amiga");
    return NULL;
}

static PyObject *
cert_store_stats(PySSLContext *self)
{
    (void)self;
    return Py_BuildValue("{s:i,s:i,s:i}",
                         "x509", 0, "crl", 0, "x509_ca", 0);
}

static PyObject *
get_ca_certs(PySSLContext *self, PyObject *args, PyObject *kwds)
{
    (void)self;
    (void)args;
    (void)kwds;
    return PyList_New(0);
}

static PyGetSetDef context_getsetlist[] = {
    {"check_hostname", (getter)get_check_hostname,
     (setter)set_check_hostname, NULL},
    {"options", (getter)get_options, (setter)set_options, NULL},
    {"verify_flags", (getter)get_verify_flags, (setter)set_verify_flags, NULL},
    {"verify_mode", (getter)get_verify_mode, (setter)set_verify_mode, NULL},
    {NULL, NULL, NULL, NULL}
};

static struct PyMethodDef context_methods[] = {
    {"_wrap_socket", (PyCFunction)context_wrap_socket,
     METH_VARARGS | METH_KEYWORDS, NULL},
    {"set_ciphers", (PyCFunction)set_ciphers, METH_VARARGS, NULL},
    {"_set_alpn_protocols", (PyCFunction)_set_alpn_protocols,
     METH_VARARGS, NULL},
    {"_set_npn_protocols", (PyCFunction)_set_npn_protocols,
     METH_VARARGS, NULL},
    {"load_cert_chain", (PyCFunction)load_cert_chain,
     METH_VARARGS | METH_KEYWORDS, NULL},
    {"load_dh_params", (PyCFunction)load_dh_params, METH_O, NULL},
    {"load_verify_locations", (PyCFunction)load_verify_locations,
     METH_VARARGS | METH_KEYWORDS, NULL},
    {"session_stats", (PyCFunction)session_stats, METH_NOARGS, NULL},
    {"set_default_verify_paths", (PyCFunction)set_default_verify_paths,
     METH_NOARGS, NULL},
    {"set_ecdh_curve", (PyCFunction)set_ecdh_curve, METH_O, NULL},
    {"set_servername_callback", (PyCFunction)set_servername_callback,
     METH_VARARGS, NULL},
    {"cert_store_stats", (PyCFunction)cert_store_stats, METH_NOARGS, NULL},
    {"get_ca_certs", (PyCFunction)get_ca_certs,
     METH_VARARGS | METH_KEYWORDS, NULL},
    {NULL, NULL, 0, NULL}
};

static PyTypeObject PySSLContext_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "_ssl._SSLContext",                 /* tp_name */
    sizeof(PySSLContext),               /* tp_basicsize */
    0,                                  /* tp_itemsize */
    (destructor)context_dealloc,        /* tp_dealloc */
    0,                                  /* tp_print */
    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,                                  /* tp_compare */
    0,                                  /* tp_repr */
    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */
    0,                                  /* tp_hash */
    0,                                  /* tp_call */
    0,                                  /* tp_str */
    PyObject_GenericGetAttr,            /* tp_getattro */
    0,                                  /* tp_setattro */
    0,                                  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    0,                                  /* tp_doc */
    0,                                  /* tp_traverse */
    0,                                  /* tp_clear */
    0,                                  /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */
    context_methods,                    /* tp_methods */
    0,                                  /* tp_members */
    context_getsetlist,                 /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    0,                                  /* tp_init */
    PyType_GenericAlloc,                /* tp_alloc */
    context_new,                        /* tp_new */
    PyObject_Del                        /* tp_free */
};

static PyObject *
asn1obj2py(const pyssl_asn1_obj *obj)
{
    return Py_BuildValue("isss", obj->nid, obj->sn, obj->ln, obj->oid);
}

static PyObject *
PySSL_txt2obj(PyObject *self, PyObject *args, PyObject *kwds)
{
    char *kwlist[3];
    char *txt;
    PyObject *pyname;
    int name;
    const pyssl_asn1_obj *p;

    (void)self;
    kwlist[0] = "txt";
    kwlist[1] = "name";
    kwlist[2] = NULL;
    pyname = Py_False;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|O:txt2obj", kwlist,
                                     &txt, &pyname))
        return NULL;
    name = PyObject_IsTrue(pyname);
    if (name < 0)
        return NULL;
    p = pyssl_asn1_table;
    while (p->oid != NULL) {
        if (name) {
            if (strcmp(txt, p->sn) == 0 || strcmp(txt, p->ln) == 0)
                return asn1obj2py(p);
        } else if (strcmp(txt, p->oid) == 0) {
            return asn1obj2py(p);
        }
        p++;
    }
    PyErr_Format(PyExc_ValueError, "unknown object '%.100s'", txt);
    return NULL;
}

static PyObject *
PySSL_nid2obj(PyObject *self, PyObject *args)
{
    int nid;
    const pyssl_asn1_obj *p;

    (void)self;
    if (!PyArg_ParseTuple(args, "i:nid2obj", &nid))
        return NULL;
    if (nid < 0) {
        PyErr_SetString(PyExc_ValueError, "NID must be positive.");
        return NULL;
    }
    p = pyssl_asn1_table;
    while (p->oid != NULL) {
        if (p->nid == nid)
            return asn1obj2py(p);
        p++;
    }
    PyErr_Format(PyExc_ValueError, "unknown NID %i", nid);
    return NULL;
}

static PyObject *
PySSL_RAND_status(PyObject *self)
{
    (void)self;
    return PyInt_FromLong(1);
}

static PyObject *
PySSL_RAND_add(PyObject *self, PyObject *args)
{
    char *buf;
    int len;
    PyObject *entropy;

    (void)self;
    if (!PyArg_ParseTuple(args, "s#O:RAND_add", &buf, &len, &entropy))
        return NULL;
    (void)entropy;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
PySSL_get_default_verify_paths(PyObject *self)
{
    (void)self;
    return Py_BuildValue("ssss",
                         "SSL_CERT_FILE", pyssl_default_ca,
                         "SSL_CERT_DIR", "");
}

static PyMethodDef PySSL_methods[] = {
    {"RAND_add", PySSL_RAND_add, METH_VARARGS, NULL},
    {"RAND_status", (PyCFunction)PySSL_RAND_status, METH_NOARGS, NULL},
    {"get_default_verify_paths", (PyCFunction)PySSL_get_default_verify_paths,
     METH_NOARGS, NULL},
    {"txt2obj", (PyCFunction)PySSL_txt2obj, METH_VARARGS | METH_KEYWORDS, NULL},
    {"nid2obj", (PyCFunction)PySSL_nid2obj, METH_VARARGS, NULL},
    {NULL, NULL, 0, NULL}
};

static int
add_flag(PyObject *m, const char *name, int on)
{
    PyObject *r;

    r = on ? Py_True : Py_False;
    Py_INCREF(r);
    return PyModule_AddObject(m, name, r);
}

PyDoc_STRVAR(module_doc,
"Implementation module for SSL socket operations, AmiTLS (BearSSL) backend.");

void
init_ssl(void)
{
    PyObject *m;
    PyObject *oi;
    int librev;

    if (amiga_open_amitls() != 0)
        return;

    Py_TYPE(&PySSLContext_Type) = &PyType_Type;
    Py_TYPE(&PySSLSocket_Type) = &PyType_Type;
    PySSLContext_Type.tp_new = context_new;
    PySSLContext_Type.tp_alloc = PyType_GenericAlloc;
    PySSLContext_Type.tp_free = PyObject_Del;
    PySSLSocket_Type.tp_alloc = PyType_GenericAlloc;
    PySSLSocket_Type.tp_free = PyObject_Del;
    if (PyType_Ready(&PySSLContext_Type) < 0)
        return;
    if (PyType_Ready(&PySSLSocket_Type) < 0)
        return;

    m = Py_InitModule3("_ssl", PySSL_methods, module_doc);
    if (m == NULL)
        return;

    if (PySocketModule_ImportModuleAndAPI())
        return;

    PySSLErrorObject = PyErr_NewException("ssl.SSLError",
                                          PySocketModule.error, NULL);
    PySSLZeroReturnErrorObject = PyErr_NewException("ssl.SSLZeroReturnError",
                                                    PySSLErrorObject, NULL);
    PySSLWantReadErrorObject = PyErr_NewException("ssl.SSLWantReadError",
                                                  PySSLErrorObject, NULL);
    PySSLWantWriteErrorObject = PyErr_NewException("ssl.SSLWantWriteError",
                                                   PySSLErrorObject, NULL);
    PySSLSyscallErrorObject = PyErr_NewException("ssl.SSLSyscallError",
                                                 PySSLErrorObject, NULL);
    PySSLEOFErrorObject = PyErr_NewException("ssl.SSLEOFError",
                                             PySSLErrorObject, NULL);
    if (PySSLErrorObject == NULL || PySSLZeroReturnErrorObject == NULL
        || PySSLWantReadErrorObject == NULL || PySSLWantWriteErrorObject == NULL
        || PySSLSyscallErrorObject == NULL || PySSLEOFErrorObject == NULL)
        return;

    Py_INCREF(PySSLErrorObject);
    PyModule_AddObject(m, "SSLError", PySSLErrorObject);
    Py_INCREF(PySSLZeroReturnErrorObject);
    PyModule_AddObject(m, "SSLZeroReturnError", PySSLZeroReturnErrorObject);
    Py_INCREF(PySSLWantReadErrorObject);
    PyModule_AddObject(m, "SSLWantReadError", PySSLWantReadErrorObject);
    Py_INCREF(PySSLWantWriteErrorObject);
    PyModule_AddObject(m, "SSLWantWriteError", PySSLWantWriteErrorObject);
    Py_INCREF(PySSLSyscallErrorObject);
    PyModule_AddObject(m, "SSLSyscallError", PySSLSyscallErrorObject);
    Py_INCREF(PySSLEOFErrorObject);
    PyModule_AddObject(m, "SSLEOFError", PySSLEOFErrorObject);
    Py_INCREF((PyObject *)&PySSLContext_Type);
    PyModule_AddObject(m, "_SSLContext", (PyObject *)&PySSLContext_Type);
    Py_INCREF((PyObject *)&PySSLSocket_Type);
    PyModule_AddObject(m, "_SSLSocket", (PyObject *)&PySSLSocket_Type);

    PyModule_AddIntConstant(m, "SSL_ERROR_ZERO_RETURN", PY_SSL_ERROR_ZERO_RETURN);
    PyModule_AddIntConstant(m, "SSL_ERROR_WANT_READ", PY_SSL_ERROR_WANT_READ);
    PyModule_AddIntConstant(m, "SSL_ERROR_WANT_WRITE", PY_SSL_ERROR_WANT_WRITE);
    PyModule_AddIntConstant(m, "SSL_ERROR_WANT_X509_LOOKUP",
                            PY_SSL_ERROR_WANT_X509_LOOKUP);
    PyModule_AddIntConstant(m, "SSL_ERROR_SYSCALL", PY_SSL_ERROR_SYSCALL);
    PyModule_AddIntConstant(m, "SSL_ERROR_SSL", PY_SSL_ERROR_SSL);
    PyModule_AddIntConstant(m, "SSL_ERROR_WANT_CONNECT",
                            PY_SSL_ERROR_WANT_CONNECT);
    PyModule_AddIntConstant(m, "SSL_ERROR_EOF", PY_SSL_ERROR_EOF);
    PyModule_AddIntConstant(m, "SSL_ERROR_INVALID_ERROR_CODE",
                            PY_SSL_ERROR_INVALID_ERROR_CODE);
    PyModule_AddIntConstant(m, "CERT_NONE", PY_SSL_CERT_NONE);
    PyModule_AddIntConstant(m, "CERT_OPTIONAL", PY_SSL_CERT_OPTIONAL);
    PyModule_AddIntConstant(m, "CERT_REQUIRED", PY_SSL_CERT_REQUIRED);
    PyModule_AddIntConstant(m, "VERIFY_DEFAULT", 0);
    PyModule_AddIntConstant(m, "PROTOCOL_TLS", PY_SSL_VERSION_TLS);
    PyModule_AddIntConstant(m, "PROTOCOL_SSLv23", PY_SSL_VERSION_TLS);
    PyModule_AddIntConstant(m, "PROTOCOL_TLSv1", PY_SSL_VERSION_TLS1);
    PyModule_AddIntConstant(m, "PROTOCOL_TLSv1_1", PY_SSL_VERSION_TLS1_1);
    PyModule_AddIntConstant(m, "PROTOCOL_TLSv1_2", PY_SSL_VERSION_TLS1_2);

    PyModule_AddIntConstant(m, "OP_ALL",
                            (long)(PYSSL_OP_ALL &
                                   ~PYSSL_OP_DONT_INSERT_EMPTY_FRAGMENTS));
    PyModule_AddIntConstant(m, "OP_NO_SSLv2", 0);
    PyModule_AddIntConstant(m, "OP_NO_SSLv3", (long)PYSSL_OP_NO_SSLv3);
    PyModule_AddIntConstant(m, "OP_NO_TLSv1", (long)PYSSL_OP_NO_TLSv1);
    PyModule_AddIntConstant(m, "OP_NO_TLSv1_1", (long)PYSSL_OP_NO_TLSv1_1);
    PyModule_AddIntConstant(m, "OP_NO_TLSv1_2", (long)PYSSL_OP_NO_TLSv1_2);
    PyModule_AddIntConstant(m, "OP_NO_TLSv1_3", (long)PYSSL_OP_NO_TLSv1_3);
    PyModule_AddIntConstant(m, "OP_CIPHER_SERVER_PREFERENCE",
                            (long)PYSSL_OP_CIPHER_SERVER_PREFERENCE);
    PyModule_AddIntConstant(m, "OP_SINGLE_DH_USE", 0);
    PyModule_AddIntConstant(m, "OP_SINGLE_ECDH_USE", 0);
    PyModule_AddIntConstant(m, "OP_NO_COMPRESSION",
                            (long)PYSSL_OP_NO_COMPRESSION);
    PyModule_AddIntConstant(m, "OP_ENABLE_MIDDLEBOX_COMPAT",
                            (long)PYSSL_OP_ENABLE_MIDDLEBOX_COMPAT);

    add_flag(m, "HAS_SNI", 1);
    add_flag(m, "HAS_ECDH", 1);
    add_flag(m, "HAS_NPN", 0);
    add_flag(m, "HAS_ALPN", 0);
    add_flag(m, "HAS_TLSv1_3", 0);
    add_flag(m, "HAS_TLS_UNIQUE", 0);

    /* Compatibility stamp only; this backend is BearSSL via amitls.library. */
    PyModule_AddIntConstant(m, "OPENSSL_VERSION_NUMBER", 0x1000200FL);
    oi = Py_BuildValue("iiiii", 1, 0, 2, 0, 15);
    if (oi != NULL)
        PyModule_AddObject(m, "OPENSSL_VERSION_INFO", oi);
    PyModule_AddStringConstant(m, "OPENSSL_VERSION",
                               "amitls.library (BearSSL)");
    if (oi != NULL) {
        Py_INCREF(oi);
        PyModule_AddObject(m, "_OPENSSL_API_VERSION", oi);
    }
    PyModule_AddIntConstant(m, "amiga_plugin_rev", 12);
    librev = -1;
    if (TlsBase != NULL)
        librev = (int)TlsBase->lib_Revision;
    PyModule_AddIntConstant(m, "amitls_lib_revision", librev);
}
