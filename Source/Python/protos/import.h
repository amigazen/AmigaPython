#ifdef WITH_THREAD
static void lock_import ( void );
static void unlock_import ( void );
#endif
static char *make_compiled_pathname ( char *pathname , char *buf , int buflen );
static FILE *check_compiled_module ( char *pathname , long mtime , char *cpathname );
static PyCodeObject *read_compiled_module ( char *cpathname , FILE *fp );
static PyObject *load_compiled_module ( char *name , char *cpathname , FILE *fp );
static PyCodeObject *parse_source_module ( char *pathname , FILE *fp );
static void write_compiled_module ( PyCodeObject *co , char *cpathname , long mtime );
static PyObject *load_source_module ( char *name , char *pathname , FILE *fp );
static PyObject *load_package ( char *name , char *pathname );
static int is_builtin ( char *name );
static struct filedescr *find_module ( char *realname , PyObject *path , char *buf , int buflen , FILE **p_fp );
#ifdef MS_WIN32
static int allcaps8x3 ( char *s );
#endif
static int find_init_module ( char *buf );
static PyObject *load_module ( char *name , FILE *fp , char *buf , int type );
static int init_builtin ( char *name );
static struct _frozen *find_frozen ( char *name );
static PyObject *get_frozen_object ( char *name );
static PyObject *import_module_ex ( char *name , PyObject *globals , PyObject *locals , PyObject *fromlist );
static PyObject *get_parent ( PyObject *globals , char *buf , int *p_buflen );
static PyObject *load_next ( PyObject *mod , PyObject *altmod , char **p_name , char *buf , int *p_buflen );
static int mark_miss ( char *name );
static int ensure_fromlist ( PyObject *mod , PyObject *fromlist , char *buf , int buflen , int recursive );
static PyObject *import_submodule ( PyObject *mod , char *subname , char *fullname );
static PyObject *imp_get_magic ( PyObject *self , PyObject *args );
static PyObject *imp_get_suffixes ( PyObject *self , PyObject *args );
static PyObject *call_find_module ( char *name , PyObject *path );
static PyObject *imp_find_module ( PyObject *self , PyObject *args );
static PyObject *imp_init_builtin ( PyObject *self , PyObject *args );
static PyObject *imp_init_frozen ( PyObject *self , PyObject *args );
static PyObject *imp_get_frozen_object ( PyObject *self , PyObject *args );
static PyObject *imp_is_builtin ( PyObject *self , PyObject *args );
static PyObject *imp_is_frozen ( PyObject *self , PyObject *args );
static FILE *get_file ( char *pathname , PyObject *fob , char *mode );
static PyObject *imp_load_compiled ( PyObject *self , PyObject *args );
static PyObject *imp_load_source ( PyObject *self , PyObject *args );
#ifdef macintosh
static PyObject *imp_load_resource ( PyObject *self , PyObject *args );
#endif
static PyObject *imp_load_module ( PyObject *self , PyObject *args );
static PyObject *imp_load_package ( PyObject *self , PyObject *args );
static PyObject *imp_new_module ( PyObject *self , PyObject *args );
static int setint ( PyObject *d , char *name , int value );
