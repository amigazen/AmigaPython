
/* AmigaPython embedded interpreter init and cleanup code. */
/* Made 19 October 1999 by Irmen de Jong */
/* Based on AmigaPython specific changes in Modules/main.c */

/* THE main exported function from this module is:   */

/*    void InitAmigaPython(int argc, char **argv)    */

/* which should be called even before Py_Initialize. */



#include <proto/dos.h>
#include <proto/exec.h>
#include "Python.h"


/* For Py_GetArgcArgv(); set by Init */
static char **orig_argv;
static int  orig_argc;


#ifdef INET225
#include <proto/socket.h>

struct Library *SockBase = NULL;

int checksocketlib(void)
{
	if(!SockBase)
	{
		if(SockBase=OpenLibrary("inet:libs/socket.library",4))
		{
			setup_sockets(FD_SETSIZE, &errno);
		}
		else
		{
			PyErr_SetString(PyExc_SystemError, "Couldn't open inet:libs/socket.library V4+ (I-Net225 started?)");
			return 0;
		}
	}
	return 1;
}

int checkusergrouplib(void)
{
	return checksocketlib();
}

#endif /* INET225 */

#ifdef AMITCP
#include <proto/socket.h>
#include <amitcp/socketbasetags.h>

/* proto for special AmiTCP utility funcion; see _chkufb.c */
extern long _install_AmiTCP_callback(void);


/* global h_errno */
int h_errno = 0;

struct Library *UserGroupBase = NULL;
struct Library *SocketBase = NULL;

int checkusergrouplib(void)
{
	if(!UserGroupBase)
	{
		if(!(UserGroupBase=OpenLibrary("usergroup.library",4)))
		{
			PyErr_SetString(PyExc_SystemError, "Couldn't open usergroup.library");
			return 0;
		}
	}
	return 1;
}

int checksocketlib(void)
{
	if(!SocketBase)
	{
		if(SocketBase=OpenLibrary("bsdsocket.library",4))
		{
			/*
			 * Succesfull. Now tell bsdsocket.library:
			 * - the address of our errno
			 * - the address of our h_errno
			 * - our program name
			 */
			SocketBaseTags(SBTM_SETVAL(SBTC_ERRNOPTR(sizeof(errno))), &errno,
						   SBTM_SETVAL(SBTC_HERRNOLONGPTR), &h_errno,
						   SBTM_SETVAL(SBTC_LOGTAGPTR), "Python",
						   TAG_END);
		}
		else
		{
			PyErr_SetString(PyExc_SystemError, "Couldn't open bsdsocket.library (start AmiTCP)");
			return 0;
		}
	}
	return 1;
}

#endif

BOOL from_WB = FALSE;	/* not static! getpath.c needs it! */

static void AmigaCleanup(void)
{
#ifdef AMITCP
	if(UserGroupBase)
	{
		CloseLibrary(UserGroupBase);
		UserGroupBase=NULL;
	}
	if(SocketBase)
	{
		CloseLibrary(SocketBase);
		SocketBase=NULL;
	}
#endif
#ifdef INET225
	if(SockBase)
	{
		cleanup_sockets();
		CloseLibrary(SockBase);
		SockBase = NULL;
	}
#endif
	if(from_WB) Delay(112); // small exit delay before closing WB window
}


/* Init routine for embedded applications */

void InitAmigaPython(int argc, char **argv)
{
	if(argc == 0)
	{
		/* Invoked from WorkBench... use WorkBench arguments  */
		/* Make sure <dos.h> was included earlier in order to */
		/* declare _WBArgc and _WBArgv.                       */
		argc = _WBArgc;
		argv = _WBArgv;
		from_WB = TRUE;
	}

	atexit(AmigaCleanup);   /* cleanup func */
//  setbuf(stderr,NULL);    /* set error output UNBUFFERED */

	orig_argc = argc;	/* For Py_GetArgcArgv() */
	orig_argv = argv;

	Py_SetProgramName(argv[0]);

#ifdef AMITCP
	/** Sadly, this function cannot be called as a SAS/C standard **/
	/** constructor function. It needs to have the interpreter **/
	/** initialised because it uses the Python Error functions... **/
	(void)_install_AmiTCP_callback();
#endif

}


/* Make the *original* argc/argv available to other modules.
   This is rare, but it is needed by the secureware extension. */

void Py_GetArgcArgv(int *argc,  char ***argv)
{
	*argc = orig_argc;
	*argv = orig_argv;
}


