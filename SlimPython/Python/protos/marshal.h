static void w_more ( int c , WFILE *p );
static void w_string ( char *s , int n , WFILE *p );
static void w_short ( int x , WFILE *p );
static void w_long ( long x , WFILE *p );
#if SIZEOF_LONG >4
static void w_long64 ( long x , WFILE *p );
#endif
static void w_object ( PyObject *v , WFILE *p );
static PyObject *marshal_dump ( PyObject *self , PyObject *args );
static PyObject *marshal_load ( PyObject *self , PyObject *args );
static PyObject *marshal_dumps ( PyObject *self , PyObject *args );
static PyObject *marshal_loads ( PyObject *self , PyObject *args );
