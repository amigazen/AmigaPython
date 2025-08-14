
/********************************************************************

	Simple Graphics module.

	©Irmen de Jong


	29-okt-00	Fixup for Python 2.0

**************************************************************************/

#include <exec/types.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <proto/intuition.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include "Python.h"

typedef struct {
	PyObject_HEAD
	struct Window *win;
	char *title;
	short pen;
	BOOL gzz;
	ULONG signal;
} windowobject;

static PyObject *error;    // Exception

/* Prototypes for functions defined in arexxmodule.c */

///*** WINDOW OBJECT MEMBER FUNCTIONS ***/

static BOOL CheckOpen(const windowobject *w)
{
	if(w->win) return TRUE;
	PyErr_SetString(error,"closed window");
	return FALSE;
}

static void FixGZZ(const windowobject *w, long *x, long *y)
{
	if(w->gzz)
	{
		*x += w->win->BorderLeft;
		*y += w->win->BorderTop;
	}
}

static PyObject *win_close(windowobject *w, PyObject *args)
{
	if(!PyArg_NoArgs(args)) return NULL;

	if(w->win) CloseWindow(w->win);
	w->win=NULL;
	if(w->title) free(w->title);
	w->title=NULL;

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *win_wait(windowobject *w, PyObject *args)
{
	if(!PyArg_NoArgs(args)) return NULL;

	if(CheckOpen(w))
	{
		ULONG sigs = Wait(w->signal | SIGBREAKF_CTRL_C);   /* XXX Abort with ^C */
		if(sigs & SIGBREAKF_CTRL_C)
		{
			PyErr_SetNone(PyExc_KeyboardInterrupt);
			return NULL;
		}
		Py_INCREF(Py_None);
		return Py_None;
	} else return NULL;
}

static PyObject *win_getmsg(windowobject *w, PyObject *args)
{
	if(!PyArg_NoArgs(args)) return NULL;

	if(CheckOpen(w))
	{
		struct IntuiMessage *msg;

		if(msg=(struct IntuiMessage*)GetMsg(w->win->UserPort))
		{
			PyObject *t = Py_BuildValue("(iiiiiii)",msg->Class,msg->Code,msg->Qualifier,msg->MouseX,msg->MouseY,msg->Seconds,msg->Micros);
			ReplyMsg((struct Message*)msg);
			return t;
		}
		Py_INCREF(Py_None);
		return Py_None;
	} else return NULL;
}

static PyObject *win_pen(windowobject *w, PyObject *args)
{
	long pen;

	if (!PyArg_ParseTuple(args, "i", &pen))
		return NULL;

	if(CheckOpen(w))
	{
		w->pen=pen;
		SetAPen(w->win->RPort,pen);
		Py_INCREF(Py_None);
		return Py_None;
	} else return NULL;
}


static PyObject *win_gzz(windowobject *w, PyObject *args)
{
	long gzz;

	if (!PyArg_ParseTuple(args, "i", &gzz))
		return NULL;

	w->gzz=gzz;
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *win_clear(windowobject *w, PyObject *args)
{
	long pen=0;

	if (!PyArg_ParseTuple(args, "|i", &pen))
		return NULL;

	if(CheckOpen(w))
	{
		SetAPen(w->win->RPort,pen);
		RectFill(w->win->RPort,w->win->BorderLeft,w->win->BorderTop,
			w->win->Width-w->win->BorderRight-1,w->win->Height-w->win->BorderBottom-1);
		SetAPen(w->win->RPort,w->pen);
		Py_INCREF(Py_None);
		return Py_None;
	} else return NULL;
}

static PyObject *win_plot(windowobject *w, PyObject *args)
{
	long x,y;

	if (!PyArg_ParseTuple(args, "ii", &x,&y))
		return NULL;

	if(CheckOpen(w))
	{
		FixGZZ(w,&x,&y);
		WritePixel(w->win->RPort,x,y);
		Py_INCREF(Py_None);
		return Py_None;
	} else return NULL;
}

static PyObject *win_line(windowobject *w, PyObject *args)
{
	long x1,y1,x2,y2;

	if (!PyArg_ParseTuple(args, "iiii", &x1,&y1,&x2,&y2))
		return NULL;

	if(CheckOpen(w))
	{
		FixGZZ(w,&x1,&y1);
		FixGZZ(w,&x2,&y2);
		Move(w->win->RPort,x1,y1);
		Draw(w->win->RPort,x2,y2);
		Py_INCREF(Py_None);
		return Py_None;
	} else return NULL;
}

static PyObject *win_lineto(windowobject *w, PyObject *args)
{
	long x,y;

	if (!PyArg_ParseTuple(args, "ii", &x,&y))
		return NULL;

	if(CheckOpen(w))
	{
		FixGZZ(w,&x,&y);
		Draw(w->win->RPort,x,y);
		Py_INCREF(Py_None);
		return Py_None;
	} else return NULL;
}

static PyMethodDef win_methods[] = {
	{"close", (PyCFunction)win_close, METH_OLDARGS},
	{"getmsg", (PyCFunction)win_getmsg, METH_OLDARGS},
	{"wait", (PyCFunction)win_wait, METH_OLDARGS},
	{"gzz", (PyCFunction)win_gzz,METH_VARARGS},
	{"pen", (PyCFunction)win_pen,METH_VARARGS},
	{"clear", (PyCFunction)win_clear,METH_VARARGS},
	{"plot", (PyCFunction)win_plot,METH_VARARGS},
	{"line", (PyCFunction)win_line,METH_VARARGS},
	{"lineto", (PyCFunction)win_lineto,METH_VARARGS},
	{NULL,      NULL}       /* sentinel */
};


static void
win_dealloc(windowobject *self)         // `destructor'
{
	if(self->win) CloseWindow(self->win);
	PyMem_DEL(self);
}

static PyObject *
win_getattr(windowobject *w, char *name)
{
	if(w->win)
	{
		if(strcmp(name,"signal")==0)
			return PyInt_FromLong(w->signal);
	}
	return Py_FindMethod(win_methods, (PyObject *)w, name);
}

static PyObject *
win_repr(windowobject *w)
{
	char buf[50];
	sprintf(buf,"<window at %lx>",(long)w);
	return PyString_FromString(buf);
}

static PyTypeObject Windowtype = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,          /*ob_size*/
	"window",        /*tp_name*/
	sizeof(windowobject),    /*tp_size*/
	0,          /*tp_itemsize*/
	/* methods */
	(destructor)win_dealloc, /*tp_dealloc*/
	0,          /*tp_print*/
	(getattrfunc)win_getattr, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	0,          /*tp_compare*/
	(reprfunc)win_repr,        /*tp_repr*/
};


static PyObject *
newwindowobject(char *name, int x, int y, int w, int h) // `constructor'
{
	windowobject *wo;

	if(wo = PyObject_NEW(windowobject, &Windowtype))
	{
		wo->title=strdup(name);
		wo->pen=1;
		wo->gzz=1;
		if(wo->win=OpenWindowTags(NULL,
				WA_Title,wo->title,
				WA_Left,x,WA_Top,y,WA_InnerWidth,w,WA_InnerHeight,h,
				WA_Flags,WFLG_SIZEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET | WFLG_SIZEBRIGHT | WFLG_ACTIVATE | WFLG_SIMPLE_REFRESH | WFLG_NOCAREREFRESH,
				WA_IDCMP,IDCMP_CLOSEWINDOW | IDCMP_VANILLAKEY | IDCMP_MOUSEBUTTONS,
				WA_MinWidth,50, WA_MinHeight,30, WA_MaxWidth,-1, WA_MaxHeight,-1,
				WA_AutoAdjust,1,TAG_DONE))
		{
			wo->signal = 1<<wo->win->UserPort->mp_SigBit;
			SetAPen(wo->win->RPort,wo->pen);
			return (PyObject*)wo;
		}
		else PyErr_SetString(error,"can't open window");

		PyMem_DEL(wo); wo=NULL;
	}
	return (PyObject*)wo;
}


///


///******************************** MODULE FUNCTIONS ************************/

static PyObject *
SimpleGFX_openwindow(PyObject *self, PyObject *args)
{
	char *n="Python Window"; long x=40,y=40,w=400,h=200;
	
	if (!PyArg_ParseTuple(args, "|siiii", &n,&x,&y,&w,&h))
		return NULL;

	return newwindowobject(n,x,y,w,h);
}


/*** FUNCTIONS FROM THE MODULE ***/

static PyMethodDef SimpleGFX_global_methods[] = {
	{"window",  SimpleGFX_openwindow, METH_VARARGS},
	{NULL,      NULL}       /* sentinel */
};
///

void
initsimplegfx Py_PROTO((void))
{
	PyObject *m, *d;

	m = Py_InitModule("simplegfx", SimpleGFX_global_methods);
	d = PyModule_GetDict(m);

	/* Initialize error exception */
	error = PyString_FromString("simplegfx.error");
	if (error == NULL || PyDict_SetItemString(d, "error", error) != 0)
		Py_FatalError("can't define simplegfx.error");
}
