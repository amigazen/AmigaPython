static PyObject *err_closed ( void );
static void file_dealloc ( PyFileObject *f );
static PyObject *file_repr ( PyFileObject *f );
static PyObject *file_close ( PyFileObject *f , PyObject *args );
static PyObject *file_seek ( PyFileObject *f , PyObject *args );
#ifdef HAVE_FTRUNCATE
static PyObject *file_truncate ( PyFileObject *f , PyObject *args );
#endif
static PyObject *file_tell ( PyFileObject *f , PyObject *args );
static PyObject *file_fileno ( PyFileObject *f , PyObject *args );
static PyObject *file_flush ( PyFileObject *f , PyObject *args );
static PyObject *file_isatty ( PyFileObject *f , PyObject *args );
static size_t new_buffersize ( PyFileObject *f , size_t currentsize );
static PyObject *file_read ( PyFileObject *f , PyObject *args );
static PyObject *file_readinto ( PyFileObject *f , PyObject *args );
static PyObject *getline ( PyFileObject *f , int n );
static PyObject *file_readline ( PyFileObject *f , PyObject *args );
static PyObject *file_readlines ( PyFileObject *f , PyObject *args );
static PyObject *file_write ( PyFileObject *f , PyObject *args );
static PyObject *file_writelines ( PyFileObject *f , PyObject *args );
static PyObject *file_getattr ( PyFileObject *f , char *name );
static int file_setattr ( PyFileObject *f , char *name , PyObject *v );
