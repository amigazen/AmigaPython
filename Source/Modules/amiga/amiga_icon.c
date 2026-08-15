/*
 * icon.library helpers for the amiga module (OS4 icon module parity).
 */

#include "amiga_posixtimer.h"
#include "Python.h"
#include <string.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <workbench/workbench.h>
#include <proto/exec.h>
#include <proto/icon.h>
#include "amiga_ext.h"

/* Defined for proto/icon.h (not static). */
struct Library *IconBase = NULL;

typedef struct {
	PyObject_HEAD
	struct DiskObject *dob;
	int tooltypes_owned;
} diskobject;

staticforward PyTypeObject DiskObject_Type;

static int
ensure_icon(void)
{
	if (IconBase == NULL)
		IconBase = OpenLibrary("icon.library", 37L);
	if (IconBase == NULL) {
		PyErr_SetString(PyExc_RuntimeError, "icon.library not available");
		return 0;
	}
	return 1;
}

static void
free_owned_tooltypes(STRPTR *tt)
{
	LONG i;

	if (tt == NULL)
		return;
	for (i = 0; tt[i] != NULL; i++)
		FreeVec(tt[i]);
	FreeVec(tt);
}

static void
diskobject_dealloc(diskobject *self)
{
	if (self->dob != NULL && IconBase != NULL) {
		if (self->tooltypes_owned) {
			free_owned_tooltypes(self->dob->do_ToolTypes);
			self->dob->do_ToolTypes = NULL;
		}
		FreeDiskObject(self->dob);
		self->dob = NULL;
	}
	PyObject_Del(self);
}

PyDoc_STRVAR(PutIcon_doc,
"PutIcon(name) -> None\n"
"Write this DiskObject back to disk (without .info suffix).");

static PyObject *
diskobject_PutIcon(diskobject *self, PyObject *args)
{
	char *name;
	BOOL ok;

	if (!PyArg_ParseTuple(args, "s:PutIcon", &name))
		return NULL;
	if (self->dob == NULL) {
		PyErr_SetString(PyExc_ValueError, "DiskObject is closed");
		return NULL;
	}
	if (!ensure_icon())
		return NULL;

	Py_BEGIN_ALLOW_THREADS
	ok = PutDiskObject(name, self->dob);
	Py_END_ALLOW_THREADS
	if (!ok) {
		PyErr_SetString(PyExc_IOError, "PutDiskObject failed");
		return NULL;
	}
	Py_INCREF(Py_None);
	return Py_None;
}

static PyMethodDef diskobject_methods[] = {
	{"PutIcon", (PyCFunction)diskobject_PutIcon, METH_VARARGS, PutIcon_doc},
	{NULL, NULL, 0, NULL}
};

static PyObject *
tooltypes_as_list(STRPTR *tt)
{
	PyObject *list;
	LONG i;
	LONG n;

	n = 0;
	if (tt != NULL) {
		for (n = 0; tt[n] != NULL; n++)
			;
	}
	list = PyList_New(n);
	if (list == NULL)
		return NULL;
	for (i = 0; i < n; i++) {
		PyObject *s = PyString_FromString((char *)tt[i]);
		if (s == NULL) {
			Py_DECREF(list);
			return NULL;
		}
		PyList_SET_ITEM(list, i, s);
	}
	return list;
}

static STRPTR *
list_as_tooltypes(PyObject *seq)
{
	PyObject *fast;
	Py_ssize_t n;
	Py_ssize_t i;
	STRPTR *tt;

	fast = PySequence_Fast(seq, "tooltypes must be a sequence of strings");
	if (fast == NULL)
		return NULL;
	n = PySequence_Fast_GET_SIZE(fast);
	tt = (STRPTR *)AllocVec((ULONG)((n + 1) * sizeof(STRPTR)),
		MEMF_CLEAR | MEMF_PUBLIC);
	if (tt == NULL) {
		Py_DECREF(fast);
		PyErr_NoMemory();
		return NULL;
	}
	for (i = 0; i < n; i++) {
		PyObject *item = PySequence_Fast_GET_ITEM(fast, i);
		char *s;
		ULONG len;

		if (!PyString_Check(item)) {
			free_owned_tooltypes(tt);
			Py_DECREF(fast);
			PyErr_SetString(PyExc_TypeError,
				"tooltypes items must be strings");
			return NULL;
		}
		s = PyString_AS_STRING(item);
		len = (ULONG)PyString_GET_SIZE(item);
		tt[i] = (STRPTR)AllocVec(len + 1, MEMF_PUBLIC);
		if (tt[i] == NULL) {
			free_owned_tooltypes(tt);
			Py_DECREF(fast);
			PyErr_NoMemory();
			return NULL;
		}
		memcpy(tt[i], s, len);
		tt[i][len] = '\0';
	}
	tt[n] = NULL;
	Py_DECREF(fast);
	return tt;
}

static PyObject *
diskobject_getattr(diskobject *self, char *name)
{
	PyObject *m;

	if (self->dob == NULL) {
		PyErr_SetString(PyExc_ValueError, "DiskObject is closed");
		return NULL;
	}
	if (strcmp(name, "tooltypes") == 0)
		return tooltypes_as_list(self->dob->do_ToolTypes);
	if (strcmp(name, "deftool") == 0) {
		if (self->dob->do_DefaultTool)
			return PyString_FromString(
				(char *)self->dob->do_DefaultTool);
		Py_INCREF(Py_None);
		return Py_None;
	}
	if (strcmp(name, "stacksize") == 0)
		return PyInt_FromLong(self->dob->do_StackSize);

	m = Py_FindMethod(diskobject_methods, (PyObject *)self, name);
	return m;
}

static int
diskobject_setattr(diskobject *self, char *name, PyObject *v)
{
	if (self->dob == NULL) {
		PyErr_SetString(PyExc_ValueError, "DiskObject is closed");
		return -1;
	}
	if (v == NULL) {
		PyErr_SetString(PyExc_TypeError, "cannot delete attributes");
		return -1;
	}
	if (strcmp(name, "tooltypes") == 0) {
		STRPTR *tt = list_as_tooltypes(v);
		if (tt == NULL)
			return -1;
		if (self->tooltypes_owned)
			free_owned_tooltypes(self->dob->do_ToolTypes);
		/* First replace of GetDiskObject tooltypes: leave original
		 * for FreeDiskObject by nulling only after we own a copy —
		 * we accept a one-time leak of the library's array. */
		self->dob->do_ToolTypes = tt;
		self->tooltypes_owned = 1;
		return 0;
	}
	if (strcmp(name, "deftool") == 0) {
		char *s;
		if (!PyString_Check(v)) {
			PyErr_SetString(PyExc_TypeError, "deftool must be a string");
			return -1;
		}
		s = PyString_AS_STRING(v);
		/* do_DefaultTool is usually in the DiskObject allocation;
		 * replace with a simple AllocVec copy. */
		{
			ULONG len = (ULONG)PyString_GET_SIZE(v);
			STRPTR ns = (STRPTR)AllocVec(len + 1, MEMF_PUBLIC);
			if (ns == NULL) {
				PyErr_NoMemory();
				return -1;
			}
			memcpy(ns, s, len);
			ns[len] = '\0';
			self->dob->do_DefaultTool = ns;
		}
		return 0;
	}
	if (strcmp(name, "stacksize") == 0) {
		long n = PyInt_AsLong(v);
		if (n == -1 && PyErr_Occurred())
			return -1;
		self->dob->do_StackSize = n;
		return 0;
	}
	PyErr_SetString(PyExc_AttributeError, name);
	return -1;
}

statichere PyTypeObject DiskObject_Type = {
	PyObject_HEAD_INIT(NULL)
	0,
	"amiga.DiskObject",
	sizeof(diskobject),
	0,
	(destructor)diskobject_dealloc,
	0,
	(getattrfunc)diskobject_getattr,
	(setattrfunc)diskobject_setattr,
	0,
	0,
	0,
	0,
	0,
	0,
};

PyDoc_STRVAR(DiskObject_doc,
"DiskObject(name) -> DiskObject\n"
"Load an Amiga icon (do not include the .info suffix).");

static PyObject *
amiga_DiskObject(PyObject *self, PyObject *args)
{
	char *name;
	struct DiskObject *dob;
	diskobject *obj;

	if (!PyArg_ParseTuple(args, "s:DiskObject", &name))
		return NULL;
	if (!ensure_icon())
		return NULL;

	Py_BEGIN_ALLOW_THREADS
	dob = GetDiskObject(name);
	Py_END_ALLOW_THREADS
	if (dob == NULL) {
		PyErr_SetString(PyExc_IOError, "GetDiskObject failed");
		return NULL;
	}

	obj = PyObject_NEW(diskobject, &DiskObject_Type);
	if (obj == NULL) {
		FreeDiskObject(dob);
		return NULL;
	}
	obj->dob = dob;
	obj->tooltypes_owned = 0;
	return (PyObject *)obj;
}

static PyMethodDef icon_mod_methods[] = {
	{"DiskObject", amiga_DiskObject, METH_VARARGS, DiskObject_doc},
	{NULL, NULL, 0, NULL}
};

void
amiga_init_icon(PyObject *m)
{
	DiskObject_Type.ob_type = &PyType_Type;
	amiga_add_methods(m, icon_mod_methods);
}
