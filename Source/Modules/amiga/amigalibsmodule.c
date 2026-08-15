/*
 * amigalibs - secondary raw LVO call support for AmigaPython.
 *
 * Port of Amiga_Misc/unused/amigalibsmodule.c (Irmen de Jong, 1996)
 * to Python 2.7 / VBCC. Prefer curated amiga / amigagui APIs when available.
 */

#include "Python.h"
#include <string.h>
#include <exec/libraries.h>
#include <proto/exec.h>

static PyObject *AmigalibsError;
static PyTypeObject Amigalib_Type;

typedef struct {
	PyObject_HEAD
	struct Library *libbase;
	char *libname;
} amigalibobject;

/* VBCC stack-arg entry; see Amiga/amigalibs_calllib.asm */
extern unsigned long amigalibs_calllib(struct Library *libbase,
	long LVOvalue, unsigned short regspec, unsigned long *regs);

static int
obj2reg(PyObject *arg, unsigned long *val)
{
	if (PyInt_Check(arg)) {
		*val = (unsigned long)PyInt_AsLong(arg);
		return 1;
	}
	if (PyLong_Check(arg)) {
		*val = PyLong_AsUnsignedLongMask(arg);
		return 1;
	}
	if (PyString_Check(arg)) {
		*val = (unsigned long)PyString_AsString(arg);
		return 1;
	}
	if (arg == Py_None) {
		*val = 0;
		return 1;
	}
	return 0;
}

static void
lib_dealloc(amigalibobject *self)
{
	if (self->libbase != NULL) {
		if (self->libname == NULL ||
		    strcmp(self->libname, "exec.library") != 0)
			CloseLibrary(self->libbase);
		self->libbase = NULL;
	}
	if (self->libname != NULL) {
		PyMem_Free(self->libname);
		self->libname = NULL;
	}
	PyObject_Del(self);
}

static PyObject *
lib_call(amigalibobject *self, PyObject *args)
{
	int LVO_value;
	int regspec_in;
	unsigned long regspec;
	unsigned long orig_regspec;
	PyObject *dic;
	int pos;
	PyObject *key, *value;
	unsigned long reg[16];
	unsigned long result;

	if (!PyArg_ParseTuple(args, "(ii)O!:call",
			&LVO_value, &regspec_in, &PyDict_Type, &dic))
		return NULL;

	regspec = (unsigned long)regspec_in & 0xFFFFUL;
	orig_regspec = regspec;
	pos = 0;

	if (LVO_value > -30 || (LVO_value % 2) != 0) {
		PyErr_SetString(PyExc_ValueError, "illegal LVO value");
		return NULL;
	}

	memset(reg, 0, sizeof(reg));

	while (PyDict_Next(dic, &pos, &key, &value)) {
		long regnr;
		unsigned long regval;

		if (!PyInt_Check(key)) {
			PyErr_SetString(PyExc_ValueError, "illegal key/regnr.");
			return NULL;
		}
		regnr = PyInt_AsLong(key);
		if (regnr < 0 || regnr > 15) {
			PyErr_SetString(PyExc_ValueError, "illegal key/regnr.");
			return NULL;
		}
		if (!obj2reg(value, &regval)) {
			PyErr_SetString(PyExc_ValueError,
				"illegal register value");
			return NULL;
		}
		if ((regspec & (1UL << regnr)) == 0) {
			PyErr_SetString(PyExc_ValueError,
				"registers not consistent with LVO spec");
			return NULL;
		}
		reg[regnr] = regval;
		regspec &= ~(1UL << regnr);
	}

	if (regspec != 0) {
		PyErr_SetString(PyExc_ValueError,
			"too few arguments provided");
		return NULL;
	}

	Py_BEGIN_ALLOW_THREADS
	result = amigalibs_calllib(self->libbase, (long)LVO_value,
		(unsigned short)orig_regspec, reg);
	Py_END_ALLOW_THREADS

	return PyLong_FromUnsignedLong(result);
}

static PyMethodDef lib_methods[] = {
	{"call", (PyCFunction)lib_call, METH_VARARGS,
		"call((lvo, regspec), {regnum: value, ...}) -> result\n"
		"Perform a raw Amiga library vector call. Unsafe."},
	{NULL, NULL, 0, NULL}
};

static PyObject *
lib_getattr(amigalibobject *ao, char *name)
{
	if (strcmp(name, "base") == 0)
		return PyLong_FromUnsignedLong((unsigned long)ao->libbase);
	if (strcmp(name, "version") == 0)
		return Py_BuildValue("(ii)",
			(int)ao->libbase->lib_Version,
			(int)ao->libbase->lib_Revision);
	return Py_FindMethod(lib_methods, (PyObject *)ao, name);
}

static PyObject *
lib_repr(amigalibobject *ao)
{
	char buf[320];

	PyOS_snprintf(buf, sizeof(buf),
		"<amiga library '%s', V %ld.%ld, base %lx, at %lx>",
		ao->libname ? ao->libname : "?",
		(long)ao->libbase->lib_Version,
		(long)ao->libbase->lib_Revision,
		(unsigned long)ao->libbase,
		(unsigned long)ao);
	return PyString_FromString(buf);
}

static PyTypeObject Amigalib_Type = {
	PyObject_HEAD_INIT(NULL)
	0,
	"amigalibs.amigalib",
	sizeof(amigalibobject),
	0,
	(destructor)lib_dealloc,
	0,
	(getattrfunc)lib_getattr,
	0,
	0,
	(reprfunc)lib_repr,
};

static PyObject *
newamigalibobject(char *libname, int libver)
{
	amigalibobject *ao;
	size_t n;

	ao = PyObject_New(amigalibobject, &Amigalib_Type);
	if (ao == NULL)
		return NULL;
	ao->libbase = NULL;
	ao->libname = NULL;

	n = strlen(libname) + 1;
	ao->libname = (char *)PyMem_Malloc(n);
	if (ao->libname == NULL) {
		PyObject_Del(ao);
		return PyErr_NoMemory();
	}
	memcpy(ao->libname, libname, n);

	if (strcmp(libname, "exec.library") == 0)
		ao->libbase = *((struct Library **)4L);
	else
		ao->libbase = OpenLibrary(libname, libver);

	if (ao->libbase == NULL) {
		PyErr_SetString(AmigalibsError, "can't open library");
		PyMem_Free(ao->libname);
		ao->libname = NULL;
		PyObject_Del(ao);
		return NULL;
	}
	return (PyObject *)ao;
}

static PyObject *
amigalibs_openlib(PyObject *self, PyObject *args)
{
	char *libname;
	int libver = 37;

	if (!PyArg_ParseTuple(args, "s|i:openlib", &libname, &libver))
		return NULL;
	return newamigalibobject(libname, libver);
}

static PyObject *
amigalibs_obj2reg(PyObject *self, PyObject *args)
{
	PyObject *arg;
	unsigned long res;

	if (!PyArg_ParseTuple(args, "O:obj2reg", &arg))
		return NULL;
	if (!obj2reg(arg, &res)) {
		PyErr_SetString(PyExc_TypeError,
			"can't convert type to register ULONG");
		return NULL;
	}
	return PyLong_FromUnsignedLong(res);
}

static PyObject *
amigalibs_addr(PyObject *self, PyObject *args)
{
	PyObject *arg;

	if (!PyArg_ParseTuple(args, "O:addr", &arg))
		return NULL;
	if (PyInt_Check(arg))
		return PyLong_FromUnsignedLong(
			(unsigned long)&((PyIntObject *)arg)->ob_ival);
	if (PyString_Check(arg))
		return PyLong_FromUnsignedLong(
			(unsigned long)PyString_AS_STRING(arg));
	if (arg == Py_None)
		return PyInt_FromLong(0);
	PyErr_SetString(PyExc_TypeError, "addr() needs int, str, or None");
	return NULL;
}

static PyObject *
amigalibs_fixstr(PyObject *self, PyObject *args)
{
	PyObject *arg;
	char *str;
	Py_ssize_t len;
	Py_ssize_t len2;

	if (!PyArg_ParseTuple(args, "S:fixstr", &arg))
		return NULL;
	str = PyString_AS_STRING(arg);
	len = PyString_GET_SIZE(arg);
	len2 = (Py_ssize_t)strlen(str);
	if (len2 < len)
		len = len2;
	return PyString_FromStringAndSize(str, len);
}

PyDoc_STRVAR(pack_tags_doc,
"pack_tags(tag, value, ...) -> str\n"
"Pack a TagItem list (pairs of ULONG) ending with TAG_DONE.\n"
"Pass the string to amigalibs via obj2reg/addr for TagList LVOs.");

static PyObject *
amigalibs_pack_tags(PyObject *self, PyObject *args)
{
	Py_ssize_t n;
	Py_ssize_t i;
	PyObject *buf;
	char *p;
	unsigned long tag, val;

	n = PyTuple_GET_SIZE(args);
	if ((n % 2) != 0) {
		PyErr_SetString(PyExc_TypeError,
			"pack_tags requires an even number of arguments");
		return NULL;
	}
	/* n/2 pairs + TAG_DONE */
	buf = PyString_FromStringAndSize(NULL, (Py_ssize_t)((n / 2 + 1) * 8));
	if (buf == NULL)
		return NULL;
	p = PyString_AS_STRING(buf);
	for (i = 0; i < n; i += 2) {
		PyObject *a = PyTuple_GET_ITEM(args, i);
		PyObject *b = PyTuple_GET_ITEM(args, i + 1);

		if (!obj2reg(a, &tag) || !obj2reg(b, &val)) {
			Py_DECREF(buf);
			PyErr_SetString(PyExc_TypeError,
				"pack_tags args must be int/str/None");
			return NULL;
		}
		memcpy(p, &tag, 4);
		memcpy(p + 4, &val, 4);
		p += 8;
	}
	tag = 0; /* TAG_DONE */
	val = 0;
	memcpy(p, &tag, 4);
	memcpy(p + 4, &val, 4);
	return buf;
}

static PyMethodDef amigalibs_methods[] = {
	{"openlib", amigalibs_openlib, METH_VARARGS,
		"openlib(name[, version]) -> amigalib\n"
		"Open an Amiga shared library (or SysBase for exec.library)."},
	{"obj2reg", amigalibs_obj2reg, METH_VARARGS,
		"obj2reg(obj) -> ULONG suitable for a register."},
	{"addr", amigalibs_addr, METH_VARARGS,
		"addr(obj) -> address of int storage or string bytes."},
	{"fixstr", amigalibs_fixstr, METH_VARARGS,
		"fixstr(s) -> s truncated at first NUL."},
	{"pack_tags", amigalibs_pack_tags, METH_VARARGS, pack_tags_doc},
	{NULL, NULL, 0, NULL}
};

void
initamigalibs(void)
{
	PyObject *m;

	Amigalib_Type.ob_type = &PyType_Type;
	m = Py_InitModule3("amigalibs", amigalibs_methods,
		"Secondary raw Amiga LVO FFI. Prefer curated amiga/amigagui APIs.\n"
		"Cannot call A5/A6-special LVOs (e.g. Supervisor, LockLayerRom).");
	if (m == NULL)
		return;

	AmigalibsError = PyErr_NewException("amigalibs.error", NULL, NULL);
	if (AmigalibsError == NULL)
		return;
	Py_INCREF(AmigalibsError);
	PyModule_AddObject(m, "error", AmigalibsError);
}
