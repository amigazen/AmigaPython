/*
 * Minimal POSIX helper functions for Amiga Python 2.7.18
 * Provides only the conversion functions needed by pwdmodule and grpmodule
 * Based on posixmodule.c but simplified for AmigaOS with PosixLib
 */

#include "Python.h"
#include <sys/types.h>

/* Convert uid_t to Python int */
PyObject *
_PyInt_FromUid(uid_t uid)
{
    return PyInt_FromLong((long)uid);
}

/* Convert gid_t to Python int */
PyObject *
_PyInt_FromGid(gid_t gid)
{
    return PyInt_FromLong((long)gid);
}

/* Convert Python object to uid_t */
int
_Py_Uid_Converter(PyObject *obj, void *p)
{
    uid_t *uid = (uid_t *)p;
    long val;
    
    if (PyInt_Check(obj)) {
        val = PyInt_AsLong(obj);
        if (val == -1 && PyErr_Occurred())
            return 0;
        *uid = (uid_t)val;
        return 1;
    }
    
    PyErr_SetString(PyExc_TypeError, "uid must be an integer");
    return 0;
}

/* Convert Python object to gid_t */
int
_Py_Gid_Converter(PyObject *obj, void *p)
{
    gid_t *gid = (gid_t *)p;
    long val;
    
    if (PyInt_Check(obj)) {
        val = PyInt_AsLong(obj);
        if (val == -1 && PyErr_Occurred())
            return 0;
        *gid = (gid_t)val;
        return 1;
    }
    
    PyErr_SetString(PyExc_TypeError, "gid must be an integer");
    return 0;
} 