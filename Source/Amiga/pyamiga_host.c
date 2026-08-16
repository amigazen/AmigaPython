/*
 * Fill the process-global PyHost for Amiga LoadSeg plugins.
 *
 * Networking goes through PosixLib (HAVE_POSIXLIB): __init_bsdsocket()
 * opens bsdsocket.library and installs errno via SocketBaseTagList.
 * Do not include AmiTCP proto/socket.h here — with PosixLib first on -I
 * those headers clash on the socket/bind/... macros.
 *
 * Gated so we do not open the TCP stack until import _socket (avoids
 * hang/block when no stack is running).
 */

#include <errno.h>
#include <proto/exec.h>

#define PYAMIGA_HOST_BUILD
#include "pyamiga_plugin.h"

/*
 * PosixLib bsdsocket.c — returns 0 on success after SocketBase is open.
 * Argument -1 means "only ensure library init", no fd check.
 */
extern int __init_bsdsocket(int);
extern void init_socket(void);
extern struct Library *SocketBase;

static struct PyHost pyamiga_host;

static void
pyamiga_init_socket_gated(void)
{
    if (SocketBase != NULL) {
        init_socket();
        return;
    }

    if (__init_bsdsocket(-1) != 0 || SocketBase == NULL) {
        PyErr_SetString(PyExc_ImportError,
                        "bsdsocket.library required for _socket "
                        "(no TCP/IP stack)");
        return;
    }

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
