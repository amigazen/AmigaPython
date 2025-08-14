#ifdef macintosh
static void initmactimezone ( void );
#endif
static PyObject *time_time ( PyObject *self , PyObject *args );
static PyObject *time_clock ( PyObject *self , PyObject *args );
static PyObject *time_clock ( PyObject *self , PyObject *args );
static PyObject *time_sleep ( PyObject *self , PyObject *args );
static PyObject *tmtotuple ( struct tm *p );
static PyObject *time_gmtime ( PyObject *self , PyObject *args );
static PyObject *time_localtime ( PyObject *self , PyObject *args );
static int gettmarg ( PyObject *args , struct tm *p );
static PyObject *time_strftime ( PyObject *self , PyObject *args );
#ifdef HAVE_STRPTIME
static PyObject *time_strptime ( PyObject *self , PyObject *args );
#endif
static PyObject *time_asctime ( PyObject *self , PyObject *args );
static PyObject *time_ctime ( PyObject *self , PyObject *args );
static PyObject *time_mktime ( PyObject *self , PyObject *args );
static void ins ( PyObject *d , char *name , PyObject *v );
static PyObject *time_convert(time_t when, struct tm * (*function) (const time_t *));
