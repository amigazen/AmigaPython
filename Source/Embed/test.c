/* Example of embedding Python in another program */

#include "Python.h"

static char *argv0;

static const char *ver = "$VER: embed demo " __AMIGADATE__ " Embedded Python\0";

extern void InitAmigaPython(int argc, char **argv);

void initxyzzy(Py_PROTO(void)); /* Forward */

int main(int argc, char **argv)
{
	FILE *fh;

	/* Save a copy of argv0 */
	argv0 = argv[0];

	/* Init Embedded AmigaPython. Required. */
	InitAmigaPython(argc, argv);

	/* Optionally, set the program name.  Default is the name of the   */
    /*   executable, which should be fine.  */
	/* Py_SetProgramName("test"); */


	/* Initialize the Python interpreter.  Required. */
	Py_Initialize();

	/* Add a static module */
	initxyzzy();

	/* Define sys.argv.  It is up to the application if you
	   want this; you can also let it undefined (since the Python 
	   code is generally not a main program it has no business
	   touching sys.argv...) */
	PySys_SetArgv(argc, argv);

	/* Do some application specific code */
	printf("Hello, brave new world\n\n");

	/* Execute some Python statements (in module __main__) */
	PyRun_SimpleString("import sys\n");
	PyRun_SimpleString("print 'BUILTIN MODULES:',sys.builtin_module_names\n");
	PyRun_SimpleString("print 'IMPORTED MODULES:',sys.modules.keys()\n");
	PyRun_SimpleString("print 'sys.argv:',sys.argv\n");

	fh = fopen("testscript.py","r");
	if(fh)
	{
		PyRun_SimpleFile(fh,"testscript.py");
		PyRun_SimpleString("print '>> testscript has set result to',result\n");
		fclose(fh);
	}

	/* Note that you can call any public function of the Python
	   interpreter here, e.g. call_object(). */

	/* Some more application specific code */
	printf("\nGoodbye, cruel world\n");

	/* Exit, cleaning up the interpreter */
	Py_Exit(0);
	/*NOTREACHED*/
}

/* A static module */

static PyObject *xyzzy_foo Py_PROTO((PyObject *, PyObject *));

static PyObject *
xyzzy_foo(self, args)
	PyObject *self; /* Not used */
	PyObject *args;
{
	if (!PyArg_ParseTuple(args, ""))
		return NULL;
	return PyInt_FromLong(42L);
}

static PyMethodDef xyzzy_methods[] = {
	{"foo",		xyzzy_foo,	1},
	{NULL,		NULL}		/* sentinel */
};

void
initxyzzy Py_PROTO((void))
{
	PyImport_AddModule("xyzzy");
	Py_InitModule("xyzzy", xyzzy_methods);
}
