static PyObject *func_getattr ( PyFunctionObject *op , char *name );
static int func_setattr ( PyFunctionObject *op , char *name , PyObject *value );
static void func_dealloc ( PyFunctionObject *op );
static PyObject *func_repr ( PyFunctionObject *op );
static int func_compare ( PyFunctionObject *f , PyFunctionObject *g );
static long func_hash ( PyFunctionObject *f );
