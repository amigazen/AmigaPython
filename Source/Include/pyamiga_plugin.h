/* Amiga Python LoadSeg plugin ABI (FastForward-style host vtable).
 * Minimal types so plugins need not include Python.h / link the interpreter.
 * C89 / ANSI.
 */
#ifndef PYAMIGA_PLUGIN_H
#define PYAMIGA_PLUGIN_H

#ifdef PYAMIGA_HOST_BUILD
#include "Python.h"
#else
/* Plugin-side opaque stand-ins (layout never dereferenced in the gate). */
typedef struct PyObject PyObject;
#endif

#define PYAMIGA_MODULE_MAGIC  0x5079416dUL  /* 'PyAm' */
#define PYAMIGA_HOST_MAGIC    0x50794873UL  /* 'PyHs' */
#define PYAMIGA_ABI_VERSION   1

struct PyHost;

typedef void (*PyAmiga_EntryFunc)(struct PyHost *host);
typedef void (*PyAmiga_InitFunc)(void);

/* First object in the plugin link so the LoadSeg image starts here. */
struct PyAmigaModuleInfo {
    unsigned long magic;
    unsigned long abi_version;
    unsigned long flags;
    char name[32];
    PyAmiga_EntryFunc entry;
};

struct PyHost {
    unsigned long magic;
    unsigned long size;
    unsigned long abi_version;

    /* Not named Py_None — that is a macro in Python.h */
    PyObject *obj_None;
    PyObject *obj_ImportError;
    PyObject *obj_RuntimeError;
    PyObject *obj_SystemError;

    void (*fn_PyErr_SetString)(PyObject *exc, const char *msg);
    void (*fn_PyErr_Clear)(void);
    PyObject *(*fn_PyErr_Occurred)(void);

    /* Opens bsdsocket.library then runs init_socket(), or sets ImportError. */
    PyAmiga_InitFunc fn_init_socket;
};

#ifdef PYAMIGA_HOST_BUILD
struct PyHost *PyAmiga_GetHost(void);
void PyAmiga_InitHost(void);
#endif

#endif /* PYAMIGA_PLUGIN_H */
