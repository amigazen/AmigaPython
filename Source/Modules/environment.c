/*
# Extended Environment module.
#
# Intended for use on the Amiga. Made by Irmen de Jong.
#
# This module contains all sorts of functions that operate on the global
# or local environment of Python. Global environment = ENV: -vars,
# local environment = the local shell variables.
#
#  11-Apr-96:    Rewrite. This module does not have to be posix compliant
#                or portable, so...
#  12-Jun-96:    Fixed zero-length var bugs. Renamed to new symbolnames.
#  18-jan-98:    Updated for Python1.5
*/

#include "Python.h"
#include "protos.h"
#include <stdlib.h>
#include <string.h>

/* protos */
static PyObject *put_environ Py_PROTO((PyObject *self, PyObject *args));
static PyObject *get_environ Py_PROTO((PyObject *self, PyObject *args));
static PyObject *set_environ Py_PROTO((PyObject *self, PyObject *args));
static PyObject *unset_environ Py_PROTO((PyObject *self, PyObject *args));
static PyObject *get_var Py_PROTO((PyObject *self, PyObject *args));
static PyObject *set_var Py_PROTO((PyObject *self, PyObject *args));
static PyObject *unset_var Py_PROTO((PyObject *self, PyObject *args));
#ifdef _AMIGA
/* OS4 amigavars API (GetEnv / SetEnv / UnSetEnv) */
static PyObject *amiga_GetEnv Py_PROTO((PyObject *self, PyObject *args));
static PyObject *amiga_SetEnv Py_PROTO((PyObject *self, PyObject *args));
static PyObject *amiga_UnSetEnv Py_PROTO((PyObject *self, PyObject *args));
#endif

 
static struct PyMethodDef environment_methods[] = {
        {"putenv", put_environ,1},
        {"setenv", set_environ,1},
        {"getenv", get_environ,1},
        {"unsetenv", unset_environ,1},
#ifdef _AMIGA
        {"setvar", set_var,1},
        {"getvar", get_var,1},
        {"unsetvar", unset_var,1},
        /* OS4 amigavars names (same semantics; see site-python/amigavars.py). */
        {"GetEnv", amiga_GetEnv,1},
        {"SetEnv", amiga_SetEnv,1},
        {"UnSetEnv", amiga_UnSetEnv,1},
#endif
        {NULL, NULL}
};
 

void initenvironment(void)
{
    (void)Py_InitModule3("environment", environment_methods,
        "Extended Environment module for Amiga. "
        "getenv/setenv/unsetenv and getvar/setvar/unsetvar for ENV: and "
        "local shell vars; GetEnv/SetEnv/UnSetEnv match OS4 amigavars "
        "(optional ENVARC: save/delete).");
}
   
/*
**  putenv("name=value")	-- this is putenv(3)
*/
static PyObject *put_environ(PyObject *self, PyObject *args)
{
	char *string;
	if(PyArg_ParseTuple(args,"s",&string))
	{
		if ( putenv( string ) )
		{
			PyErr_SetString(PyExc_SystemError, "Error in system putenv call");
			return NULL;
		}
		Py_INCREF(Py_None); return Py_None;
	}
	return NULL;
}

/*
** setenv("name","value",overwrite?)
*/
static PyObject *set_environ(PyObject *self, PyObject *args)
{
	char *name, *value;
	int overwrite;

	if(PyArg_ParseTuple(args,"ssi",&name,&value,&overwrite))
	{
		if ( setenv(name,value,overwrite) )
		{
			PyErr_SetString(PyExc_SystemError, "Error in system setenv call");
			return NULL;
		}
		Py_INCREF(Py_None); return Py_None;
	}
	return NULL;
}

/*
** value = getenv("name")
*/
static PyObject *get_environ(PyObject *self, PyObject *args)
{
	char *name, *val;
	if(PyArg_ParseTuple(args,"s",&name))
	{
		if(val=getenv(name))
		{
			PyObject *s = PyString_FromString(val);
			free(val);
			return s; /* ok if s=NULL */
		}
		Py_INCREF(Py_None); return Py_None; /* var not found in env. */
	}
	return NULL;
}

/*
** unsetenv("name")
*/
static PyObject *unset_environ(PyObject *self, PyObject *args)
{
	char *string;
	if(PyArg_ParseTuple(args,"s",&string))
	{
		unsetenv(string);
		Py_INCREF(Py_None); return Py_None;
	}
	return NULL;
}

#ifdef _AMIGA
/* Define DEVICES_TIMER_H to prevent timer.h conflicts with Amiga headers */
#define DEVICES_TIMER_H
#include <proto/dos.h>
#include <dos/var.h>

/*
** setvar("name","value",overwrite?)
*/
static PyObject *set_var(PyObject *self, PyObject *args)
{
	char *name, *value;
	int overwrite;

	if(PyArg_ParseTuple(args,"ssi",&name,&value,&overwrite))
	{
		if(!overwrite && FindVar(name,GVF_LOCAL_ONLY))
		{
			Py_INCREF(Py_None); return Py_None;
		}

		if(SetVar(name,value,-1,GVF_LOCAL_ONLY))
		{
			Py_INCREF(Py_None); return Py_None;
		}		
		PyErr_SetString(PyExc_SystemError, "Error in dos SetVar call");
		return NULL;
	}
	return NULL;
}

/*
** value = getvar("name")
*/
static PyObject *get_var(PyObject *self, PyObject *args)
{
	char *name;
	char buf[200];

	if(PyArg_ParseTuple(args,"s",&name))
	{
		if(GetVar(name,buf,200,GVF_LOCAL_ONLY)>=0)
		{
			PyObject *s = PyString_FromString(buf);
			return s; /* ok if s=NULL */
		}
		Py_INCREF(Py_None); return Py_None; /* var not found */
	}
	return NULL;
}

/*
** unsetvar("name")
*/
static PyObject *unset_var(PyObject *self, PyObject *args)
{
	char *string;
	if(PyArg_ParseTuple(args,"s",&string))
	{
		DeleteVar(string,GVF_LOCAL_ONLY);
		Py_INCREF(Py_None); return Py_None;
	}
	return NULL;
}

/*
 * OS4 amigavars: GetEnv(name) -> string or None.
 * Global ENV: only (GVF_GLOBAL_ONLY).
 */
static PyObject *amiga_GetEnv(PyObject *self, PyObject *args)
{
	char *name;
	char buf[2048];
	LONG len;

	if (!PyArg_ParseTuple(args, "s:GetEnv", &name))
		return NULL;
	len = GetVar(name, buf, (LONG)sizeof(buf), GVF_GLOBAL_ONLY);
	if (len >= 0)
		return PyString_FromString(buf);
	Py_INCREF(Py_None);
	return Py_None;
}

/*
 * OS4 amigavars: SetEnv(name, value, save=0).
 * save True -> also write ENVARC: (GVF_SAVE_VAR).
 */
static PyObject *amiga_SetEnv(PyObject *self, PyObject *args)
{
	char *name;
	char *value;
	int save;
	ULONG flag;

	save = 0;
	if (!PyArg_ParseTuple(args, "ss|i:SetEnv", &name, &value, &save))
		return NULL;
	flag = GVF_GLOBAL_ONLY;
	if (save)
		flag |= GVF_SAVE_VAR;
	if (!SetVar(name, value, -1, flag)) {
		PyErr_SetString(PyExc_SystemError, "Error in dos SetVar call");
		return NULL;
	}
	Py_INCREF(Py_None);
	return Py_None;
}

/*
 * OS4 amigavars: UnSetEnv(name, delete=0).
 * delete True -> also remove ENVARC: (GVF_SAVE_VAR).
 */
static PyObject *amiga_UnSetEnv(PyObject *self, PyObject *args)
{
	char *name;
	int delete_flag;
	ULONG flag;

	delete_flag = 0;
	if (!PyArg_ParseTuple(args, "s|i:UnSetEnv", &name, &delete_flag))
		return NULL;
	flag = GVF_GLOBAL_ONLY;
	if (delete_flag)
		flag |= GVF_SAVE_VAR;
	DeleteVar(name, flag);
	Py_INCREF(Py_None);
	return Py_None;
}

#endif
