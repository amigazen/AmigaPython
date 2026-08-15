/*
 * Helpers for Modules/_io when Py_USING_UNICODE is disabled.
 * C89 / ANSI C.
 */

#include "Python.h"

#ifndef Py_USING_UNICODE

#include "io_nounicode.h"
#include <string.h>

PyObject *
_PyIO_NoUni_Concat(PyObject *left, PyObject *right)
{
    PyObject *result;

    result = left;
    Py_INCREF(result);
    PyString_Concat(&result, right);
    return result;
}

PyObject *
_PyIO_NoUni_Replace(PyObject *str, PyObject *old, PyObject *newv,
                    Py_ssize_t count)
{
    /* Delegate to str.replace; keeps C small and matches unicode semantics
       for 8-bit strings. */
    return PyObject_CallMethod(str, "replace", "OOn", old, newv, count);
}

PyObject *
_PyIO_NoUni_Encode8Bit(const Py_UNICODE *s, Py_ssize_t size,
                       const char *errors, int ascii_only)
{
    Py_ssize_t i;
    const char *err;

    err = errors;
    if (err == NULL)
        err = "strict";

    if (ascii_only) {
        for (i = 0; i < size; i++) {
            if ((unsigned char)s[i] > 127) {
                if (strcmp(err, "strict") == 0) {
                    /* UnicodeEncodeError is not built without Py_USING_UNICODE */
                    PyErr_SetString(PyExc_ValueError,
                                    "ascii codec can't encode character");
                    return NULL;
                }
                /* non-strict: still copy bytes (Amiga 8-bit text path) */
                break;
            }
        }
    }
    return PyBytes_FromStringAndSize((const char *)s, size);
}

PyObject *
_PyIO_NoUni_EncodeUnsupported(const char *name)
{
    PyErr_Format(PyExc_ValueError,
                 "encoding '%s' requires Unicode (disabled on this build)",
                 name);
    return NULL;
}

PyObject *
_PyIO_NoUni_FromObject(PyObject *o)
{
    if (o == NULL) {
        PyErr_BadInternalCall();
        return NULL;
    }
    if (PyString_Check(o)) {
        Py_INCREF(o);
        return o;
    }
    PyErr_Format(PyExc_TypeError,
                 "coercing to str: need string, %.200s found",
                 Py_TYPE(o)->tp_name);
    return NULL;
}

PyObject *
_PyIO_NoUni_AsEncodedString(PyObject *unicode, const char *encoding,
                            const char *errors)
{
    /* Identity: text is already 8-bit bytes. */
    (void)encoding;
    (void)errors;
    if (!PyString_Check(unicode)) {
        PyErr_SetString(PyExc_TypeError, "string expected");
        return NULL;
    }
    Py_INCREF(unicode);
    return unicode;
}

int
_PyIO_NoUni_EncodingAllowed(const char *encoding)
{
    char buf[64];
    size_t i;
    size_t n;

    if (encoding == NULL)
        return 1;

    n = strlen(encoding);
    if (n >= sizeof(buf))
        return 0;
    for (i = 0; i < n; i++) {
        char c = encoding[i];
        if (c >= 'A' && c <= 'Z')
            c = (char)(c - 'A' + 'a');
        if (c == '_')
            c = '-';
        buf[i] = c;
    }
    buf[n] = '\0';

    if (strcmp(buf, "ascii") == 0)
        return 1;
    if (strcmp(buf, "latin-1") == 0 || strcmp(buf, "latin1") == 0)
        return 1;
    if (strcmp(buf, "iso-8859-1") == 0 || strcmp(buf, "iso8859-1") == 0)
        return 1;
    /* utf-8 accepted as 8-bit pass-through (no multi-byte validation). */
    if (strcmp(buf, "utf-8") == 0 || strcmp(buf, "utf8") == 0)
        return 1;
    return 0;
}

#endif /* !Py_USING_UNICODE */
