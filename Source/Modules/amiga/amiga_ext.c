/*
 * Shared helpers for registering extra methods on the amiga module.
 */

#include "amiga_ext.h"

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
