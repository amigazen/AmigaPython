
/********************************************************************

	Amiga Exec module. ('amiga_exec')

	Provides access to various lowlevel Amiga Exec functions.

-----------------------------------------------
	(c) 1999 Irmen de Jong.

	History:

	4-aug-99   Created.


Module members:

	error         -- Exeption string object.  ('amiga_exec.error')
	list_MsgPorts -- function returning list of known public message ports.
	CreateMsgPort -- function returning a new MsgPort object.
    FindPort      -- function returning an existing public MsgPort object, or None.

MsgPort object members:

	name        -- attribute, name of the port (RO)
	signal      -- attribute, signal mask of the MsgPort's sigbit (RO)
	close       -- function, closes the port
	wait        -- function, waits for message to arrive
	getmsg      -- function, returns a received message as a string.
	putmsg      -- function, puts a new message on the port.

**************************************************************************/

#include <stdio.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/execbase.h>
#include <dos/dos.h>
#include <proto/exec.h>
#include "Python.h"

#define MAX_PORTNAME_LEN 80

struct DataMessage
{
	struct Message m;
	char data[0];
};

typedef struct {
	PyObject_HEAD
	struct MsgPort *port;
	struct MsgPort *replyPort;
	char name[MAX_PORTNAME_LEN];
	int fromExisting;
} MsgPortobject;

static PyObject *error;    // Exception

/* Prototypes for functions defined */
static BOOL Valid_Port(MsgPortobject *mpo);
static void DeleteMsgPortSafely(struct MsgPort *port, int noreply);


///*** MSGPORT OBJECT MEMBER FUNCTIONS ***/

static BOOL Valid_Port(MsgPortobject *mpo)
{
	if(mpo->port) return TRUE;
	PyErr_SetString(error,"closed port");
	return FALSE;
}

static void DeleteMsgPortSafely(struct MsgPort *port, int noreply)
{
	if(port)
	{
		struct Message *msg;

		if(port->mp_Node.ln_Name)
			RemPort(port);

		while(msg = GetMsg(port))
		{
			if(msg->mn_ReplyPort && !noreply)
				ReplyMsg(msg);
		}		
		DeleteMsgPort(port);
	}
}

static PyObject *port_close(MsgPortobject *mpo, PyObject *args)
{
	if(!PyArg_NoArgs(args)) return NULL;

	if(mpo->fromExisting)
	{
		PyErr_SetString(error,"I don't own this port");
		return NULL;
	}
	DeleteMsgPortSafely(mpo->port,0);
	mpo->port=NULL;
	DeleteMsgPortSafely(mpo->replyPort,1);
	mpo->replyPort=NULL;
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *port_wait(MsgPortobject *mpo, PyObject *args)
{
	if(!PyArg_NoArgs(args)) return NULL;

	if(Valid_Port(mpo))
	{
		ULONG sigs = (1<<mpo->port->mp_SigBit) | SIGBREAKF_CTRL_C;
		sigs = Wait(sigs);   /* XXX Abort with ^C */
		if(sigs & SIGBREAKF_CTRL_C)
		{
			PyErr_SetNone(PyExc_KeyboardInterrupt);
			return NULL;
		}

		Py_INCREF(Py_None);
		return Py_None;
	} else return NULL;
}

static PyObject *port_getmsg(MsgPortobject *mpo, PyObject *args)
{
	int blocking = 1;
	int async = 0;


	if (!PyArg_ParseTuple(args, "|ii", &blocking, &async))
		return NULL;

	if(Valid_Port(mpo))
	{
		struct DataMessage *msg;

		if(blocking)
		{
			/* Wait() only if there is no message on the port */
			
			if(IsListEmpty(&mpo->port->mp_MsgList))
			{
				ULONG sig;
				sig = Wait((1<<mpo->port->mp_SigBit) | SIGBREAKF_CTRL_C);   /* XXX Abort with ^C */
				if(sig & SIGBREAKF_CTRL_C)
				{
					PyErr_SetNone(PyExc_KeyboardInterrupt);
					return NULL;
				}
				else
				{
					msg=(struct DataMessage*)GetMsg(mpo->port);
				}
			}
			else
			{
				msg=(struct DataMessage*)GetMsg(mpo->port);
			}
		}
		else
		{
			/* non-blocking */
			msg=(struct DataMessage*)GetMsg(mpo->port);
		}

		if(msg)
		{
			PyObject *str;
			int datalen = msg->m.mn_Length - sizeof(struct Message);
			str = PyString_FromStringAndSize(msg->data,datalen);
			if(async)
			{
				// we should not reply, and deallocate the message ourselves
				FreeMem(msg,msg->m.mn_Length);
			}
			else if(msg->m.mn_ReplyPort)
			{
				// reply the message and let the sender deallocate the storage
				ReplyMsg(&msg->m);
			}
			return str;
		}

		Py_INCREF(Py_None);
		return Py_None;
	} else return NULL;
}

static PyObject *port_putmsg(MsgPortobject *mpo, PyObject *args)
{
	char *str;
	int stringlen;
	int async = 0;

	if (!PyArg_ParseTuple(args, "s#|i", &str, &stringlen, &async))
		return NULL;

	if(Valid_Port(mpo))
	{
		struct DataMessage *msg;
		ULONG allocsize = sizeof(struct Message)+stringlen;
		if(msg = (struct DataMessage*) AllocMem(allocsize, MEMF_ANY))
		{
			memset(&msg->m,0,sizeof(struct Message));
			msg->m.mn_Node.ln_Type = NT_MESSAGE;
			memcpy(msg->data,str,stringlen);
			msg->m.mn_Length = allocsize;

			if(async)
			{
				/* no reply, don't free msg */
				msg->m.mn_ReplyPort = NULL;
				PutMsg(mpo->port, &msg->m);
			}
			else
			{
				/* we should wait for a reply and then free the message */
				ULONG sigs;
				msg->m.mn_ReplyPort = mpo->replyPort;
				PutMsg(mpo->port,&msg->m);
				sigs = Wait((1<<msg->m.mn_ReplyPort->mp_SigBit) | SIGBREAKF_CTRL_C);   /* XXX Abort with ^C */
				if(sigs & SIGBREAKF_CTRL_C)
				{
					PyErr_SetNone(PyExc_KeyboardInterrupt);
					return NULL;
				}
				(void)GetMsg(msg->m.mn_ReplyPort);
				FreeMem(msg,msg->m.mn_Length);
			}
			Py_INCREF(Py_None);
			return Py_None;
		}
		else return PyErr_NoMemory();
	}
	return NULL;
}

static struct PyMethodDef port_methods[] = {
	{"close", (PyCFunction)port_close, 0},
	{"wait", (PyCFunction)port_wait, 0},
	{"getmsg", (PyCFunction)port_getmsg, 1},
	{"putmsg", (PyCFunction)port_putmsg, 1},
	{NULL,      NULL}       /* sentinel */
};

static void
port_dealloc(MsgPortobject *self)         // `destructor'
{
	if(!self->fromExisting) DeleteMsgPortSafely(self->port,0);
	DeleteMsgPortSafely(self->replyPort,1);
	PyMem_DEL(self);
}

static PyObject *
port_getattr(MsgPortobject *mpo, char *name)
{
	if(mpo->port)
	{
		if (strcmp(name, "name")==0)
		{
			if(!mpo->port->mp_Node.ln_Name)
			{
				// port has no name
				PyErr_SetString(PyExc_AttributeError,name); return NULL;
			}   
			return PyString_FromString(mpo->port->mp_Node.ln_Name);
		}
		else if(strcmp(name,"signal")==0)
			return PyInt_FromLong(1<<mpo->port->mp_SigBit);
	}
	return Py_FindMethod(port_methods, (PyObject *)mpo, name);
}

static PyObject *
port_repr(MsgPortobject *mpo)
{
	char buf[200];
	char *w;

	if(!(mpo->port))
		w="(closed)";
	else if(!mpo->port->mp_Node.ln_Name)
		w="(private)";
	else
		w=mpo->port->mp_Node.ln_Name;
	sprintf(buf,"<MsgPort %s at %lx>",w,(long)mpo);
	return PyString_FromString(buf);
}

static PyTypeObject MsgPortType = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,          /*ob_size*/
	"MsgPort",        /*tp_name*/
	sizeof(MsgPortobject),    /*tp_size*/
	0,          /*tp_itemsize*/
	/* methods */
	(destructor)port_dealloc, /*tp_dealloc*/
	0,          /*tp_print*/
	(getattrfunc)port_getattr, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	0,          /*tp_compare*/
	(reprfunc)port_repr,        /*tp_repr*/
};


///


///******************************** MODULE FUNCTIONS ************************/

static PyObject *
amiga_exec_FindPort(PyObject *self, PyObject *args)
{
	char *name;
	struct MsgPort *port;

	if (!PyArg_ParseTuple(args, "s", &name))
		return NULL;

	if(port=FindPort(name))
	{
		MsgPortobject *mpo;
		if(mpo = PyObject_NEW(MsgPortobject, &MsgPortType))
		{
			mpo->port = port;
			mpo->fromExisting = 1;
			if(mpo->replyPort = CreateMsgPort())
			{
				return (PyObject*) mpo;
			}
			/* could not create replyport */
			Py_DECREF(mpo);
			return PyErr_NoMemory();
		}
		else return NULL;
	}
	PyErr_SetString(error,"Can't find MsgPort");
	return NULL;
}

static PyObject *
amiga_exec_listMsgPorts(PyObject *self, PyObject *args)
{
	PyObject *list;
	PyObject *item;

	if (!PyArg_ParseTuple(args, "")) return NULL;

	if(list = PyList_New(0))
	{
		struct Node *node;
		int failed=0;
		Forbid();
		for (node = SysBase->PortList.lh_Head; node->ln_Succ; node = node->ln_Succ)
		{
			if(item = PyString_FromString(node->ln_Name))
			{
				if(PyList_Append(list,item)!=0)
				{
					Py_DECREF(item);
					failed=1; break;
				}
			}
			else
			{
				failed=1; break;
			}
		}
		Permit();
		if(failed)
		{
			Py_DECREF(list);
			return PyErr_NoMemory();
		}
		return list;
	}
	else return NULL;
}


static PyObject *
amiga_exec_CreateMsgPort(PyObject *self, PyObject *args)
{
	MsgPortobject *mpo;
	char *name;
	
	if (!PyArg_ParseTuple(args, "z", &name))
		return NULL;

	if(name)
	{
		if(strlen(name)>MAX_PORTNAME_LEN)
		{
			PyErr_SetString(error,"Portname too long");
			return NULL;
		}
		if(name[0]=='\0')
		{
			name=NULL;
		}
	}

	if(name && FindPort(name))
	{
		PyErr_SetString(error,"Port already exists with this name");
		return NULL;
	}

	if(mpo = PyObject_NEW(MsgPortobject, &MsgPortType))
	{
		mpo->fromExisting=0;
		if(mpo->port = CreateMsgPort())
		{
			if(name)
			{
				strcpy(mpo->name,name);
				mpo->port->mp_Node.ln_Name = &(mpo->name[0]);
				AddPort(mpo->port);
			}

			if(mpo->replyPort = CreateMsgPort())
			{	
				/** all went OK! **/
				return (PyObject*) mpo;
			}

			/** something went wrong, clean up **/
			if(name)
				RemPort(mpo->port);

			DeleteMsgPortSafely(mpo->port,0);
			mpo->port=NULL;
		}
		PyErr_SetString(error,"can't open port");

		PyMem_DEL(mpo); mpo=NULL;
	}
	return (PyObject*)mpo;
}


/*** FUNCTIONS FROM THE MODULE ***/

static struct PyMethodDef amiga_exec_global_methods[] = {
	{"CreateMsgPort",  amiga_exec_CreateMsgPort, 1},
	{"FindPort",  amiga_exec_FindPort, 1},
	{"list_MsgPorts",  amiga_exec_listMsgPorts, 1},
	{NULL,      NULL}       /* sentinel */
};
///

void
initamiga_exec Py_PROTO((void))
{
	PyObject *m, *d;

	m = Py_InitModule("amiga_exec", amiga_exec_global_methods);
	d = PyModule_GetDict(m);

	/* Initialize error exception */
	error = PyErr_NewException("amiga_exec.error", NULL, NULL);
	if (error != NULL)
		PyDict_SetItemString(d, "error", error);
}
