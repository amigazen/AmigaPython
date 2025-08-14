static void meth_dealloc ( PyCFunctionObject *m );
static PyObject *meth_getattr ( PyCFunctionObject *m , char *name );
static PyObject *meth_repr ( PyCFunctionObject *m );
static int meth_compare ( PyCFunctionObject *a , PyCFunctionObject *b );
static long meth_hash ( PyCFunctionObject *a );
static PyObject *listmethodchain ( PyMethodChain *chain );
