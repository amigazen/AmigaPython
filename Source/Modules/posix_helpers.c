/*
 * Minimal POSIX helper functions for Amiga Python 2.7.18
 * - uid/gid converters for pwd/grp
 * - Amiga volume:path -> PosixLib /volume/path conversion
 */

#include "Python.h"
#include "amiga_paths.h"
#include <sys/types.h>
#include <string.h>

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

#ifdef _AMIGA
/*
 * PosixLib accepts Amiga volume:path natively.  Rewriting as /volume/path
 * makes isdir() fail, so NullImporter claims Lib and site cannot import.
 * Keep this as a straight copy shared by fopen/stat call sites.
 */
void
Py_AmigaToPosixPath(char *dest, size_t dest_len, const char *amiga_path)
{
    if (dest == NULL || dest_len == 0)
        return;
    dest[0] = '\0';
    if (amiga_path == NULL || amiga_path[0] == '\0')
        return;
    strncpy(dest, amiga_path, dest_len - 1);
    dest[dest_len - 1] = '\0';
}
#endif /* _AMIGA */
