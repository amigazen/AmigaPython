/*
 * Amiga _ssl: CPython 2.7 ssl.py API for OS3.
 *
 * Built as lib/lib-dynload/_ssl.module (not linked into python27).
 * Default plugin build uses amitls.library (-DPYAMIGA_USE_AMITLS).
 * This file is the AmiSSL 5 / OpenSSL 3 backend (no PYAMIGA_USE_AMITLS).
 *
 * Socket I/O uses host psockets (posix sock_fd over AmiTCP). SSL_set_fd
 * gets the AmiTCP id via PyHost->fn_socket_native_fd. FIONBIO uses host
 * fn_socket_set_nbio (D7-safe IoctlSocket) — not a plugin AmiTCP jsr.
 *
 * AmiSSL I/O (AWeb Assl_* / SDK https.c), opposite of AmiTLS:
 *   - Clear FIONBIO before SSL_set_fd / SSL_do_handshake.
 *   - Handshake on a blocking fd; WaitSelect only on WANT_READ/WRITE.
 *   - Around SSL_read (and write): FIONBIO on + BIO NBIO, then restore
 *     blocking. Blocking SSL_read on 68k AmiSSL never returns for Ctrl-C.
 *
 * C89 / ANSI; plugin FAR data; no AmiSSL autoinit CRT.
 */
#ifdef PYAMIGA_USE_AMITLS
#include "sslmodule_amitls.c"
#else
#include "Python.h"

#include <exec/types.h>
#include <exec/execbase.h>
#include <dos/dosextens.h>
#include <proto/exec.h>
#include <utility/tagitem.h>
#include <libraries/amisslmaster.h>
#include <amissl/tags.h>
#include <proto/amisslmaster.h>
#include <proto/amissl.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/asn1.h>
#include <openssl/objects.h>
#include <openssl/opensslv.h>
#include <openssl/crypto.h>
#include <openssl/dh.h>
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

/* AmiSSL OS3 bases — proto/amissl*.h LVOs use these (A6), not IAmiSSL. */
struct Library *AmiSSLMasterBase = NULL;
struct Library *AmiSSLBase = NULL;
struct Library *AmiSSLExtBase = NULL;

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

static PyObject *PySSLErrorObject;
static PyObject *PySSLZeroReturnErrorObject;
static PyObject *PySSLWantReadErrorObject;
static PyObject *PySSLWantWriteErrorObject;
static PyObject *PySSLSyscallErrorObject;
static PyObject *PySSLEOFErrorObject;

typedef struct {
    PyObject_HEAD
    SSL_CTX *ctx;
    unsigned char *alpn_protocols;
    unsigned int alpn_protocols_len;
    int check_hostname;
    char *pw_buf;
    int pw_len;
} PySSLContext;

typedef struct {
    PyObject_HEAD
    PySSLContext *ctx;
    PySocketSockObject *Socket;
    SSL *ssl;
    SSL_CTX *own_ctx;
    X509 *peer_cert;
    int native_fd;
    int handshake_done;
    int shutdown_seen_zero;
    enum py_ssl_server_or_client socket_type;
    char sni[256];
} PySSLSocket;

static PyTypeObject PySSLContext_Type;
static PyTypeObject PySSLSocket_Type;

static int amiga_amissl_ready = 0;

/*
 * OpenSSL 3 SSL_OP_BIT() is (uint64_t)1<<n. VBCC emits 64-bit shifts and
 * AmiSSL SSL_CTX_set_options takes uint64_t in d0/d1. Passing a 32-bit
 * long leaves d1 as garbage and can set SSL_OP_NO_TLSv1..1_3 together
 * ("ssl ctx has no default ssl version" at SSL_new). Build bits 0..31
 * as unsigned long and zero-extend with a 32-bit word store.
 */
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

typedef union {
    unsigned long long ull;
    unsigned long w[2]; /* 68k BE: w[0]=high, w[1]=low */
} pyssl_u64;

static unsigned long long
pyssl_op64(unsigned long low)
{
    pyssl_u64 u;

    u.w[0] = 0;
    u.w[1] = low;
    return u.ull;
}

static unsigned long
pyssl_op32(unsigned long long op)
{
    pyssl_u64 u;

    u.ull = op;
    return u.w[1];
}

static PyObject *
ssl_nomem(void)
{
    PyErr_SetString(PyExc_MemoryError, "out of memory");
    return NULL;
}

static void
amiga_close_amissl(void)
{
    if (AmiSSLBase != NULL) {
        CloseAmiSSL();
        AmiSSLBase = NULL;
        AmiSSLExtBase = NULL;
    }
    if (AmiSSLMasterBase != NULL) {
        CloseLibrary(AmiSSLMasterBase);
        AmiSSLMasterBase = NULL;
    }
    amiga_amissl_ready = 0;
}

static int
amiga_open_amissl(void)
{
    struct TagItem tags[8];
    struct Process *pr;
    APTR oldwin;
    LONG err;
    int n;

    if (amiga_amissl_ready && AmiSSLBase != NULL)
        return 0;

#ifdef PYAMIGA_PLUGIN_BUILD
    if (PyAmiga_Host == NULL || PyAmiga_Host->ptr_SocketBase == NULL) {
        PyErr_SetString(PyExc_ImportError,
                        "AmiSSL needs host SocketBase (import _socket first)");
        return -1;
    }
#endif

    pr = (struct Process *)FindTask(NULL);
    oldwin = pr->pr_WindowPtr;
    pr->pr_WindowPtr = (APTR)-1L;

    AmiSSLMasterBase = OpenLibrary("amisslmaster.library",
                                   AMISSLMASTER_MIN_VERSION);
    if (AmiSSLMasterBase == NULL) {
        pr->pr_WindowPtr = oldwin;
        PyErr_SetString(PyExc_ImportError,
                        "amisslmaster.library required for _ssl "
                        "(AmiSSL not installed)");
        return -1;
    }

    n = 0;
    /* SDK Examples/httpget.c OS3: UsesOpenSSLStructs FALSE. TRUE pins
     * OpenSSL struct layout; we only use opaque SSL/SSL_CTX pointers. */
    tags[n].ti_Tag = AmiSSL_UsesOpenSSLStructs;
    tags[n].ti_Data = FALSE;
    n++;
    tags[n].ti_Tag = AmiSSL_GetAmiSSLBase;
    tags[n].ti_Data = (ULONG)&AmiSSLBase;
    n++;
    tags[n].ti_Tag = AmiSSL_GetAmiSSLExtBase;
    tags[n].ti_Data = (ULONG)&AmiSSLExtBase;
    n++;
    tags[n].ti_Tag = AmiSSL_SocketBase;
#ifdef PYAMIGA_PLUGIN_BUILD
    tags[n].ti_Data = (ULONG)PyAmiga_Host->ptr_SocketBase;
#else
    tags[n].ti_Data = 0;
#endif
    n++;
    tags[n].ti_Tag = AmiSSL_ErrNoPtr;
#ifdef PYAMIGA_PLUGIN_BUILD
    tags[n].ti_Data = (ULONG)PyAmiga_Host->ptr_errno;
#else
    tags[n].ti_Data = 0;
#endif
    n++;
    tags[n].ti_Tag = TAG_DONE;
    tags[n].ti_Data = 0;

    err = OpenAmiSSLTagList(AMISSL_CURRENT_VERSION, tags);
    pr->pr_WindowPtr = oldwin;

    if (err != 0 || AmiSSLBase == NULL) {
        CloseLibrary(AmiSSLMasterBase);
        AmiSSLMasterBase = NULL;
        AmiSSLBase = NULL;
        PyErr_SetString(PyExc_ImportError,
                        "OpenAmiSSLTagList failed (need AmiSSL 5.x)");
        return -1;
    }
    amiga_amissl_ready = 1;
    Py_AtExit(amiga_close_amissl);
    return 0;
}

static PyObject *
_setSSLError(const char *errstr, int errcode, const char *filename, int lineno)
{
    unsigned long e;
    char buf[256];
    PyObject *v;

    (void)filename;
    (void)lineno;
    e = ERR_peek_last_error();
    if (e != 0) {
        ERR_error_string_n(e, buf, sizeof(buf));
        errstr = buf;
        ERR_clear_error();
    } else if (errstr == NULL) {
        errstr = "unknown error";
    }
    if (errcode != 0)
        v = Py_BuildValue("is", errcode, errstr);
    else
        v = Py_BuildValue("s", errstr);
    if (v == NULL)
        return NULL;
    PyErr_SetObject(PySSLErrorObject, v);
    Py_DECREF(v);
    return NULL;
}

static PyObject *
PySSL_SetError(PySSLSocket *obj, int ret)
{
    int err;
    enum py_ssl_error p;
    const char *errstr;
    unsigned long e;
    char buf[256];
    PyObject *v;
    PyObject *exc;

    err = SSL_get_error(obj->ssl, ret);
    exc = PySSLErrorObject;
    switch (err) {
    case SSL_ERROR_ZERO_RETURN:
        errstr = "TLS/SSL connection has been closed";
        p = PY_SSL_ERROR_ZERO_RETURN;
        exc = PySSLZeroReturnErrorObject;
        break;
    case SSL_ERROR_WANT_READ:
        errstr = "The operation did not complete (read)";
        p = PY_SSL_ERROR_WANT_READ;
        exc = PySSLWantReadErrorObject;
        break;
    case SSL_ERROR_WANT_WRITE:
        errstr = "The operation did not complete (write)";
        p = PY_SSL_ERROR_WANT_WRITE;
        exc = PySSLWantWriteErrorObject;
        break;
    case SSL_ERROR_WANT_X509_LOOKUP:
        errstr = "The operation did not complete (X509 lookup)";
        p = PY_SSL_ERROR_WANT_X509_LOOKUP;
        break;
    case SSL_ERROR_WANT_CONNECT:
        errstr = "The operation did not complete (connect)";
        p = PY_SSL_ERROR_WANT_CONNECT;
        break;
    case SSL_ERROR_SYSCALL:
        e = ERR_get_error();
        if (e == 0) {
            if (ret == 0 || obj->Socket == NULL) {
                p = PY_SSL_ERROR_EOF;
                errstr = "EOF occurred in violation of protocol";
                exc = PySSLEOFErrorObject;
            } else if (ret == -1 && obj->Socket != NULL
                       && obj->Socket->errorhandler != NULL) {
                return obj->Socket->errorhandler();
            } else {
                p = PY_SSL_ERROR_SYSCALL;
                errstr = "Some I/O error occurred";
                exc = PySSLSyscallErrorObject;
            }
        } else {
            p = PY_SSL_ERROR_SYSCALL;
            exc = PySSLSyscallErrorObject;
            ERR_error_string_n(e, buf, sizeof(buf));
            errstr = buf;
        }
        break;
    case SSL_ERROR_SSL:
        e = ERR_get_error();
        p = PY_SSL_ERROR_SSL;
        if (e != 0) {
            ERR_error_string_n(e, buf, sizeof(buf));
            errstr = buf;
        } else {
            int en;
            int want;
            const char *stname;

            en = 0;
            want = 0;
            stname = "?";
#ifdef PYAMIGA_PLUGIN_BUILD
            if (PyAmiga_Host != NULL && PyAmiga_Host->ptr_errno != NULL)
                en = *PyAmiga_Host->ptr_errno;
#endif
            if (obj->ssl != NULL) {
                want = SSL_want(obj->ssl);
                stname = SSL_state_string(obj->ssl);
                if (stname == NULL)
                    stname = "?";
            }
            sprintf(buf,
                    "SSL library failure (ssl_err=%d ret=%d errno=%d want=%d state=%s)",
                    err, ret, en, want, stname);
            errstr = buf;
        }
        break;
    default:
        p = PY_SSL_ERROR_INVALID_ERROR_CODE;
        errstr = "Invalid error code";
        break;
    }
    v = Py_BuildValue("is", (int)p, errstr);
    if (v == NULL)
        return NULL;
    PyErr_SetObject(exc, v);
    Py_DECREF(v);
    return NULL;
}

#ifdef PYAMIGA_PLUGIN_BUILD
/* 1.0 as big-endian IEEE-754; plugin must not do soft-float. */
static const unsigned char ssl_slice_1s[8] = {
    0x3F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
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

static int
ssl_timeout_nbio(PySocketSockObject *s)
{
    (void)s;
    /* Always NBIO on Amiga: AmiSSL SSL_read on a blocking fd never
     * returns, so Python never sees Ctrl-C. */
    return 1;
}

static void
ssl_sock_nbio(PySocketSockObject *sock, int nonblock)
{
    int posix_fd;

    /* AWeb Assl_read: IoctlSocket(FIONBIO) on the AmiTCP socket via host
     * fn_socket_set_nbio (D7-safe). Do this only around SSL_read/write —
     * FIONBIO before SSL_do_handshake makes AmiSSL return SSL_ERROR_SSL /
     * SSL_NOTHING instead of WANT_READ. */
    if (sock == NULL)
        return;
    posix_fd = sock->sock_fd;
    if (posix_fd < 0)
        return;
    if (PyAmiga_Host == NULL || PyAmiga_Host->fn_socket_set_nbio == NULL)
        return;
    (void)PyAmiga_Host->fn_socket_set_nbio(posix_fd, nonblock ? 1 : 0);
}

static void
ssl_force_nbio(PySSLSocket *self, PySocketSockObject *sock)
{
    BIO *rbio;
    BIO *wbio;

    (void)sock;
    rbio = SSL_get_rbio(self->ssl);
    wbio = SSL_get_wbio(self->ssl);
    if (rbio != NULL)
        BIO_ctrl(rbio, BIO_C_SET_NBIO, 1, NULL);
    if (wbio != NULL)
        BIO_ctrl(wbio, BIO_C_SET_NBIO, 1, NULL);
}
#else
static int
ssl_wait(PySocketSockObject *s, int writing)
{
    (void)s;
    (void)writing;
    return SOCK_IS_BLOCKING;
}
static int
ssl_timeout_nbio(PySocketSockObject *s)
{
    (void)s;
    return 0;
}
static void
ssl_force_nbio(PySSLSocket *self, PySocketSockObject *sock)
{
    (void)self;
    (void)sock;
}
static void
ssl_sock_nbio(PySocketSockObject *sock, int nonblock)
{
    (void)sock;
    (void)nonblock;
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
ssl_want_wait(SSL *ssl, int err, int ret, int *writing)
{
    /* pythonssl OS4 retries only WANT_READ/WANT_WRITE from SSL_get_error.
     * 68k AmiSSL often returns SSL_ERROR_SSL with an empty ERR queue
     * instead; SSL_want is the direction. Do not retry SSL_NOTHING as
     * write: a TCP socket stays writable and the handshake spins 20s. */
    if (ret > 0)
        return 0;
    if (err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_CONNECT) {
        *writing = 1;
        return 1;
    }
    if (err == SSL_ERROR_WANT_READ) {
        *writing = 0;
        return 1;
    }
    if (err != SSL_ERROR_SYSCALL && err != SSL_ERROR_SSL)
        return 0;
    if (ERR_peek_error() != 0)
        return 0;
    if (SSL_want_write(ssl)) {
        *writing = 1;
        return 1;
    }
    if (SSL_want_read(ssl)) {
        *writing = 0;
        return 1;
    }
    return 0;
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

static PySSLSocket *
newPySSLSocket(PySSLContext *sslctx, PySocketSockObject *sock,
               enum py_ssl_server_or_client socket_type,
               char *server_hostname, PyObject *ssl_sock)
{
    PySSLSocket *self;
    PySocketSockObject * volatile socksaved;
    int posix_fd;
    int nfd;
    const SSL_METHOD *meth;
    SSL_CTX *tmpctx;

    (void)ssl_sock;
    /* Copy pointer-derived values BEFORE any host trampoline.
     * 68k scratch regs (A0/A1/D0/D1) do not survive PyObject_New. */
    socksaved = sock;
    posix_fd = (sock != NULL) ? sock->sock_fd : -1;
    nfd = ssl_native_fd(posix_fd);
    if (nfd < 0) {
        PyErr_SetString(PySSLErrorObject, "invalid socket fd");
        return NULL;
    }

    meth = NULL;
    tmpctx = NULL;
    self = PyObject_New(PySSLSocket, &PySSLSocket_Type);
    if (self == NULL)
        return NULL;
    self->peer_cert = NULL;
    self->ssl = NULL;
    self->own_ctx = NULL;
    self->Socket = NULL;
    self->ctx = sslctx;
    self->native_fd = nfd;
    self->handshake_done = 0;
    self->shutdown_seen_zero = 0;
    self->socket_type = socket_type;
    self->sni[0] = '\0';
    if (server_hostname != NULL && server_hostname[0] != '\0') {
        strncpy(self->sni, server_hostname, sizeof(self->sni) - 1);
        self->sni[sizeof(self->sni) - 1] = '\0';
    }
    Py_INCREF(sslctx);

    ERR_clear_error();
    /* SDK Examples/https.c: SSL_CTX_new(TLS_client_method) then SSL_new
     * from THAT ctx. ssl.py SSLContext.__new__ calls set_ciphers; even a
     * no-op leaves sslctx->ctx with 0A0000E4 (no default ssl version).
     * Keep own_ctx until dealloc — AmiSSL 68k SSL_new does not up-ref. */
    meth = TLS_client_method();
    if (meth == NULL)
        meth = TLS_method();
    if (meth != NULL)
        tmpctx = SSL_CTX_new(meth);
    if (tmpctx != NULL) {
        SSL_CTX_set_verify(tmpctx, SSL_VERIFY_NONE, NULL);
        self->ssl = SSL_new(tmpctx);
        if (self->ssl == NULL)
            SSL_CTX_free(tmpctx);
        else
            self->own_ctx = tmpctx;
    }
    if (self->ssl == NULL) {
        _setSSLError(NULL, 0, __FILE__, __LINE__);
        Py_DECREF(self);
        return NULL;
    }
    /* SDK https.c / Assl_connect: blocking AmiTCP fd before SSL_set_fd.
     * settimeout() may have left FIONBIO on; that breaks handshake. */
    ssl_sock_nbio(socksaved, 0);
    if (SSL_set_fd(self->ssl, nfd) != 1) {
        _setSSLError("SSL_set_fd failed", 0, __FILE__, __LINE__);
        Py_DECREF(self);
        return NULL;
    }
    if (self->sni[0] != '\0') {
        if (SSL_set_tlsext_host_name(self->ssl, self->sni) != 1) {
            _setSSLError("SSL_set_tlsext_host_name failed", 0,
                         __FILE__, __LINE__);
            Py_DECREF(self);
            return NULL;
        }
    }
    if (socket_type == PY_SSL_CLIENT)
        SSL_set_connect_state(self->ssl);
    else
        SSL_set_accept_state(self->ssl);

    self->Socket = socksaved;
    Py_INCREF(self->Socket);
    return self;
}

static void
PySSL_dealloc(PySSLSocket *self)
{
    if (amiga_amissl_ready) {
        if (self->peer_cert != NULL)
            X509_free(self->peer_cert);
        if (self->ssl != NULL)
            SSL_free(self->ssl);
        if (self->own_ctx != NULL)
            SSL_CTX_free(self->own_ctx);
    }
    Py_XDECREF(self->ctx);
    Py_XDECREF(self->Socket);
    PyObject_Del(self);
}

static PyObject *
PySSL_SSLdo_handshake(PySSLSocket *self)
{
    int ret;
    int err;
    int st;
    int writing;
    PySocketSockObject *sock;
    unsigned char deadline[8];
    int have_dl;

    sock = self->Socket;
    if (sock == NULL) {
        PyErr_SetString(PySSLErrorObject, "underlying socket is None");
        return NULL;
    }
    Py_INCREF(sock);
    writing = 0;
    /* Blocking fd for handshake (AWeb Assl_connect / SDK https.c). */
    ssl_sock_nbio(sock, 0);
    ssl_deadline_begin(sock, deadline, &have_dl);
    /* 68k plugin: SSL_connect returns 0 / SSLERR / want=NOTHING with an
     * empty ERR queue (all three test hosts). The run that completed
     * TLSv1.3 used SSL_do_handshake. Do not IoctlSocket(FIONBIO) first. */
    for (;;) {
        if (ssl_deadline_hit(deadline, have_dl)) {
            Py_DECREF(sock);
            PyErr_SetString(PySSLErrorObject,
                            "The handshake operation timed out");
            return NULL;
        }
        ret = SSL_do_handshake(self->ssl);
        err = SSL_get_error(self->ssl, ret);
        if (PyErr_CheckSignals()) {
            Py_DECREF(sock);
            return NULL;
        }
        if (!ssl_want_wait(self->ssl, err, ret, &writing))
            break;
        st = ssl_wait(sock, writing);
        if (st == SOCK_HAS_BEEN_CLOSED) {
            Py_DECREF(sock);
            PyErr_SetString(PySSLErrorObject,
                            "Underlying socket has been closed.");
            return NULL;
        }
        if (st == SOCK_IS_NONBLOCKING)
            break;
    }
    Py_DECREF(sock);
    if (ret <= 0)
        return PySSL_SetError(self, ret);
    if (self->peer_cert != NULL)
        X509_free(self->peer_cert);
    self->peer_cert = SSL_get1_peer_certificate(self->ssl);
    self->handshake_done = 1;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_certificate_to_der(X509 *cert)
{
    unsigned char *p;
    int len;
    PyObject *retval;

    p = NULL;
    len = i2d_X509(cert, &p);
    if (len < 0)
        return _setSSLError(NULL, 0, __FILE__, __LINE__);
    retval = PyString_FromStringAndSize((const char *)p, len);
    OPENSSL_free(p);
    return retval;
}

static PyObject *
_x509_name_to_tuple(X509_NAME *name)
{
    PyObject *dn;
    PyObject *rdn;
    PyObject *pair;
    PyObject *k;
    PyObject *v;
    X509_NAME_ENTRY *ent;
    ASN1_OBJECT *obj;
    ASN1_STRING *data;
    const char *ln;
    const unsigned char *s;
    int i;
    int n;
    int nid;

    n = X509_NAME_entry_count(name);
    dn = PyTuple_New(n);
    if (dn == NULL)
        return NULL;
    for (i = 0; i < n; i++) {
        ent = X509_NAME_get_entry(name, i);
        obj = X509_NAME_ENTRY_get_object(ent);
        data = X509_NAME_ENTRY_get_data(ent);
        nid = OBJ_obj2nid(obj);
        ln = OBJ_nid2ln(nid);
        if (ln == NULL)
            ln = OBJ_nid2sn(nid);
        if (ln == NULL)
            ln = "unknown";
        s = ASN1_STRING_get0_data(data);
        k = PyString_FromString(ln);
        v = PyString_FromStringAndSize((const char *)s,
                                       ASN1_STRING_length(data));
        pair = PyTuple_New(2);
        rdn = PyTuple_New(1);
        if (k == NULL || v == NULL || pair == NULL || rdn == NULL) {
            Py_XDECREF(k);
            Py_XDECREF(v);
            Py_XDECREF(pair);
            Py_XDECREF(rdn);
            Py_DECREF(dn);
            return NULL;
        }
        PyTuple_SET_ITEM(pair, 0, k);
        PyTuple_SET_ITEM(pair, 1, v);
        PyTuple_SET_ITEM(rdn, 0, pair);
        PyTuple_SET_ITEM(dn, i, rdn);
    }
    return dn;
}

/* Subject/issuer as a tuple of RDNs; SAN as (('DNS', name), ...). */
static PyObject *
_decode_certificate(X509 *cert)
{
    PyObject *retval;
    PyObject *subj;
    PyObject *issu;
    PyObject *san;
    PyObject *item;
    PyObject *pair;
    PyObject *k;
    PyObject *v;
    GENERAL_NAMES *names;
    GENERAL_NAME *gn;
    const unsigned char *s;
    const ASN1_TIME *t;
    BIO *bio;
    char tbuf[64];
    int i;
    int n;
    int ndns;
    int slot;
    int nread;

    retval = PyDict_New();
    if (retval == NULL)
        return NULL;
    subj = _x509_name_to_tuple(X509_get_subject_name(cert));
    issu = _x509_name_to_tuple(X509_get_issuer_name(cert));
    if (subj == NULL || issu == NULL) {
        Py_XDECREF(subj);
        Py_XDECREF(issu);
        Py_DECREF(retval);
        return NULL;
    }
    names = X509_get_ext_d2i(cert, NID_subject_alt_name, NULL, NULL);
    ndns = 0;
    if (names != NULL) {
        n = sk_GENERAL_NAME_num(names);
        for (i = 0; i < n; i++) {
            gn = sk_GENERAL_NAME_value(names, i);
            if (gn != NULL && gn->type == GEN_DNS)
                ndns++;
        }
    }
    san = PyTuple_New(ndns);
    if (san == NULL) {
        if (names != NULL)
            GENERAL_NAMES_free(names);
        Py_DECREF(subj);
        Py_DECREF(issu);
        Py_DECREF(retval);
        return NULL;
    }
    slot = 0;
    if (names != NULL) {
        n = sk_GENERAL_NAME_num(names);
        for (i = 0; i < n; i++) {
            gn = sk_GENERAL_NAME_value(names, i);
            if (gn == NULL || gn->type != GEN_DNS)
                continue;
            s = ASN1_STRING_get0_data(gn->d.dNSName);
            k = PyString_FromString("DNS");
            v = PyString_FromStringAndSize((const char *)s,
                                           ASN1_STRING_length(gn->d.dNSName));
            pair = PyTuple_New(2);
            if (k == NULL || v == NULL || pair == NULL) {
                Py_XDECREF(k);
                Py_XDECREF(v);
                Py_XDECREF(pair);
                GENERAL_NAMES_free(names);
                Py_DECREF(san);
                Py_DECREF(subj);
                Py_DECREF(issu);
                Py_DECREF(retval);
                return NULL;
            }
            PyTuple_SET_ITEM(pair, 0, k);
            PyTuple_SET_ITEM(pair, 1, v);
            PyTuple_SET_ITEM(san, slot, pair);
            slot++;
        }
        GENERAL_NAMES_free(names);
    }
    if (PyDict_SetItemString(retval, "subject", subj) < 0
        || PyDict_SetItemString(retval, "issuer", issu) < 0
        || PyDict_SetItemString(retval, "subjectAltName", san) < 0) {
        Py_DECREF(subj);
        Py_DECREF(issu);
        Py_DECREF(san);
        Py_DECREF(retval);
        return NULL;
    }
    Py_DECREF(subj);
    Py_DECREF(issu);
    Py_DECREF(san);

    bio = BIO_new(BIO_s_mem());
    if (bio != NULL) {
        t = X509_get0_notBefore(cert);
        if (t != NULL && ASN1_TIME_print(bio, t) > 0) {
            nread = BIO_read(bio, tbuf, (int)sizeof(tbuf) - 1);
            if (nread > 0) {
                tbuf[nread] = '\0';
                item = PyString_FromString(tbuf);
                if (item != NULL) {
                    PyDict_SetItemString(retval, "notBefore", item);
                    Py_DECREF(item);
                }
            }
        }
        BIO_reset(bio);
        t = X509_get0_notAfter(cert);
        if (t != NULL && ASN1_TIME_print(bio, t) > 0) {
            nread = BIO_read(bio, tbuf, (int)sizeof(tbuf) - 1);
            if (nread > 0) {
                tbuf[nread] = '\0';
                item = PyString_FromString(tbuf);
                if (item != NULL) {
                    PyDict_SetItemString(retval, "notAfter", item);
                    Py_DECREF(item);
                }
            }
        }
        BIO_free(bio);
    }
    return retval;
}

static PyObject *
PySSL_peercert(PySSLSocket *self, PyObject *args)
{
    PyObject *binary_mode = Py_None;
    int b;
    int verification;

    if (!PyArg_ParseTuple(args, "|O:peer_certificate", &binary_mode))
        return NULL;
    if (!self->handshake_done) {
        PyErr_SetString(PyExc_ValueError, "handshake not done yet");
        return NULL;
    }
    if (self->peer_cert == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    b = PyObject_IsTrue(binary_mode);
    if (b < 0)
        return NULL;
    if (b)
        return _certificate_to_der(self->peer_cert);
    verification = SSL_CTX_get_verify_mode(SSL_get_SSL_CTX(self->ssl));
    if ((verification & SSL_VERIFY_PEER) == 0)
        return PyDict_New();
    return _decode_certificate(self->peer_cert);
}

static PyObject *
PySSL_cipher(PySSLSocket *self)
{
    const SSL_CIPHER *current;
    PyObject *retval;
    PyObject *v;
    const char *cipher_name;
    const char *cipher_protocol;
    int bits;

    if (self->ssl == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    current = SSL_get_current_cipher(self->ssl);
    if (current == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    retval = PyTuple_New(3);
    if (retval == NULL)
        return NULL;
    cipher_name = SSL_CIPHER_get_name(current);
    cipher_protocol = SSL_CIPHER_get_version(current);
    bits = SSL_CIPHER_get_bits(current, NULL);
    v = PyString_FromString(cipher_name ? cipher_name : "");
    if (v == NULL) {
        Py_DECREF(retval);
        return NULL;
    }
    PyTuple_SET_ITEM(retval, 0, v);
    v = PyString_FromString(cipher_protocol ? cipher_protocol : "");
    if (v == NULL) {
        Py_DECREF(retval);
        return NULL;
    }
    PyTuple_SET_ITEM(retval, 1, v);
    v = PyInt_FromLong(bits);
    if (v == NULL) {
        Py_DECREF(retval);
        return NULL;
    }
    PyTuple_SET_ITEM(retval, 2, v);
    return retval;
}

static PyObject *
PySSL_version(PySSLSocket *self)
{
    const char *v;

    if (self->ssl == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    v = SSL_get_version(self->ssl);
    return PyString_FromString(v ? v : "unknown");
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
    const unsigned char *out;
    unsigned int len;

    if (self->ssl == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    out = NULL;
    len = 0;
    SSL_get0_alpn_selected(self->ssl, &out, &len);
    if (out == NULL || len == 0) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return PyString_FromStringAndSize((const char *)out, (Py_ssize_t)len);
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
    (void)closure;
    if (value == NULL || Py_TYPE(value) != &PySSLContext_Type) {
        PyErr_SetString(PyExc_TypeError, "context must be an _SSLContext");
        return -1;
    }
    Py_INCREF(value);
    Py_DECREF(self->ctx);
    self->ctx = (PySSLContext *)value;
    SSL_set_SSL_CTX(self->ssl, self->ctx->ctx);
    return 0;
}

static PyObject *
PySSL_SSLwrite(PySSLSocket *self, PyObject *args)
{
    char *data;
    int len;
    int ret;
    int err;
    int st;
    PySocketSockObject *sock;
    unsigned char deadline[8];
    int have_dl;
    int writing;

    if (!PyArg_ParseTuple(args, "s#:write", &data, &len))
        return NULL;
    sock = self->Socket;
    Py_INCREF(sock);
    ssl_deadline_begin(sock, deadline, &have_dl);
    /* AWeb Assl_read model applied to write: FIONBIO + WANT_* so SSL_write
     * cannot wedge the interpreter on a blocking AmiTCP fd. */
    ssl_force_nbio(self, sock);
    ssl_sock_nbio(sock, 1);
    /* pythonssl OS4: WaitSelect writable, then SSL_write; retry WANT_* only. */
    for (;;) {
        if (ssl_deadline_hit(deadline, have_dl)) {
            ssl_sock_nbio(sock, 0);
            Py_DECREF(sock);
            PyErr_SetString(PySSLErrorObject,
                            "The write operation timed out");
            return NULL;
        }
        st = ssl_wait(sock, 1);
        if (st == SOCK_HAS_BEEN_CLOSED) {
            ssl_sock_nbio(sock, 0);
            Py_DECREF(sock);
            PyErr_SetString(PySSLErrorObject,
                            "Underlying socket has been closed.");
            return NULL;
        }
        if (st != SOCK_HAS_TIMED_OUT)
            break;
    }
    if (st == SOCK_HAS_TIMED_OUT) {
        ssl_sock_nbio(sock, 0);
        Py_DECREF(sock);
        PyErr_SetString(PySSLErrorObject,
                        "The write operation timed out");
        return NULL;
    }
    for (;;) {
        if (ssl_deadline_hit(deadline, have_dl)) {
            ssl_sock_nbio(sock, 0);
            Py_DECREF(sock);
            PyErr_SetString(PySSLErrorObject,
                            "The write operation timed out");
            return NULL;
        }
        ret = SSL_write(self->ssl, data, len);
        err = SSL_get_error(self->ssl, ret);
        if (PyErr_CheckSignals()) {
            ssl_sock_nbio(sock, 0);
            Py_DECREF(sock);
            return NULL;
        }
        if (!ssl_want_wait(self->ssl, err, ret, &writing))
            break;
        st = ssl_wait(sock, writing);
        if (st == SOCK_IS_NONBLOCKING)
            break;
    }
    ssl_sock_nbio(sock, 0);
    Py_DECREF(sock);
    if (ret <= 0)
        return PySSL_SetError(self, ret);
    return PyInt_FromLong(ret);
}

static PyObject *
PySSL_SSLpending(PySSLSocket *self)
{
    int n;

    n = SSL_pending(self->ssl);
    return PyInt_FromLong(n);
}

static PyObject *
PySSL_SSLread(PySSLSocket *self, PyObject *args)
{
    PyObject *buffer = NULL;
    PyObject *dest = NULL;
    char *data;
    int len = 1024;
    int ret;
    int err;
    int st;
    Py_ssize_t buflen;
    void *wbuf;
    PySocketSockObject *sock;
    unsigned char deadline[8];
    int have_dl;
    int writing;

    if (!PyArg_ParseTuple(args, "|iO:read", &len, &buffer))
        return NULL;
    if (len < 0) {
        PyErr_SetString(PyExc_ValueError, "size should not be negative");
        return NULL;
    }
    sock = self->Socket;
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
    ssl_deadline_begin(sock, deadline, &have_dl);
    /* AWeb Assl_read: FIONBIO around SSL_read so WANT_READ/WRITE can
     * drive WaitSelect; blocking SSL_read never returns on 68k AmiSSL. */
    ssl_force_nbio(self, sock);
    ssl_sock_nbio(sock, 1);
    /* pythonssl OS4: WaitSelect readable first unless SSL_pending, then
     * SSL_read; retry WANT_READ/WANT_WRITE only. */
    if (SSL_pending(self->ssl) <= 0) {
        for (;;) {
            if (ssl_deadline_hit(deadline, have_dl)) {
                ssl_sock_nbio(sock, 0);
                Py_XDECREF(dest);
                Py_DECREF(sock);
                PyErr_SetString(PySSLErrorObject,
                                "The read operation timed out");
                return NULL;
            }
            st = ssl_wait(sock, 0);
            if (st == SOCK_HAS_BEEN_CLOSED) {
                ssl_sock_nbio(sock, 0);
                Py_XDECREF(dest);
                Py_DECREF(sock);
                PyErr_SetString(PySSLErrorObject,
                                "Underlying socket has been closed.");
                return NULL;
            }
            if (st != SOCK_HAS_TIMED_OUT)
                break;
        }
        if (st == SOCK_HAS_TIMED_OUT) {
            ssl_sock_nbio(sock, 0);
            Py_XDECREF(dest);
            Py_DECREF(sock);
            PyErr_SetString(PySSLErrorObject,
                            "The read operation timed out");
            return NULL;
        }
    }
    writing = 0;
    for (;;) {
        if (ssl_deadline_hit(deadline, have_dl)) {
            ssl_sock_nbio(sock, 0);
            Py_XDECREF(dest);
            Py_DECREF(sock);
            PyErr_SetString(PySSLErrorObject,
                            "The read operation timed out");
            return NULL;
        }
        data = (dest != NULL) ? PyString_AsString(dest) : (char *)wbuf;
        ret = SSL_read(self->ssl, data, len);
        err = SSL_get_error(self->ssl, ret);
        if (PyErr_CheckSignals()) {
            ssl_sock_nbio(sock, 0);
            Py_XDECREF(dest);
            Py_DECREF(sock);
            return NULL;
        }
        if (!ssl_want_wait(self->ssl, err, ret, &writing))
            break;
        st = ssl_wait(sock, writing);
        if (st == SOCK_IS_NONBLOCKING)
            break;
    }
    ssl_sock_nbio(sock, 0);
    Py_DECREF(sock);
    if (ret < 0) {
        Py_XDECREF(dest);
        return PySSL_SetError(self, ret);
    }
    if (ret == 0) {
        err = SSL_get_error(self->ssl, ret);
        if (err == SSL_ERROR_ZERO_RETURN) {
            if (dest != NULL) {
                if (_PyString_Resize(&dest, 0) < 0)
                    return NULL;
                return dest;
            }
            return PyInt_FromLong(0);
        }
        Py_XDECREF(dest);
        return PySSL_SetError(self, ret);
    }
    if (dest != NULL) {
        if (_PyString_Resize(&dest, ret) < 0)
            return NULL;
        return dest;
    }
    return PyInt_FromLong(ret);
}

static PyObject *
PySSL_SSLshutdown(PySSLSocket *self)
{
    int ret;
    int err;
    int st;
    int zeros;
    PySocketSockObject *sock;
    PyObject *sockobj;

    sock = self->Socket;
    if (sock == NULL) {
        PyErr_SetString(PySSLErrorObject, "underlying socket is None");
        return NULL;
    }
    zeros = 0;
    Py_INCREF(sock);
    /* Assl_closessl: never block forever in SSL_shutdown. */
    ssl_sock_nbio(sock, 1);
    while (1) {
        ret = SSL_shutdown(self->ssl);
        err = SSL_get_error(self->ssl, ret);
        if (ret > 0)
            break;
        if (ret == 0) {
            zeros++;
            if (zeros > 1)
                break;
            st = ssl_wait(sock, 0);
            if (st == SOCK_HAS_TIMED_OUT || st == SOCK_HAS_BEEN_CLOSED)
                break;
            continue;
        }
        if (err == SSL_ERROR_WANT_READ)
            st = ssl_wait(sock, 0);
        else if (err == SSL_ERROR_WANT_WRITE)
            st = ssl_wait(sock, 1);
        else {
            ssl_sock_nbio(sock, 0);
            Py_DECREF(sock);
            return PySSL_SetError(self, ret);
        }
        if (st == SOCK_HAS_TIMED_OUT) {
            ssl_sock_nbio(sock, 0);
            Py_DECREF(sock);
            PyErr_SetString(PySSLErrorObject,
                            "The shutdown operation timed out");
            return NULL;
        }
        if (st == SOCK_IS_NONBLOCKING) {
            ssl_sock_nbio(sock, 0);
            Py_DECREF(sock);
            return PySSL_SetError(self, ret);
        }
    }
    ssl_sock_nbio(sock, 0);
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
ssl_set_proto_version(SSL_CTX *ctx, int proto)
{
    int minv;
    int maxv;

    minv = TLS1_VERSION;
    maxv = TLS1_3_VERSION;
    if (proto == PY_SSL_VERSION_TLS) {
        /* Negotiate any TLS. */
    } else if (proto == PY_SSL_VERSION_TLS1) {
        minv = maxv = TLS1_VERSION;
    } else if (proto == PY_SSL_VERSION_TLS1_1) {
        minv = maxv = TLS1_1_VERSION;
    } else if (proto == PY_SSL_VERSION_TLS1_2) {
        minv = maxv = TLS1_2_VERSION;
    } else {
        return -1;
    }
    if (SSL_CTX_set_min_proto_version(ctx, minv) != 1)
        return -1;
    if (SSL_CTX_set_max_proto_version(ctx, maxv) != 1)
        return -1;
    return 0;
}

static PyObject *
context_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    char *kwlist[2];
    PySSLContext *self;
    int proto_version;
    SSL_CTX *ctx;
    const SSL_METHOD *meth;

    kwlist[0] = "protocol";
    kwlist[1] = NULL;
    proto_version = PY_SSL_VERSION_TLS;
    ctx = NULL;
    meth = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "i:_SSLContext", kwlist,
                                     &proto_version))
        return NULL;
    /* pythonssl/OS4: TLS_client_method then SSL_CTX_new, never nested.
     * Do not SSL_CTX_set_options: AmiSSL takes uint64 in d0/d1 and a
     * 32-bit long leaves d1 garbage (SSL_OP_NO_TLSv* all set). Do not
     * SSL_CTX_set_min_proto_version either: that ctrl has cleared
     * ctx->method on 68k AmiSSL. */
    meth = TLS_client_method();
    if (meth == NULL)
        meth = TLS_method();
    if (meth == NULL)
        return _setSSLError(NULL, 0, __FILE__, __LINE__);
    ctx = SSL_CTX_new(meth);
    if (ctx == NULL)
        return _setSSLError(NULL, 0, __FILE__, __LINE__);
    if (proto_version != PY_SSL_VERSION_TLS) {
        if (ssl_set_proto_version(ctx, proto_version) < 0) {
            SSL_CTX_free(ctx);
            PyErr_SetString(PyExc_ValueError, "invalid protocol version");
            return NULL;
        }
    }
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
    self = (PySSLContext *)type->tp_alloc(type, 0);
    if (self == NULL) {
        SSL_CTX_free(ctx);
        return NULL;
    }
    self->ctx = ctx;
    self->alpn_protocols = NULL;
    self->alpn_protocols_len = 0;
    self->check_hostname = 0;
    self->pw_buf = NULL;
    self->pw_len = 0;
    return (PyObject *)self;
}

static void
context_dealloc(PySSLContext *self)
{
    if (amiga_amissl_ready && self->ctx != NULL)
        SSL_CTX_free(self->ctx);
    self->ctx = NULL;
    if (self->alpn_protocols != NULL)
        PyMem_Free(self->alpn_protocols);
    self->alpn_protocols = NULL;
    if (self->pw_buf != NULL)
        PyMem_Free(self->pw_buf);
    self->pw_buf = NULL;
    /* ssl.SSLContext is a heap subclass: host subtype_dealloc calls this
     * plugin tp_dealloc. Free via PyObject_Del -> host fn_object_dealloc
     * (correct A4 + GC head layout). Never _Py_AS_GC in the plugin. */
    PyObject_Del(self);
}

static PyObject *
set_ciphers(PySSLContext *self, PyObject *args)
{
    const char *cipherlist;

    (void)self;
    if (!PyArg_ParseTuple(args, "s:set_ciphers", &cipherlist))
        return NULL;
    /* AmiSSL 68k: SSL_CTX_set_cipher_list after SSL_CTX_new has left
     * ctx->method NULL (SSL_new: ssl ctx has no default ssl version).
     * Keep OpenSSL's default cipher list. */
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_set_npn_protocols(PySSLContext *self, PyObject *args)
{
    (void)self;
    (void)args;
    PyErr_SetString(PyExc_NotImplementedError,
                    "NPN is not available in AmiSSL/OpenSSL 3");
    return NULL;
}

static PyObject *
_set_alpn_protocols(PySSLContext *self, PyObject *args)
{
    Py_buffer protos;
    void *p;
    Py_ssize_t n;

    if (!PyArg_ParseTuple(args, "s*:set_alpn_protocols", &protos))
        return NULL;
    p = protos.buf;
    n = protos.len;
    if (self->alpn_protocols != NULL)
        PyMem_Free(self->alpn_protocols);
    self->alpn_protocols = (unsigned char *)PyMem_Malloc((size_t)n);
    if (self->alpn_protocols == NULL) {
        PyBuffer_Release(&protos);
        return ssl_nomem();
    }
    memcpy(self->alpn_protocols, p, (size_t)n);
    self->alpn_protocols_len = (unsigned int)n;
    PyBuffer_Release(&protos);
    if (SSL_CTX_set_alpn_protos(self->ctx, self->alpn_protocols,
                                self->alpn_protocols_len) != 0)
        return ssl_nomem();
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
get_verify_mode(PySSLContext *self, void *c)
{
    int mode;

    (void)c;
    mode = SSL_CTX_get_verify_mode(self->ctx);
    if (mode == SSL_VERIFY_NONE)
        return PyLong_FromLong(PY_SSL_CERT_NONE);
    if (mode == SSL_VERIFY_PEER)
        return PyLong_FromLong(PY_SSL_CERT_OPTIONAL);
    if (mode == (SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT))
        return PyLong_FromLong(PY_SSL_CERT_REQUIRED);
    PyErr_SetString(PySSLErrorObject,
                    "invalid return value from SSL_CTX_get_verify_mode");
    return NULL;
}

static int
set_verify_mode(PySSLContext *self, PyObject *arg, void *c)
{
    int n;
    int mode;

    (void)c;
    if (!PyArg_Parse(arg, "i", &n))
        return -1;
    if (n == PY_SSL_CERT_NONE)
        mode = SSL_VERIFY_NONE;
    else if (n == PY_SSL_CERT_OPTIONAL)
        mode = SSL_VERIFY_PEER;
    else if (n == PY_SSL_CERT_REQUIRED)
        mode = SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
    else {
        PyErr_SetString(PyExc_ValueError, "invalid value for verify_mode");
        return -1;
    }
    if (mode == SSL_VERIFY_NONE && self->check_hostname) {
        PyErr_SetString(PyExc_ValueError,
                        "Cannot set verify_mode to CERT_NONE when "
                        "check_hostname is enabled.");
        return -1;
    }
    SSL_CTX_set_verify(self->ctx, mode, NULL);
    return 0;
}

static PyObject *
get_verify_flags(PySSLContext *self, void *c)
{
    X509_STORE *store;
    X509_VERIFY_PARAM *param;
    unsigned long flags;

    (void)c;
    store = SSL_CTX_get_cert_store(self->ctx);
    param = X509_STORE_get0_param(store);
    flags = X509_VERIFY_PARAM_get_flags(param);
    return PyLong_FromUnsignedLong(flags);
}

static int
set_verify_flags(PySSLContext *self, PyObject *arg, void *c)
{
    X509_STORE *store;
    X509_VERIFY_PARAM *param;
    unsigned long new_flags;
    unsigned long flags;
    unsigned long set;
    unsigned long clear;

    (void)c;
    if (!PyArg_Parse(arg, "k", &new_flags))
        return -1;
    store = SSL_CTX_get_cert_store(self->ctx);
    param = X509_STORE_get0_param(store);
    flags = X509_VERIFY_PARAM_get_flags(param);
    clear = flags & ~new_flags;
    set = ~flags & new_flags;
    if (clear)
        X509_VERIFY_PARAM_clear_flags(param, clear);
    if (set)
        X509_VERIFY_PARAM_set_flags(param, set);
    return 0;
}

static PyObject *
get_options(PySSLContext *self, void *c)
{
    unsigned long low;

    (void)c;
    low = pyssl_op32(SSL_CTX_get_options(self->ctx));
    return PyLong_FromUnsignedLong(low);
}

static int
set_options(PySSLContext *self, PyObject *arg, void *c)
{
    unsigned long new_opts;
    unsigned long opts;
    unsigned long set;
    unsigned long clear;

    (void)c;
    if (!PyArg_Parse(arg, "k", &new_opts))
        return -1;
    opts = pyssl_op32(SSL_CTX_get_options(self->ctx));
    clear = opts & ~new_opts;
    set = ~opts & new_opts;
    if (clear)
        SSL_CTX_clear_options(self->ctx, pyssl_op64(clear));
    if (set)
        SSL_CTX_set_options(self->ctx, pyssl_op64(set));
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
    if (check_hostname &&
        SSL_CTX_get_verify_mode(self->ctx) == SSL_VERIFY_NONE) {
        PyErr_SetString(PyExc_ValueError,
                        "check_hostname needs a SSL context with either "
                        "CERT_OPTIONAL or CERT_REQUIRED");
        return -1;
    }
    self->check_hostname = check_hostname;
    return 0;
}

static int
passwd_cb(char *buf, int size, int rwflag, void *userdata)
{
    PySSLContext *self;
    int n;

    (void)rwflag;
    self = (PySSLContext *)userdata;
    if (self == NULL || self->pw_buf == NULL)
        return 0;
    n = self->pw_len;
    if (n > size)
        n = size;
    memcpy(buf, self->pw_buf, (size_t)n);
    return n;
}

static PyObject *
load_cert_chain(PySSLContext *self, PyObject *args, PyObject *kwds)
{
    char *kwlist[4];
    char *certfile;
    char *keyfile;
    PyObject *password;
    int r;

    kwlist[0] = "certfile";
    kwlist[1] = "keyfile";
    kwlist[2] = "password";
    kwlist[3] = NULL;
    certfile = NULL;
    keyfile = NULL;
    password = NULL;
    errno = 0;
    ERR_clear_error();
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|zO:load_cert_chain",
                                     kwlist, &certfile, &keyfile, &password))
        return NULL;
    if (password != NULL && password != Py_None) {
        char *pw;
        Py_ssize_t n;
        if (!PyString_Check(password)) {
            PyErr_SetString(PyExc_TypeError,
                            "password should be a string");
            return NULL;
        }
        pw = PyString_AsString(password);
        n = PyString_Size(password);
        if (self->pw_buf != NULL)
            PyMem_Free(self->pw_buf);
        self->pw_buf = (char *)PyMem_Malloc((size_t)n + 1);
        if (self->pw_buf == NULL)
            return ssl_nomem();
        memcpy(self->pw_buf, pw, (size_t)n);
        self->pw_buf[n] = '\0';
        self->pw_len = (int)n;
        SSL_CTX_set_default_passwd_cb(self->ctx, passwd_cb);
        SSL_CTX_set_default_passwd_cb_userdata(self->ctx, self);
    }
    r = SSL_CTX_use_certificate_chain_file(self->ctx, certfile);
    if (r != 1) {
        if (errno != 0) {
            ERR_clear_error();
            return PyErr_SetFromErrno(PyExc_IOError);
        }
        return _setSSLError(NULL, 0, __FILE__, __LINE__);
    }
    r = SSL_CTX_use_PrivateKey_file(self->ctx,
                                    keyfile ? keyfile : certfile,
                                    SSL_FILETYPE_PEM);
    if (r != 1) {
        if (errno != 0) {
            ERR_clear_error();
            return PyErr_SetFromErrno(PyExc_IOError);
        }
        return _setSSLError(NULL, 0, __FILE__, __LINE__);
    }
    r = SSL_CTX_check_private_key(self->ctx);
    if (r != 1)
        return _setSSLError(NULL, 0, __FILE__, __LINE__);
    Py_INCREF(Py_None);
    return Py_None;
}

static int
_add_ca_certs(PySSLContext *self, void *data, Py_ssize_t len, int filetype)
{
    BIO *biobuf;
    X509_STORE *store;
    X509 *cert;
    int loaded;
    int r;
    unsigned long err;

    if (len <= 0) {
        PyErr_SetString(PyExc_ValueError, "Empty certificate data");
        return -1;
    }
    biobuf = BIO_new_mem_buf(data, (int)len);
    if (biobuf == NULL) {
        _setSSLError("Can't allocate buffer", 0, __FILE__, __LINE__);
        return -1;
    }
    store = SSL_CTX_get_cert_store(self->ctx);
    loaded = 0;
    while (1) {
        cert = NULL;
        if (filetype == SSL_FILETYPE_ASN1)
            cert = d2i_X509_bio(biobuf, NULL);
        else
            cert = PEM_read_bio_X509(biobuf, NULL, NULL, NULL);
        if (cert == NULL)
            break;
        r = X509_STORE_add_cert(store, cert);
        X509_free(cert);
        if (!r) {
            err = ERR_peek_last_error();
            if (ERR_GET_LIB(err) == ERR_LIB_X509 &&
                ERR_GET_REASON(err) == X509_R_CERT_ALREADY_IN_HASH_TABLE)
                ERR_clear_error();
            else
                break;
        }
        loaded++;
    }
    err = ERR_peek_last_error();
    if (loaded > 0)
        ERR_clear_error();
    else {
        BIO_free(biobuf);
        _setSSLError(NULL, 0, __FILE__, __LINE__);
        return -1;
    }
    (void)err;
    BIO_free(biobuf);
    return 0;
}

static PyObject *
load_verify_locations(PySSLContext *self, PyObject *args, PyObject *kwds)
{
    char *kwlist[4];
    PyObject *cafile;
    PyObject *capath;
    PyObject *cadata;
    const char *cafile_buf;
    const char *capath_buf;
    int r;

    kwlist[0] = "cafile";
    kwlist[1] = "capath";
    kwlist[2] = "cadata";
    kwlist[3] = NULL;
    cafile = NULL;
    capath = NULL;
    cadata = NULL;
    cafile_buf = NULL;
    capath_buf = NULL;
    errno = 0;
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
        if (PyString_Check(cadata)) {
            buf = PyString_AsString(cadata);
            n = PyString_Size(cadata);
            if (_add_ca_certs(self, buf, n, SSL_FILETYPE_PEM) < 0)
                return NULL;
        } else {
            PyErr_SetString(PyExc_TypeError,
                            "cadata should be a string of PEM or ASN1");
            return NULL;
        }
    }
    if (cafile != NULL) {
        if (!PyString_Check(cafile)) {
            PyErr_SetString(PyExc_TypeError, "cafile should be a string");
            return NULL;
        }
        cafile_buf = PyString_AsString(cafile);
    }
    if (capath != NULL) {
        if (!PyString_Check(capath)) {
            PyErr_SetString(PyExc_TypeError, "capath should be a string");
            return NULL;
        }
        capath_buf = PyString_AsString(capath);
    }
    if (cafile_buf != NULL || capath_buf != NULL) {
        r = SSL_CTX_load_verify_locations(self->ctx, cafile_buf, capath_buf);
        if (r != 1) {
            if (errno != 0) {
                ERR_clear_error();
                return PyErr_SetFromErrno(PyExc_IOError);
            }
            return _setSSLError(NULL, 0, __FILE__, __LINE__);
        }
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
load_dh_params(PySSLContext *self, PyObject *filepath)
{
    BIO *bio;
    DH *dh;
    const char *path;

    if (!PyString_Check(filepath)) {
        PyErr_SetString(PyExc_TypeError, "path should be a string");
        return NULL;
    }
    path = PyString_AsString(filepath);
    bio = BIO_new_file(path, "r");
    if (bio == NULL)
        return PyErr_SetFromErrno(PyExc_IOError);
    dh = PEM_read_bio_DHparams(bio, NULL, NULL, NULL);
    BIO_free(bio);
    if (dh == NULL)
        return _setSSLError(NULL, 0, __FILE__, __LINE__);
    if (SSL_CTX_set_tmp_dh(self->ctx, dh) != 1) {
        DH_free(dh);
        return _setSSLError(NULL, 0, __FILE__, __LINE__);
    }
    DH_free(dh);
    Py_INCREF(Py_None);
    return Py_None;
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
    PyObject *stats;
    PyObject *value;
    int r;

    (void)unused;
    stats = PyDict_New();
    if (stats == NULL)
        return NULL;
#define ADD_STATS(fn, key) \
    do { \
        value = PyLong_FromLong(SSL_CTX_sess_##fn(self->ctx)); \
        if (value == NULL) { Py_DECREF(stats); return NULL; } \
        r = PyDict_SetItemString(stats, key, value); \
        Py_DECREF(value); \
        if (r < 0) { Py_DECREF(stats); return NULL; } \
    } while (0)
    ADD_STATS(number, "number");
    ADD_STATS(connect, "connect");
    ADD_STATS(connect_good, "connect_good");
    ADD_STATS(connect_renegotiate, "connect_renegotiate");
    ADD_STATS(accept, "accept");
    ADD_STATS(accept_good, "accept_good");
    ADD_STATS(accept_renegotiate, "accept_renegotiate");
    ADD_STATS(hits, "hits");
    ADD_STATS(misses, "misses");
    ADD_STATS(timeouts, "timeouts");
    ADD_STATS(cache_full, "cache_full");
#undef ADD_STATS
    return stats;
}

static PyObject *
set_default_verify_paths(PySSLContext *self, PyObject *unused)
{
    (void)unused;
    if (!SSL_CTX_set_default_verify_paths(self->ctx))
        return _setSSLError(NULL, 0, __FILE__, __LINE__);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
set_ecdh_curve(PySSLContext *self, PyObject *name)
{
    const char *s;

    if (!PyString_Check(name)) {
        PyErr_SetString(PyExc_TypeError, "curve name must be a string");
        return NULL;
    }
    s = PyString_AsString(name);
    if (SSL_CTX_set1_groups_list(self->ctx, s) != 1)
        return _setSSLError(NULL, 0, __FILE__, __LINE__);
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
asn1obj2py(ASN1_OBJECT *obj)
{
    int nid;
    const char *ln;
    const char *sn;
    char oid[80];
    int n;

    nid = OBJ_obj2nid(obj);
    if (nid == NID_undef) {
        PyErr_SetString(PyExc_ValueError, "Unknown object");
        return NULL;
    }
    sn = OBJ_nid2sn(nid);
    ln = OBJ_nid2ln(nid);
    n = OBJ_obj2txt(oid, (int)sizeof(oid), obj, 1);
    if (n < 0)
        oid[0] = '\0';
    return Py_BuildValue("isss", nid, sn ? sn : "", ln ? ln : "", oid);
}

static PyObject *
PySSL_txt2obj(PyObject *self, PyObject *args, PyObject *kwds)
{
    char *kwlist[3];
    char *txt;
    PyObject *pyname;
    int name;
    ASN1_OBJECT *obj;
    PyObject *result;

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
    obj = OBJ_txt2obj(txt, name ? 0 : 1);
    if (obj == NULL) {
        PyErr_Format(PyExc_ValueError, "unknown object '%.100s'", txt);
        return NULL;
    }
    result = asn1obj2py(obj);
    ASN1_OBJECT_free(obj);
    return result;
}

static PyObject *
PySSL_nid2obj(PyObject *self, PyObject *args)
{
    int nid;
    ASN1_OBJECT *obj;
    PyObject *result;

    (void)self;
    if (!PyArg_ParseTuple(args, "i:nid2obj", &nid))
        return NULL;
    if (nid < NID_undef) {
        PyErr_SetString(PyExc_ValueError, "NID must be positive.");
        return NULL;
    }
    obj = OBJ_nid2obj(nid);
    if (obj == NULL) {
        PyErr_Format(PyExc_ValueError, "unknown NID %i", nid);
        return NULL;
    }
    result = asn1obj2py(obj);
    return result;
}

static PyObject *
PySSL_RAND_status(PyObject *self)
{
    (void)self;
    return PyInt_FromLong(RAND_status());
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
    /* Ignore the double entropy estimate (soft-float in the plugin). */
    RAND_seed(buf, len);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
PySSL_get_default_verify_paths(PyObject *self)
{
    const char *a;
    const char *b;
    const char *c;
    const char *d;

    (void)self;
    a = X509_get_default_cert_file_env();
    b = X509_get_default_cert_file();
    c = X509_get_default_cert_dir_env();
    d = X509_get_default_cert_dir();
    return Py_BuildValue("ssss",
                         a ? a : "", b ? b : "",
                         c ? c : "", d ? d : "");
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
"Implementation module for SSL socket operations, AmiSSL 5 backend.");

void
init_ssl(void)
{
    PyObject *m;
    unsigned long libver;
    unsigned int major;
    unsigned int minor;
    unsigned int fix;
    unsigned int patch;
    unsigned int status;
    PyObject *oi;

    if (amiga_open_amissl() != 0)
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
#ifdef X509_V_FLAG_CRL_CHECK
    PyModule_AddIntConstant(m, "VERIFY_CRL_CHECK_LEAF", X509_V_FLAG_CRL_CHECK);
    PyModule_AddIntConstant(m, "VERIFY_CRL_CHECK_CHAIN",
                            X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL);
#endif
#ifdef X509_V_FLAG_X509_STRICT
    PyModule_AddIntConstant(m, "VERIFY_X509_STRICT", X509_V_FLAG_X509_STRICT);
#endif
#ifdef X509_V_FLAG_TRUSTED_FIRST
    PyModule_AddIntConstant(m, "VERIFY_X509_TRUSTED_FIRST",
                            X509_V_FLAG_TRUSTED_FIRST);
#endif
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

#ifdef SSL_AD_CLOSE_NOTIFY
    PyModule_AddIntConstant(m, "ALERT_DESCRIPTION_CLOSE_NOTIFY",
                            SSL_AD_CLOSE_NOTIFY);
#endif
#ifdef SSL_AD_UNEXPECTED_MESSAGE
    PyModule_AddIntConstant(m, "ALERT_DESCRIPTION_UNEXPECTED_MESSAGE",
                            SSL_AD_UNEXPECTED_MESSAGE);
#endif
#ifdef SSL_AD_BAD_RECORD_MAC
    PyModule_AddIntConstant(m, "ALERT_DESCRIPTION_BAD_RECORD_MAC",
                            SSL_AD_BAD_RECORD_MAC);
#endif
#ifdef SSL_AD_RECORD_OVERFLOW
    PyModule_AddIntConstant(m, "ALERT_DESCRIPTION_RECORD_OVERFLOW",
                            SSL_AD_RECORD_OVERFLOW);
#endif
#ifdef SSL_AD_DECOMPRESSION_FAILURE
    PyModule_AddIntConstant(m, "ALERT_DESCRIPTION_DECOMPRESSION_FAILURE",
                            SSL_AD_DECOMPRESSION_FAILURE);
#endif
#ifdef SSL_AD_HANDSHAKE_FAILURE
    PyModule_AddIntConstant(m, "ALERT_DESCRIPTION_HANDSHAKE_FAILURE",
                            SSL_AD_HANDSHAKE_FAILURE);
#endif
#ifdef SSL_AD_BAD_CERTIFICATE
    PyModule_AddIntConstant(m, "ALERT_DESCRIPTION_BAD_CERTIFICATE",
                            SSL_AD_BAD_CERTIFICATE);
#endif
#ifdef SSL_AD_UNSUPPORTED_CERTIFICATE
    PyModule_AddIntConstant(m, "ALERT_DESCRIPTION_UNSUPPORTED_CERTIFICATE",
                            SSL_AD_UNSUPPORTED_CERTIFICATE);
#endif
#ifdef SSL_AD_CERTIFICATE_REVOKED
    PyModule_AddIntConstant(m, "ALERT_DESCRIPTION_CERTIFICATE_REVOKED",
                            SSL_AD_CERTIFICATE_REVOKED);
#endif
#ifdef SSL_AD_CERTIFICATE_EXPIRED
    PyModule_AddIntConstant(m, "ALERT_DESCRIPTION_CERTIFICATE_EXPIRED",
                            SSL_AD_CERTIFICATE_EXPIRED);
#endif
#ifdef SSL_AD_CERTIFICATE_UNKNOWN
    PyModule_AddIntConstant(m, "ALERT_DESCRIPTION_CERTIFICATE_UNKNOWN",
                            SSL_AD_CERTIFICATE_UNKNOWN);
#endif
#ifdef SSL_AD_ILLEGAL_PARAMETER
    PyModule_AddIntConstant(m, "ALERT_DESCRIPTION_ILLEGAL_PARAMETER",
                            SSL_AD_ILLEGAL_PARAMETER);
#endif
#ifdef SSL_AD_UNKNOWN_CA
    PyModule_AddIntConstant(m, "ALERT_DESCRIPTION_UNKNOWN_CA",
                            SSL_AD_UNKNOWN_CA);
#endif
#ifdef SSL_AD_ACCESS_DENIED
    PyModule_AddIntConstant(m, "ALERT_DESCRIPTION_ACCESS_DENIED",
                            SSL_AD_ACCESS_DENIED);
#endif
#ifdef SSL_AD_DECODE_ERROR
    PyModule_AddIntConstant(m, "ALERT_DESCRIPTION_DECODE_ERROR",
                            SSL_AD_DECODE_ERROR);
#endif
#ifdef SSL_AD_DECRYPT_ERROR
    PyModule_AddIntConstant(m, "ALERT_DESCRIPTION_DECRYPT_ERROR",
                            SSL_AD_DECRYPT_ERROR);
#endif
#ifdef SSL_AD_PROTOCOL_VERSION
    PyModule_AddIntConstant(m, "ALERT_DESCRIPTION_PROTOCOL_VERSION",
                            SSL_AD_PROTOCOL_VERSION);
#endif
#ifdef SSL_AD_INSUFFICIENT_SECURITY
    PyModule_AddIntConstant(m, "ALERT_DESCRIPTION_INSUFFICIENT_SECURITY",
                            SSL_AD_INSUFFICIENT_SECURITY);
#endif
#ifdef SSL_AD_INTERNAL_ERROR
    PyModule_AddIntConstant(m, "ALERT_DESCRIPTION_INTERNAL_ERROR",
                            SSL_AD_INTERNAL_ERROR);
#endif
#ifdef SSL_AD_USER_CANCELLED
    PyModule_AddIntConstant(m, "ALERT_DESCRIPTION_USER_CANCELLED",
                            SSL_AD_USER_CANCELLED);
#endif
#ifdef SSL_AD_NO_RENEGOTIATION
    PyModule_AddIntConstant(m, "ALERT_DESCRIPTION_NO_RENEGOTIATION",
                            SSL_AD_NO_RENEGOTIATION);
#endif
#ifdef SSL_AD_UNSUPPORTED_EXTENSION
    PyModule_AddIntConstant(m, "ALERT_DESCRIPTION_UNSUPPORTED_EXTENSION",
                            SSL_AD_UNSUPPORTED_EXTENSION);
#endif
#ifdef SSL_AD_CERTIFICATE_UNOBTAINABLE
    PyModule_AddIntConstant(m, "ALERT_DESCRIPTION_CERTIFICATE_UNOBTAINABLE",
                            SSL_AD_CERTIFICATE_UNOBTAINABLE);
#endif
#ifdef SSL_AD_UNRECOGNIZED_NAME
    PyModule_AddIntConstant(m, "ALERT_DESCRIPTION_UNRECOGNIZED_NAME",
                            SSL_AD_UNRECOGNIZED_NAME);
#endif
#ifdef SSL_AD_BAD_CERTIFICATE_STATUS_RESPONSE
    PyModule_AddIntConstant(m, "ALERT_DESCRIPTION_BAD_CERTIFICATE_STATUS_RESPONSE",
                            SSL_AD_BAD_CERTIFICATE_STATUS_RESPONSE);
#endif
#ifdef SSL_AD_BAD_CERTIFICATE_HASH_VALUE
    PyModule_AddIntConstant(m, "ALERT_DESCRIPTION_BAD_CERTIFICATE_HASH_VALUE",
                            SSL_AD_BAD_CERTIFICATE_HASH_VALUE);
#endif
#ifdef SSL_AD_UNKNOWN_PSK_IDENTITY
    PyModule_AddIntConstant(m, "ALERT_DESCRIPTION_UNKNOWN_PSK_IDENTITY",
                            SSL_AD_UNKNOWN_PSK_IDENTITY);
#endif

    add_flag(m, "HAS_SNI", 1);
    add_flag(m, "HAS_ECDH", 1);
    add_flag(m, "HAS_NPN", 0);
    add_flag(m, "HAS_ALPN", 1);
    add_flag(m, "HAS_TLSv1_3", 1);
    add_flag(m, "HAS_TLS_UNIQUE", 0);

    libver = OPENSSL_VERSION_NUMBER;
    major = (unsigned int)((libver >> 28) & 0xFF);
    minor = (unsigned int)((libver >> 20) & 0xFF);
    fix = (unsigned int)((libver >> 12) & 0xFF);
    patch = (unsigned int)((libver >> 4) & 0xFF);
    status = (unsigned int)(libver & 0xF);
    PyModule_AddIntConstant(m, "OPENSSL_VERSION_NUMBER", (long)libver);
    oi = Py_BuildValue("iiiii", (int)major, (int)minor, (int)fix,
                       (int)patch, (int)status);
    if (oi != NULL)
        PyModule_AddObject(m, "OPENSSL_VERSION_INFO", oi);
    PyModule_AddStringConstant(m, "OPENSSL_VERSION", OPENSSL_VERSION_TEXT);
    if (oi != NULL) {
        Py_INCREF(oi);
        PyModule_AddObject(m, "_OPENSSL_API_VERSION", oi);
    }
    PyModule_AddIntConstant(m, "amiga_plugin_rev", 10);
}
#endif /* !PYAMIGA_USE_AMITLS */
