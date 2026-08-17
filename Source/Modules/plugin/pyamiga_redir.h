/* Redirect singletons/types/errno to host pointers for LoadSeg plugins. */
#ifndef PYAMIGA_REDIR_H
#define PYAMIGA_REDIR_H

#ifndef PYAMIGA_HOST_BUILD

#undef Py_None
#undef Py_True
#undef Py_False
#define Py_None  (PyAmiga_Host->obj_None)
#define Py_True  (PyAmiga_Host->obj_True)
#define Py_False (PyAmiga_Host->obj_False)

/* Py_InitModule3 is a macro wrapping Py_InitModule4 — use our stub. */
#undef Py_InitModule3
#undef Py_InitModule
PyObject *Py_InitModule3(char *name, PyMethodDef *methods, char *doc);

#undef PyExc_ImportError
#undef PyExc_RuntimeError
#undef PyExc_SystemError
#undef PyExc_ValueError
#undef PyExc_TypeError
#undef PyExc_OverflowError
#undef PyExc_OSError
#undef PyExc_IOError
#undef PyExc_MemoryError
#undef PyExc_AttributeError
#undef PyExc_KeyboardInterrupt
#undef PyExc_NotImplementedError
#define PyExc_ImportError          (PyAmiga_Host->obj_ImportError)
#define PyExc_RuntimeError         (PyAmiga_Host->obj_RuntimeError)
#define PyExc_SystemError          (PyAmiga_Host->obj_SystemError)
#define PyExc_ValueError           (PyAmiga_Host->obj_ValueError)
#define PyExc_TypeError            (PyAmiga_Host->obj_TypeError)
#define PyExc_OverflowError        (PyAmiga_Host->obj_OverflowError)
#define PyExc_OSError              (PyAmiga_Host->obj_OSError)
#define PyExc_IOError              (PyAmiga_Host->obj_IOError)
#define PyExc_MemoryError          (PyAmiga_Host->obj_MemoryError)
#define PyExc_AttributeError       (PyAmiga_Host->obj_AttributeError)
#define PyExc_KeyboardInterrupt    (PyAmiga_Host->obj_KeyboardInterrupt)
#define PyExc_NotImplementedError  (PyAmiga_Host->obj_NotImplementedError)

#undef PyType_Type
#undef PyInt_Type
#undef PyLong_Type
#undef PyString_Type
#undef PyTuple_Type
#undef PyList_Type
#undef PyFloat_Type
#define PyType_Type   (*PyAmiga_Host->type_Type)
#define PyInt_Type    (*PyAmiga_Host->type_Int)
#define PyLong_Type   (*PyAmiga_Host->type_Long)
#define PyString_Type (*PyAmiga_Host->type_String)
#define PyTuple_Type  (*PyAmiga_Host->type_Tuple)
#define PyList_Type   (*PyAmiga_Host->type_List)
#define PyFloat_Type  (*PyAmiga_Host->type_Float)

#undef errno
#define errno (*PyAmiga_Host->ptr_errno)

#undef h_errno
#define h_errno (*PyAmiga_Host->ptr_h_errno)

/*
 * Host-heap PyObjects must not be dissected with layout macros compiled
 * into the LoadSeg image. Wrong ob_sval offsets make recv() write into
 * random heap (binary "HTTP" bodies + "GC object already tracked").
 */
#undef PyString_AS_STRING
#define PyString_AS_STRING(op) PyString_AsString((PyObject *)(op))

/* Do not use _Py_AS_GC / _PyObject_GC_UNTRACK in plugins; call
 * PyObject_Del (fn_object_dealloc on the host) from tp_dealloc. */

#endif /* !PYAMIGA_HOST_BUILD */

#endif /* PYAMIGA_REDIR_H */
