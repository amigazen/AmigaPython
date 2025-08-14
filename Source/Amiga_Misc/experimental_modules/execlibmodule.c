
/********************************************************************

	Lowlevel Amiga exec.library module.

	Currently only for MsgPorts and Messages.

-----------------------------------------------
	©1997 by Irmen de Jong.

	History:

	14-apr-96   Created.

Module members:

	error       -- Exeption string object.  ('execlib.error')


**************************************************************************/

#include <exec/memory.h>
#include <proto/exec.h>
#include "allobjects.h"
#include "modsupport.h"


static PyObject *error;    // Exception



/************** MODULE FUNCTIONS *******************/


static PyObject *
Execlib_Avail(PyObject *self, PyObject *arg)
{
	if(!PyArg_NoArgs(arg)) return NULL;
	return PyInt_FromLong(AvailMem(MEMF_ANY|MEMF_TOTAL));
}

/*** FUNCTIONS FROM THE MODULE ***/

static struct methodlist Execlib_global_methods[] = {
	{"Avail", Execlib_Avail, 0},
	{NULL,      NULL}       /* sentinel */
};
///

void
initexeclib Py_PROTO((void))
{
	PyObject *m, *d;

	m = Py_InitModule("execlib", Execlib_global_methods);
	d = PyModule_GetDict(m);

	/* Initialize error exception */
	error = PyString_FromString("execlib.error");
	if (error == NULL || PyDict_SetItemString(d, "error", error) != 0)
		Py_FatalError("can't define execlib.error");
}

