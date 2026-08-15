/*
 * amigagui - curated Intuition/Graphics window helpers for AmigaPython.
 *
 * Port of Amiga_Misc/experimental_modules/simplegfxmodule.c (renamed;
 * simplegfx was never a Python stdlib name).
 */

#include "amiga_posixtimer.h"
#include "Python.h"
#include <stdlib.h>
#include <string.h>
#include <exec/types.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <graphics/gfxbase.h>
#include <proto/intuition.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include "amiga_ext.h"

typedef struct {
	PyObject_HEAD
	struct Window *win;
	char *title;
	short pen;
	BOOL gzz;
	ULONG signal;
} windowobject;

static PyObject *amigagui_error;
static PyTypeObject Window_Type;

static int
CheckOpen(windowobject *w)
{
	if (w->win != NULL)
		return 1;
	PyErr_SetString(amigagui_error, "closed window");
	return 0;
}

static void
FixGZZ(windowobject *w, long *x, long *y)
{
	if (w->gzz && w->win != NULL) {
		*x += w->win->BorderLeft;
		*y += w->win->BorderTop;
	}
}

static PyObject *
win_close(windowobject *w, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":close"))
		return NULL;
	if (w->win != NULL) {
		CloseWindow(w->win);
		w->win = NULL;
	}
	if (w->title != NULL) {
		free(w->title);
		w->title = NULL;
	}
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
win_wait(windowobject *w, PyObject *args)
{
	ULONG sigs;

	if (!PyArg_ParseTuple(args, ":wait"))
		return NULL;
	if (!CheckOpen(w))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	sigs = Wait(w->signal | SIGBREAKF_CTRL_C);
	Py_END_ALLOW_THREADS
	if (sigs & SIGBREAKF_CTRL_C) {
		PyErr_SetNone(PyExc_KeyboardInterrupt);
		return NULL;
	}
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
win_getmsg(windowobject *w, PyObject *args)
{
	struct IntuiMessage *msg;

	if (!PyArg_ParseTuple(args, ":getmsg"))
		return NULL;
	if (!CheckOpen(w))
		return NULL;
	msg = (struct IntuiMessage *)GetMsg(w->win->UserPort);
	if (msg != NULL) {
		PyObject *t;

		t = Py_BuildValue("(iiiiiii)",
			(int)msg->Class, (int)msg->Code, (int)msg->Qualifier,
			(int)msg->MouseX, (int)msg->MouseY,
			(int)msg->Seconds, (int)msg->Micros);
		ReplyMsg((struct Message *)msg);
		return t;
	}
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
win_pen(windowobject *w, PyObject *args)
{
	long pen;

	if (!PyArg_ParseTuple(args, "i:pen", &pen))
		return NULL;
	if (!CheckOpen(w))
		return NULL;
	if (!amiga_ensure_graphics())
		return NULL;
	w->pen = (short)pen;
	SetAPen(w->win->RPort, pen);
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
win_gzz(windowobject *w, PyObject *args)
{
	long gzz;

	if (!PyArg_ParseTuple(args, "i:gzz", &gzz))
		return NULL;
	w->gzz = gzz ? TRUE : FALSE;
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
win_clear(windowobject *w, PyObject *args)
{
	long pen = 0;

	if (!PyArg_ParseTuple(args, "|i:clear", &pen))
		return NULL;
	if (!CheckOpen(w))
		return NULL;
	if (!amiga_ensure_graphics())
		return NULL;
	SetAPen(w->win->RPort, pen);
	RectFill(w->win->RPort,
		w->win->BorderLeft, w->win->BorderTop,
		w->win->Width - w->win->BorderRight - 1,
		w->win->Height - w->win->BorderBottom - 1);
	SetAPen(w->win->RPort, w->pen);
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
win_plot(windowobject *w, PyObject *args)
{
	long x, y;

	if (!PyArg_ParseTuple(args, "ii:plot", &x, &y))
		return NULL;
	if (!CheckOpen(w))
		return NULL;
	if (!amiga_ensure_graphics())
		return NULL;
	FixGZZ(w, &x, &y);
	WritePixel(w->win->RPort, x, y);
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
win_line(windowobject *w, PyObject *args)
{
	long x1, y1, x2, y2;

	if (!PyArg_ParseTuple(args, "iiii:line", &x1, &y1, &x2, &y2))
		return NULL;
	if (!CheckOpen(w))
		return NULL;
	if (!amiga_ensure_graphics())
		return NULL;
	FixGZZ(w, &x1, &y1);
	FixGZZ(w, &x2, &y2);
	Move(w->win->RPort, x1, y1);
	Draw(w->win->RPort, x2, y2);
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
win_lineto(windowobject *w, PyObject *args)
{
	long x, y;

	if (!PyArg_ParseTuple(args, "ii:lineto", &x, &y))
		return NULL;
	if (!CheckOpen(w))
		return NULL;
	if (!amiga_ensure_graphics())
		return NULL;
	FixGZZ(w, &x, &y);
	Draw(w->win->RPort, x, y);
	Py_INCREF(Py_None);
	return Py_None;
}

static PyMethodDef win_methods[] = {
	{"close", (PyCFunction)win_close, METH_VARARGS,
		"close() -- Close the window."},
	{"getmsg", (PyCFunction)win_getmsg, METH_VARARGS,
		"getmsg() -- Return next IDCMP message tuple or None."},
	{"wait", (PyCFunction)win_wait, METH_VARARGS,
		"wait() -- Wait for window signal (Ctrl-C raises KeyboardInterrupt)."},
	{"gzz", (PyCFunction)win_gzz, METH_VARARGS,
		"gzz(flag) -- Treat coords relative to inner area when true."},
	{"pen", (PyCFunction)win_pen, METH_VARARGS,
		"pen(n) -- Set drawing pen."},
	{"clear", (PyCFunction)win_clear, METH_VARARGS,
		"clear([pen]) -- Fill inner area."},
	{"plot", (PyCFunction)win_plot, METH_VARARGS,
		"plot(x, y) -- Draw a pixel."},
	{"line", (PyCFunction)win_line, METH_VARARGS,
		"line(x1, y1, x2, y2) -- Draw a line."},
	{"lineto", (PyCFunction)win_lineto, METH_VARARGS,
		"lineto(x, y) -- Draw to point from current pen position."},
	{NULL, NULL, 0, NULL}
};

static void
win_dealloc(windowobject *self)
{
	if (self->win != NULL) {
		CloseWindow(self->win);
		self->win = NULL;
	}
	if (self->title != NULL) {
		free(self->title);
		self->title = NULL;
	}
	PyObject_Del(self);
}

static PyObject *
win_getattr(windowobject *w, char *name)
{
	if (w->win != NULL && strcmp(name, "signal") == 0)
		return PyInt_FromLong((long)w->signal);
	return Py_FindMethod(win_methods, (PyObject *)w, name);
}

static PyObject *
win_repr(windowobject *w)
{
	char buf[80];

	PyOS_snprintf(buf, sizeof(buf), "<amigagui.window at %lx>",
		(unsigned long)w);
	return PyString_FromString(buf);
}

static PyTypeObject Window_Type = {
	PyObject_HEAD_INIT(NULL)
	0,
	"amigagui.window",
	sizeof(windowobject),
	0,
	(destructor)win_dealloc,
	0,
	(getattrfunc)win_getattr,
	0,
	0,
	(reprfunc)win_repr,
};

static PyObject *
newwindowobject(char *name, int x, int y, int w, int h)
{
	windowobject *wo;

	if (!amiga_ensure_intuition())
		return NULL;
	if (!amiga_ensure_graphics())
		return NULL;

	wo = PyObject_New(windowobject, &Window_Type);
	if (wo == NULL)
		return NULL;
	wo->win = NULL;
	wo->title = NULL;
	wo->pen = 1;
	wo->gzz = TRUE;
	wo->signal = 0;

	wo->title = strdup(name);
	if (wo->title == NULL) {
		PyObject_Del(wo);
		return PyErr_NoMemory();
	}

	wo->win = OpenWindowTags(NULL,
		WA_Title, (ULONG)wo->title,
		WA_Left, x,
		WA_Top, y,
		WA_InnerWidth, w,
		WA_InnerHeight, h,
		WA_Flags, WFLG_SIZEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET |
			WFLG_CLOSEGADGET | WFLG_SIZEBRIGHT | WFLG_ACTIVATE |
			WFLG_SIMPLE_REFRESH | WFLG_NOCAREREFRESH,
		WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_VANILLAKEY |
			IDCMP_MOUSEBUTTONS,
		WA_MinWidth, 50,
		WA_MinHeight, 30,
		WA_MaxWidth, -1,
		WA_MaxHeight, -1,
		WA_AutoAdjust, TRUE,
		TAG_DONE);
	if (wo->win == NULL) {
		free(wo->title);
		wo->title = NULL;
		PyObject_Del(wo);
		PyErr_SetString(amigagui_error, "can't open window");
		return NULL;
	}
	wo->signal = 1UL << wo->win->UserPort->mp_SigBit;
	SetAPen(wo->win->RPort, wo->pen);
	return (PyObject *)wo;
}

PyDoc_STRVAR(window_doc,
"window([title[, left, top, width, height]]) -> window object\n"
"Open a simple Intuition window for plotting and line drawing.");

static PyObject *
amigagui_window(PyObject *self, PyObject *args)
{
	char *n = "Python Window";
	int x = 40, y = 40, w = 400, h = 200;

	if (!PyArg_ParseTuple(args, "|siiii:window", &n, &x, &y, &w, &h))
		return NULL;
	return newwindowobject(n, x, y, w, h);
}

static PyMethodDef amigagui_methods[] = {
	{"window", amigagui_window, METH_VARARGS, window_doc},
	{NULL, NULL, 0, NULL}
};

void
initamigagui(void)
{
	PyObject *m;

	Window_Type.ob_type = &PyType_Type;
	m = Py_InitModule3("amigagui", amigagui_methods,
		"Curated Intuition/Graphics window helpers for AmigaPython.");
	if (m == NULL)
		return;
	amigagui_error = PyErr_NewException("amigagui.error", NULL, NULL);
	if (amigagui_error == NULL)
		return;
	Py_INCREF(amigagui_error);
	PyModule_AddObject(m, "error", amigagui_error);
}
