static PcreObject *newPcreObject ( PyObject *arg );
static void PyPcre_dealloc ( PcreObject *self );
static PyObject *PyPcre_exec ( PcreObject *self , PyObject *args );
static PyObject *PyPcre_getattr ( PcreObject *self , char *name );
static PyObject *PyPcre_compile ( PyObject *self , PyObject *args );
static PyObject *PyPcre_expand_escape ( unsigned char *pattern , int pattern_len , int *indexptr , int *typeptr );
static PyObject *PyPcre_expand ( PyObject *self , PyObject *args );
static void insint ( PyObject *d , char *name , int value );
