/* Amiga Python LoadSeg plugin ABI v8.
 * Plugins carry extension .o code and call Python/Posix only via PyHost.
 * Socket trampolines are a posix fd table over AmiTCP LVOs (unix.lib2
 * psockets). sock_fd is never 0/1/2. AmiTLS uses fn_socket_native_fd()
 * for the bsdsocket id. DNS (gethostbyname, inet_*) still uses PosixLib.
 * C89 / ANSI.
 *
 * ABI v6 adds host SocketBase, native-fd lookup, and extra Python
 * C API entries for the AmiSSL _ssl.module (PyCapsule, dict, bool, …).
 * ABI v7 adds fn_object_dealloc (host GC untrack + GC_Del or Del; never
 * use plugin-compiled _Py_AS_GC on host-heap PyObjects).
 * ABI v8 adds fn_socket_set_nbio (AmiTCP IoctlSocket/FIONBIO via host
 * A4; never jsr IoctlSocket with A6 as the VBCC frame pointer) and
 * fn_object_gc_untrack.
 *
 * Author guide: Docs/PLUGIN_GUIDE.md
 */
#ifndef PYAMIGA_PLUGIN_H
#define PYAMIGA_PLUGIN_H

#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>

#ifdef PYAMIGA_HOST_BUILD
#include "Python.h"
#else
/* Plugin build: full Python.h for types/macros; link stubs, not libpython. */
#include "Python.h"
#endif

#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>

/*
 * Module head mirrors FastForward FFPluginHead (see FastForward
 * Source/include/libraries/ffplugin.h and Docs/PLUGIN_GUIDE.md):
 *   - security word MOVEQ #-1,D0 / RTS so Workbench cannot "run" the file
 *   - ID magic for LoadSeg scanners
 *   - first linked object must be this struct (extheader pattern)
 *
 * Plugins must not link the C runtime (-nostdlib, no -lvc/-lposix).
 * All Python C API, AmiTCP sockets, and libc helpers go through PyHost
 * so SocketBase and the interpreter heap stay in the host image.
 */
#define PYAMIGA_MODULE_SECURITY  0x70FF4E75UL  /* MOVEQ #-1,D0 / RTS */
#define PYAMIGA_MODULE_ID        0x5079416dUL  /* 'PyAm' */
#define PYAMIGA_MODULE_MAGIC     PYAMIGA_MODULE_ID  /* alias for scanners */
#define PYAMIGA_HOST_MAGIC       0x50794873UL  /* 'PyHs' */
#define PYAMIGA_ABI_VERSION      8

struct PyHost;

typedef void (*PyAmiga_EntryFunc)(struct PyHost *host);
typedef int (*PyAmiga_EnsureSocketFunc)(void);

/* First linked object in the plugin (FastForward extheader role). */
struct PyAmigaModuleInfo {
    unsigned long security;      /* PYAMIGA_MODULE_SECURITY */
    unsigned long id;            /* PYAMIGA_MODULE_ID */
    unsigned long abi_version;
    unsigned long flags;
    char name[32];
    PyAmiga_EntryFunc entry;
};

struct PyHost {
    unsigned long magic;
    unsigned long size;
    unsigned long abi_version;

    /* Like FFHost: exec/dos bases for plugins that need them. */
    void *sysbase;
    void *dosbase;

    /* Host errno / h_errno (plugin #defines errno to *ptr_errno). */
    int *ptr_errno;
    int *ptr_h_errno;

    /* Open PosixLib bsdsocket; 0 = ok, nonzero = ImportError already set. */
    PyAmiga_EnsureSocketFunc fn_ensure_bsdsocket;

    /* Minimal libc via host (FastForward: plugins do not link sc.lib/vc). */
    void *(*fn_malloc)(size_t n);
    void (*fn_free)(void *p);
    void *(*fn_memcpy)(void *dst, const void *src, size_t n);
    void *(*fn_memset)(void *dst, int c, size_t n);
    void *(*fn_memmove)(void *dst, const void *src, size_t n);
    size_t (*fn_strlen)(const char *s);
    char *(*fn_strcpy)(char *dst, const char *src);
    char *(*fn_strncpy)(char *dst, const char *src, size_t n);
    int (*fn_strcmp)(const char *a, const char *b);
    int (*fn_vsprintf)(char *s, const char *fmt, va_list v);
    int (*fn_vsscanf)(const char *s, const char *fmt, va_list v);
    unsigned long (*fn_strtoul)(const char *s, char **end, int base);

    PyObject *obj_None;
    PyObject *obj_True;
    PyObject *obj_False;
    PyObject *obj_ImportError;
    PyObject *obj_RuntimeError;
    PyObject *obj_SystemError;
    PyObject *obj_ValueError;
    PyObject *obj_TypeError;
    PyObject *obj_OverflowError;
    PyObject *obj_OSError;
    PyObject *obj_IOError;
    PyObject *obj_MemoryError;
    PyObject *obj_AttributeError;
    PyObject *obj_KeyboardInterrupt;
    PyObject *obj_NotImplementedError;

    PyTypeObject *type_Type;
    PyTypeObject *type_Int;
    PyTypeObject *type_Long;
    PyTypeObject *type_String;
    PyTypeObject *type_Tuple;
    PyTypeObject *type_List;
    PyTypeObject *type_Float;
    PyTypeObject *type_Slice;

    PyObject * (*fn_Py_InitModule3)(char *name, PyMethodDef *methods, char *doc);
    void (*fn_PyErr_SetString)(PyObject *t, const char *m);
    void (*fn_PyErr_SetObject)(PyObject *t, PyObject *v);
    void (*fn_PyErr_Clear)(void);
    PyObject * (*fn_PyErr_Occurred)(void);
    int (*fn_PyErr_CheckSignals)(void);
    PyObject * (*fn_PyErr_SetFromErrno)(PyObject *t);
    PyObject * (*fn_PyErr_SetFromErrnoWithFilenameObject)(PyObject *t, PyObject *fn);
    PyObject * (*fn_PyErr_NewException)(char *name, PyObject *base, PyObject *dict);
    PyObject * (*fn_PyInt_FromLong)(long v);
    long (*fn_PyInt_AsLong)(PyObject *o);
    PyObject * (*fn_PyInt_FromSsize_t)(Py_ssize_t v);
    int (*fn__PyInt_AsInt)(PyObject *o);
    PyObject * (*fn_PyLong_FromLong)(long v);
    PyObject * (*fn_PyLong_FromUnsignedLong)(unsigned long v);
    long (*fn_PyLong_AsLong)(PyObject *o);
    unsigned long (*fn_PyLong_AsUnsignedLong)(PyObject *o);
    PyObject * (*fn_PyLong_FromLongLong)(PY_LONG_LONG v);
    PyObject * (*fn_PyFloat_FromDouble)(double v);
    double (*fn_PyFloat_AsDouble)(PyObject *o);
    PyObject * (*fn_PyString_FromString)(const char *s);
    PyObject * (*fn_PyString_FromStringAndSize)(const char *s, Py_ssize_t n);
    char * (*fn_PyString_AsString)(PyObject *o);
    PyObject * (*fn_PyString_FromFormatV)(const char *f, va_list v);
    int (*fn__PyString_Resize)(PyObject **pv, Py_ssize_t n);
    PyObject * (*fn_PyTuple_New)(Py_ssize_t n);
    Py_ssize_t (*fn_PyTuple_Size)(PyObject *o);
    PyObject * (*fn_PyList_New)(Py_ssize_t n);
    int (*fn_PyList_Append)(PyObject *l, PyObject *i);
    int (*fn_PyModule_AddObject)(PyObject *m, const char *n, PyObject *o);
    int (*fn_PyModule_AddIntConstant)(PyObject *m, const char *n, long v);
    int (*fn_PyModule_AddStringConstant)(PyObject *m, const char *n, const char *v);
    int (*fn_PyType_Ready)(PyTypeObject *t);
    PyObject * (*fn_PyType_GenericNew)(PyTypeObject *t, PyObject *a, PyObject *k);
    PyObject * (*fn_PyType_GenericAlloc)(PyTypeObject *t, Py_ssize_t n);
    PyObject * (*fn_PyObject_GenericGetAttr)(PyObject *o, PyObject *n);
    void (*fn_PyObject_ClearWeakRefs)(PyObject *o);
    void (*fn_PyObject_Free)(void *p);
    void * (*fn_PyMem_Malloc)(size_t n);
    void (*fn_PyMem_Free)(void *p);
    int (*fn_Py_AtExit)(void (*func)(void));
    PyObject * (*fn_PyCapsule_New)(void *p, const char *n, PyCapsule_Destructor d);
    PyObject * (*fn_PyFile_FromFile)(FILE *f, char *n, char *m, int (*c)(FILE *));
    void (*fn_PyFile_SetBufSize)(PyObject *f, int b);
    void (*fn_PyBuffer_Release)(Py_buffer *v);
    double (*fn__PyTime_FloatTime)(void);
    int (*fn_PyArg_VaParse)(PyObject *args, const char *format, va_list v);
    int (*fn_PyArg_VaParseTupleAndKeywords)(PyObject *args, PyObject *kw, const char *format, char **keywords, va_list v);
    PyObject * (*fn_Py_VaBuildValue)(const char *format, va_list v);
    int (*fn_PyOS_vsnprintf)(char *str, size_t size, const char *format, va_list v);

    int (*fn_socket)(int domain, int type, int protocol);
    int (*fn_bind)(int s, const struct sockaddr *addr, socklen_t len);
    int (*fn_listen)(int s, int backlog);
    int (*fn_accept)(int s, struct sockaddr *addr, socklen_t *len);
    int (*fn_connect)(int s, const struct sockaddr *addr, socklen_t len);
    int (*fn_shutdown)(int s, int how);
    int (*fn_close)(int fd);
    int (*fn_closesocket)(int s);
    int (*fn_select)(int nfds, fd_set *rd, fd_set *wr, fd_set *ex, struct timeval *tv);
    int (*fn_recv)(int s, void *buf, size_t len, int flags);
    int (*fn_send)(int s, const void *buf, size_t len, int flags);
    int (*fn_recvfrom)(int s, void *buf, size_t len, int flags, struct sockaddr *addr, socklen_t *alen);
    int (*fn_sendto)(int s, const void *buf, size_t len, int flags, const struct sockaddr *addr, socklen_t alen);
    int (*fn_setsockopt)(int s, int level, int optname, const void *optval, socklen_t optlen);
    int (*fn_getsockopt)(int s, int level, int optname, void *optval, socklen_t *optlen);
    int (*fn_getsockname)(int s, struct sockaddr *addr, socklen_t *len);
    int (*fn_getpeername)(int s, struct sockaddr *addr, socklen_t *len);
    int (*fn_gethostname)(char *name, size_t len);
    struct hostent * (*fn_gethostbyname)(const char *name);
    struct hostent * (*fn_gethostbyaddr)(const void *addr, socklen_t len, int type);
    struct servent * (*fn_getservbyname)(const char *name, const char *proto);
    struct servent * (*fn_getservbyport)(int port, const char *proto);
    struct protoent * (*fn_getprotobyname)(const char *name);
    unsigned long (*fn_inet_addr)(const char *cp);
    char * (*fn_inet_ntoa)(struct in_addr in);
    unsigned short (*fn_htons)(unsigned short hostshort);
    unsigned long (*fn_htonl)(unsigned long hostlong);
    unsigned short (*fn_ntohs)(unsigned short netshort);
    unsigned long (*fn_ntohl)(unsigned long netlong);
    int (*fn_ioctl)(int fd, unsigned long request, char *arg);
    int (*fn_fcntl)(int fd, int cmd, int arg);
    int (*fn_dup)(int fd);
    FILE * (*fn_fdopen)(int fd, const char *mode);
    int (*fn_fclose)(FILE *fp);
    char * (*fn_strerror)(int errnum);

    /* Debug breadcrumbs from LoadSeg plugins (printf+fflush on host). */
    void (*fn_note)(const char *msg);

    /*
     * Soft-float helpers: plugin images link -lmieee without a CRT A4 for
     * its near data, so all double math for timeouts runs on the host.
     * bits8 is an 8-byte IEEE double (big-endian on m68k).
     */
    int (*fn_sock_timeout_from_arg)(PyObject *arg, void *bits8, int *block);
    PyObject *(*fn_sock_timeout_to_obj)(const void *bits8);
    int (*fn_sock_timeout_cmp0)(const void *bits8); /* -1, 0, or 1 vs 0.0 */
    void (*fn_sock_timeout_to_tv)(const void *bits8, struct timeval *tv);
    void (*fn_sock_deadline_init)(const void *timeout_bits, void *deadline_bits);
    int (*fn_sock_deadline_remaining)(const void *deadline_bits,
                                      void *interval_bits);
    /*
     * One-fd WaitSelect for socket timeouts. fd_set lives on the host so
     * plugin vs host FD_SETSIZE / layout cannot smash the plugin stack.
     * fd is an AmiTCP id. Returns 0 ready, 1 timeout, -1 error (errno set).
     */
    int (*fn_sock_select1)(int fd, int writing, const void *timeout_bits);
    /*
     * Full recv on the host: select1 + AmiTCP recv + PyString.
     * Plugin must not own the write buffer (layout / calling issues).
     * timeout_bits: IEEE double seconds (<=0 means blocking, no select).
     * On timeout: returns NULL and sets *timed_out to 1 (no exception).
     * On OS error: returns NULL, *timed_out 0, exception already set.
     */
    PyObject *(*fn_sock_recv)(int fd, int len, int flags,
                              const void *timeout_bits, int *timed_out);
    /*
     * Full send on the host: select1 + AmiTCP send.
     * Plugin must not keep a char* across host trampoline calls (68k
     * scratch regs); pass the PyString and send from the host image.
     * On timeout: returns -1 and sets *timed_out to 1.
     * On OS error: returns -1, *timed_out 0, exception already set.
     * On success: returns bytes sent (>= 0).
     */
    Py_ssize_t (*fn_sock_send)(int fd, PyObject *data, int flags,
                               const void *timeout_bits, int *timed_out);

    /*
     * ABI v6: same bsdsocket base __init_bsdsocket opened. sock_fd is a
     * posix slot (>=3); fn_socket_native_fd returns the AmiTCP id for
     * TlsAttachSocket. Extra Python C API for ssl.py.
     */
    void *ptr_SocketBase;
    int (*fn_socket_native_fd)(int posix_fd);
    void *(*fn_PyCapsule_Import)(const char *name, int no_block);
    PyObject *(*fn__PyObject_New)(PyTypeObject *t);
    PyObject *(*fn_PyDict_New)(void);
    int (*fn_PyDict_SetItemString)(PyObject *d, const char *k, PyObject *v);
    PyObject *(*fn_PyBool_FromLong)(long v);
    int (*fn_PyObject_IsTrue)(PyObject *o);
    Py_ssize_t (*fn_PyString_Size)(PyObject *o);
    PyObject *(*fn_PyModule_GetDict)(PyObject *m);
    PyObject *(*fn_PyObject_GetAttrString)(PyObject *o, const char *n);
    int (*fn_PyObject_SetAttrString)(PyObject *o, const char *n, PyObject *v);
    int (*fn_PyObject_AsWriteBuffer)(PyObject *o, void **buf, Py_ssize_t *len);

    /*
     * ABI v7: free a host-heap PyObject from plugin tp_dealloc.
     * Runs PyObject_GC_UnTrack + PyObject_GC_Del or PyObject_Del on the
     * host with host A4. Plugins must not call _Py_AS_GC / PyObject_Free
     * on GC instances (wrong layout => memory header panic).
     */
    void (*fn_object_dealloc)(PyObject *op);

    /*
     * ABI v8: FIONBIO via host AmiTCP IoctlSocket (Roadshow FIONBIO).
     * Plugin sys/ioctl.h can disagree with AmiTCP. Do not jsr IoctlSocket
     * with A6=SocketBase from VBCC (A6 is the frame pointer) except via
     * a syslog.c-style __reg("a6") stub.
     * fn_object_gc_untrack is present but SSL dealloc must not call it;
     * subtype_dealloc already untracks before the plugin tp_dealloc.
     */
    int (*fn_socket_set_nbio)(int posix_fd, int nonblock);
    /* Untrack a host-heap GC object; safe to call twice. Does not free. */
    void (*fn_object_gc_untrack)(PyObject *op);
};

#ifdef PYAMIGA_HOST_BUILD
struct PyHost *PyAmiga_GetHost(void);
void PyAmiga_InitHost(void);
#else
/* Set by plugin entry before init_*(). */
extern struct PyHost *PyAmiga_Host;
void PyAmiga_InstallHost(struct PyHost *host);
void PyAmiga_Note(const char *msg);
#endif

#endif /* PYAMIGA_PLUGIN_H */
