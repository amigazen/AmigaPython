static XxoObject *newXxoObject ( PyObject *arg );
static void Xxo_dealloc ( XxoObject *self );
static PyObject *Xxo_demo ( XxoObject *self , PyObject *args );
static PyObject *Xxo_getattr ( XxoObject *self , char *name );
static int Xxo_setattr ( XxoObject *self , char *name , PyObject *v );
static PyObject *xx_foo ( PyObject *self , PyObject *args );
static PyObject *xx_new ( PyObject *self , PyObject *args );
static PyObject *xx_bug ( PyObject *self , PyObject *args );
static PyObject *xx_roj ( PyObject *self , PyObject *args );
