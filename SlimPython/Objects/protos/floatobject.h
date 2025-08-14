
void PyFloat_AsString(char *buf, PyFloatObject *v);

static PyFloatObject *fill_free_list ( void );
static void float_dealloc ( PyFloatObject *op );
static int float_print ( PyFloatObject *v , FILE *fp , int flags );
static PyObject *float_repr ( PyFloatObject *v );
static int float_compare ( PyFloatObject *v , PyFloatObject *w );
static long float_hash ( PyFloatObject *v );
static PyObject *float_add ( PyFloatObject *v , PyFloatObject *w );
static PyObject *float_sub ( PyFloatObject *v , PyFloatObject *w );
static PyObject *float_mul ( PyFloatObject *v , PyFloatObject *w );
static PyObject *float_div ( PyFloatObject *v , PyFloatObject *w );
static PyObject *float_rem ( PyFloatObject *v , PyFloatObject *w );
static PyObject *float_divmod ( PyFloatObject *v , PyFloatObject *w );
static double powu ( double x , long n );
static PyObject *float_pow ( PyFloatObject *v , PyObject *w , PyFloatObject *z );
static PyObject *float_neg ( PyFloatObject *v );
static PyObject *float_pos ( PyFloatObject *v );
static PyObject *float_abs ( PyFloatObject *v );
static int float_nonzero ( PyFloatObject *v );
static int float_coerce ( PyObject **pv , PyObject **pw );
static PyObject *float_int ( PyObject *v );
static PyObject *float_long ( PyObject *v );
static PyObject *float_float ( PyObject *v );
