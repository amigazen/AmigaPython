/*
 * Locale catalog helpers for the amiga module (OS4 catalog module parity).
 */

#include "amiga_posixtimer.h"
#include "Python.h"
#include <string.h>
#include <exec/libraries.h>
#include <libraries/locale.h>
#include <proto/exec.h>
#include <proto/locale.h>
#include "amiga_ext.h"

/* LocaleBase is defined and opened in Amiga/libcheck.c (proto/locale.h). */

typedef struct {
	PyObject_HEAD
	struct Catalog *catalog;
} catalogobject;

staticforward PyTypeObject Catalog_Type;

static int
ensure_locale(void)
{
	if (LocaleBase == NULL)
		LocaleBase = (struct LocaleBase *)
			OpenLibrary("locale.library", 38L);
	if (LocaleBase == NULL) {
		PyErr_SetString(PyExc_RuntimeError,
			"locale.library not available");
		return 0;
	}
	return 1;
}

static void
catalog_dealloc(catalogobject *self)
{
	if (self->catalog != NULL && LocaleBase != NULL) {
		CloseCatalog(self->catalog);
		self->catalog = NULL;
	}
	PyObject_Del(self);
}

PyDoc_STRVAR(GetString_doc,
"GetString(id, default=None) -> string\n"
"Return catalog string id, or default if missing.");

static PyObject *
catalog_GetString(catalogobject *self, PyObject *args)
{
	long id;
	char *defstr = NULL;
	STRPTR s;

	if (!PyArg_ParseTuple(args, "l|z:GetString", &id, &defstr))
		return NULL;
	if (self->catalog == NULL) {
		PyErr_SetString(PyExc_ValueError, "catalog is closed");
		return NULL;
	}
	if (!ensure_locale())
		return NULL;

	s = GetCatalogStr(self->catalog, (LONG)id,
		(STRPTR)(defstr ? defstr : (char *)""));
	if (s == NULL)
		s = (STRPTR)(defstr ? defstr : (char *)"");
	return PyString_FromString((char *)s);
}

static PyMethodDef catalog_methods[] = {
	{"GetString", (PyCFunction)catalog_GetString, METH_VARARGS,
		GetString_doc},
	{NULL, NULL, 0, NULL}
};

static PyObject *
catalog_getattr(catalogobject *self, char *name)
{
	return Py_FindMethod(catalog_methods, (PyObject *)self, name);
}

statichere PyTypeObject Catalog_Type = {
	PyObject_HEAD_INIT(NULL)
	0,
	"amiga.Catalog",
	sizeof(catalogobject),
	0,
	(destructor)catalog_dealloc,
	0,
	(getattrfunc)catalog_getattr,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
};

PyDoc_STRVAR(OpenCatalog_doc,
"OpenCatalog(name, languagename=None, builtinlanguage=None) -> Catalog\n"
"Open an Amiga locale catalog file.");

static PyObject *
amiga_OpenCatalog(PyObject *self, PyObject *args, PyObject *kw)
{
	static char *kwlist[] = {
		"name", "languagename", "builtinlanguage", NULL
	};
	char *name;
	char *lang = NULL;
	char *builtin = NULL;
	struct Catalog *cat;
	catalogobject *obj;

	if (!PyArg_ParseTupleAndKeywords(args, kw, "s|zz:OpenCatalog", kwlist,
			&name, &lang, &builtin))
		return NULL;

	if (!ensure_locale())
		return NULL;

	cat = OpenCatalog(NULL, name,
		OC_Language, lang,
		OC_BuiltInLanguage, builtin,
		TAG_DONE);
	if (cat == NULL) {
		PyErr_SetString(PyExc_IOError, "OpenCatalog failed");
		return NULL;
	}

	obj = PyObject_NEW(catalogobject, &Catalog_Type);
	if (obj == NULL) {
		CloseCatalog(cat);
		return NULL;
	}
	obj->catalog = cat;
	return (PyObject *)obj;
}

static PyMethodDef catalog_mod_methods[] = {
	{"OpenCatalog", (PyCFunction)amiga_OpenCatalog,
		METH_VARARGS | METH_KEYWORDS, OpenCatalog_doc},
	{NULL, NULL, 0, NULL}
};

void
amiga_init_catalog(PyObject *m)
{
	Catalog_Type.ob_type = &PyType_Type;
	amiga_add_methods(m, catalog_mod_methods);
}
