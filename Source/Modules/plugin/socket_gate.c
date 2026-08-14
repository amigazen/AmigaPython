/*
 * _socket.module - optional Amiga networking gate (LoadSeg plugin).
 *
 * import _socket -> LoadSeg this file -> entry(host) -> host opens
 * bsdsocket.library and runs init_socket(). No TCP stack => ImportError.
 */
#include <exec/types.h>

#include "pyamiga_plugin.h"

static void
socket_plugin_entry(struct PyHost *host)
{
    if (host == NULL || host->magic != PYAMIGA_HOST_MAGIC) {
        return;
    }
    if (host->abi_version != PYAMIGA_ABI_VERSION) {
        if (host->fn_PyErr_SetString && host->obj_ImportError) {
            host->fn_PyErr_SetString(
                host->obj_ImportError,
                "_socket.module: ABI mismatch with Python host");
        }
        return;
    }
    if (host->fn_init_socket == NULL) {
        if (host->fn_PyErr_SetString && host->obj_RuntimeError) {
            host->fn_PyErr_SetString(
                host->obj_RuntimeError,
                "_socket host init is NULL");
        }
        return;
    }
    host->fn_init_socket();
}

/* Must be first linked object so magic sits at the start of the image. */
struct PyAmigaModuleInfo PyAmiga_Module = {
    PYAMIGA_MODULE_MAGIC,
    PYAMIGA_ABI_VERSION,
    0,
    "_socket",
    socket_plugin_entry
};
