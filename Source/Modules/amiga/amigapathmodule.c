/*
 * Path conversion helpers registered on the amiga module
 * (former standalone amigapath builtin).
 *
 * amiga.fullpath already exists in amigamodule.c; only to_unix/from_unix
 * are added here.
 */

#include "Python.h"
#include <string.h>

#define DEVICES_TIMER_H
#include <proto/dos.h>
#include "amiga_ext.h"

static PyObject *amigapath_to_unix(PyObject *, PyObject *);
static PyObject *amigapath_from_unix(PyObject *, PyObject *);

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

    /* Convert ':' to '/' and prepend '/' */
    p = strchr(result, ':');
    if (p) {
        *p = '/';
        memmove(result + 1, result, p - result + 1);
        result[0] = '/';
    }

    return PyString_FromString(result);
}

/* Convert a Unix pathname to Amiga pathname */
static PyObject *
amigapath_from_unix(PyObject *self, PyObject *args)
{
    char *path;
    char *result;
    char *p;
    int len;

    if (!PyArg_ParseTuple(args, "s", &path))
        return NULL;

    len = (int)strlen(path);
    result = PyMem_Malloc(len + 1);
    if (result == NULL)
        return PyErr_NoMemory();

    strcpy(result, path);

    /* If path starts with "/", treat first component as device */
    if (result[0] == '/' && result[1] != '\0') {
        memmove(result, result + 1, strlen(result));

        p = strchr(result, '/');
        if (p)
            *p = ':';
    }

    return PyString_FromString(result);
}

static PyMethodDef amigapath_methods[] = {
    {"to_unix", amigapath_to_unix, METH_VARARGS,
     "to_unix(path) -> Convert Amiga path to Unix-style path"},
    {"from_unix", amigapath_from_unix, METH_VARARGS,
     "from_unix(path) -> Convert Unix-style path to Amiga path"},
    {NULL, NULL, 0, NULL}
};

void
amiga_init_path(PyObject *m)
{
    amiga_add_methods(m, amigapath_methods);
}
