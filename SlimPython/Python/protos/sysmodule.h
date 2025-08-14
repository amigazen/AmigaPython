static PyObject *sys_exc_info ( PyObject *self , PyObject *args );
static PyObject *sys_exit ( PyObject *self , PyObject *args );
static PyObject *sys_settrace ( PyObject *self , PyObject *args );
static PyObject *sys_setprofile ( PyObject *self , PyObject *args );
static PyObject *sys_setcheckinterval ( PyObject *self , PyObject *args );
#ifdef USE_MALLOPT
static PyObject *sys_mdebug ( PyObject *self , PyObject *args );
#endif
static PyObject *sys_getrefcount ( PyObject *self , PyObject *args );
#ifdef COUNT_ALLOCS
static PyObject *sys_getcounts ( PyObject *self , PyObject *args );
#endif
static PyObject *list_builtin_module_names ( void );
static PyObject *makepathobject ( char *path , int delim );
static PyObject *makeargvobject ( int argc , char **argv );
static void mywrite ( char *name , FILE *fp , const char *format , va_list va );
