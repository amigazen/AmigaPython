
/*
 * Amiga ARexx module for Python 2.7
 * 
 * This module provides an interface to ARexx, the Amiga's scripting language.
 * It allows Python to send commands to ARexx ports and receive responses.
 * 
 * Features:
 * - Low-level ARexx port management
 * - High-level dorexx() function for simple command sending
 * - ARexx message handling with error() method
 * - Variable get/set functionality
 * - Support for both synchronous and asynchronous communication
 * 
 * Based on original work by Irmen de Jong (1996-1999)
 * Updated for Python 2.7 and AmigaOS 3.2 compatibility
 */

/* Define DEVICES_TIMER_H to prevent timer.h conflicts with Amiga headers */
#define DEVICES_TIMER_H
#define CLIB_ALIB_PROTOS_H

#include <stdlib.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <exec/libraries.h>
#include <rexx/storage.h>
#include <rexx/rxslib.h>
#include <proto/alib.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include "Python.h"

#ifdef __VBCC__
/* VBCC chokes on the NDK 3.2 REXX proto headers so until that gets fixed, we need this */
#include <inline/rexxsyslib.h>
/* Define RexxSysBase for VBCC ErrorMsg macro */
extern struct Library *RexxSysBase;

/* Type definitions if not already defined */
#ifndef CONST_STRPTR
#define CONST_STRPTR const char *
#endif
#ifndef STRPTR
#define STRPTR char *
#endif
#endif

#define RHF_CMDSHELL     (1L << 0)
#define RHF_USRMSGPORT   (1L << 1)
#define RHF_NOTPUBLIC    (1L << 2)
#define RHF_STRINGMSGS   (1L << 3)
#define RHF_TOKENIZELINE (1L << 4)

#define ToUpper(c) (c&(~32))

struct RexxHost
{
	struct MsgPort *port;
	char portname[ 80 ];
	long replies;
	long flags;
	APTR userdata;      /* Not used yet */
};

typedef struct {
	PyObject_HEAD
	struct RexxHost *host;
	ULONG signal;
} arexxportobject;

typedef struct {
	PyObject_HEAD
	struct RexxMsg *msg;
	BOOL replied;           /* has this message been replied to yet? */
	int rc;                 /* primary return code (RC -- int) */
	PyObject *rc2;          /* secondary return code (RC2 -- string) */
	PyObject *result;       /* RESULT string */
} arexxmsgobject;

static PyObject *error;    /* Exception */

/* Prototypes for functions defined in arexxmodule.c */

struct RexxMsg *CreateRexxCommand( struct RexxHost *host, char *buff, BPTR fh );
static void ReplyRexxCommand( struct RexxMsg *rxmsg, long prim, long sec, char *res );
static void FreeRexxCommand( struct RexxMsg *rxmsg );
static void CloseDownARexxHost( struct RexxHost *host );
static struct RexxHost *SetupARexxHost( char *basename);
static struct RexxMsg *GetARexxMsg( struct RexxHost *host );
struct RexxMsg *SendRexxCommandToPort( struct RexxHost *host,
		char *port, char *cmd, BPTR fh );

static BOOL Test_Open(arexxportobject * );
static BOOL Test_Replied(arexxmsgobject * );
static PyObject * port_close(arexxportobject * , PyObject * );
static PyObject * port_wait(arexxportobject * , PyObject * );
static PyObject * port_getmsg(arexxportobject * , PyObject * );
static PyObject * port_send(arexxportobject * , PyObject * );
static PyObject * port_setstringmsgs(arexxportobject * , PyObject * );
static PyObject * port_settokenizeline(arexxportobject * , PyObject * );
static void port_dealloc(arexxportobject * );
static PyObject * port_getattr(arexxportobject * , char * );
static PyObject * port_repr(arexxportobject * );
static PyObject * newarexxportobject(char * );
static void msg_dealloc(arexxmsgobject * );
static PyObject * msg_getattr(arexxmsgobject * , char * );
static PyObject * msg_setattr(arexxmsgobject * , char * , PyObject * );
static PyObject * msg_repr(arexxmsgobject * );
static PyObject * newarexxmsgobject(struct RexxMsg * );
static PyObject * msg_reply(arexxmsgobject * , PyObject * );
static PyObject * msg_setvar(arexxmsgobject * , PyObject * );
static PyObject * msg_getvar(arexxmsgobject * , PyObject * );
static PyObject * msg_error(arexxmsgobject * , PyObject * );
static PyObject * ARexx_openport(PyObject * , PyObject * );
static PyObject * ARexx_errorstring(PyObject * , PyObject * );
static PyObject * ARexx_dorexx(PyObject * , PyObject * );

static PyTypeObject ARexxMsgtype;   /* fwd */

#define isARexxMsgObject(ob) ((ob)->ob_type == &ARexxMsgtype)

/* *** AREXXPORT OBJECT MEMBER FUNCTIONS *** */

static BOOL Test_Open(arexxportobject *ao)
{
	if(ao->host && ao->host->port) return TRUE;
	PyErr_SetString(error,"closed port");
	return FALSE;
}

static PyObject *port_close(arexxportobject *ao, PyObject *args)
{
	if(!PyArg_NoArgs(args)) return NULL;

	if(ao->host) CloseDownARexxHost(ao->host);
	ao->host=NULL;
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *port_wait(arexxportobject *ao, PyObject *args)
{
	if(!PyArg_NoArgs(args)) return NULL;

	if(Test_Open(ao))
	{
		ULONG sigs;
		sigs = Wait(ao->signal | SIGBREAKF_CTRL_C);   /* XXX Abort with ^C */
		if(sigs & SIGBREAKF_CTRL_C)
		{
			PyErr_SetNone(PyExc_KeyboardInterrupt);
			return NULL;
		}

		Py_INCREF(Py_None);
		return Py_None;
	} else return NULL;
}

static PyObject *port_getmsg(arexxportobject *ao, PyObject *args)
{
	if(!PyArg_NoArgs(args)) return NULL;

	if(Test_Open(ao))
	{
		struct RexxMsg *msg;

		msg = GetARexxMsg(ao->host);
		if(msg)
		{
			return newarexxmsgobject(msg);
		}

		Py_INCREF(Py_None);
		return Py_None;
	} else return NULL;
}

static PyObject *port_send(arexxportobject *ao, PyObject *args)
{
	char *to, *cmd;
	int async;
	struct RexxMsg *sentrm;
	if(!PyArg_ParseTuple(args,"ssi",&to,&cmd,&async)) return NULL;

	if(Test_Open(ao))
	{
		/* create a new RexxMsg from the command string and send it */
		sentrm = SendRexxCommandToPort(ao->host, to, cmd,NULL);
		if( !sentrm )
		{
			PyErr_SetString(error,"can't send to port");
			return NULL;
		}

		if(!async)
		{
			/* wait for the reply */
			PyObject *reso = NULL;
			long rc;
			struct RexxMsg *rm;
			BOOL waiting = TRUE;
			
			do
			{
				WaitPort( ao->host->port );
					
				rm = (struct RexxMsg *) GetMsg(ao->host->port);
				while( rm )
				{
					/* Reply? */
					if( rm->rm_Node.mn_Node.ln_Type == NT_REPLYMSG )
					{
						/* 'our' Msg? */
						if( rm == sentrm )
						{
							rc = rm->rm_Result1;
								
							if( !rc && rm->rm_Result2 )
							{
								/* Res2 is String */
								reso = Py_BuildValue("(iss)",rc,NULL,rm->rm_Result2);
							}
							else
							{
								/* Res2 is number */
								reso = Py_BuildValue("(iis)",rc,rm->rm_Result2,NULL);
							}

							waiting = FALSE;
						}
							
						FreeRexxCommand( rm );

						--ao->host->replies;
					}
						
					/* it's a command, error */
					else if( ARG0(rm) )
					{
						ReplyRexxCommand( rm, -20, (long)
							"invalid port", NULL );
					}
					
					rm = (struct RexxMsg *) GetMsg(ao->host->port);
				}
			}
			while( waiting );

			return reso;
		}
		else
		{
			Py_INCREF(Py_None);
			return Py_None;
		}
	} else return NULL;
}

static PyObject *port_setstringmsgs(arexxportobject *ao, PyObject *args)
{
	int i;
	if(!PyArg_ParseTuple(args,"i",&i)) return NULL;

	if(i==0)  ao->host->flags &= ~RHF_STRINGMSGS;
	else      ao->host->flags |=  RHF_STRINGMSGS;

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *port_settokenizeline(arexxportobject *ao, PyObject *args)
{
	int i;
	if(!PyArg_ParseTuple(args,"i",&i)) return NULL;

	if(i==0)  ao->host->flags &= ~RHF_TOKENIZELINE;
	else      ao->host->flags |=  RHF_TOKENIZELINE;

	Py_INCREF(Py_None);
	return Py_None;
}

static struct PyMethodDef port_methods[] = {
	{"close", (PyCFunction)port_close, 0},
	{"getmsg", (PyCFunction)port_getmsg, 0},
	{"wait", (PyCFunction)port_wait, 0},
	{"send", (PyCFunction)port_send, 1},
	{"setstringmsgs", (PyCFunction)port_setstringmsgs, 1 },
	{"settokenizeline", (PyCFunction)port_settokenizeline, 1 },
	{NULL,      NULL}       /* sentinel */
};

static void
port_dealloc(arexxportobject *self)         /* `destructor' */
{
	if(self->host) CloseDownARexxHost(self->host);
	PyMem_DEL(self);
}

static PyObject *
port_getattr(arexxportobject *ao, char *name)
{
	if(ao->host)
	{
		if (strcmp(name, "name")==0)
		{
			if(ao->host->flags & RHF_NOTPUBLIC)
			{
				/* A non-public port doesn't have a name! */
				PyErr_SetString(PyExc_AttributeError,name); return NULL;
			}   
			return PyString_FromString(ao->host->portname);
		}
		else if(strcmp(name,"signal")==0)
			return PyInt_FromLong(ao->signal);
	}
	return Py_FindMethod(port_methods, (PyObject *)ao, name);
}

static PyObject *
port_repr(arexxportobject *ao)
{
	char buf[200];
	char *w;

	if(!(ao->host))
		w="(closed)";
	else if(ao->host->flags & RHF_NOTPUBLIC)
		w="(private)";
	else
		w=ao->host->portname;
	sprintf(buf,"<arexx port %s at %lx>",w,(long)ao);
	return PyString_FromString(buf);
}

static PyTypeObject ARexxPorttype = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,          /*ob_size*/
	"arexxport",        /*tp_name*/
	sizeof(arexxportobject),    /*tp_size*/
	0,          /*tp_itemsize*/
	/* methods */
	(destructor)port_dealloc, /*tp_dealloc*/
	0,          /*tp_print*/
	(getattrfunc)port_getattr, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	0,          /*tp_compare*/
	(reprfunc)port_repr,        /*tp_repr*/
};

static PyObject *
newarexxportobject(char *name)              /* `constructor' */
{
	arexxportobject *ao;

	if(name)
	{
		if(strlen(name)<2)
		{
			PyErr_SetString(PyExc_ValueError,"port name too short");
			return NULL;
		}

		(void)strupr(name);
	}

	ao = PyObject_NEW(arexxportobject, &ARexxPorttype);
	if(ao)
	{
		ao->host = SetupARexxHost(name);
		if(ao->host)
		{
			ao->signal = 1<<ao->host->port->mp_SigBit;
			return (PyObject*)ao;
		}
		else PyErr_SetString(error,"can't open port");

		PyMem_DEL(ao); ao=NULL;
	}
	return (PyObject*)ao;
}

/* *** AREXXMSG OBJECT MEMBER FUNCTIONS *** */

static BOOL Test_Replied(arexxmsgobject *am)
{
	if(am->replied)
	{
		PyErr_SetString(error,"already replied");
		return FALSE;
	}
	return TRUE;
}

static PyObject *msg_reply(arexxmsgobject *am, PyObject *args)
{
	if(!PyArg_NoArgs(args)) return NULL;

	if(Test_Replied(am))
	{
		char *rc2, *result;

		rc2 = PyString_Check(am->rc2)? PyString_AsString(am->rc2) : NULL;
		result = PyString_Check(am->result)? PyString_AsString(am->result) : NULL;

		am->rc = -am->rc;       /* for ReplyRexxCommand; 'rc2 is string' */
		ReplyRexxCommand(am->msg,am->rc,(long)rc2,result);
		am->replied=TRUE;

		Py_INCREF(Py_None);
		return Py_None;
	} else return NULL;
}

static PyObject *msg_error(arexxmsgobject *am, PyObject *args)
{
	char *error_msg;
	if(!PyArg_ParseTuple(args,"s",&error_msg)) return NULL;

	if(Test_Replied(am))
	{
		/* Reply with error code and error message */
		ReplyRexxCommand(am->msg, 1, 0, error_msg);
		am->replied=TRUE;

		Py_INCREF(Py_None);
		return Py_None;
	} else return NULL;
}

static PyObject *msg_setvar(arexxmsgobject *am, PyObject *args)
{
	char *name, *val;
	int vlen;
	if(!PyArg_ParseTuple(args,"ss#",&name,&val,&vlen)) return NULL;

	if(Test_Replied(am))
	{
		if(CheckRexxMsg((struct Message*)am->msg))
		{
			if(0==SetRexxVar((struct Message*)am->msg,name,val,vlen))
			{
				Py_INCREF(Py_None);
				return Py_None;
			}
		}
		PyErr_SetString(error,"can't set var - invalid message?");
	}
	return NULL;
}

static PyObject *msg_getvar(arexxmsgobject *am, PyObject *args)
{
	char *name, *val;
	if(!PyArg_ParseTuple(args,"s",&name)) return NULL;

	if(Test_Replied(am))
	{
		if(CheckRexxMsg((struct Message*)am->msg))
		{
			if(0==GetRexxVar((struct Message*)am->msg,name,&val))
			{
				if(val!=NULL) return PyString_FromString(val);
			}
		}
		PyErr_SetString(error,"can't get var - invalid message?");
	}
	return NULL;
}

static struct PyMethodDef msg_methods[] = {
	{"reply", (PyCFunction) msg_reply, 0},
	{"error", (PyCFunction) msg_error, 1},
	{"setvar", (PyCFunction) msg_setvar, 1},
	{"getvar", (PyCFunction) msg_getvar, 1},
	{NULL,      NULL}       /* sentinel */
};

static void
msg_dealloc(arexxmsgobject *am)         /* `destructor' */
{   
	if(!am->replied)
	{
		char *rc2, *result;

		rc2 = PyString_Check(am->rc2)? PyString_AsString(am->rc2) : NULL;
		result = PyString_Check(am->result)? PyString_AsString(am->result) : NULL;

		am->rc = -am->rc;       /* for ReplyRexxCommand; 'rc2 is string' */
		ReplyRexxCommand(am->msg,am->rc,(long)rc2,result);
	}
	Py_DECREF(am->rc2);
	Py_DECREF(am->result);

	/* Do NOT delete the msg structure itself; we've got it from someone else! */

	PyMem_DEL(am);
}

static PyObject *
msg_getattr(arexxmsgobject *am, char *name)
{
	if (strcmp(name, "msg")==0)
		return PyString_FromString(ARG0(am->msg));
	if (strcmp(name, "rc")==0)
		return PyInt_FromLong(am->rc);
	if (strcmp(name, "rc2")==0)
	{
		Py_INCREF(am->rc2);
		return am->rc2;
	}
	if (strcmp(name, "result")==0)
	{
		Py_INCREF(am->result);
		return am->result;
	}
	if (strcmp(name, "wantresult")==0)
	{
		if( am->msg->rm_Action & RXFF_RESULT )
			return PyInt_FromLong(1);
		else
			return PyInt_FromLong(0);
	}

	return Py_FindMethod(msg_methods, (PyObject *)am, name);
}

static int
msg_setattr(arexxmsgobject *am, char *name, PyObject *args)
{
	if(args==NULL)
	{
		PyErr_SetString(PyExc_AttributeError,"can't delete msg attrs"); return -1;
	}

	if (strcmp(name, "rc")==0)
	{
		if(PyInt_Check(args))
		{
			long rc = PyInt_AsLong(args);
			if(rc>=0)
			{
				am->rc=rc;
			}
			else
			{
				PyErr_SetString(PyExc_ValueError,"rc must be >=0"); return -1;
			}
		}
		else
		{
			PyErr_SetString(PyExc_TypeError,"expecting int arg"); return -1;
		}
	}
	else if (strcmp(name, "rc2")==0)
	{
		if(PyString_Check(args))
		{
			Py_DECREF(am->rc2);
			Py_INCREF(args);
			am->rc2 = args;
		}
		else
		{
			PyErr_SetString(PyExc_TypeError,"expecting string arg"); return -1;
		}
	}
	else if (strcmp(name, "result")==0)
	{
		if(PyString_Check(args))
		{
			Py_DECREF(am->result);
			Py_INCREF(args);
			am->result = args;
		}
		else
		{
			PyErr_SetString(PyExc_TypeError,"expecting string arg"); return -1;
		}
	}
	else
	{
		PyErr_SetString(PyExc_AttributeError,name); return -1;
	}

	return 0;
}

static PyObject *
msg_repr(arexxmsgobject *am)
{
	char buf[100];
	sprintf(buf, "<arexx msg at %lx>", (long)am);
	return PyString_FromString(buf);
}

static PyTypeObject ARexxMsgtype = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,          /*ob_size*/
	"arexxmsg",        /*tp_name*/
	sizeof(arexxmsgobject),    /*tp_size*/
	0,          /*tp_itemsize*/
	/* methods */
	(destructor)msg_dealloc, /*tp_dealloc*/
	0,          /*tp_print*/
	(getattrfunc)msg_getattr, /*tp_getattr*/
	(setattrfunc)msg_setattr, /*tp_setattr*/
	0,          /*tp_compare*/
	(reprfunc)msg_repr,        /*tp_repr*/
};

static PyObject *
newarexxmsgobject(struct RexxMsg *msg)             /* `constructor' */
{
	arexxmsgobject *am;

	am = PyObject_NEW(arexxmsgobject, &ARexxMsgtype);
	if(am)
	{
		am->msg=msg;
		am->rc=0;
		Py_INCREF(Py_None); am->rc2=Py_None;
		Py_INCREF(Py_None); am->result=Py_None;
		am->replied=FALSE;
	}
	return (PyObject*)am;
}

/******************************* AREXX SUPPORT FUNCTIONS ****************** */

struct RexxMsg *CreateRexxCommand( struct RexxHost *host, char *buff, BPTR fh )
{
	struct RexxMsg *rexx_command_message;

	rexx_command_message = CreateRexxMsg( host->port,
		"python", host->port->mp_Node.ln_Name);
	if( rexx_command_message == NULL )
	{
		return( NULL );
	}

	rexx_command_message->rm_Args[0] = CreateArgstring(buff,strlen(buff));
	if( rexx_command_message->rm_Args[0] == NULL )
	{
		DeleteRexxMsg(rexx_command_message);
		return( NULL );
	}

	rexx_command_message->rm_Action = RXCOMM | RXFF_RESULT;
	rexx_command_message->rm_Stdin  = fh;
	rexx_command_message->rm_Stdout = fh;

	if(host->flags & RHF_STRINGMSGS)
		rexx_command_message->rm_Action |= RXFF_STRING;
	if(host->flags & RHF_TOKENIZELINE)
		rexx_command_message->rm_Action |= RXFF_TOKEN;

	return( rexx_command_message );
}

static void ReplyRexxCommand(
	struct RexxMsg  *rexxmessage,
	long            primary,
	long            secondary,
	char            *result )
{
	if( rexxmessage->rm_Action & RXFF_RESULT )
	{
		if( primary == 0 )
		{
			secondary = result
				? (long) CreateArgstring( result, strlen(result) )
				: (long) NULL;
		}
		else
		{
			char buf[16];
			
			if( primary > 0 )
			{
				sprintf( buf, "%ld", secondary );
				result = buf;
			}
			else
			{
				primary = -primary;
				result = (char *) secondary;
			}

			if(CheckRexxMsg((struct Message*)rexxmessage))
			{
				SetRexxVar( (struct Message *) rexxmessage,
					"RC2", result, strlen(result) );
			}

			secondary = 0;
		}
	}
	else if( primary < 0 )
		primary = -primary;
	
	rexxmessage->rm_Result1 = primary;
	rexxmessage->rm_Result2 = secondary;
	ReplyMsg( (struct Message *) rexxmessage );
}

static void FreeRexxCommand( struct RexxMsg *rexxmessage )
{
	if( !rexxmessage->rm_Result1 && rexxmessage->rm_Result2 )
		DeleteArgstring( (char *) rexxmessage->rm_Result2 );

	if( rexxmessage->rm_Stdin &&
		rexxmessage->rm_Stdin != Input() )
		Close( rexxmessage->rm_Stdin );

	if( rexxmessage->rm_Stdout &&
		rexxmessage->rm_Stdout != rexxmessage->rm_Stdin &&
		rexxmessage->rm_Stdout != Output() )
		Close( rexxmessage->rm_Stdout );

	DeleteArgstring( (char *) ARG0(rexxmessage) );
	DeleteRexxMsg( rexxmessage );
}

static void CloseDownARexxHost( struct RexxHost *host )
{
	struct RexxMsg *rexxmsg;
	
	if( host->port )
	{
		/* Remove port */
		if(!(host->flags & RHF_NOTPUBLIC))
			RemPort( host->port );
		
		/* Wait for pending replies */
		while( host->replies > 0 )
		{
			WaitPort( host->port );
			
			rexxmsg = (struct RexxMsg *) GetMsg(host->port);
			while( rexxmsg )
			{
				if( rexxmsg->rm_Node.mn_Node.ln_Type == NT_REPLYMSG )
				{
					FreeRexxCommand( rexxmsg );
					--host->replies;
				}
				else
					ReplyRexxCommand( rexxmsg, -20, (long) "Host closing down", NULL );
			}
		}
		
		/* flush the MsgPort */
		rexxmsg = (struct RexxMsg *) GetMsg(host->port);
		while( rexxmsg )
			ReplyRexxCommand( rexxmsg, -20, (long) "Host closing down", NULL );
		
/*	    if( !(host->flags & RHF_USRMSGPORT) ) */
			DeleteMsgPort( host->port );
	}
	
	free( host );
}

static struct RexxHost *SetupARexxHost( char *basename)
{
	struct RexxHost *host;
	int ext = 0;
	
	host = calloc(sizeof *host, 1);
	if( !host )
		return NULL;

	if(basename) strcpy( host->portname, basename );
	else host->flags |= RHF_NOTPUBLIC;

	host->port = CreateMsgPort();
	if( !host->port )
	{
		free( host );
		return NULL;
	}
	else
	{
		host->port->mp_Node.ln_Pri = 0;
	}
	

	if(!(host->flags & RHF_NOTPUBLIC))
	{
		Forbid();
		while( FindPort(host->portname) )
			sprintf( host->portname, "%s.%d", basename, ++ext );

		host->port->mp_Node.ln_Name = host->portname;
		AddPort( host->port );

		Permit();
	}

	/* default flags */
	/* host->flags |= RHF_STRINGMSGS; */
	/* host->flags |= RHF_TOKENIZELINE; */

	return( host );
}

/* GetARexxMsg:
** returns the RexxMsg that is waiting at the port, 
** or NULL if no message is present. 
*/
static struct RexxMsg *GetARexxMsg( struct RexxHost *host )
{
	struct RexxMsg *rexxmsg;

	rexxmsg = (struct RexxMsg *) GetMsg(host->port);
	while( rexxmsg )
	{
		if( (rexxmsg->rm_Action & RXCODEMASK) != RXCOMM )
		{
			/* Not a Rexx-Message */
			ReplyMsg( (struct Message *) rexxmsg );
		}
		else if( rexxmsg->rm_Node.mn_Node.ln_Type == NT_REPLYMSG )
		{
			struct RexxMsg *org = (struct RexxMsg *) rexxmsg->rm_Args[15];
			
			if( org )
			{
				/* Reply through the message */
				if( rexxmsg->rm_Result1 != 0 )
				{
					/* Command unknown */
					ReplyRexxCommand( org, 20, ERROR_NOT_IMPLEMENTED, NULL );
				}
				else
				{
					ReplyRexxCommand( org, 0, 0, (char *) rexxmsg->rm_Result2 );
				}
			}

			FreeRexxCommand( rexxmsg );
			--host->replies;
		}
		else if( ARG0(rexxmsg) )
		{
			return rexxmsg;     /* return the ARexx message! */
		}
		else
		{
			ReplyMsg( (struct Message *) rexxmsg );
		}
	}

	return NULL;    /* no important message arrived. */
}

struct RexxMsg *SendRexxCommandToPort( struct RexxHost *host, char *port, char *cmd, BPTR fh )
{
	struct RexxMsg *rcm;
	
	rcm = CreateRexxCommand(host, cmd, fh);
	if( rcm )
	{
		struct MsgPort *rexxport;
	
		Forbid();

		rexxport = FindPort(port);
		if( rexxport == NULL )
		{
			Permit();
			return( NULL );
		}

		PutMsg( rexxport, &rcm->rm_Node );
	
		Permit();
	
		++host->replies;
		return( rcm );
	}
	else
		return NULL;
}

/* *** MODULE FUNCTIONS *** */

static PyObject *
ARexx_openport(PyObject *self, PyObject *args)
{
	PyObject *p=NULL;
	
	if (!PyArg_ParseTuple(args, "|O", &p))
		return NULL;

	if(!p)
		return newarexxportobject("PYTHON");
	else if(p==Py_None)
		/* open anonymous port (only for sending) */
		return newarexxportobject(NULL);
	else if(PyString_Check(p))
		return newarexxportobject(PyString_AsString(p));
	else
		return (PyObject*)PyErr_BadArgument();
}

static PyObject *
ARexx_errorstring(PyObject *self, PyObject *args)
{
/* undocumented function: */
	long err;
	char *errmsg;
	
#ifdef __SASC
	extern BOOL ErrorMsg(LONG err);
	#pragma libcall RexxSysBase ErrorMsg 60 001
#endif

#ifdef __VBCC__
	/* ErrorMsg is a private RexxSysLib function - offset 60 */
	#define ErrorMsg(err) ({ \
	  LONG _ErrorMsg_err = (err); \
	  ({ \
	  register char * _ErrorMsg__bn __asm("a6") = (char *) (REXXSYSLIB_BASE_NAME);\
	  ((BOOL (*)(char * __asm("a6"), LONG __asm("d0"))) \
	  (_ErrorMsg__bn - 60))(_ErrorMsg__bn, _ErrorMsg_err); \
	});})
#endif

	if (!PyArg_ParseTuple(args, "i", &err))
		return NULL;

	if(ErrorMsg(err))
	{
		return PyString_FromString("");
	}
	else
	{
		PyErr_SetString(PyExc_ValueError,"invalid error code");
		return NULL;
	}
}

/* High-level dorexx function for simple command sending */
static PyObject *
ARexx_dorexx(PyObject *self, PyObject *args)
{
	char *port, *message;
	
	if (!PyArg_ParseTuple(args, "ss", &port, &message))
		return NULL;

	/* Create a temporary anonymous host for sending */
	struct RexxHost *temp_host = SetupARexxHost(NULL);
	if (!temp_host)
	{
		PyErr_SetString(error, "can't create temporary host");
		return NULL;
	}

	/* Send the command synchronously */
	struct RexxMsg *sentrm = SendRexxCommandToPort(temp_host, port, message, NULL);
	if (!sentrm)
	{
		CloseDownARexxHost(temp_host);
		PyErr_SetString(error, "can't send to port");
		return NULL;
	}

	/* Wait for the reply */
	PyObject *result = NULL;
	long rc;
	struct RexxMsg *rm;
	BOOL waiting = TRUE;
	
	do
	{
		WaitPort(temp_host->port);
			
		rm = (struct RexxMsg *) GetMsg(temp_host->port);
		while(rm)
		{
			/* Reply? */
			if(rm->rm_Node.mn_Node.ln_Type == NT_REPLYMSG)
			{
				/* 'our' Msg? */
				if(rm == sentrm)
				{
					rc = rm->rm_Result1;
						
					if(!rc && rm->rm_Result2)
					{
						/* Res2 is String */
						result = Py_BuildValue("(iss)", rc, NULL, rm->rm_Result2);
					}
					else
					{
						/* Res2 is number */
						result = Py_BuildValue("(iis)", rc, rm->rm_Result2, NULL);
					}

					waiting = FALSE;
				}
					
				FreeRexxCommand(rm);
				--temp_host->replies;
			}
				
			/* it's a command, error */
			else if(ARG0(rm))
			{
				ReplyRexxCommand(rm, -20, (long) "invalid port", NULL);
			}
			
			rm = (struct RexxMsg *) GetMsg(temp_host->port);
		}
	}
	while(waiting);

	/* Clean up */
	CloseDownARexxHost(temp_host);
	
	return result;
}

/*** FUNCTIONS FROM THE MODULE ***/

static struct PyMethodDef ARexx_global_methods[] = {
	{"port",  ARexx_openport, 1},
	{"errorstring", ARexx_errorstring, 1},
	{"dorexx", ARexx_dorexx, 1},
	{NULL,      NULL}       /* sentinel */
};

void
initarexx(void)
{
	PyObject *m, *d;

	m = Py_InitModule3("arexx", ARexx_global_methods, 
		"Amiga ARexx module for Python.\n\n"
		"This module provides support for ARexx communication and ARexx hosts.\n"
		"The module's methods may only be used if the module has been imported.\n\n"
		"Functions:\n"
		"  port(portname) -- Create a new ARexx port object\n"
		"  dorexx(port, message) -- Send ARexx message to port, returns (rc, rc2, result)\n"
		"  errorstring(err) -- Get error string for ARexx error code\n\n"
		"Classes:\n"
		"  Port -- ARexx port object with methods: close(), getmsg(), wait(), send()\n"
		"  Msg -- ARexx message object with methods: reply(), error(), setvar(), getvar()\n\n"
		"Example:\n"
		"  import arexx\n"
		"  rc, rc2, result = arexx.dorexx('WORKBENCH', 'menu window root invoke workbench.about')");
	d = PyModule_GetDict(m);

	/* Initialize error exception */
	error = PyErr_NewException("arexx.error", NULL, NULL);
	if (error != NULL)
		PyDict_SetItemString(d, "error", error);
}
