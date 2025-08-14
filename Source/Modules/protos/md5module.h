static md5object *newmd5object ( void );
static void md5_dealloc ( md5object *md5p );
static PyObject *md5_update ( md5object *self , PyObject *args );
static PyObject *md5_digest ( md5object *self , PyObject *args );
static PyObject *md5_copy ( md5object *self , PyObject *args );
static PyObject *md5_getattr ( md5object *self , char *name );
static PyObject *MD5_new ( PyObject *self , PyObject *args );
