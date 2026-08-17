/*
 * Python C API trampolines for LoadSeg plugins (call through PyHost).
 * Varargs APIs are implemented here via Va* host entries.
 */
#include <stdarg.h>
#include <exec/execbase.h>

#include "pyamiga_plugin.h"
#include "pyamiga_redir.h"

/* Ensure trampoline definitions are not eaten by Python.h macros. */
#undef Py_InitModule3
#undef Py_InitModule
#undef PyArg_Parse
#undef PyArg_ParseTuple
#undef PyArg_ParseTupleAndKeywords
#undef Py_BuildValue
#undef PyObject_Free
#undef PyObject_Del
#undef PyMem_Malloc
#undef PyMem_Free
#undef PyCapsule_Import
#undef PyDict_New
#undef PyDict_SetItemString
#undef PyBool_FromLong
#undef PyObject_IsTrue
#undef PyString_Size
#undef PyModule_GetDict
#undef PyObject_GetAttrString
#undef PyObject_SetAttrString
#undef PyObject_AsWriteBuffer
#undef _PyObject_New

struct PyHost *PyAmiga_Host = NULL;

/* Defined in pyamiga_minirt.c for -lmieee */
extern struct ExecBase *SysBase;

PyObject *
_PyObject_New(PyTypeObject *t)
{
    return PyAmiga_Host->fn__PyObject_New(t);
}


void
PyObject_Del(void *op)
{
    PyAmiga_Host->fn_object_dealloc((PyObject *)op);
}


PyObject *
PyDict_New(void)
{
    return PyAmiga_Host->fn_PyDict_New();
}


int
PyDict_SetItemString(PyObject *d, const char *k, PyObject *v)
{
    return PyAmiga_Host->fn_PyDict_SetItemString(d, k, v);
}


PyObject *
PyBool_FromLong(long v)
{
    return PyAmiga_Host->fn_PyBool_FromLong(v);
}


int
PyObject_IsTrue(PyObject *o)
{
    return PyAmiga_Host->fn_PyObject_IsTrue(o);
}


Py_ssize_t
PyString_Size(PyObject *o)
{
    return PyAmiga_Host->fn_PyString_Size(o);
}


PyObject *
PyModule_GetDict(PyObject *m)
{
    return PyAmiga_Host->fn_PyModule_GetDict(m);
}


PyObject *
PyObject_GetAttrString(PyObject *o, const char *n)
{
    return PyAmiga_Host->fn_PyObject_GetAttrString(o, n);
}


int
PyObject_SetAttrString(PyObject *o, const char *n, PyObject *v)
{
    return PyAmiga_Host->fn_PyObject_SetAttrString(o, n, v);
}


int
PyObject_AsWriteBuffer(PyObject *o, void **buf, Py_ssize_t *len)
{
    return PyAmiga_Host->fn_PyObject_AsWriteBuffer(o, buf, len);
}


void *
PyCapsule_Import(const char *name, int no_block)
{
    return PyAmiga_Host->fn_PyCapsule_Import(name, no_block);
}


void
PyAmiga_InstallHost(struct PyHost *host)
{
    PyAmiga_Host = host;
    if (host != NULL && host->sysbase != NULL)
        SysBase = (struct ExecBase *)host->sysbase;
}


void
PyAmiga_Note(const char *msg)
{
    if (PyAmiga_Host != NULL && PyAmiga_Host->fn_note != NULL)
        PyAmiga_Host->fn_note(msg);
}


PyObject *
Py_InitModule3(char *name, PyMethodDef *methods, char *doc)
{
    return PyAmiga_Host->fn_Py_InitModule3(name, methods, doc);
}


void
PyErr_SetString(PyObject *t, const char *m)
{
    PyAmiga_Host->fn_PyErr_SetString(t, m);
}


void
PyErr_SetObject(PyObject *t, PyObject *v)
{
    PyAmiga_Host->fn_PyErr_SetObject(t, v);
}


void
PyErr_Clear(void)
{
    PyAmiga_Host->fn_PyErr_Clear();
}


PyObject *
PyErr_Occurred(void)
{
    return PyAmiga_Host->fn_PyErr_Occurred();
}


int
PyErr_CheckSignals(void)
{
    return PyAmiga_Host->fn_PyErr_CheckSignals();
}


PyObject *
PyErr_SetFromErrno(PyObject *t)
{
    return PyAmiga_Host->fn_PyErr_SetFromErrno(t);
}


PyObject *
PyErr_SetFromErrnoWithFilenameObject(PyObject *t, PyObject *fn)
{
    return PyAmiga_Host->fn_PyErr_SetFromErrnoWithFilenameObject(t, fn);
}


PyObject *
PyErr_NewException(char *name, PyObject *base, PyObject *dict)
{
    return PyAmiga_Host->fn_PyErr_NewException(name, base, dict);
}


PyObject *
PyInt_FromLong(long v)
{
    return PyAmiga_Host->fn_PyInt_FromLong(v);
}


long
PyInt_AsLong(PyObject *o)
{
    return PyAmiga_Host->fn_PyInt_AsLong(o);
}


PyObject *
PyInt_FromSsize_t(Py_ssize_t v)
{
    return PyAmiga_Host->fn_PyInt_FromSsize_t(v);
}


int
_PyInt_AsInt(PyObject *o)
{
    return PyAmiga_Host->fn__PyInt_AsInt(o);
}


PyObject *
PyLong_FromLong(long v)
{
    return PyAmiga_Host->fn_PyLong_FromLong(v);
}


PyObject *
PyLong_FromUnsignedLong(unsigned long v)
{
    return PyAmiga_Host->fn_PyLong_FromUnsignedLong(v);
}


long
PyLong_AsLong(PyObject *o)
{
    return PyAmiga_Host->fn_PyLong_AsLong(o);
}


unsigned long
PyLong_AsUnsignedLong(PyObject *o)
{
    return PyAmiga_Host->fn_PyLong_AsUnsignedLong(o);
}


PyObject *
PyLong_FromLongLong(PY_LONG_LONG v)
{
    return PyAmiga_Host->fn_PyLong_FromLongLong(v);
}


PyObject *
PyFloat_FromDouble(double v)
{
    return PyAmiga_Host->fn_PyFloat_FromDouble(v);
}


double
PyFloat_AsDouble(PyObject *o)
{
    return PyAmiga_Host->fn_PyFloat_AsDouble(o);
}


PyObject *
PyString_FromString(const char *s)
{
    return PyAmiga_Host->fn_PyString_FromString(s);
}


PyObject *
PyString_FromStringAndSize(const char *s, Py_ssize_t n)
{
    return PyAmiga_Host->fn_PyString_FromStringAndSize(s, n);
}


char *
PyString_AsString(PyObject *o)
{
    return PyAmiga_Host->fn_PyString_AsString(o);
}


PyObject *
PyString_FromFormatV(const char *f, va_list v)
{
    return PyAmiga_Host->fn_PyString_FromFormatV(f, v);
}


int
_PyString_Resize(PyObject **pv, Py_ssize_t n)
{
    return PyAmiga_Host->fn__PyString_Resize(pv, n);
}


PyObject *
PyTuple_New(Py_ssize_t n)
{
    return PyAmiga_Host->fn_PyTuple_New(n);
}


Py_ssize_t
PyTuple_Size(PyObject *o)
{
    return PyAmiga_Host->fn_PyTuple_Size(o);
}


PyObject *
PyList_New(Py_ssize_t n)
{
    return PyAmiga_Host->fn_PyList_New(n);
}


int
PyList_Append(PyObject *l, PyObject *i)
{
    return PyAmiga_Host->fn_PyList_Append(l, i);
}


int
PyModule_AddObject(PyObject *m, const char *n, PyObject *o)
{
    return PyAmiga_Host->fn_PyModule_AddObject(m, n, o);
}


int
PyModule_AddIntConstant(PyObject *m, const char *n, long v)
{
    return PyAmiga_Host->fn_PyModule_AddIntConstant(m, n, v);
}


int
PyModule_AddStringConstant(PyObject *m, const char *n, const char *v)
{
    return PyAmiga_Host->fn_PyModule_AddStringConstant(m, n, v);
}


int
PyType_Ready(PyTypeObject *t)
{
    return PyAmiga_Host->fn_PyType_Ready(t);
}


PyObject *
PyType_GenericNew(PyTypeObject *t, PyObject *a, PyObject *k)
{
    return PyAmiga_Host->fn_PyType_GenericNew(t, a, k);
}


PyObject *
PyType_GenericAlloc(PyTypeObject *t, Py_ssize_t n)
{
    return PyAmiga_Host->fn_PyType_GenericAlloc(t, n);
}


PyObject *
PyObject_GenericGetAttr(PyObject *o, PyObject *n)
{
    return PyAmiga_Host->fn_PyObject_GenericGetAttr(o, n);
}


void
PyObject_ClearWeakRefs(PyObject *o)
{
    PyAmiga_Host->fn_PyObject_ClearWeakRefs(o);
}


void
PyObject_Free(void *p)
{
    PyAmiga_Host->fn_PyObject_Free(p);
}


void *
PyMem_Malloc(size_t n)
{
    return PyAmiga_Host->fn_PyMem_Malloc(n);
}


void
PyMem_Free(void *p)
{
    PyAmiga_Host->fn_PyMem_Free(p);
}


int
Py_AtExit(void (*func)(void))
{
    return PyAmiga_Host->fn_Py_AtExit(func);
}


PyObject *
PyCapsule_New(void *p, const char *n, PyCapsule_Destructor d)
{
    return PyAmiga_Host->fn_PyCapsule_New(p, n, d);
}


PyObject *
PyFile_FromFile(FILE *f, char *n, char *m, int (*c)(FILE *))
{
    return PyAmiga_Host->fn_PyFile_FromFile(f, n, m, c);
}


void
PyFile_SetBufSize(PyObject *f, int b)
{
    PyAmiga_Host->fn_PyFile_SetBufSize(f, b);
}


void
PyBuffer_Release(Py_buffer *v)
{
    PyAmiga_Host->fn_PyBuffer_Release(v);
}


double
_PyTime_FloatTime(void)
{
    return PyAmiga_Host->fn__PyTime_FloatTime();
}


int
PyArg_VaParse(PyObject *args, const char *format, va_list v)
{
    return PyAmiga_Host->fn_PyArg_VaParse(args, format, v);
}


int
PyArg_VaParseTupleAndKeywords(PyObject *args, PyObject *kw, const char *format, char **keywords, va_list v)
{
    return PyAmiga_Host->fn_PyArg_VaParseTupleAndKeywords(args, kw, format, keywords, v);
}


PyObject *
Py_VaBuildValue(const char *format, va_list v)
{
    return PyAmiga_Host->fn_Py_VaBuildValue(format, v);
}


int
PyOS_vsnprintf(char *str, size_t size, const char *format, va_list v)
{
    return PyAmiga_Host->fn_PyOS_vsnprintf(str, size, format, v);
}


int
PyArg_Parse(PyObject *args, const char *format, ...)
{
    int r;
    va_list vargs;

    /* Host trampoline wraps a non-tuple (FLAG_COMPAT / getsetters). */
    va_start(vargs, format);
    r = PyAmiga_Host->fn_PyArg_VaParse(args, format, vargs);
    va_end(vargs);
    return r;
}

int
PyArg_ParseTuple(PyObject *args, const char *format, ...)
{
    int r;
    va_list vargs;

    va_start(vargs, format);
    r = PyAmiga_Host->fn_PyArg_VaParse(args, format, vargs);
    va_end(vargs);
    return r;
}

int
PyArg_ParseTupleAndKeywords(PyObject *args, PyObject *kw, const char *format,
                            char **keywords, ...)
{
    int r;
    va_list vargs;

    va_start(vargs, keywords);
    r = PyAmiga_Host->fn_PyArg_VaParseTupleAndKeywords(args, kw, format,
                                                       keywords, vargs);
    va_end(vargs);
    return r;
}

PyObject *
Py_BuildValue(const char *format, ...)
{
    PyObject *r;
    va_list vargs;

    va_start(vargs, format);
    r = PyAmiga_Host->fn_Py_VaBuildValue(format, vargs);
    va_end(vargs);
    return r;
}

PyObject *
PyErr_Format(PyObject *exc, const char *format, ...)
{
    va_list vargs;
    PyObject *str;

    va_start(vargs, format);
    str = PyAmiga_Host->fn_PyString_FromFormatV(format, vargs);
    va_end(vargs);
    if (str != NULL) {
        PyAmiga_Host->fn_PyErr_SetObject(exc, str);
        Py_DECREF(str);
    }
    return NULL;
}

int
PyOS_snprintf(char *str, size_t size, const char *format, ...)
{
    int r;
    va_list vargs;

    va_start(vargs, format);
    r = PyAmiga_Host->fn_PyOS_vsnprintf(str, size, format, vargs);
    va_end(vargs);
    return r;
}

PyObject *
PyTuple_Pack(Py_ssize_t n, ...)
{
    PyObject *result;
    PyObject *o;
    Py_ssize_t i;
    va_list vargs;

    result = PyAmiga_Host->fn_PyTuple_New(n);
    if (result == NULL)
        return NULL;
    va_start(vargs, n);
    for (i = 0; i < n; i++) {
        o = va_arg(vargs, PyObject *);
        Py_INCREF(o);
        PyTuple_SET_ITEM(result, i, o);
    }
    va_end(vargs);
    return result;
}
