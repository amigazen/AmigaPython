/*
 * ASL helpers for the amiga module (OS4 asl module parity).
 * Library bases live in amiga_ext.c.
 */

#include "amiga_posixtimer.h"
#include "Python.h"
#include <string.h>
#include <exec/libraries.h>
#include <libraries/asl.h>
#include <intuition/intuition.h>
#include <proto/exec.h>
#include <proto/asl.h>
#include <proto/intuition.h>
#include "amiga_ext.h"

PyDoc_STRVAR(FileRequest_doc,
"FileRequest(title=None, drawer=None, filename=None, pattern=None)\n"
"-> (drawer, filename) or None if cancelled.\n"
"Pops up an asl.library file requester.");

static PyObject *
amiga_FileRequest(PyObject *self, PyObject *args, PyObject *kw)
{
	static char *kwlist[] = {
		"title", "drawer", "filename", "pattern", NULL
	};
	char *title = NULL;
	char *drawer = NULL;
	char *filename = NULL;
	char *pattern = NULL;
	struct FileRequester *fr;
	PyObject *result;
	BOOL ok;

	if (!PyArg_ParseTupleAndKeywords(args, kw, "|zzzz:FileRequest", kwlist,
			&title, &drawer, &filename, &pattern))
		return NULL;

	if (!amiga_ensure_asl())
		return NULL;

	fr = (struct FileRequester *)AllocAslRequestTags(ASL_FileRequest,
		ASL_Hail, (title ? title : (char *)"Select a file"),
		ASL_Dir, (drawer ? drawer : (char *)""),
		ASL_File, (filename ? filename : (char *)""),
		ASL_Pattern, (pattern ? pattern : (char *)"#?"),
		TAG_DONE);
	if (fr == NULL) {
		PyErr_SetString(PyExc_RuntimeError, "AllocAslRequest failed");
		return NULL;
	}

	Py_BEGIN_ALLOW_THREADS
	ok = AslRequest(fr, NULL);
	Py_END_ALLOW_THREADS

	if (!ok) {
		FreeAslRequest(fr);
		Py_INCREF(Py_None);
		return Py_None;
	}

	result = Py_BuildValue("(ss)",
		fr->fr_Drawer ? (char *)fr->fr_Drawer : "",
		fr->fr_File ? (char *)fr->fr_File : "");
	FreeAslRequest(fr);
	return result;
}

PyDoc_STRVAR(MessageBox_doc,
"MessageBox(title, body, gadgets) -> gadget index (0-based).\n"
"Shows an Intuition EasyRequest message box.");

static PyObject *
amiga_MessageBox(PyObject *self, PyObject *args)
{
	char *title;
	char *body;
	char *gadgets;
	struct EasyStruct es;
	LONG choice;

	if (!PyArg_ParseTuple(args, "sss:MessageBox", &title, &body, &gadgets))
		return NULL;

	if (!amiga_ensure_intuition())
		return NULL;

	es.es_StructSize = sizeof(struct EasyStruct);
	es.es_Flags = 0;
	es.es_Title = (UBYTE *)title;
	es.es_TextFormat = (UBYTE *)body;
	es.es_GadgetFormat = (UBYTE *)gadgets;

	Py_BEGIN_ALLOW_THREADS
	choice = EasyRequest(NULL, &es, NULL);
	Py_END_ALLOW_THREADS

	return PyInt_FromLong(choice);
}

static PyMethodDef asl_methods[] = {
	{"FileRequest", (PyCFunction)amiga_FileRequest,
		METH_VARARGS | METH_KEYWORDS, FileRequest_doc},
	{"MessageBox", amiga_MessageBox, METH_VARARGS, MessageBox_doc},
	{NULL, NULL, 0, NULL}
};

void
amiga_init_asl(PyObject *m)
{
	amiga_add_methods(m, asl_methods);
}
