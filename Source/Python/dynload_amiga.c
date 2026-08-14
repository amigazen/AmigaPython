/*
 * Amiga dynamic loader for Python C extensions (LoadSeg plugins).
 *
 * Plugin files are fully linked hunk binaries containing
 * struct PyAmigaModuleInfo (magic PYAMIGA_MODULE_MAGIC). The returned
 * init thunk runs in the host and calls plugin->entry(PyHost *).
 */
#include "Python.h"
#include "importdl.h"

#define PYAMIGA_HOST_BUILD
#include "pyamiga_plugin.h"

#include <stdlib.h>
#include <string.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

const struct filedescr _PyImport_DynLoadFiletab[] = {
    {".module", "rb", C_EXTENSION},
    {0, 0}
};

typedef struct PyAmigaLoaded {
    struct PyAmigaLoaded *next;
    BPTR seglist;
    char *pathname;
    struct PyAmigaModuleInfo *info;
} PyAmigaLoaded;

static PyAmigaLoaded *pyamiga_loaded;
static struct PyAmigaModuleInfo *pyamiga_pending;

static void
amiga_call_pending_init(void)
{
    struct PyAmigaModuleInfo *info;

    info = pyamiga_pending;
    pyamiga_pending = NULL;
    if (info == NULL || info->entry == NULL) {
        PyErr_SetString(PyExc_ImportError,
                        "Amiga module entry is NULL");
        return;
    }
    info->entry(PyAmiga_GetHost());
}

static struct PyAmigaModuleInfo *
find_module_info(BPTR seglist)
{
    BPTR seg;
    ULONG *p;
    ULONG i;

    /*
     * LoadSeg builds a chain: each block is BPTR next, then hunk bytes.
     * Initialized PyAmiga_Module lives in DATA, not the first CODE hunk,
     * so every segment must be scanned (CODE alone only has immediates
     * like 'PyHs' from comparisons).
     */
    for (seg = seglist; seg != 0; seg = *(BPTR *)BADDR(seg)) {
        p = (ULONG *)BADDR(seg) + 1;
        for (i = 0; i < 256UL; i++) {
            if (p[i] == PYAMIGA_MODULE_MAGIC)
                return (struct PyAmigaModuleInfo *)(p + i);
        }
    }
    return NULL;
}

static void
unload_all(void)
{
    PyAmigaLoaded *L;
    PyAmigaLoaded *n;

    for (L = pyamiga_loaded; L != NULL; L = n) {
        n = L->next;
        if (L->seglist)
            UnLoadSeg(L->seglist);
        if (L->pathname)
            free(L->pathname);
        free(L);
    }
    pyamiga_loaded = NULL;
}

dl_funcptr
_PyImport_GetDynLoadFunc(const char *fqname, const char *shortname,
                         const char *pathname, FILE *fp)
{
    BPTR seglist;
    struct PyAmigaModuleInfo *info;
    PyAmigaLoaded *L;
    static int atexit_set;

    (void)fqname;
    (void)fp;

    if (pathname == NULL || pathname[0] == '\0') {
        PyErr_SetString(PyExc_ImportError,
                        "Amiga dynload: empty pathname");
        return NULL;
    }

    PyAmiga_InitHost();

    seglist = LoadSeg((STRPTR)pathname);
    if (seglist == 0) {
        PyErr_Format(PyExc_ImportError,
                     "LoadSeg failed for %.200s", pathname);
        return NULL;
    }

    info = find_module_info(seglist);
    if (info == NULL) {
        UnLoadSeg(seglist);
        PyErr_Format(PyExc_ImportError,
                     "%.200s is not an Amiga Python module (bad magic)",
                     pathname);
        return NULL;
    }
    if (info->abi_version != PYAMIGA_ABI_VERSION) {
        UnLoadSeg(seglist);
        PyErr_Format(PyExc_ImportError,
                     "%.200s: unsupported plugin ABI %lu (need %d)",
                     pathname,
                     (unsigned long)info->abi_version,
                     PYAMIGA_ABI_VERSION);
        return NULL;
    }

    L = (PyAmigaLoaded *)malloc(sizeof(PyAmigaLoaded));
    if (L == NULL) {
        UnLoadSeg(seglist);
        PyErr_NoMemory();
        return NULL;
    }
    L->seglist = seglist;
    L->pathname = (char *)malloc(strlen(pathname) + 1);
    if (L->pathname == NULL) {
        UnLoadSeg(seglist);
        free(L);
        PyErr_NoMemory();
        return NULL;
    }
    strcpy(L->pathname, pathname);
    L->info = info;
    L->next = pyamiga_loaded;
    pyamiga_loaded = L;

    if (!atexit_set) {
        Py_AtExit(unload_all);
        atexit_set = 1;
    }

    if (Py_VerboseFlag)
        PySys_WriteStderr("# Amiga LoadSeg module %.200s from %.200s\n",
                          shortname, pathname);

    pyamiga_pending = info;
    return (dl_funcptr)amiga_call_pending_init;
}
