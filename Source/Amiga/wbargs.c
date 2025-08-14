/* Copyright (c) 1993 SAS Institute, Inc, Cary, NC USA */
/* All Rights Reserved */

/* Adapted for Python1.4 by Irmen de Jong, 28 dec. 1996 */

/* Changed the order in which the Workbench Tooltypes are
   passed to the interpreter. For Python 1.5.1 build 13. */

#include <workbench/startup.h>
#include <exec/execbase.h>
#include <string.h>
#include <dos.h>
#include <stdlib.h>
#include <constructor.h>

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/icon.h>

/* These two symbols, _WBArgc and _WBArgv, are initialized if     */
/* the program was invoked from WorkBench.  They look like normal */
/* C (argc, argv) parameters.  The parameters are gathered as     */
/* follows:                                                       */
/*   Name of the program                                          */
/*   Any tooltypes specified in the ToolTypes array               */
/*   Any icons supplied as arguments (with SHIFT-CLICK)           */

int _WBArgc;       /* Count of the number of WorkBench arguments */
char **_WBArgv;    /* The actual arguments                       */

static int _WBArgMax;  /* Internal: tells us how much space is left */
					   /* in _WBArgv.                               */

// NAME:    FullName
// PURPOSE: get the full pathname of a file.
static void FullName(BPTR parent, char *name, char *buf, int len)
{
	int i = 0;
	if(NameFromLock(parent, buf, len-1))
	{
		i = strlen(buf);
		if(buf[i-1] != ':' && buf[i-1] != '/')
		{
			buf[i++] = '/';
			buf[i] = 0;
		}
	}

	if(i < len-1) strncpy(buf+i, name, len-i-1);
	return;
}

/* Add an argument to the _WBArgv array.  We must allocate */
/* memory for the argument and copy the incoming data.     */
static int AddArg(char *arg)
{
	if(_WBArgc >= _WBArgMax-1)
	{
		/* Out of space in _WBArgv.  Reallocate it bigger. */
		_WBArgMax += 10;
		_WBArgv = realloc(_WBArgv, _WBArgMax*sizeof(char *));
		if(_WBArgv == NULL) return -1;
	}

	/* Allocate memory for the new argument */
	_WBArgv[_WBArgc] = malloc(strlen(arg)+1);
	if(_WBArgv[_WBArgc] == NULL) return -1;

	/* Copy the argument data over */
	strcpy(_WBArgv[_WBArgc], arg);

	/* Increment our argument count */
	_WBArgc++;

	return 0;
}


static void ReorderArgv(int argc, char** argv)
{
	int i;
	for(i=1; i<argc; i++)
	{
		// check if it is the 'magic tooltype' PYTHONSCRIPT
		if(strnicmp("PYTHONSCRIPT=",argv[i],13)==0)
		{
			short j;
			char *thisarg;
			// skip the magic word
			strcpy(argv[i], argv[i]+13);
			// place this tooltype as the first after the programname
			thisarg = argv[i];
			for(j=i; j>1; )
			{
				argv[j]=argv[--j];
			}
			argv[1]=thisarg;
		}

		// check if it is the 'magic tooltype' PYSCRIPTARG
		if(strnicmp("PYSCRIPTARG=",argv[i],12)==0)
		{
			short j;
			char *thisarg;
			// skip the magic word
			strcpy(argv[i], argv[i]+12);
			// place this tooltype at the end of the array.
			thisarg = argv[i];
			for(j=i; j<argc; j++)
			{
				argv[j]=argv[j+1];
			}
			argv[argc-1]=thisarg;
			i--;
		}
	}
}



/* This is an autoinitializer routine that will be run if this  */
/* module is linked in to user code.  Referring to the external */
/* data items above is sufficient to pull this in from a library*/

// int __stdargs _STI_20000_WBArgParse(void)
CONSTRUCTOR_P(WBArgParse,20000)
{
	struct WBArg *wba;
	int nargs;
	char buf[512];
	struct DiskObject *dob;
	struct Library *IconBase;
	BPTR dir;

	if(_WBenchMsg == NULL) return 0;  // Not invoked from WorkBench

	/* Put the program name in */
	wba=_WBenchMsg->sm_ArgList;
	FullName(wba->wa_Lock, wba->wa_Name, buf, sizeof(buf));
	if(AddArg(buf)) return -1;

	/* Find the tool types and add them */

	if(IconBase = OpenLibrary("icon.library", 0L))
	{
		dir = CurrentDir(wba->wa_Lock);
		if(dob = GetDiskObject(wba->wa_Name))
		{
			if(dob->do_ToolTypes)
			{
				for(nargs=0; dob->do_ToolTypes[nargs]; nargs++)
					if(AddArg(dob->do_ToolTypes[nargs])) return -1;
			}
			FreeDiskObject(dob);
		}
		CurrentDir(dir);
		CloseLibrary(IconBase);
	}


	/* Now add the file arguments */
	/* For Python1.4, also insert the tooltypes of the first file (if any) */
	for(nargs=1, wba++; 
		nargs<_WBenchMsg->sm_NumArgs;
		nargs++, wba++)
	{
		// add only the tooltypes from the FIRST file (=the script)
		if(nargs==1)
		{
			if(IconBase = OpenLibrary("icon.library", 0L))
			{
				dir=CurrentDir(wba->wa_Lock);
				if(dob = GetDiskObject(wba->wa_Name))
				{
					if(dob->do_ToolTypes)
					{
						int i;
						for(i=0; dob->do_ToolTypes[i]; i++)
							if(AddArg(dob->do_ToolTypes[i])) return -1;
					}
					FreeDiskObject(dob);
				}
				CurrentDir(dir);
				CloseLibrary(IconBase);
			}
		}

		FullName(wba->wa_Lock, wba->wa_Name, buf, sizeof(buf));
		{
			BPTR test=Lock(buf,ACCESS_READ);
			if(test)
			{
				UnLock(test);
				if(AddArg(buf)) return -1;
			}
		}
	}

	/* Make sure _WBArgv is terminated with a NULL pointer */
	/* like ANSI C argv lists are.                         */
	_WBArgv[_WBArgc] = NULL;


	// reorder the options
	ReorderArgv(_WBArgc,_WBArgv);

	return 0;
}
