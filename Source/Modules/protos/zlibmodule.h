/* Prototypes for functions defined in zlibmodule.c  */

static compobject * newcompobject(PyTypeObject * type);

static PyObject * PyZlib_compress(PyObject * self,
                                  PyObject * args);

static PyObject * PyZlib_decompress(PyObject * self,
                                    PyObject * args);

static PyObject * PyZlib_compressobj(PyObject * selfptr,
                                     PyObject * args);

static PyObject * PyZlib_decompressobj(PyObject * selfptr,
                                       PyObject * args);

static void Comp_dealloc(compobject * self);

static void Decomp_dealloc(compobject * self);

static PyObject * PyZlib_objcompress(compobject * self,
                                     PyObject * args);

static PyObject * PyZlib_objdecompress(compobject * self,
                                       PyObject * args);

static PyObject * PyZlib_flush(compobject * self,
                               PyObject * args);

static PyObject * PyZlib_unflush(compobject * self,
                                 PyObject * args);

static PyObject * Comp_getattr(compobject * self,
                               unsigned char * name);

static PyObject * Decomp_getattr(compobject * self,
                                 unsigned char * name);

static PyObject * PyZlib_adler32(PyObject * self,
                                 PyObject * args);

static PyObject * PyZlib_crc32(PyObject * self,
                               PyObject * args);

static void insint(PyObject * d,
                   unsigned char * name,
                   int value);

void initzlib(void);

