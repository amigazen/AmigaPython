#include "Python.h"

dl_funcptr _PyImport_GetDynLoadFunc(const char *shortname,
                                    const char *pathname, FILE *fp)
{
    PyErr_SetString(PyExc_NotImplementedError,
                    "Dynamic loading not supported on Amiga");
    return NULL;
} 