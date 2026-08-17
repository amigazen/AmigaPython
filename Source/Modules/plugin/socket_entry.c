/*
 * _socket.module entry — real LoadSeg plugin owning socketmodule.o.
 *
 * Layout matches FastForward extheader: PyAmigaModuleInfo is the first
 * linked object (security word + ID), then entry(host) installs the
 * host vtable and runs init_socket() in this image.
 */
#include "pyamiga_plugin.h"
#include "pyamiga_redir.h"

extern void init_socket(void);

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
    PyAmiga_InstallHost(host);
    if (host->fn_ensure_bsdsocket == NULL) {
        host->fn_PyErr_SetString(host->obj_RuntimeError,
                                 "_socket host ensure_bsdsocket is NULL");
        return;
    }
    if (host->fn_ensure_bsdsocket() != 0)
        return;
    init_socket();
}

/* First linked object — FastForward FFPluginHead / extheader pattern. */
struct PyAmigaModuleInfo PyAmiga_Module = {
    PYAMIGA_MODULE_SECURITY,
    PYAMIGA_MODULE_ID,
    PYAMIGA_ABI_VERSION,
    0,
    "_socket",
    socket_plugin_entry
};
