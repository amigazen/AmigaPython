
/********************************************************************

	Lowlevel amiga shared library support.

	Provides `amigalib' object, which is an opened library.
	The following methods are defined on the object:

		call 		- perform lib call
		base		- return library base
		version		- return library version & revision
		repr		- print object

	The `amigalibs' module provides the following functions:

		openlib		- open library (return amigalib object)
		obj2reg		- convert Python object to 680x0 ULONG register value
					  (needed for library calls, to pass python int or string
					  objects)
					  Handy feature: returns 0 for None objects.
		addr		- get address of value of passed python object.
					  For strings: the same as obj2reg.
					  Handy feature: returns 0 for None objects.
		fixstr		- Fix a string that contains a \0 character (truncate it).
					  Returns new string.

	Note: libraries closes themself only (but always) when the object
		  is discarded.

-----------------------------------------------
	(c) 1996 Irmen de Jong.

	History:

	15-feb-96	Created.
	17-feb-96	Added version memberfunc + lvo tuple calling convention.
	18-feb-96	Implemented register argument passing.
				Added addr.
	20-feb-96	Added fixstr.
	24-feb-96	Added check for exec.library -> 4.w instead of OpenLibrary()

	TODO:
	- some sort of structure support, including pointers.

**************************************************************************/



#include <exec/libraries.h>
#include <proto/exec.h>


#include "allobjects.h"
#include "modsupport.h"


#ifndef min
#define min(a,b) ((a)<=(b) ? (a):(b))
#endif

static object *AmigalibsError;	/* Exception */	

typedef struct {
	OB_HEAD
	struct Library *libbase;	/* the library base */
	char* libname;				/* library name */
} amigalibobject;



/* Prototypes for functions defined in amigalibsmodule.c */

/* Assembly functions to be found in amigalibsmodule_asm.asm */

extern __asm __regargs ULONG
amigalibs_calllib(register __a0 struct Library *libbase, 
				  register __d0 LONG LVOvalue,
				  register __d1 UWORD regspec,
				  register __a1 ULONG *regs);

static void lib_dealloc(amigalibobject * );
static PyObject * lib_call(amigalibobject * , PyObject * );
static PyObject * lib_getattr(amigalibobject * , unsigned char * );
static PyObject * lib_repr(amigalibobject * );
static PyObject * newamigalibobject(unsigned char * , int );
static PyObject * amigalibs_openlib(PyObject * , PyObject * );
static BOOL obj2reg(PyObject * , ULONG *val);
static PyObject * amigalibs_obj2reg(PyObject * , PyObject * );
static PyObject * amigalibs_addr(PyObject * , PyObject * );
static PyObject * amigalibs_fixstr(PyObject * , PyObject * );



/******************************** amigalib object methods ****************/

/*** AMIGALIB OBJECT MEMBER FUNCTIONS ***/

static struct methodlist lib_methods[] = {
	{"call",	(method)lib_call  , 1},
	{NULL,		NULL}		/* sentinel */
};


static void
lib_dealloc(amigalibobject *self)			// `destructor'
{
	if(self->libbase)
	{
		if(strcmp(self->libname,"exec.library")!=0)
			CloseLibrary(self->libbase);

		self->libbase=NULL;
	}
	DEL(self);
}


/*
**	CALL - perform library call
*/
static object *
lib_call(amigalibobject *self, object *args)				// call
{
	object *dic;
	int LVO_value;
	ULONG regspec, orig_regspec;
	int pos=0;
	object *key, *value;

	/* the registers: */
	ULONG reg[16];

	if (!newgetargs(args, "(ii)O", &LVO_value, &regspec, &dic)) return NULL;

	if(!PyDict_Check(dic))
	{
		err_setstr(TypeError, "2nd arg must be dictionary");
		return NULL;
	}

	if(LVO_value>-30 || LVO_value%2)
	{
		err_setstr(ValueError, "illegal LVO value");
		return NULL;
	}

	if(regspec<0 || regspec >0xFFFF)
	{
		err_setstr(ValueError, "illegal regspec");
		return NULL;
	}
	
	orig_regspec=regspec;

	while(PyDict_Next(dic,&pos,&key,&value))
	{
		int regnr =  PyInt_AsLong(key);
		if(!PyInt_Check(key) || regnr<0 || regnr>15)
		{
			err_setstr(ValueError, "illegal key/regnr.");
			return NULL;
		}
		else
		{
			ULONG regval;
			if(!obj2reg(value,&regval))
			{
				err_setstr(ValueError, "illegal register value");
				return NULL;
			}

			/* OK: the register `regnr' must get the value `regval' */
			/* Check if register regnr indeed must get a value */
			if(regspec & (1<<regnr))
			{
				/* Yes, put value in register & clear register bit */
				reg[regnr]=regval;
				regspec &= ~(1<<regnr);
			}
			else
			{
				err_setstr(ValueError, "registers not consistent with LVO spec");
				return NULL;
			}
		}
	}

	/* Are there still registers to assign a value to? */
	if(regspec)
	{
		err_setstr(ValueError, "too few arguments provided");
		return NULL;
	}

	/* XXXX TODO: SHOULD PASS ARGUMENTS ALSO */
	return newintobject(
			amigalibs_calllib(self->libbase,
				LVO_value,orig_regspec,reg));
}


static object *
lib_getattr(amigalibobject *ao, char *name)
{
	if (strcmp(name, "base") == 0)
		return newintobject((long)ao->libbase);
	if (strcmp(name, "version") == 0)
		return mkvalue("(ii)", ao->libbase->lib_Version, ao->libbase->lib_Revision);

	return findmethod(lib_methods, (object *)ao, name);
}

static object *
lib_repr(amigalibobject *ao)
{
    char buf[300];
    sprintf(buf, "<amiga library '%.256s', V %ld.%ld, base %lx, at %lx>",
		ao->libbase->lib_Node.ln_Name,
		ao->libbase->lib_Version,
		ao->libbase->lib_Revision,
		ao->libbase,(long)ao);
    return newstringobject(buf);
}

static typeobject Amigalibtype = {
	OB_HEAD_INIT(&Typetype)
	0,			/*ob_size*/
	"amigalib",		/*tp_name*/
	sizeof(amigalibobject),	/*tp_size*/
	0,			/*tp_itemsize*/
	/* methods */
	(destructor)lib_dealloc, /*tp_dealloc*/
	0,			/*tp_print*/
	(getattrfunc)lib_getattr, /*tp_getattr*/
	0,			/*tp_setattr*/
	0,			/*tp_compare*/
	(reprfunc)lib_repr,		/*tp_repr*/
};



/******************************** MODULE FUNCTIONS ************************/

static object *
newamigalibobject(char *libname, int libver)
{
	amigalibobject *ao;

	if(ao = NEWOBJ(amigalibobject, &Amigalibtype))
	{
		if(strcmp(libname,"exec.library")==0)
			ao->libbase = *(struct Library**)4L;
		else
			ao->libbase = OpenLibrary(libname,libver);

		if(!ao->libbase)
		{
			err_setstr(AmigalibsError, "can't open library");
			DECREF(ao);
			ao = NULL;
		}
	}
	return (object *)ao;
}


static object *
amigalibs_openlib(object *self, object *args)
{
	char *libname;
	int libver;
	
	if (!getargs(args, "(si)", &libname, &libver))
		return NULL;

	return newamigalibobject(libname,libver);
}


static BOOL
obj2reg(object *arg, ULONG *val)
{
	     if(PyInt_Check(arg)) *val=(ULONG)PyInt_AsLong(arg);
	else if(PyString_Check(arg)) *val=(ULONG)PyString_AsString(arg);
	else if(PyFloat_Check(arg)) *val=(ULONG)PyFloat_AsDouble(arg);
	else if(arg==Py_None) *val=0;

//	else if(PyLong_Check(arg)) *val=
//	else if(PyAccess_Check(arg)) *val=
//	else if(PyAnyNumber_Check(arg)) *val=
//	else if(PyAnySequence_Check(arg)) *val=
//	else if(PyAnyMapping_Check(arg)) *val=
//	else if(PyNothing_Check(arg)) *val=
//	else if(PyType_Check(arg)) *val=
//	else if(PyList_Check(arg)) *val=
//	else if(PyDict_Check(arg)) *val=
//	else if(PyTuple_Check(arg)) *val=
//	else if(PyFile_Check(arg)) *val=
//	else if(PyClass_Check(arg)) *val=
//	else if(PyFunction_Check(arg)) *val=
//	else if(PyMethod_Check(arg)) *val=
//	else if(PyInstance_Check(arg)) *val=
//	else if(PyCFunction_Check(arg)) *val=
//	else if(PyModule_Check(arg)) *val=
//	else if(PyCode_Check(arg)) *val=
//	else if(PyFrame_Check(arg)) *val=
//	else if(PyRange_Check(arg)) *val=

	else return FALSE;

	return TRUE;
}

static object *
amigalibs_obj2reg(object *self, object *arg)
{
	ULONG res;
	if(!arg)
		return (object*) PyErr_BadArgument();

	if(obj2reg(arg, &res))
		return newintobject(res);
	else
	{
		err_setstr(TypeError, "can't convert type to register ULONG");
		return NULL;
	}
}

static object *
amigalibs_addr(object *self, object *arg)
{
	if(arg)
	{
		if(PyInt_Check(arg))
			return newintobject((long) &((PyIntObject*)arg)->ob_ival);
		else if(PyFloat_Check(arg))
			return newintobject((long) &((PyFloatObject*)arg)->ob_fval);
		else if(PyString_Check(arg))
			return newintobject((long) &((PyStringObject*)arg)->ob_sval);
		else if(arg==Py_None)
			return newintobject(0);
	}
	return (object*)PyErr_BadArgument();
}

static object *
amigalibs_fixstr(object *self, object *arg)
{
	if(arg && PyString_Check(arg))
	{
		char * str = PyString_AS_STRING((PyStringObject*)arg);
		int len = PyString_Size(arg);
		int len2 = strlen(str);
		return PyString_FromStringAndSize(str,min(len,len2));
	}
	return (object*)PyErr_BadArgument();
}


/*** FUNCTIONS FROM THE MODULE ***/

static struct methodlist amigalibs_global_methods[] = {
	{"openlib",	amigalibs_openlib, 0},
	{"obj2reg", amigalibs_obj2reg, 0},
	{"addr",  amigalibs_addr, 0},
	{"fixstr",  amigalibs_fixstr, 0},
	{NULL,		NULL}		/* sentinel */
};


void
initamigalibs Py_PROTO((void))
{
	object *m, *d;
	
	m = initmodule("amigalibs", amigalibs_global_methods);
	d = getmoduledict(m);
	
	/* Initialize amigalibs.error exception */
	AmigalibsError = PyErr_NewException("amigalibs.error", NULL, NULL);

	if (AmigalibsError != NULL)
		dictinsert(d, "error", AmigalibsError);
}
