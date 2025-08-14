static PyObject *math_error ( void );
static PyObject *math_1 ( PyObject *args , double (*func )Py_FPROTO ((double )));
static PyObject *math_2 ( PyObject *args , double (*func )Py_FPROTO ((double ,double )));
static PyObject *math_frexp (PyObject *self , PyObject *args );
static PyObject *math_ldexp ( PyObject *self , PyObject *args );
static PyObject *math_modf ( PyObject *self , PyObject *args );
