static PyObject *tb_getattr ( tracebackobject *tb , char *name );
static void tb_dealloc ( tracebackobject *tb );
static tracebackobject *newtracebackobject ( tracebackobject *next , PyFrameObject *frame , int lasti , int lineno );
static int tb_displayline ( PyObject *f , char *filename , int lineno , char *name );
static int tb_printinternal ( tracebackobject *tb , PyObject *f , int limit );
