/*
 * Extra Amiga APIs registered onto the builtin amiga module
 * (former Doslib/amigapath, plus ASL/catalog/icon).
 */
#ifndef Py_AMIGA_EXT_H
#define Py_AMIGA_EXT_H

#include "Python.h"

void amiga_add_methods(PyObject *m, PyMethodDef *methods);
void amiga_init_dos(PyObject *m);
void amiga_init_path(PyObject *m);
void amiga_init_asl(PyObject *m);
void amiga_init_catalog(PyObject *m);
void amiga_init_icon(PyObject *m);

#endif /* Py_AMIGA_EXT_H */
