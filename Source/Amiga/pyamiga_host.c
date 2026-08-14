/*
 * Fill the process-global PyHost for Amiga LoadSeg plugins.
 *
 * Opening bsdsocket.library must also call SocketBaseTags so the stack
 * uses this process's errno/h_errno. Without that, console I/O can stop
 * updating after import _socket (no further >>> prompt).
 */
#define PYAMIGA_HOST_BUILD
#include "pyamiga_plugin.h"

#include <errno.h>
#include <proto/exec.h>

/*
 * PosixLib #defines gethostid()/gethostname()/getdtablesize() as macros.
 * proto/socket.h prototypes the same names and vbcc then errors. Undef
 * only in this file before the bsdsocket headers.
 */
#ifdef gethostid
#undef gethostid
#endif
#ifdef gethostname
#undef gethostname
#endif
#ifdef getdtablesize
#undef getdtablesize
#endif

#include <proto/socket.h>
#include <libraries/bsdsocket.h>

extern void init_socket(void);
extern struct Library *SocketBase;
extern int h_errno;

static struct PyHost pyamiga_host;

static void
pyamiga_init_socket_gated(void)
{
    struct Library *base;

    if (SocketBase != NULL && SocketBase != (struct Library *)1) {
        init_socket();
        return;
    }

    base = OpenLibrary((STRPTR)"bsdsocket.library", 4);
    if (base == NULL) {
        PyErr_SetString(PyExc_ImportError,
                        "bsdsocket.library required for _socket "
                        "(no TCP/IP stack)");
        return;
    }

    SocketBase = base;
    SocketBaseTags(SBTM_SETVAL(SBTC_ERRNOPTR(sizeof(errno))), &errno,
                   SBTM_SETVAL(SBTC_HERRNOLONGPTR), &h_errno,
                   SBTM_SETVAL(SBTC_LOGTAGPTR), "Python",
                   TAG_END);

    init_socket();
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

    h = &pyamiga_host;
    h->magic = PYAMIGA_HOST_MAGIC;
    h->size = (unsigned long)sizeof(struct PyHost);
    h->abi_version = PYAMIGA_ABI_VERSION;

    h->obj_None = Py_None;
    h->obj_ImportError = PyExc_ImportError;
    h->obj_RuntimeError = PyExc_RuntimeError;
    h->obj_SystemError = PyExc_SystemError;

    h->fn_PyErr_SetString = PyErr_SetString;
    h->fn_PyErr_Clear = PyErr_Clear;
    h->fn_PyErr_Occurred = PyErr_Occurred;

    h->fn_init_socket = pyamiga_init_socket_gated;
}
