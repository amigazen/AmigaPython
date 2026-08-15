/*
 * Extra Amiga APIs registered onto the builtin amiga module
 * (former Doslib/amigapath, plus ASL/catalog/icon/intuition helpers).
 */
#ifndef Py_AMIGA_EXT_H
#define Py_AMIGA_EXT_H

#include "Python.h"

struct Library;
struct IntuitionBase;
struct GfxBase;

/* Shared bases for proto/*.h (defined in amiga_ext.c). */
extern struct Library *AslBase;
extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;

int amiga_ensure_asl(void);
int amiga_ensure_intuition(void);
int amiga_ensure_graphics(void);

void amiga_add_methods(PyObject *m, PyMethodDef *methods);
void amiga_init_dos(PyObject *m);
void amiga_init_path(PyObject *m);
void amiga_init_asl(PyObject *m);
void amiga_init_intuition(PyObject *m);
void amiga_init_catalog(PyObject *m);
void amiga_init_icon(PyObject *m);

#endif /* Py_AMIGA_EXT_H */
