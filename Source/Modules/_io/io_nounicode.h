/*
 * Amiga / no-Unicode adapters for Modules/_io.
 *
 * When Py_USING_UNICODE is off, text I/O uses 8-bit PyString. Macros map the
 * Unicode APIs that _io calls onto string equivalents; helpers that need
 * real code live in io_nounicode.c.
 *
 * C89 / ANSI C.
 */
#ifndef Py_IO_NOUNICODE_H
#define Py_IO_NOUNICODE_H

#ifndef Py_USING_UNICODE

/* Text unit is one byte (latin-1 / ascii identity). */
#ifndef Py_UNICODE
typedef char Py_UNICODE;
#endif

#undef PyUnicode_Check
#define PyUnicode_Check(op) PyString_Check(op)

#undef PyUnicode_GET_SIZE
#define PyUnicode_GET_SIZE(op) PyString_GET_SIZE(op)

#undef PyUnicode_GetSize
#define PyUnicode_GetSize(op) PyString_Size(op)

#undef PyUnicode_AS_UNICODE
#define PyUnicode_AS_UNICODE(op) ((Py_UNICODE *)PyString_AS_STRING(op))

#undef PyUnicode_FromString
#define PyUnicode_FromString(s) PyString_FromString(s)

#undef PyUnicode_FromStringAndSize
#define PyUnicode_FromStringAndSize(s, n) PyString_FromStringAndSize((s), (n))

/* FromUnicode(NULL, n) allocates an uninitialized buffer of length n. */
#undef PyUnicode_FromUnicode
#define PyUnicode_FromUnicode(u, n) \
    (((u) == NULL) \
     ? PyString_FromStringAndSize(NULL, (n)) \
     : PyString_FromStringAndSize((const char *)(u), (n)))

#undef PyUnicode_Resize
#define PyUnicode_Resize(p, n) _PyString_Resize((p), (n))

#undef PyUnicode_Concat
#define PyUnicode_Concat(a, b) _PyIO_NoUni_Concat((a), (b))

#undef PyUnicode_Join
#define PyUnicode_Join(sep, seq) _PyString_Join((sep), (seq))

#undef PyUnicode_Replace
#define PyUnicode_Replace(str, old, newv, count) \
    _PyIO_NoUni_Replace((str), (old), (newv), (count))

#undef PyUnicode_EncodeASCII
#define PyUnicode_EncodeASCII(s, size, errors) \
    _PyIO_NoUni_Encode8Bit((s), (size), (errors), 1)

#undef PyUnicode_EncodeLatin1
#define PyUnicode_EncodeLatin1(s, size, errors) \
    _PyIO_NoUni_Encode8Bit((s), (size), (errors), 0)

#undef PyUnicode_EncodeUTF8
#define PyUnicode_EncodeUTF8(s, size, errors) \
    _PyIO_NoUni_Encode8Bit((s), (size), (errors), 0)

#undef PyUnicode_EncodeUTF16
#define PyUnicode_EncodeUTF16(s, size, errors, byteorder) \
    _PyIO_NoUni_EncodeUnsupported("utf-16")

#undef PyUnicode_EncodeUTF32
#define PyUnicode_EncodeUTF32(s, size, errors, byteorder) \
    _PyIO_NoUni_EncodeUnsupported("utf-32")

#undef PyUnicode_FromObject
#define PyUnicode_FromObject(o) _PyIO_NoUni_FromObject(o)

#undef PyUnicode_AsEncodedString
#define PyUnicode_AsEncodedString(unicode, encoding, errors) \
    _PyIO_NoUni_AsEncodedString((unicode), (encoding), (errors))

/* Helpers (io_nounicode.c) */
PyObject *_PyIO_NoUni_Concat(PyObject *left, PyObject *right);
PyObject *_PyIO_NoUni_Replace(PyObject *str, PyObject *old,
                              PyObject *newv, Py_ssize_t count);
PyObject *_PyIO_NoUni_Encode8Bit(const Py_UNICODE *s, Py_ssize_t size,
                                 const char *errors, int ascii_only);
PyObject *_PyIO_NoUni_EncodeUnsupported(const char *name);
PyObject *_PyIO_NoUni_FromObject(PyObject *o);
PyObject *_PyIO_NoUni_AsEncodedString(PyObject *unicode,
                                      const char *encoding,
                                      const char *errors);
int _PyIO_NoUni_EncodingAllowed(const char *encoding);

#endif /* !Py_USING_UNICODE */

#endif /* Py_IO_NOUNICODE_H */
