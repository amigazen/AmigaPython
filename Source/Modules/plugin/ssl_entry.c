/*
 * _ssl.module entry — LoadSeg plugin owning sslmodule.o.
 *
 * Default backend is amitls.library (PYAMIGA_USE_AMITLS). AmiSSL is the
 * #ifndef branch in sslmodule.c (SSL_CFLAGS_AMISSL / sslinclude:).
 *
 * Layout matches FastForward extheader: PyAmigaModuleInfo is the first
 * linked object (security word + ID), then entry(host) installs the
 * host vtable, opens bsdsocket via the host, and runs init_ssl().
 */
#include "pyamiga_plugin.h"
#include "pyamiga_redir.h"

extern void init_ssl(void);

static void
ssl_plugin_entry(struct PyHost *host)
{
    if (host == NULL || host->magic != PYAMIGA_HOST_MAGIC) {
        return;
    }
    if (host->abi_version != PYAMIGA_ABI_VERSION) {
        if (host->fn_PyErr_SetString && host->obj_ImportError) {
            host->fn_PyErr_SetString(
                host->obj_ImportError,
                "_ssl.module: ABI mismatch with Python host");
        }
        return;
    }
    PyAmiga_InstallHost(host);
    if (host->fn_ensure_bsdsocket == NULL) {
        host->fn_PyErr_SetString(host->obj_RuntimeError,
                                 "_ssl host ensure_bsdsocket is NULL");
        return;
    }
    /* AmiTLS/AmiSSL talk to the same SocketBase PosixLib opened for _socket. */
    if (host->fn_ensure_bsdsocket() != 0)
        return;
    init_ssl();
}

/* First linked object — FastForward FFPluginHead / extheader pattern. */
struct PyAmigaModuleInfo PyAmiga_Module = {
    PYAMIGA_MODULE_SECURITY,
    PYAMIGA_MODULE_ID,
    PYAMIGA_ABI_VERSION,
    0,
    "_ssl",
    ssl_plugin_entry
};
