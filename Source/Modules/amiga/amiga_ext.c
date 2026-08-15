/*
 * Shared Amiga library bases and helpers for amiga_* / amigagui modules.
 */

#include "amiga_ext.h"
#include <exec/libraries.h>
#include <intuition/intuitionbase.h>
#include <graphics/gfxbase.h>
#include <proto/exec.h>

/* Used by proto/*.h inlines across translation units. */
struct Library *AslBase = NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase = NULL;

void
amiga_add_methods(PyObject *m, PyMethodDef *methods)
{
	PyObject *d;
	PyMethodDef *ml;

	d = PyModule_GetDict(m);
	for (ml = methods; ml->ml_name != NULL; ml++) {
		PyObject *v;

		v = (PyObject *)PyCFunction_New(ml, NULL);
		if (v == NULL)
			return;
		if (PyDict_SetItemString(d, ml->ml_name, v) != 0) {
			Py_DECREF(v);
			return;
		}
		Py_DECREF(v);
	}
}

int
amiga_ensure_asl(void)
{
	if (AslBase == NULL)
		AslBase = OpenLibrary("asl.library", 37L);
	if (AslBase == NULL) {
		PyErr_SetString(PyExc_RuntimeError, "asl.library not available");
		return 0;
	}
	return 1;
}

int
amiga_ensure_intuition(void)
{
	if (IntuitionBase == NULL)
		IntuitionBase = (struct IntuitionBase *)
			OpenLibrary("intuition.library", 37L);
	if (IntuitionBase == NULL) {
		PyErr_SetString(PyExc_RuntimeError,
			"intuition.library not available");
		return 0;
	}
	return 1;
}

int
amiga_ensure_graphics(void)
{
	if (GfxBase == NULL)
		GfxBase = (struct GfxBase *)
			OpenLibrary("graphics.library", 37L);
	if (GfxBase == NULL) {
		PyErr_SetString(PyExc_RuntimeError,
			"graphics.library not available");
		return 0;
	}
	return 1;
}
