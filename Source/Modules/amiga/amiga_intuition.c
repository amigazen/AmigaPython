/*
 * Curated Intuition helpers on the amiga module (EasyRequest/CurrentTime/DisplayBeep).
 * Window drawing lives in the separate amigagui builtin.
 */

#include "amiga_posixtimer.h"
#include "Python.h"
#include <intuition/intuition.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include "amiga_ext.h"

PyDoc_STRVAR(DisplayBeep_doc,
"DisplayBeep(screen=None)\n"
"Flash the display (screen pointer or None for default).");

static PyObject *
amiga_DisplayBeep(PyObject *self, PyObject *args)
{
	PyObject *screen_obj = Py_None;
	struct Screen *screen = NULL;

	if (!PyArg_ParseTuple(args, "|O:DisplayBeep", &screen_obj))
		return NULL;
	if (screen_obj != Py_None) {
		if (!PyInt_Check(screen_obj)) {
			PyErr_SetString(PyExc_TypeError,
				"DisplayBeep screen must be int address or None");
			return NULL;
		}
		screen = (struct Screen *)PyInt_AsLong(screen_obj);
	}
	if (!amiga_ensure_intuition())
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	DisplayBeep(screen);
	Py_END_ALLOW_THREADS
	Py_INCREF(Py_None);
	return Py_None;
}

PyDoc_STRVAR(CurrentTime_doc,
"CurrentTime() -> (seconds, micros)\n"
"Return Intuition's current time counters.");

static PyObject *
amiga_CurrentTime(PyObject *self, PyObject *args)
{
	ULONG secs;
	ULONG micros;

	if (!PyArg_ParseTuple(args, ":CurrentTime"))
		return NULL;
	if (!amiga_ensure_intuition())
		return NULL;
	secs = 0;
	micros = 0;
	Py_BEGIN_ALLOW_THREADS
	CurrentTime(&secs, &micros);
	Py_END_ALLOW_THREADS
	return Py_BuildValue("(kk)", (unsigned long)secs, (unsigned long)micros);
}

PyDoc_STRVAR(EasyRequest_doc,
"EasyRequest(body, buttons='Ok|Cancel', title='Requester') -> gadget index\n"
"Convenience wrapper around Intuition EasyRequest (same as MessageBox).");

static PyObject *
amiga_EasyRequest(PyObject *self, PyObject *args, PyObject *kw)
{
	static char *kwlist[] = {"body", "buttons", "title", NULL};
	char *body;
	char *buttons = "Ok|Cancel";
	char *title = "Requester";
	struct EasyStruct es;
	LONG choice;

	if (!PyArg_ParseTupleAndKeywords(args, kw, "s|zz:EasyRequest", kwlist,
			&body, &buttons, &title))
		return NULL;
	if (!amiga_ensure_intuition())
		return NULL;

	es.es_StructSize = sizeof(struct EasyStruct);
	es.es_Flags = 0;
	es.es_Title = (UBYTE *)title;
	es.es_TextFormat = (UBYTE *)body;
	es.es_GadgetFormat = (UBYTE *)buttons;

	Py_BEGIN_ALLOW_THREADS
	choice = EasyRequest(NULL, &es, NULL);
	Py_END_ALLOW_THREADS

	return PyInt_FromLong(choice);
}

static PyMethodDef intuition_methods[] = {
	{"DisplayBeep", amiga_DisplayBeep, METH_VARARGS, DisplayBeep_doc},
	{"CurrentTime", amiga_CurrentTime, METH_VARARGS, CurrentTime_doc},
	{"EasyRequest", (PyCFunction)amiga_EasyRequest,
		METH_VARARGS | METH_KEYWORDS, EasyRequest_doc},
	{NULL, NULL, 0, NULL}
};

void
amiga_init_intuition(PyObject *m)
{
	amiga_add_methods(m, intuition_methods);
}
