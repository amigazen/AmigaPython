/* cryptmodule.c - by Steve Majewski
 *
 * Amiga: crypt() is an AmiTCP usergroup.library LVO. Include
 * proto/usergroup.h only after have_usergrouplib() has opened a real
 * UserGroupBase — never call through a dummy base.
 */

#include "Python.h"

#include <sys/types.h>

#ifdef _AMIGA
#include "libcheck.h"
#include <proto/usergroup.h>
#endif

#ifdef __VMS
#include <openssl/des.h>
#endif

/* Module crypt */


static PyObject *crypt_crypt(PyObject *self, PyObject *args)
{
    char *word, *salt;
    char *result;
#ifndef __VMS
#ifndef _AMIGA
    extern char * crypt(const char *, const char *);
#endif
#endif

    if (!PyArg_ParseTuple(args, "ss:crypt", &word, &salt)) {
        return NULL;
    }
#ifdef _AMIGA
    if (!checkusergrouplib())
        return NULL;
#endif
    /* On some platforms (AtheOS) crypt returns NULL for an invalid
       salt. Return None in that case. XXX Maybe raise an exception?  */
    result = crypt(word, salt);
    if (result == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return Py_BuildValue("s", result);

}

PyDoc_STRVAR(crypt_crypt__doc__,
"crypt(word, salt) -> string\n\
word will usually be a user's password. salt is a 2-character string\n\
which will be used to select one of 4096 variations of DES. The characters\n\
in salt must be either \".\", \"/\", or an alphanumeric character. Returns\n\
the hashed password as a string, which will be composed of characters from\n\
the same alphabet as the salt.");


static PyMethodDef crypt_methods[] = {
    {"crypt",           crypt_crypt, METH_VARARGS, crypt_crypt__doc__},
    {NULL,              NULL}           /* sentinel */
};

PyMODINIT_FUNC
initcrypt(void)
{
    Py_InitModule("crypt", crypt_methods);
}
