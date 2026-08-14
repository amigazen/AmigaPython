/*
 * _hashlib for Amiga -- wraps crc.library digests (MD5, SHA-1, SHA-256).
 *
 * API matches Modules/_hashopenssl.c enough for Lib/hashlib.py:
 *   new(name[, string]), openssl_md5/sha1/sha256, openssl_md_meth_names
 *
 * Uses only the public one-shot LVOs (DoMD5Sum / DoSHA1 / DoSHA256).
 * Input is kept in a Python string so digest()/copy() need no private
 * streaming-handle clone (CopyMem of CRCNew contexts corrupted the heap).
 *
 * Requires OpenLibrary("crc.library", 2). Missing library => ImportError.
 */

#include "Python.h"
#include "structmember.h"

#include <exec/types.h>
#include <exec/libraries.h>
#include <proto/exec.h>
#include <libraries/crc.h>

/* CRCBase is defined in Amiga/libcheck.c (opened on import _hashlib). */
extern struct Library *CRCBase;
#define __NOLIBBASE__
#include <proto/crc.h>
#undef __NOLIBBASE__

#define HASHCRC_DIGEST_MAX  SIZEOF_SHA256SUM

typedef struct {
    PyObject_HEAD
    ULONG type;             /* CRC_MD5 / CRC_SHA1 / CRC_SHA256 */
    ULONG digest_size;
    ULONG block_size;
    PyObject *name;         /* algorithm name string */
    PyObject *buf;          /* accumulated message bytes */
} HashObject;

static PyTypeObject HashType;

static void
hash_oneshot(ULONG type, const char *data, Py_ssize_t len, UBYTE *digest)
{
    UBYTE *mem;
    LONG size;

    if (data == NULL || len <= 0) {
        mem = (UBYTE *)"";
        size = 0;
    }
    else if (len > 0x7fffffffL) {
        mem = (UBYTE *)data;
        size = 0x7fffffffL;
    }
    else {
        mem = (UBYTE *)data;
        size = (LONG)len;
    }

    if (type == CRC_MD5)
        DoMD5Sum(mem, size, digest);
    else if (type == CRC_SHA1)
        DoSHA1(mem, size, digest);
    else
        DoSHA256(mem, size, digest);
}

static HashObject *
new_hash_object(ULONG type, const char *name, ULONG digest_size,
                ULONG block_size, const char *data, Py_ssize_t len)
{
    HashObject *self;

    self = PyObject_New(HashObject, &HashType);
    if (self == NULL)
        return NULL;

    self->type = type;
    self->digest_size = digest_size;
    self->block_size = block_size;
    self->name = NULL;
    self->buf = NULL;

    self->name = PyString_FromString(name);
    if (self->name == NULL) {
        PyObject_Del(self);
        return NULL;
    }

    if (data != NULL && len > 0)
        self->buf = PyString_FromStringAndSize(data, len);
    else
        self->buf = PyString_FromStringAndSize("", 0);

    if (self->buf == NULL) {
        Py_DECREF(self->name);
        PyObject_Del(self);
        return NULL;
    }

    return self;
}

static void
hash_dealloc(HashObject *self)
{
    Py_XDECREF(self->name);
    Py_XDECREF(self->buf);
    PyObject_Del(self);
}

static PyObject *
hash_repr(HashObject *self)
{
    return PyString_FromFormat("<%s HASH object @ %p>",
                               self->name ? PyString_AsString(self->name)
                                          : "?",
                               (void *)self);
}

static PyObject *
hash_update(HashObject *self, PyObject *args)
{
    Py_buffer view;
    PyObject *piece;

    if (!PyArg_ParseTuple(args, "s*:update", &view))
        return NULL;

    piece = PyString_FromStringAndSize((const char *)view.buf, view.len);
    PyBuffer_Release(&view);
    if (piece == NULL)
        return NULL;

    /* Replaces self->buf; decrefs the old string on success or failure. */
    PyString_Concat(&self->buf, piece);
    Py_DECREF(piece);
    if (self->buf == NULL)
        return NULL;
    Py_RETURN_NONE;
}

static PyObject *
hash_digest(HashObject *self)
{
    UBYTE digest[HASHCRC_DIGEST_MAX];
    const char *data;
    Py_ssize_t len;

    if (self->digest_size > HASHCRC_DIGEST_MAX) {
        PyErr_SetString(PyExc_SystemError, "digest too large");
        return NULL;
    }

    data = PyString_AS_STRING(self->buf);
    len = PyString_GET_SIZE(self->buf);
    hash_oneshot(self->type, data, len, digest);

    return PyString_FromStringAndSize((char *)digest,
                                      (Py_ssize_t)self->digest_size);
}

static PyObject *
hash_hexdigest(HashObject *self)
{
    PyObject *digest;
    PyObject *hex;
    char *src;
    char *dst;
    Py_ssize_t i;
    Py_ssize_t n;
    static const char hexdigits[] = "0123456789abcdef";

    digest = hash_digest(self);
    if (digest == NULL)
        return NULL;

    n = PyString_GET_SIZE(digest);
    hex = PyString_FromStringAndSize(NULL, n * 2);
    if (hex == NULL) {
        Py_DECREF(digest);
        return NULL;
    }

    src = PyString_AS_STRING(digest);
    dst = PyString_AS_STRING(hex);
    for (i = 0; i < n; i++) {
        unsigned char c;

        c = (unsigned char)src[i];
        dst[i * 2] = hexdigits[(c >> 4) & 0xf];
        dst[i * 2 + 1] = hexdigits[c & 0xf];
    }

    Py_DECREF(digest);
    return hex;
}

static PyObject *
hash_copy(HashObject *self)
{
    HashObject *copy;

    copy = PyObject_New(HashObject, &HashType);
    if (copy == NULL)
        return NULL;

    copy->type = self->type;
    copy->digest_size = self->digest_size;
    copy->block_size = self->block_size;
    Py_INCREF(self->name);
    copy->name = self->name;
    copy->buf = PyString_FromStringAndSize(PyString_AS_STRING(self->buf),
                                           PyString_GET_SIZE(self->buf));
    if (copy->buf == NULL) {
        Py_DECREF(copy->name);
        PyObject_Del(copy);
        return NULL;
    }
    return (PyObject *)copy;
}

static PyMethodDef hash_methods[] = {
    {"update",    (PyCFunction)hash_update,    METH_VARARGS,
     PyDoc_STR("Update this hash object's state with the provided string.")},
    {"digest",    (PyCFunction)hash_digest,    METH_NOARGS,
     PyDoc_STR("Return the digest value as a string of binary data.")},
    {"hexdigest", (PyCFunction)hash_hexdigest, METH_NOARGS,
     PyDoc_STR("Return the digest value as a string of hexadecimal digits.")},
    {"copy",      (PyCFunction)hash_copy,      METH_NOARGS,
     PyDoc_STR("Return a copy of the hash object.")},
    {NULL, NULL}  /* sentinel */
};

static PyMemberDef hash_members[] = {
    {"digest_size", T_LONG, offsetof(HashObject, digest_size), READONLY, NULL},
    {"block_size",  T_LONG, offsetof(HashObject, block_size),  READONLY, NULL},
    {"name", T_OBJECT, offsetof(HashObject, name), READONLY, NULL},
    {NULL}  /* sentinel */
};

static PyTypeObject HashType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "_hashlib.HASH",            /*tp_name*/
    sizeof(HashObject),         /*tp_basicsize*/
    0,                          /*tp_itemsize*/
    (destructor)hash_dealloc,   /*tp_dealloc*/
    0,                          /*tp_print*/
    0,                          /*tp_getattr*/
    0,                          /*tp_setattr*/
    0,                          /*tp_compare*/
    (reprfunc)hash_repr,        /*tp_repr*/
    0,                          /*tp_as_number*/
    0,                          /*tp_as_sequence*/
    0,                          /*tp_as_mapping*/
    0,                          /*tp_hash*/
    0,                          /*tp_call*/
    0,                          /*tp_str*/
    0,                          /*tp_getattro*/
    0,                          /*tp_setattro*/
    0,                          /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,         /*tp_flags*/
    0,                          /*tp_doc*/
    0,                          /*tp_traverse*/
    0,                          /*tp_clear*/
    0,                          /*tp_richcompare*/
    0,                          /*tp_weaklistoffset*/
    0,                          /*tp_iter*/
    0,                          /*tp_iternext*/
    hash_methods,               /*tp_methods*/
    hash_members,               /*tp_members*/
};

static PyObject *
HASH_new_typed(ULONG type, const char *name, ULONG digest_size,
               ULONG block_size, PyObject *args)
{
    Py_buffer view;
    HashObject *self;
    const char *data;
    Py_ssize_t len;
    int have_view;

    data = NULL;
    len = 0;
    have_view = 0;

    if (PyTuple_GET_SIZE(args) != 0) {
        if (!PyArg_ParseTuple(args, "s*:HASH", &view))
            return NULL;
        have_view = 1;
        data = (const char *)view.buf;
        len = view.len;
    }

    self = new_hash_object(type, name, digest_size, block_size, data, len);
    if (have_view)
        PyBuffer_Release(&view);

    return (PyObject *)self;
}

static PyObject *
openssl_md5(PyObject *self, PyObject *args)
{
    return HASH_new_typed(CRC_MD5, "md5", SIZEOF_MD5SUM, 64, args);
}

static PyObject *
openssl_sha1(PyObject *self, PyObject *args)
{
    return HASH_new_typed(CRC_SHA1, "sha1", SIZEOF_SHA1SUM, 64, args);
}

static PyObject *
openssl_sha256(PyObject *self, PyObject *args)
{
    return HASH_new_typed(CRC_SHA256, "sha256", SIZEOF_SHA256SUM, 64, args);
}

static PyObject *
hashlib_new(PyObject *self, PyObject *args)
{
    char *name;
    PyObject *data_obj;
    Py_buffer view;
    HashObject *obj;
    const char *data;
    Py_ssize_t len;
    ULONG type;
    ULONG digest_size;
    const char *canon;
    int have_view;

    data_obj = NULL;
    data = NULL;
    len = 0;
    have_view = 0;

    if (!PyArg_ParseTuple(args, "s|O:new", &name, &data_obj))
        return NULL;

    if (strcmp(name, "md5") == 0 || strcmp(name, "MD5") == 0) {
        type = CRC_MD5;
        digest_size = SIZEOF_MD5SUM;
        canon = "md5";
    }
    else if (strcmp(name, "sha1") == 0 || strcmp(name, "SHA1") == 0 ||
             strcmp(name, "sha") == 0 || strcmp(name, "SHA") == 0) {
        type = CRC_SHA1;
        digest_size = SIZEOF_SHA1SUM;
        canon = "sha1";
    }
    else if (strcmp(name, "sha256") == 0 || strcmp(name, "SHA256") == 0) {
        type = CRC_SHA256;
        digest_size = SIZEOF_SHA256SUM;
        canon = "sha256";
    }
    else {
        PyErr_Format(PyExc_ValueError, "unsupported hash type %s", name);
        return NULL;
    }

    if (data_obj != NULL) {
        if (PyObject_GetBuffer(data_obj, &view, PyBUF_SIMPLE) < 0)
            return NULL;
        have_view = 1;
        data = (const char *)view.buf;
        len = view.len;
    }

    obj = new_hash_object(type, canon, digest_size, 64, data, len);
    if (have_view)
        PyBuffer_Release(&view);
    return (PyObject *)obj;
}

static PyMethodDef module_methods[] = {
    {"new",           (PyCFunction)hashlib_new,    METH_VARARGS,
     PyDoc_STR("new(name, string='') - Return a new hashing object using "
               "the named crc.library algorithm (md5, sha1, sha256).")},
    {"openssl_md5",   (PyCFunction)openssl_md5,    METH_VARARGS,
     PyDoc_STR("Returns a md5 hash object; optionally initialized with a string")},
    {"openssl_sha1",  (PyCFunction)openssl_sha1,   METH_VARARGS,
     PyDoc_STR("Returns a sha1 hash object; optionally initialized with a string")},
    {"openssl_sha256",(PyCFunction)openssl_sha256, METH_VARARGS,
     PyDoc_STR("Returns a sha256 hash object; optionally initialized with a string")},
    {NULL, NULL}  /* sentinel */
};

PyMODINIT_FUNC
init_hashlib(void)
{
    PyObject *m;
    PyObject *names;

    if (CRCBase == NULL) {
        CRCBase = OpenLibrary((STRPTR)"crc.library", 2);
        if (CRCBase == NULL) {
            PyErr_SetString(PyExc_ImportError,
                            "crc.library 2+ required for _hashlib "
                            "(MD5/SHA-1/SHA-256)");
            return;
        }
    }

    Py_TYPE(&HashType) = &PyType_Type;
    if (PyType_Ready(&HashType) < 0)
        return;

    m = Py_InitModule("_hashlib", module_methods);
    if (m == NULL)
        return;

    {
        PyObject *tmp;
        PyObject *s;
        int i;
        static const char *const algos[] = {"md5", "sha1", "sha256"};

        tmp = PySet_New(NULL);
        if (tmp == NULL)
            return;
        for (i = 0; i < 3; i++) {
            s = PyString_FromString(algos[i]);
            if (s == NULL) {
                Py_DECREF(tmp);
                return;
            }
            if (PySet_Add(tmp, s) < 0) {
                Py_DECREF(s);
                Py_DECREF(tmp);
                return;
            }
            Py_DECREF(s);
        }
        names = PyFrozenSet_New(tmp);
        Py_DECREF(tmp);
        if (names == NULL)
            return;
    }
    if (PyModule_AddObject(m, "openssl_md_meth_names", names) < 0) {
        Py_DECREF(names);
        return;
    }
}
