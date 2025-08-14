static int get_long ( PyObject *v , long *p );
static int get_ulong ( PyObject *v , unsigned long *p );
static int pack_float ( double x , char *p , int incr );
static int pack_double ( double x , char *p , int incr );
static PyObject *unpack_float ( char *p , int incr );
static PyObject *unpack_double ( char *p , int incr );
static PyObject *struct_calcsize ( PyObject *self , PyObject *args );
static PyObject *struct_pack ( PyObject *self , PyObject *args );
static PyObject *struct_unpack ( PyObject *self , PyObject *args );
