
/********************************************************************

	Lowlevel Amiga exec.library module.

	Currently only for MsgPorts and Messages.

-----------------------------------------------
	©Irmen de Jong.

	History:

	14-apr-96   Created.
	29-okt-00	Fixup for Python 2.0


Module members:

	error       -- Exeption string object.  ('Execlib.error')


**************************************************************************/

#include <exec/memory.h>
#include <proto/exec.h>
#include "Python.h"
//#include "modsupport.h"


static PyObject *error;    // Exception



/************** MODULE FUNCTIONS *******************/


static PyObject *
Execlib_Avail(PyObject *self, PyObject *arg)
{
	if(!PyArg_NoArgs(arg)) return NULL;
	return PyInt_FromLong(AvailMem(MEMF_ANY|MEMF_TOTAL));
}

/*** FUNCTIONS FROM THE MODULE ***/

static PyMethodDef Execlib_global_methods[] = {
	{"Avail", Execlib_Avail,  METH_OLDARGS},
	{NULL,      NULL}       /* sentinel */
};
///

void
initExeclib Py_PROTO((void))
{
	PyObject *m, *d;

	m = Py_InitModule("Execlib", Execlib_global_methods);
	d = PyModule_GetDict(m);

	/* Initialize error exception */
	error = PyString_FromString("Execlib.error");
	if (error == NULL || PyDict_SetItemString(d, "error", error) != 0)
		Py_FatalError("can't define Execlib.error");
}

