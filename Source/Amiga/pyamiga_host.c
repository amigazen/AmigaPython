/*
 * Fill the process-global PyHost for Amiga LoadSeg plugins (ABI v8).
 * Socket syscalls are a posix fd table over AmiTCP LVOs (unix.lib2
 * psockets: POSIX socket() in _socket, bsdsocket underneath). sock_fd is
 * never 0/1/2 so it cannot alias stdin/stdout. AmiTLS uses
 * fn_socket_native_fd() -> the bsdsocket id. Do not call PosixLib
 * socket(): those fds made AmiTLS TlsRead return 8808/EWOULDBLOCK.
 *
 * VBCC cannot use proto/bsdsocket.h (A6=base clobbers the frame pointer).
 * D7-safe LVO stubs are the VBCC equivalent of SAS/C proto/bsdsocket.h.
 *
 * PosixLib defines socket/bind/... as function-like macros to __P*;
 * those cannot be used as function pointers. Use AmiTCP LVO stubs instead.
 */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/filio.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dosextens.h>

#ifndef EWOULDBLOCK
#ifdef EAGAIN
#define EWOULDBLOCK EAGAIN
#else
#define EWOULDBLOCK 35
#endif
#endif
#ifndef EAGAIN
#define EAGAIN EWOULDBLOCK
#endif
#ifndef EINTR
#define EINTR 4
#endif
#ifndef ENOTSOCK
#define ENOTSOCK 38
#endif
#ifndef EMFILE
#define EMFILE 24
#endif
#ifndef EBADF
#define EBADF 9
#endif
#undef FIONBIO
#define FIONBIO 0x8004667eUL
#define PYSOCK_MAX 64
#define PYSOCK_BASE 3
#define PYAMIGA_HOST_BUILD
#include "pyamiga_plugin.h"
#include "timefuncs.h"

extern int __init_bsdsocket(int);
extern struct Library *SocketBase;
extern int h_errno;
extern struct ExecBase *SysBase;
extern struct DosLibrary *DOSBase;

static struct PyHost pyamiga_host;

/*
 * Host is +aos68k_posix (no -sd). VBCC __saveds cannot reload the CRT A4
 * PosixLib uses for SocketBase. Capture host A4 in InitHost and
 * load it on every trampoline entry.
 *
 * Do NOT restore the caller's A4 on exit: the plugin is FARONLY (no near
 * data) but still links -lmieee soft-float, which needs the host A4. Leaving
 * the plugin's A4 in place after socket() hung in init_sockobject's double
 * assignment (defaulttimeout).
 */
static APTR pyamiga_host_a4;

/* VBCC register stubs (same style as Amiga/syslog.c LVO stubs).
 * Do not append rts — vbcc already emits one after the string body. */
static APTR pyamiga_get_a4(void)
    = "\tmove.l\ta4,d0";

static void pyamiga_set_a4(__reg("d0") APTR v)
    = "\tmove.l\td0,a4";

/*
 * AmiTCP LVOs (netinclude/inline/bsdsocket_protos.h).
 *
 * Do not pass SocketBase in A6 from C: VBCC uses A6 as the frame pointer.
 * send/recv/WaitSelect live in functions with locals (fd_set, buffers);
 * loading A6=SocketBase for the stub then made errno 38 (ENOTSOCK) and
 * CloseSocket deadlocked. Pass the base in D7; the stub saves A6.
 */
#define AMITCP_LVO(nnn) \
    "\tmove.l\ta6,-(sp)\n" \
    "\tmove.l\td7,a6\n" \
    "\tjsr\t-" #nnn "(a6)\n" \
    "\tmove.l\t(sp)+,a6"

static LONG amitcp_lvo_socket(__reg("d7") void *base,
    __reg("d0") LONG domain, __reg("d1") LONG type, __reg("d2") LONG protocol)
    = AMITCP_LVO(30);
static LONG amitcp_lvo_bind(__reg("d7") void *base,
    __reg("d0") LONG sock, __reg("a0") APTR name, __reg("d1") LONG namelen)
    = AMITCP_LVO(36);
static LONG amitcp_lvo_listen(__reg("d7") void *base,
    __reg("d0") LONG sock, __reg("d1") LONG backlog)
    = AMITCP_LVO(42);
static LONG amitcp_lvo_accept(__reg("d7") void *base,
    __reg("d0") LONG sock, __reg("a0") APTR addr, __reg("a1") APTR addrlen)
    = AMITCP_LVO(48);
static LONG amitcp_lvo_connect(__reg("d7") void *base,
    __reg("d0") LONG sock, __reg("a0") APTR name, __reg("d1") LONG namelen)
    = AMITCP_LVO(54);
static LONG amitcp_lvo_sendto(__reg("d7") void *base,
    __reg("d0") LONG sock, __reg("a0") APTR buf, __reg("d1") LONG len,
    __reg("d2") LONG flags, __reg("a1") APTR to, __reg("d3") LONG tolen)
    = AMITCP_LVO(60);
static LONG amitcp_lvo_send(__reg("d7") void *base,
    __reg("d0") LONG sock, __reg("a0") APTR buf, __reg("d1") LONG len,
    __reg("d2") LONG flags)
    = AMITCP_LVO(66);
static LONG amitcp_lvo_recvfrom(__reg("d7") void *base,
    __reg("d0") LONG sock, __reg("a0") APTR buf, __reg("d1") LONG len,
    __reg("d2") LONG flags, __reg("a1") APTR addr, __reg("a2") APTR addrlen)
    = AMITCP_LVO(72);
static LONG amitcp_lvo_recv(__reg("d7") void *base,
    __reg("d0") LONG sock, __reg("a0") APTR buf, __reg("d1") LONG len,
    __reg("d2") LONG flags)
    = AMITCP_LVO(78);
static LONG amitcp_lvo_shutdown(__reg("d7") void *base,
    __reg("d0") LONG sock, __reg("d1") LONG how)
    = AMITCP_LVO(84);
static LONG amitcp_lvo_setsockopt(__reg("d7") void *base,
    __reg("d0") LONG sock, __reg("d1") LONG level, __reg("d2") LONG optname,
    __reg("a0") APTR optval, __reg("d3") LONG optlen)
    = AMITCP_LVO(90);
static LONG amitcp_lvo_getsockopt(__reg("d7") void *base,
    __reg("d0") LONG sock, __reg("d1") LONG level, __reg("d2") LONG optname,
    __reg("a0") APTR optval, __reg("a1") APTR optlen)
    = AMITCP_LVO(96);
static LONG amitcp_lvo_getsockname(__reg("d7") void *base,
    __reg("d0") LONG sock, __reg("a0") APTR name, __reg("a1") APTR namelen)
    = AMITCP_LVO(102);
static LONG amitcp_lvo_getpeername(__reg("d7") void *base,
    __reg("d0") LONG sock, __reg("a0") APTR name, __reg("a1") APTR namelen)
    = AMITCP_LVO(108);
static LONG amitcp_lvo_IoctlSocket(__reg("d7") void *base,
    __reg("d0") LONG sock, __reg("d1") ULONG req, __reg("a0") APTR argp)
    = AMITCP_LVO(114);
static LONG amitcp_lvo_CloseSocket(__reg("d7") void *base,
    __reg("d0") LONG sock)
    = AMITCP_LVO(120);
static LONG amitcp_lvo_WaitSelect(__reg("d7") void *base,
    __reg("d0") LONG nfds, __reg("a0") APTR r, __reg("a1") APTR w,
    __reg("a2") APTR e, __reg("a3") APTR tv, __reg("d1") ULONG *sigs)
    = AMITCP_LVO(126);

static int pyamiga_ensure_bsdsocket(void);

static LONG pysock_native[PYSOCK_MAX];
static int pysock_ready;

static void
pysock_init(void)
{
    int i;

    if (pysock_ready)
        return;
    for (i = 0; i < PYSOCK_MAX; i++)
        pysock_native[i] = -1L;
    pysock_ready = 1;
}

static int
pysock_alloc(LONG nfd)
{
    int i;

    pysock_init();
    if (nfd < 0) {
        errno = EBADF;
        return -1;
    }
    for (i = 0; i < PYSOCK_MAX; i++) {
        if (pysock_native[i] < 0) {
            pysock_native[i] = nfd;
            return i + PYSOCK_BASE;
        }
    }
    errno = EMFILE;
    return -1;
}

static LONG
pysock_to_native(int fd)
{
    int i;

    pysock_init();
    i = fd - PYSOCK_BASE;
    if (i < 0 || i >= PYSOCK_MAX)
        return -1L;
    return pysock_native[i];
}

static LONG
pysock_take(int fd)
{
    int i;
    LONG nfd;

    pysock_init();
    i = fd - PYSOCK_BASE;
    if (i < 0 || i >= PYSOCK_MAX)
        return -1L;
    nfd = pysock_native[i];
    pysock_native[i] = -1L;
    return nfd;
}

static LONG
pysock_require(int fd)
{
    LONG nfd;

    nfd = pysock_to_native(fd);
    if (nfd < 0) {
        errno = ENOTSOCK;
        return -1L;
    }
    return nfd;
}

static void
pysock_force_blocking(LONG nfd)
{
    ULONG nb;

    nb = 0UL;
    (void)amitcp_lvo_IoctlSocket(SocketBase, nfd, (ULONG)FIONBIO, (APTR)&nb);
}

static void
pysock_fdset_to_native(fd_set *dst, fd_set *src, int nfds, int *maxn)
{
    int fd;
    LONG nfd;

    FD_ZERO(dst);
    if (src == NULL)
        return;
    for (fd = 0; fd < nfds && fd < FD_SETSIZE; fd++) {
        if (!FD_ISSET(fd, src))
            continue;
        nfd = pysock_to_native(fd);
        if (nfd < 0 || nfd >= FD_SETSIZE)
            continue;
        FD_SET((int)nfd, dst);
        if ((int)nfd + 1 > *maxn)
            *maxn = (int)nfd + 1;
    }
}

static void
pysock_fdset_from_native(fd_set *dst, fd_set *src, int nfds)
{
    int fd;
    LONG nfd;

    if (dst == NULL)
        return;
    FD_ZERO(dst);
    if (src == NULL)
        return;
    for (fd = 0; fd < nfds && fd < FD_SETSIZE; fd++) {
        nfd = pysock_to_native(fd);
        if (nfd < 0 || nfd >= FD_SETSIZE)
            continue;
        if (FD_ISSET((int)nfd, src))
            FD_SET(fd, dst);
    }
}

static int
pyamiga_amitcp_ok(void)
{
    if (pyamiga_ensure_bsdsocket() != 0)
        return 0;
    if (SocketBase == NULL)
        return 0;
    return 1;
}

static int
pyamiga_ensure_bsdsocket(void)
{
    struct Process *pr;
    APTR oldwin;
    int rc;

    pyamiga_set_a4(pyamiga_host_a4);

    if (SocketBase != NULL) {
        pyamiga_host.ptr_SocketBase = (void *)SocketBase;
        return 0;
    }

    /*
     * PosixLib OpenLibrary("bsdsocket.library") can put up an
     * "insert volume AmiTCP:" (or similar) requester when the stack
     * assign is dangling. That looks like a full system deadlock.
     * pr_WindowPtr = -1 suppresses DOS requesters for this call.
     */
    pr = (struct Process *)FindTask(NULL);
    oldwin = pr->pr_WindowPtr;
    pr->pr_WindowPtr = (APTR)-1L;
    rc = __init_bsdsocket(-1);
    pr->pr_WindowPtr = oldwin;

    if (rc != 0 || SocketBase == NULL) {
        PyErr_SetString(PyExc_ImportError,
                        "bsdsocket.library required for _socket "
                        "(no TCP/IP stack)");
        return -1;
    }
    pyamiga_host.ptr_SocketBase = (void *)SocketBase;
    return 0;
}

/* Py_InitModule3 is a macro — need a real function pointer for plugins. */
static PyObject *
pyamiga_InitModule3(char *name, PyMethodDef *methods, char *doc)
{
    PyObject *res;

    pyamiga_set_a4(pyamiga_host_a4);
    res = Py_InitModule3(name, methods, doc);
    return res;
}

static int
pyamiga_ioctl(int fd, unsigned long request, char *arg)
{
    LONG rc;
    LONG nfd;

    pyamiga_set_a4(pyamiga_host_a4);
    if (!pyamiga_amitcp_ok())
        return -1;
    nfd = pysock_require(fd);
    if (nfd < 0)
        return -1;
    rc = amitcp_lvo_IoctlSocket(SocketBase, nfd, (ULONG)request, (APTR)arg);
    return (int)rc;
}

static int
pyamiga_fcntl(int fd, int cmd, int arg)
{
    int rc;

    pyamiga_set_a4(pyamiga_host_a4);
    rc = fcntl(fd, cmd, arg);
    return rc;
}

/*
 * unix.lib2 sockets.c / ATlsTest: AmiTCP LVO, then a posix fd slot so
 * Python fileno() is never stdin/stdout. AmiTLS gets the native id.
 */
static int
pyamiga_socket(int domain, int type, int protocol)
{
    LONG rc;
    LONG fd2;
    int pfd;

    pyamiga_set_a4(pyamiga_host_a4);
    if (!pyamiga_amitcp_ok())
        return -1;
    rc = amitcp_lvo_socket(SocketBase, (LONG)domain, (LONG)type,
                           (LONG)protocol);
    /* ATlsTest: AmiTCP may return 0; do not keep stdin-looking native ids. */
    if (rc == 0) {
        fd2 = amitcp_lvo_socket(SocketBase, (LONG)domain, (LONG)type,
                                (LONG)protocol);
        if (fd2 >= 0) {
            (void)amitcp_lvo_CloseSocket(SocketBase, rc);
            rc = fd2;
        }
    }
    if (rc < 0)
        return -1;
    /* ATlsTest: AmiTCP recv()s this id. BearSSL low_read treats
     * EWOULDBLOCK as fatal 8808; keep the native fd blocking. */
    pysock_force_blocking(rc);
    pfd = pysock_alloc(rc);
    if (pfd < 0) {
        (void)amitcp_lvo_CloseSocket(SocketBase, rc);
        return -1;
    }
    return pfd;
}

static int
pyamiga_bind(int s, const struct sockaddr *addr, socklen_t len)
{
    LONG rc;
    LONG nfd;

    pyamiga_set_a4(pyamiga_host_a4);
    if (!pyamiga_amitcp_ok())
        return -1;
    nfd = pysock_require(s);
    if (nfd < 0)
        return -1;
    rc = amitcp_lvo_bind(SocketBase, nfd, (APTR)addr, (LONG)len);
    return (int)rc;
}

static int
pyamiga_listen(int s, int backlog)
{
    LONG rc;
    LONG nfd;

    pyamiga_set_a4(pyamiga_host_a4);
    if (!pyamiga_amitcp_ok())
        return -1;
    nfd = pysock_require(s);
    if (nfd < 0)
        return -1;
    rc = amitcp_lvo_listen(SocketBase, nfd, (LONG)backlog);
    return (int)rc;
}

static int
pyamiga_accept(int s, struct sockaddr *addr, socklen_t *len)
{
    LONG rc;
    LONG nfd;
    int pfd;

    pyamiga_set_a4(pyamiga_host_a4);
    if (!pyamiga_amitcp_ok())
        return -1;
    nfd = pysock_require(s);
    if (nfd < 0)
        return -1;
    rc = amitcp_lvo_accept(SocketBase, nfd, (APTR)addr, (APTR)len);
    if (rc < 0)
        return -1;
    pysock_force_blocking(rc);
    pfd = pysock_alloc(rc);
    if (pfd < 0) {
        (void)amitcp_lvo_CloseSocket(SocketBase, rc);
        return -1;
    }
    return pfd;
}

static int
pyamiga_connect(int s, const struct sockaddr *addr, socklen_t len)
{
    LONG rc;
    LONG nfd;

    pyamiga_set_a4(pyamiga_host_a4);
    if (!pyamiga_amitcp_ok())
        return -1;
    nfd = pysock_require(s);
    if (nfd < 0)
        return -1;
    rc = amitcp_lvo_connect(SocketBase, nfd, (APTR)addr, (LONG)len);
    return (int)rc;
}

static int
pyamiga_shutdown(int s, int how)
{
    LONG rc;
    LONG nfd;

    pyamiga_set_a4(pyamiga_host_a4);
    if (!pyamiga_amitcp_ok())
        return -1;
    nfd = pysock_require(s);
    if (nfd < 0)
        return -1;
    rc = amitcp_lvo_shutdown(SocketBase, nfd, (LONG)how);
    return (int)rc;
}

static int
pyamiga_close(int fd)
{
    int rc;

    pyamiga_set_a4(pyamiga_host_a4);
    rc = close(fd);
    return rc;
}

static int
pyamiga_closesocket(int s)
{
    LONG rc;
    LONG nfd;

    pyamiga_set_a4(pyamiga_host_a4);
    if (!pyamiga_amitcp_ok())
        return -1;
    nfd = pysock_take(s);
    if (nfd < 0) {
        errno = ENOTSOCK;
        return -1;
    }
    rc = amitcp_lvo_CloseSocket(SocketBase, nfd);
    return (int)rc;
}

static int
pyamiga_select(int nfds, fd_set *rd, fd_set *wr, fd_set *ex,
               struct timeval *tv)
{
    fd_set nrd;
    fd_set nwr;
    fd_set nex;
    APTR prd;
    APTR pwr;
    APTR pex;
    int maxn;
    LONG rc;
    ULONG ticks;

    pyamiga_set_a4(pyamiga_host_a4);
    if (!pyamiga_amitcp_ok())
        return -1;
    maxn = 0;
    prd = NULL;
    pwr = NULL;
    pex = NULL;
    if (rd != NULL) {
        pysock_fdset_to_native(&nrd, rd, nfds, &maxn);
        prd = (APTR)&nrd;
    }
    if (wr != NULL) {
        pysock_fdset_to_native(&nwr, wr, nfds, &maxn);
        pwr = (APTR)&nwr;
    }
    if (ex != NULL) {
        pysock_fdset_to_native(&nex, ex, nfds, &maxn);
        pex = (APTR)&nex;
    }
    /* AmiTCP WaitSelect(0, ...) ignores the timeout and hangs. PosixLib
     * select() on empty sets Delay()s instead of calling the stack. */
    if (maxn <= 0) {
        if (rd != NULL)
            FD_ZERO(rd);
        if (wr != NULL)
            FD_ZERO(wr);
        if (ex != NULL)
            FD_ZERO(ex);
        if (tv != NULL && tv->tv_sec > 0) {
            ticks = (ULONG)tv->tv_sec * TICKS_PER_SECOND;
            if (ticks > 0UL)
                Delay(ticks);
        }
        return 0;
    }
    rc = amitcp_lvo_WaitSelect(SocketBase, (LONG)maxn, prd, pwr, pex,
                               (APTR)tv, NULL);
    if (rc >= 0) {
        if (rd != NULL)
            pysock_fdset_from_native(rd, &nrd, nfds);
        if (wr != NULL)
            pysock_fdset_from_native(wr, &nwr, nfds);
        if (ex != NULL)
            pysock_fdset_from_native(ex, &nex, nfds);
    }
    return (int)rc;
}

/* Builtin select module: AmiTCP ids, not PosixLib __fdesc (stdout is fd 1). */
int
pyamiga_host_select(int nfds, fd_set *rd, fd_set *wr, fd_set *ex,
                    struct timeval *tv)
{
    return pyamiga_select(nfds, rd, wr, ex, tv);
}

static int
pyamiga_recv(int s, void *buf, size_t len, int flags)
{
    LONG rc;
    LONG nfd;

    pyamiga_set_a4(pyamiga_host_a4);
    if (!pyamiga_amitcp_ok())
        return -1;
    nfd = pysock_require(s);
    if (nfd < 0)
        return -1;
    rc = amitcp_lvo_recv(SocketBase, nfd, (APTR)buf, (LONG)len, (LONG)flags);
    return (int)rc;
}

static int
pyamiga_send(int s, const void *buf, size_t len, int flags)
{
    LONG rc;
    LONG nfd;

    pyamiga_set_a4(pyamiga_host_a4);
    if (!pyamiga_amitcp_ok())
        return -1;
    nfd = pysock_require(s);
    if (nfd < 0)
        return -1;
    rc = amitcp_lvo_send(SocketBase, nfd, (APTR)buf, (LONG)len, (LONG)flags);
    return (int)rc;
}

static int
pyamiga_recvfrom(int s, void *buf, size_t len, int flags,
                 struct sockaddr *addr, socklen_t *alen)
{
    LONG rc;
    LONG nfd;

    pyamiga_set_a4(pyamiga_host_a4);
    if (!pyamiga_amitcp_ok())
        return -1;
    nfd = pysock_require(s);
    if (nfd < 0)
        return -1;
    rc = amitcp_lvo_recvfrom(SocketBase, nfd, (APTR)buf, (LONG)len,
                             (LONG)flags, (APTR)addr, (APTR)alen);
    return (int)rc;
}

static int
pyamiga_sendto(int s, const void *buf, size_t len, int flags,
               const struct sockaddr *addr, socklen_t alen)
{
    LONG rc;
    LONG nfd;

    pyamiga_set_a4(pyamiga_host_a4);
    if (!pyamiga_amitcp_ok())
        return -1;
    nfd = pysock_require(s);
    if (nfd < 0)
        return -1;
    rc = amitcp_lvo_sendto(SocketBase, nfd, (APTR)buf, (LONG)len,
                           (LONG)flags, (APTR)addr, (LONG)alen);
    return (int)rc;
}

static int
pyamiga_setsockopt(int s, int level, int optname, const void *optval,
                   socklen_t optlen)
{
    LONG rc;
    LONG nfd;

    pyamiga_set_a4(pyamiga_host_a4);
    if (!pyamiga_amitcp_ok())
        return -1;
    nfd = pysock_require(s);
    if (nfd < 0)
        return -1;
    rc = amitcp_lvo_setsockopt(SocketBase, nfd, (LONG)level, (LONG)optname,
                               (APTR)optval, (LONG)optlen);
    return (int)rc;
}

static int
pyamiga_getsockopt(int s, int level, int optname, void *optval,
                   socklen_t *optlen)
{
    LONG rc;
    LONG nfd;

    pyamiga_set_a4(pyamiga_host_a4);
    if (!pyamiga_amitcp_ok())
        return -1;
    nfd = pysock_require(s);
    if (nfd < 0)
        return -1;
    rc = amitcp_lvo_getsockopt(SocketBase, nfd, (LONG)level, (LONG)optname,
                               (APTR)optval, (APTR)optlen);
    return (int)rc;
}

static int
pyamiga_getsockname(int s, struct sockaddr *addr, socklen_t *len)
{
    LONG rc;
    LONG nfd;

    pyamiga_set_a4(pyamiga_host_a4);
    if (!pyamiga_amitcp_ok())
        return -1;
    nfd = pysock_require(s);
    if (nfd < 0)
        return -1;
    rc = amitcp_lvo_getsockname(SocketBase, nfd, (APTR)addr, (APTR)len);
    return (int)rc;
}

static int
pyamiga_getpeername(int s, struct sockaddr *addr, socklen_t *len)
{
    LONG rc;
    LONG nfd;

    pyamiga_set_a4(pyamiga_host_a4);
    if (!pyamiga_amitcp_ok())
        return -1;
    nfd = pysock_require(s);
    if (nfd < 0)
        return -1;
    rc = amitcp_lvo_getpeername(SocketBase, nfd, (APTR)addr, (APTR)len);
    return (int)rc;
}

static int
pyamiga_gethostname(char *name, size_t len)
{
    int rc;

    pyamiga_set_a4(pyamiga_host_a4);
    rc = gethostname(name, len);
    return rc;
}

static struct hostent *
pyamiga_gethostbyname(const char *name)
{
    struct hostent *res;

    pyamiga_set_a4(pyamiga_host_a4);
    res = gethostbyname(name);
    return res;
}

static struct hostent *
pyamiga_gethostbyaddr(const void *addr, socklen_t len, int type)
{
    struct hostent *res;

    pyamiga_set_a4(pyamiga_host_a4);
    res = gethostbyaddr(addr, len, type);
    return res;
}

static struct servent *
pyamiga_getservbyname(const char *name, const char *proto)
{
    struct servent *res;

    pyamiga_set_a4(pyamiga_host_a4);
    res = getservbyname(name, proto);
    return res;
}

static struct servent *
pyamiga_getservbyport(int port, const char *proto)
{
    struct servent *res;

    pyamiga_set_a4(pyamiga_host_a4);
    res = getservbyport(port, proto);
    return res;
}

static struct protoent *
pyamiga_getprotobyname(const char *name)
{
    struct protoent *res;

    pyamiga_set_a4(pyamiga_host_a4);
    res = getprotobyname(name);
    return res;
}

static unsigned long
pyamiga_inet_addr(const char *cp)
{
    unsigned long res;

    pyamiga_set_a4(pyamiga_host_a4);
    res = inet_addr(cp);
    return res;
}

static char *
pyamiga_inet_ntoa(struct in_addr in)
{
    char *res;

    pyamiga_set_a4(pyamiga_host_a4);
    res = inet_ntoa(in);
    return res;
}

static unsigned short
pyamiga_htons(unsigned short hostshort)
{
    unsigned short res;

    pyamiga_set_a4(pyamiga_host_a4);
    res = htons(hostshort);
    return res;
}

static unsigned long
pyamiga_htonl(unsigned long hostlong)
{
    unsigned long res;

    pyamiga_set_a4(pyamiga_host_a4);
    res = htonl(hostlong);
    return res;
}

static unsigned short
pyamiga_ntohs(unsigned short netshort)
{
    unsigned short res;

    pyamiga_set_a4(pyamiga_host_a4);
    res = ntohs(netshort);
    return res;
}

static unsigned long
pyamiga_ntohl(unsigned long netlong)
{
    unsigned long res;

    pyamiga_set_a4(pyamiga_host_a4);
    res = ntohl(netlong);
    return res;
}

static int
pyamiga_dup(int fd)
{
    int rc;

    pyamiga_set_a4(pyamiga_host_a4);
    rc = dup(fd);
    return rc;
}

static FILE *
pyamiga_fdopen(int fd, const char *mode)
{
    FILE *res;

    pyamiga_set_a4(pyamiga_host_a4);
    res = fdopen(fd, mode);
    return res;
}

static int
pyamiga_fclose(FILE *fp)
{
    int rc;

    pyamiga_set_a4(pyamiga_host_a4);
    rc = fclose(fp);
    return rc;
}

static char *
pyamiga_strerror(int errnum)
{
    char *res;

    pyamiga_set_a4(pyamiga_host_a4);
    res = strerror(errnum);
    return res;
}

static void
pyamiga_note(const char *msg)
{
    pyamiga_set_a4(pyamiga_host_a4);
    printf("  NOTE: %s\n", msg ? msg : "(null)");
    fflush(stdout);
}

static int
pyamiga_sock_timeout_from_arg(PyObject *arg, void *bits8, int *block)
{
    double timeout;

    pyamiga_set_a4(pyamiga_host_a4);
    if (arg == Py_None)
        timeout = -1.0;
    else {
        timeout = PyFloat_AsDouble(arg);
        if (timeout < 0.0) {
            if (!PyErr_Occurred())
                PyErr_SetString(PyExc_ValueError,
                                "Timeout value out of range");
            return -1;
        }
        if (PyErr_Occurred())
            return -1;
    }
    memcpy(bits8, &timeout, sizeof(double));
    if (block != NULL)
        *block = (timeout < 0.0);
    return 0;
}

static PyObject *
pyamiga_sock_timeout_to_obj(const void *bits8)
{
    double timeout;

    pyamiga_set_a4(pyamiga_host_a4);
    memcpy(&timeout, bits8, sizeof(double));
    if (timeout < 0.0) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return PyFloat_FromDouble(timeout);
}

static int
pyamiga_sock_timeout_cmp0(const void *bits8)
{
    double timeout;

    pyamiga_set_a4(pyamiga_host_a4);
    memcpy(&timeout, bits8, sizeof(double));
    if (timeout < 0.0)
        return -1;
    if (timeout > 0.0)
        return 1;
    return 0;
}

static void
pyamiga_sock_timeout_to_tv(const void *bits8, struct timeval *tv)
{
    double timeout;

    pyamiga_set_a4(pyamiga_host_a4);
    memcpy(&timeout, bits8, sizeof(double));
    tv->tv_sec = (long)timeout;
    tv->tv_usec = (long)((timeout - (double)tv->tv_sec) * 1.0e6);
}

static void
pyamiga_sock_deadline_init(const void *timeout_bits, void *deadline_bits)
{
    double timeout;
    double deadline;

    pyamiga_set_a4(pyamiga_host_a4);
    memcpy(&timeout, timeout_bits, sizeof(double));
    deadline = _PyTime_FloatTime() + timeout;
    memcpy(deadline_bits, &deadline, sizeof(double));
}

static int
pyamiga_sock_deadline_remaining(const void *deadline_bits, void *interval_bits)
{
    double deadline;
    double interval;

    pyamiga_set_a4(pyamiga_host_a4);
    memcpy(&deadline, deadline_bits, sizeof(double));
    interval = deadline - _PyTime_FloatTime();
    memcpy(interval_bits, &interval, sizeof(double));
    return (interval < 0.0);
}

/*
 * Build fd_set on the host. Plugin-side FD_SETSIZE can disagree with
 * the host; WaitSelect then corrupts the plugin stack.
 */
static int
pyamiga_sock_select1(int fd, int writing, const void *timeout_bits)
{
    fd_set fds;
    struct timeval tv;
    double timeout;
    int n;
    LONG nfd;

    pyamiga_set_a4(pyamiga_host_a4);
    memcpy(&timeout, timeout_bits, sizeof(double));
    if (timeout < 0.0)
        return 1;
    if (!pyamiga_amitcp_ok())
        return -1;
    nfd = pysock_require(fd);
    if (nfd < 0)
        return -1;
    if (nfd >= FD_SETSIZE) {
        errno = EBADF;
        return -1;
    }
    tv.tv_sec = (long)timeout;
    tv.tv_usec = (long)((timeout - (double)tv.tv_sec) * 1.0e6);
    FD_ZERO(&fds);
    FD_SET((int)nfd, &fds);
    if (writing)
        n = (int)amitcp_lvo_WaitSelect(SocketBase, nfd + 1, NULL,
                                       (APTR)&fds, NULL, (APTR)&tv, NULL);
    else
        n = (int)amitcp_lvo_WaitSelect(SocketBase, nfd + 1,
                                       (APTR)&fds, NULL, NULL, (APTR)&tv,
                                       NULL);
    if (n < 0)
        return -1;
    if (n == 0)
        return 1;
    return 0;
}

/*
 * Select + recv + PyString entirely in the host image.
 * On timeout: returns NULL and sets *timed_out to 1 (no Python exception).
 * On OS error: returns NULL, *timed_out 0, exception set from errno.
 */
static PyObject *
pyamiga_sock_recv(int fd, int len, int flags, const void *timeout_bits,
                  int *timed_out)
{
    char *tmp;
    double timeout;
    double deadline;
    double interval;
    int has_timeout;
    int sel;
    int n;
    LONG nfd;
    PyObject *result;

    pyamiga_set_a4(pyamiga_host_a4);

    if (timed_out != NULL)
        *timed_out = 0;
    if (!pyamiga_amitcp_ok())
        return NULL;
    nfd = pysock_require(fd);
    if (nfd < 0)
        return PyErr_SetFromErrno(PyExc_IOError);

    if (len < 0) {
        PyErr_SetString(PyExc_ValueError, "negative buffersize in recv");
        return NULL;
    }
    if (len == 0)
        return PyString_FromStringAndSize("", 0);

    memcpy(&timeout, timeout_bits, sizeof(double));
    has_timeout = (timeout > 0.0);
    if (has_timeout)
        deadline = _PyTime_FloatTime() + timeout;

    tmp = (char *)PyMem_Malloc((size_t)len);
    if (tmp == NULL)
        return PyErr_NoMemory();

    for (;;) {
        errno = 0;
        if (has_timeout) {
            interval = deadline - _PyTime_FloatTime();
            if (interval < 0.0) {
                PyMem_Free(tmp);
                if (timed_out != NULL)
                    *timed_out = 1;
                return NULL;
            }
            sel = pyamiga_sock_select1(fd, 0, &interval);
            if (sel == 1) {
                PyMem_Free(tmp);
                if (timed_out != NULL)
                    *timed_out = 1;
                return NULL;
            }
            if (sel < 0) {
                PyMem_Free(tmp);
                return PyErr_SetFromErrno(PyExc_IOError);
            }
        }
        n = (int)amitcp_lvo_recv(SocketBase, nfd, (APTR)tmp,
                                 (LONG)len, (LONG)flags);
        if (n >= 0)
            break;
        if (!has_timeout || (errno != EWOULDBLOCK && errno != EAGAIN
                             && errno != EINTR)) {
            PyMem_Free(tmp);
            return PyErr_SetFromErrno(PyExc_IOError);
        }
        /* EINTR / EWOULDBLOCK: retry with select */
    }

    result = PyString_FromStringAndSize(tmp, n);
    PyMem_Free(tmp);
    return result;
}

static Py_ssize_t
pyamiga_sock_send(int fd, PyObject *data, int flags,
                  const void *timeout_bits, int *timed_out)
{
    char *buf;
    Py_ssize_t len;
    Py_ssize_t total;
    Py_ssize_t n;
    double timeout;
    double deadline;
    double interval;
    int has_timeout;
    int sel;
    LONG nfd;

    pyamiga_set_a4(pyamiga_host_a4);

    if (timed_out != NULL)
        *timed_out = 0;
    if (!pyamiga_amitcp_ok())
        return -1;
    nfd = pysock_require(fd);
    if (nfd < 0) {
        PyErr_SetFromErrno(PyExc_IOError);
        return -1;
    }

    if (PyString_AsStringAndSize(data, &buf, &len) < 0)
        return -1;
    if (len == 0)
        return 0;

    memcpy(&timeout, timeout_bits, sizeof(double));
    has_timeout = (timeout > 0.0);
    if (has_timeout)
        deadline = _PyTime_FloatTime() + timeout;

    total = 0;
    while (total < len) {
        errno = 0;
        if (has_timeout) {
            interval = deadline - _PyTime_FloatTime();
            if (interval < 0.0) {
                if (timed_out != NULL)
                    *timed_out = 1;
                return -1;
            }
            sel = pyamiga_sock_select1(fd, 1, &interval);
            if (sel == 1) {
                if (timed_out != NULL)
                    *timed_out = 1;
                return -1;
            }
            if (sel < 0) {
                PyErr_SetFromErrno(PyExc_IOError);
                return -1;
            }
        }
        n = (Py_ssize_t)amitcp_lvo_send(SocketBase, nfd,
                                        (APTR)(buf + total),
                                        (LONG)(len - total), (LONG)flags);
        if (n < 0) {
            if (!has_timeout || (errno != EWOULDBLOCK && errno != EAGAIN
                                 && errno != EINTR)) {
                PyErr_SetFromErrno(PyExc_IOError);
                return -1;
            }
            continue;
        }
        total += n;
    }
    return total;
}

static int
pyamiga_socket_native_fd(int fd)
{
    LONG nfd;

    pyamiga_set_a4(pyamiga_host_a4);
    nfd = pysock_to_native(fd);
    return (int)nfd;
}

static int
pyamiga_socket_set_nbio(int fd, int nonblock)
{
    ULONG nb;
    LONG rc;
    LONG nfd;

    pyamiga_set_a4(pyamiga_host_a4);
    if (!pyamiga_amitcp_ok() || fd < 0)
        return -1;
    nfd = pysock_require(fd);
    if (nfd < 0)
        return -1;
    nb = nonblock ? 1UL : 0UL;
    rc = amitcp_lvo_IoctlSocket(SocketBase, nfd, (ULONG)FIONBIO, (APTR)&nb);
    return (int)rc;
}

static void
pyamiga_object_gc_untrack(PyObject *op)
{
    pyamiga_set_a4(pyamiga_host_a4);
    if (op != NULL && PyObject_IS_GC(op))
        PyObject_GC_UnTrack(op);
}

static void *
pyamiga_PyCapsule_Import(const char *name, int no_block)
{
    void *p;

    pyamiga_set_a4(pyamiga_host_a4);
    p = PyCapsule_Import(name, no_block);
    return p;
}

static PyObject *
pyamiga__PyObject_New(PyTypeObject *t)
{
    PyObject *r;

    pyamiga_set_a4(pyamiga_host_a4);
    r = _PyObject_New(t);
    return r;
}

static void
pyamiga_object_dealloc(PyObject *op)
{
    pyamiga_set_a4(pyamiga_host_a4);
    if (PyObject_IS_GC(op)) {
        PyObject_GC_UnTrack(op);
        PyObject_GC_Del(op);
    } else {
        PyObject_Del(op);
    }
}

static PyObject *
pyamiga_PyDict_New(void)
{
    PyObject *r;

    pyamiga_set_a4(pyamiga_host_a4);
    r = PyDict_New();
    return r;
}

static int
pyamiga_PyDict_SetItemString(PyObject *d, const char *k, PyObject *v)
{
    int r;

    pyamiga_set_a4(pyamiga_host_a4);
    r = PyDict_SetItemString(d, k, v);
    return r;
}

static PyObject *
pyamiga_PyBool_FromLong(long v)
{
    PyObject *r;

    pyamiga_set_a4(pyamiga_host_a4);
    r = PyBool_FromLong(v);
    return r;
}

static int
pyamiga_PyObject_IsTrue(PyObject *o)
{
    int r;

    pyamiga_set_a4(pyamiga_host_a4);
    r = PyObject_IsTrue(o);
    return r;
}

static Py_ssize_t
pyamiga_PyString_Size(PyObject *o)
{
    Py_ssize_t r;

    pyamiga_set_a4(pyamiga_host_a4);
    r = PyString_Size(o);
    return r;
}

static PyObject *
pyamiga_PyModule_GetDict(PyObject *m)
{
    PyObject *r;

    pyamiga_set_a4(pyamiga_host_a4);
    r = PyModule_GetDict(m);
    return r;
}

static PyObject *
pyamiga_PyObject_GetAttrString(PyObject *o, const char *n)
{
    PyObject *r;

    pyamiga_set_a4(pyamiga_host_a4);
    r = PyObject_GetAttrString(o, n);
    return r;
}

static int
pyamiga_PyObject_SetAttrString(PyObject *o, const char *n, PyObject *v)
{
    int r;

    pyamiga_set_a4(pyamiga_host_a4);
    r = PyObject_SetAttrString(o, n, v);
    return r;
}

static int
pyamiga_PyObject_AsWriteBuffer(PyObject *o, void **buf, Py_ssize_t *len)
{
    int r;

    pyamiga_set_a4(pyamiga_host_a4);
    r = PyObject_AsWriteBuffer(o, buf, len);
    return r;
}

/*
 * Plugin PyArg_Parse and PyArg_ParseTuple both call this. Host
 * PyArg_VaParse is ParseTuple (tuple required). Getters/setters use
 * PyArg_Parse on a single object (FLAG_COMPAT). Wrap non-tuples here
 * so the plugin does not PyTuple_SET_ITEM on a host tuple.
 */
static int
pyamiga_PyArg_VaParse(PyObject *args, const char *format, va_list va)
{
    PyObject *tuple;
    int r;

    pyamiga_set_a4(pyamiga_host_a4);
    tuple = NULL;
    if (args != NULL && !PyTuple_Check(args)) {
        tuple = PyTuple_Pack(1, args);
        if (tuple == NULL)
            return 0;
        r = PyArg_VaParse(tuple, format, va);
        Py_DECREF(tuple);
        return r;
    }
    r = PyArg_VaParse(args, format, va);
    return r;
}

struct PyHost *
PyAmiga_GetHost(void)
{
    return &pyamiga_host;
}

void
PyAmiga_InitHost(void)
{
    struct PyHost *h;

    /* Capture CRT A4 while still in the host image. */
    pyamiga_host_a4 = pyamiga_get_a4();
    pysock_init();

    h = &pyamiga_host;
    h->magic = PYAMIGA_HOST_MAGIC;
    h->size = (unsigned long)sizeof(struct PyHost);
    h->abi_version = PYAMIGA_ABI_VERSION;

    h->sysbase = (void *)SysBase;
    h->dosbase = (void *)DOSBase;

    h->ptr_errno = &errno;
    h->ptr_h_errno = &h_errno;
    h->fn_ensure_bsdsocket = pyamiga_ensure_bsdsocket;

    h->fn_malloc = malloc;
    h->fn_free = free;
    h->fn_memcpy = memcpy;
    h->fn_memset = memset;
    h->fn_memmove = memmove;
    h->fn_strlen = strlen;
    h->fn_strcpy = strcpy;
    h->fn_strncpy = strncpy;
    h->fn_strcmp = strcmp;
    h->fn_vsprintf = vsprintf;
    h->fn_vsscanf = vsscanf;
    h->fn_strtoul = strtoul;

    h->obj_None = Py_None;
    h->obj_True = Py_True;
    h->obj_False = Py_False;
    h->obj_ImportError = PyExc_ImportError;
    h->obj_RuntimeError = PyExc_RuntimeError;
    h->obj_SystemError = PyExc_SystemError;
    h->obj_ValueError = PyExc_ValueError;
    h->obj_TypeError = PyExc_TypeError;
    h->obj_OverflowError = PyExc_OverflowError;
    h->obj_OSError = PyExc_OSError;
    h->obj_IOError = PyExc_IOError;
    h->obj_MemoryError = PyExc_MemoryError;
    h->obj_AttributeError = PyExc_AttributeError;
    h->obj_KeyboardInterrupt = PyExc_KeyboardInterrupt;
    h->obj_NotImplementedError = PyExc_NotImplementedError;

    h->type_Type = &PyType_Type;
    h->type_Int = &PyInt_Type;
    h->type_Long = &PyLong_Type;
    h->type_String = &PyString_Type;
    h->type_Tuple = &PyTuple_Type;
    h->type_List = &PyList_Type;
    h->type_Float = &PyFloat_Type;
    h->type_Slice = &PySlice_Type;

    h->fn_Py_InitModule3 = pyamiga_InitModule3;
    h->fn_PyErr_SetString = PyErr_SetString;
    h->fn_PyErr_SetObject = PyErr_SetObject;
    h->fn_PyErr_Clear = PyErr_Clear;
    h->fn_PyErr_Occurred = PyErr_Occurred;
    h->fn_PyErr_CheckSignals = PyErr_CheckSignals;
    h->fn_PyErr_SetFromErrno = PyErr_SetFromErrno;
    h->fn_PyErr_SetFromErrnoWithFilenameObject =
        PyErr_SetFromErrnoWithFilenameObject;
    h->fn_PyErr_NewException = PyErr_NewException;
    h->fn_PyInt_FromLong = PyInt_FromLong;
    h->fn_PyInt_AsLong = PyInt_AsLong;
    h->fn_PyInt_FromSsize_t = PyInt_FromSsize_t;
    h->fn__PyInt_AsInt = _PyInt_AsInt;
    h->fn_PyLong_FromLong = PyLong_FromLong;
    h->fn_PyLong_FromUnsignedLong = PyLong_FromUnsignedLong;
    h->fn_PyLong_AsLong = PyLong_AsLong;
    h->fn_PyLong_AsUnsignedLong = PyLong_AsUnsignedLong;
    h->fn_PyLong_FromLongLong = PyLong_FromLongLong;
    h->fn_PyFloat_FromDouble = PyFloat_FromDouble;
    h->fn_PyFloat_AsDouble = PyFloat_AsDouble;
    h->fn_PyString_FromString = PyString_FromString;
    h->fn_PyString_FromStringAndSize = PyString_FromStringAndSize;
    h->fn_PyString_AsString = PyString_AsString;
    h->fn_PyString_FromFormatV = PyString_FromFormatV;
    h->fn__PyString_Resize = _PyString_Resize;
    h->fn_PyTuple_New = PyTuple_New;
    h->fn_PyTuple_Size = PyTuple_Size;
    h->fn_PyList_New = PyList_New;
    h->fn_PyList_Append = PyList_Append;
    h->fn_PyModule_AddObject = PyModule_AddObject;
    h->fn_PyModule_AddIntConstant = PyModule_AddIntConstant;
    h->fn_PyModule_AddStringConstant = PyModule_AddStringConstant;
    h->fn_PyType_Ready = PyType_Ready;
    h->fn_PyType_GenericNew = PyType_GenericNew;
    h->fn_PyType_GenericAlloc = PyType_GenericAlloc;
    h->fn_PyObject_GenericGetAttr = PyObject_GenericGetAttr;
    h->fn_PyObject_ClearWeakRefs = PyObject_ClearWeakRefs;
    h->fn_PyObject_Free = PyObject_Free;
    h->fn_PyMem_Malloc = PyMem_Malloc;
    h->fn_PyMem_Free = PyMem_Free;
    h->fn_Py_AtExit = Py_AtExit;
    h->fn_PyCapsule_New = PyCapsule_New;
    h->fn_PyFile_FromFile = PyFile_FromFile;
    h->fn_PyFile_SetBufSize = PyFile_SetBufSize;
    h->fn_PyBuffer_Release = PyBuffer_Release;
    h->fn__PyTime_FloatTime = _PyTime_FloatTime;
    h->fn_PyArg_VaParse = pyamiga_PyArg_VaParse;
    h->fn_PyArg_VaParseTupleAndKeywords = PyArg_VaParseTupleAndKeywords;
    h->fn_Py_VaBuildValue = Py_VaBuildValue;
    h->fn_PyOS_vsnprintf = PyOS_vsnprintf;

    h->fn_socket = pyamiga_socket;
    h->fn_bind = pyamiga_bind;
    h->fn_listen = pyamiga_listen;
    h->fn_accept = pyamiga_accept;
    h->fn_connect = pyamiga_connect;
    h->fn_shutdown = pyamiga_shutdown;
    h->fn_close = pyamiga_close;
    h->fn_closesocket = pyamiga_closesocket;
    h->fn_select = pyamiga_select;
    h->fn_recv = pyamiga_recv;
    h->fn_send = pyamiga_send;
    h->fn_recvfrom = pyamiga_recvfrom;
    h->fn_sendto = pyamiga_sendto;
    h->fn_setsockopt = pyamiga_setsockopt;
    h->fn_getsockopt = pyamiga_getsockopt;
    h->fn_getsockname = pyamiga_getsockname;
    h->fn_getpeername = pyamiga_getpeername;
    h->fn_gethostname = pyamiga_gethostname;
    h->fn_gethostbyname = pyamiga_gethostbyname;
    h->fn_gethostbyaddr = pyamiga_gethostbyaddr;
    h->fn_getservbyname = pyamiga_getservbyname;
    h->fn_getservbyport = pyamiga_getservbyport;
    h->fn_getprotobyname = pyamiga_getprotobyname;
    h->fn_inet_addr = pyamiga_inet_addr;
    h->fn_inet_ntoa = pyamiga_inet_ntoa;
    h->fn_htons = pyamiga_htons;
    h->fn_htonl = pyamiga_htonl;
    h->fn_ntohs = pyamiga_ntohs;
    h->fn_ntohl = pyamiga_ntohl;
    h->fn_ioctl = pyamiga_ioctl;
    h->fn_fcntl = pyamiga_fcntl;
    h->fn_dup = pyamiga_dup;
    h->fn_fdopen = pyamiga_fdopen;
    h->fn_fclose = pyamiga_fclose;
    h->fn_strerror = pyamiga_strerror;
    h->fn_note = pyamiga_note;
    h->fn_sock_timeout_from_arg = pyamiga_sock_timeout_from_arg;
    h->fn_sock_timeout_to_obj = pyamiga_sock_timeout_to_obj;
    h->fn_sock_timeout_cmp0 = pyamiga_sock_timeout_cmp0;
    h->fn_sock_timeout_to_tv = pyamiga_sock_timeout_to_tv;
    h->fn_sock_deadline_init = pyamiga_sock_deadline_init;
    h->fn_sock_deadline_remaining = pyamiga_sock_deadline_remaining;
    h->fn_sock_select1 = pyamiga_sock_select1;
    h->fn_sock_recv = pyamiga_sock_recv;
    h->fn_sock_send = pyamiga_sock_send;
    h->ptr_SocketBase = (void *)SocketBase;
    h->fn_socket_native_fd = pyamiga_socket_native_fd;
    h->fn_PyCapsule_Import = pyamiga_PyCapsule_Import;
    h->fn__PyObject_New = pyamiga__PyObject_New;
    h->fn_PyDict_New = pyamiga_PyDict_New;
    h->fn_PyDict_SetItemString = pyamiga_PyDict_SetItemString;
    h->fn_PyBool_FromLong = pyamiga_PyBool_FromLong;
    h->fn_PyObject_IsTrue = pyamiga_PyObject_IsTrue;
    h->fn_PyString_Size = pyamiga_PyString_Size;
    h->fn_PyModule_GetDict = pyamiga_PyModule_GetDict;
    h->fn_PyObject_GetAttrString = pyamiga_PyObject_GetAttrString;
    h->fn_PyObject_SetAttrString = pyamiga_PyObject_SetAttrString;
    h->fn_PyObject_AsWriteBuffer = pyamiga_PyObject_AsWriteBuffer;
    h->fn_object_dealloc = pyamiga_object_dealloc;
    h->fn_socket_set_nbio = pyamiga_socket_set_nbio;
    h->fn_object_gc_untrack = pyamiga_object_gc_untrack;
}
