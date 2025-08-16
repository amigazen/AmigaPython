/*
 * AMIGAPATH module implementation for Python 2.7.18
 *
 * Based on Irmen de Jong's original Amiga port
 * Updated for VBCC and AmigaOS 3.2 compatibility
 *
 * Features:
 * - Path conversion between Amiga and Unix styles
 * - Full path resolution using AmigaOS dos.library
 *
 * Notes:
 * - Compatible with Python 2.7.18 and VBCC toolchain
 * - Designed for AmigaOS 3.2 and later
 */

#include "Python.h"
#include <string.h>

/* Prevent timer.h from being included to avoid struct timeval conflicts */
#define DEVICES_TIMER_H

#include <proto/dos.h>

/* Forward declarations */
static PyObject *amigapath_to_unix(PyObject *, PyObject *);
static PyObject *amigapath_from_unix(PyObject *, PyObject *);
static PyObject *amigapath_fullpath(PyObject *, PyObject *);

/* Convert an Amiga pathname to Unix pathname */
static PyObject *
amigapath_to_unix(PyObject *self, PyObject *args)
{
    char *path;
    char *result;
    char *p;
    
    if (!PyArg_ParseTuple(args, "s", &path))
        return NULL;
    
    result = PyMem_Malloc(strlen(path) + 1);
    if (result == NULL)
        return PyErr_NoMemory();
    
    strcpy(result, path);
    
    /* Convert ':' to '/' */
    p = strchr(result, ':');
    if (p) {
        *p = '/';
        memmove(result + 1, result, p - result + 1);
        result[0] = '/';
    }
    
    /* Convert all '/' to '/' */
    for (p = result; *p; p++) {
        if (*p == '/')
            *p = '/';
    }
    
    return PyString_FromString(result);
}

/* Convert a Unix pathname to Amiga pathname */
static PyObject *
amigapath_from_unix(PyObject *self, PyObject *args)
{
    char *path;
    char *result;
    char *p, *q;
    int len;
    
    if (!PyArg_ParseTuple(args, "s", &path))
        return NULL;
    
    result = PyMem_Malloc(strlen(path) + 1);
    if (result == NULL)
        return PyErr_NoMemory();
    
    strcpy(result, path);
    
    /* If path starts with "/", assume it's a device */
    if (result[0] == '/' && result[1] != '\0') {
        memmove(result, result + 1, strlen(result));
        
        /* Convert first '/' to ':' */
        p = strchr(result, '/');
        if (p)
            *p = ':';
    }
    
    /* Convert all '/' to '/' */
    for (p = result; *p; p++) {
        if (*p == '/')
            *p = '/';
    }
    
    return PyString_FromString(result);
}

/* Get the full path of a file */
static PyObject *
amigapath_fullpath(PyObject *self, PyObject *args)
{
    char *path;
    char result[1024]; /* Should be enough for Amiga paths */
    BPTR lock;
    
    if (!PyArg_ParseTuple(args, "s", &path))
        return NULL;
    
    lock = Lock(path, ACCESS_READ);
    if (!lock) {
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }
    
    if (!NameFromLock(lock, result, sizeof(result))) {
        UnLock(lock);
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }
    
    UnLock(lock);
    return PyString_FromString(result);
}

/* Method table */
static PyMethodDef amigapath_methods[] = {
    {"to_unix", amigapath_to_unix, METH_VARARGS,
     "to_unix(path) -> Convert Amiga path to Unix path"},
    {"from_unix", amigapath_from_unix, METH_VARARGS,
     "from_unix(path) -> Convert Unix path to Amiga path"},
    {"fullpath", amigapath_fullpath, METH_VARARGS,
     "fullpath(path) -> Return the full path of a file"},
    {NULL, NULL, 0, NULL}  /* sentinel */
};

/* Module initialization */
PyMODINIT_FUNC
initamigapath(void)
{
    PyObject *m;

    m = Py_InitModule("amigapath", amigapath_methods);
    if (m == NULL)
        return;

    /* Add version info */
    PyModule_AddStringConstant(m, "__version__", "2.7.18");
} 