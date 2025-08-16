/*
 *      wbargs.c - Workbench argument handling for Amiga
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 *
 *      Collects arguments from Workbench startup message
 *      and creates a C-style argv/argc for the program.
 */

#include <workbench/startup.h>
#include <exec/execbase.h>
#include <string.h>
#include <dos/dos.h>
#include <stdlib.h>
#include <constructor.h>

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/utility.h>

#define GRAPHICS_GFXNODES_H  /* Prevent inclusion of problematic gfxnodes.h */
#define GRAPHICS_VIEW_H      /* Prevent inclusion of view.h which includes gfxnodes.h */
#define INTUITION_SCREENS_H  /* Prevent inclusion of screens.h which has graphics dependencies */
#include <proto/icon.h>

/* Workbench startup message - should be provided by startup code */
extern struct WBStartup *WBenchMsg;

/* These two symbols, _WBArgc and _WBArgv, are initialized if the program was invoked from WorkBench.
   They look like normal C (argc, argv) parameters. The parameters are gathered as follows:
   - Name of the program
   - Any tooltypes specified in the ToolTypes array
   - Any icons supplied as arguments (with SHIFT-CLICK)
*/

int _WBArgc;       /* Count of the number of WorkBench arguments */
char **_WBArgv;    /* The actual arguments */

static int _WBArgMax;  /* Internal: tells us how much space is left in _WBArgv */

/*
 * Get the full pathname of a file
 * Returns 1 on success, 0 on error or if path would be truncated
 */
static int FullPathName(BPTR parent, char *name, char *buf, int len)
{
    int i = 0;
    int name_len;
    
    /* Ensure valid inputs */
    if (!parent || !name || !buf || len <= 0)
        return 0;
        
    /* Get parent directory path */
    if(NameFromLock(parent, buf, len-1))
    {
        i = strlen(buf);
        /* Add separator if needed */
        if(i > 0 && buf[i-1] != ':' && buf[i-1] != '/')
        {
            /* Ensure we have space for separator + name + null terminator */
            if (i >= len-2)
                return 0;
                
            buf[i++] = '/';
            buf[i] = 0;
        }
    }
    else
    {
        /* Failed to get parent path */
        return 0;
    }

    /* Ensure we have space for name + null terminator */
    name_len = strlen(name);
    if (i + name_len >= len)
        return 0;
        
    strncpy(buf+i, name, len-i-1);
    buf[len-1] = '\0'; /* Ensure null termination */
    
    return 1;
}

/*
 * Clean up resources: Free all arguments in _WBArgv
 */
static void FreeWBArgs(void)
{
    int i;
    
    if (_WBArgv) {
        for (i = 0; i < _WBArgc; i++) {
            if (_WBArgv[i]) {
                free(_WBArgv[i]);
                _WBArgv[i] = NULL;
            }
        }
        free(_WBArgv);
        _WBArgv = NULL;
    }
    _WBArgc = 0;
    _WBArgMax = 0;
}

/*
 * Add an argument to the _WBArgv array
 * We must allocate memory for the argument and copy the incoming data
 * Returns 0 on success, -1 on failure
 */
static int AddWBArg(char *arg)
{
    int new_max;
    char **new_argv;
    int arg_len;
    
    /* Validate arg */
    if (!arg)
        return -1;
        
    if(_WBArgc >= _WBArgMax-1)
    {
        /* Out of space in _WBArgv. Reallocate it bigger. */
        new_max = _WBArgMax + 10;
        new_argv = realloc(_WBArgv, new_max*sizeof(char *));
        if(new_argv == NULL) return -1;
        
        _WBArgv = new_argv;
        _WBArgMax = new_max;
    }

    /* Allocate memory for the new argument */
    arg_len = strlen(arg);
    _WBArgv[_WBArgc] = malloc(arg_len + 1);
    if(_WBArgv[_WBArgc] == NULL) return -1;

    /* Copy the argument data over */
    strcpy(_WBArgv[_WBArgc], arg);

    /* Increment our argument count */
    _WBArgc++;

    return 0;
}

/*
 * Reorder the arguments to handle special tooltypes
 */
static void ReorderWBArgv(int argc, char** argv)
{
    int i;
    
    if (!argv || argc <= 1)
        return;
        
    for(i=1; i<argc; i++)
    {
        if (!argv[i])
            continue;
            
        /* check if it is the 'magic tooltype' PYTHONSCRIPT */
        if(Strnicmp("PYTHONSCRIPT=", argv[i], 13)==0)
        {
            short j;
            char *thisarg;
            /* skip the magic word */
            memmove(argv[i], argv[i]+13, strlen(argv[i]+13)+1);
            /* place this tooltype as the first after the programname */
            thisarg = argv[i];
            for(j=i; j>1; )
            {
                argv[j]=argv[--j];
            }
            argv[1]=thisarg;
        }

        /* check if it is the 'magic tooltype' PYSCRIPTARG */
        if(Strnicmp("PYSCRIPTARG=", argv[i], 12)==0)
        {
            short j;
            char *thisarg;
            /* skip the magic word */
            memmove(argv[i], argv[i]+12, strlen(argv[i]+12)+1);
            /* place this tooltype at the end of the array. */
            thisarg = argv[i];
            for(j=i; j<argc-1; j++)
            {
                argv[j]=argv[j+1];
            }
            argv[argc-1]=thisarg;
            i--;
        }
    }
}

/*
 * Constructor that parses Workbench arguments into C-style argc/argv
 */
CONSTRUCTOR_P(WBArgParse,20000)
{
    struct WBArg *wba;
    int nargs, status = 0;
    char buf[512];
    struct DiskObject *dob = NULL;
    struct Library *IconBase = NULL;
    BPTR dir = 0;

    /* Initialize globals */
    _WBArgc = 0;
    _WBArgv = NULL;
    _WBArgMax = 0;

    if(WBenchMsg == NULL) return 0;  /* Not invoked from WorkBench */

    /* Allocate initial _WBArgv array */
    _WBArgMax = 10;  /* Start with space for 10 arguments */
    _WBArgv = calloc(_WBArgMax, sizeof(char *));
    if (_WBArgv == NULL) return -1;

    /* Put the program name in */
    wba=WBenchMsg->sm_ArgList;
    if (FullPathName(wba->wa_Lock, wba->wa_Name, buf, sizeof(buf))) {
        if(AddWBArg(buf)) {
            FreeWBArgs();
            return -1;
        }
    } else {
        /* If we can't get the full path, use just the name */
        if(AddWBArg(wba->wa_Name)) {
            FreeWBArgs();
            return -1;
        }
    }

    /* Find the tool types and add them */
    if((IconBase = OpenLibrary("icon.library", 0L)))
    {
        dir = CurrentDir(wba->wa_Lock);
        if((dob = GetDiskObject(wba->wa_Name)))
        {
            if(dob->do_ToolTypes)
            {
                for(nargs=0; dob->do_ToolTypes[nargs]; nargs++) {
                    if(AddWBArg(dob->do_ToolTypes[nargs])) {
                        status = -1;
                        break;
                    }
                }
            }
            FreeDiskObject(dob);
            dob = NULL;
        }
        CurrentDir(dir);
        dir = 0;
        CloseLibrary(IconBase);
        IconBase = NULL;
    }

    /* Return if we had an error */
    if (status != 0) {
        FreeWBArgs();
        return status;
    }

    /* Now add the file arguments */
    /* Also insert the tooltypes of the first file (if any) */
    for(nargs=1, wba++; 
        nargs<WBenchMsg->sm_NumArgs;
        nargs++, wba++)
    {
        /* add only the tooltypes from the FIRST file (=the script) */
        if(nargs==1)
        {
            if((IconBase = OpenLibrary("icon.library", 0L)))
            {
                dir=CurrentDir(wba->wa_Lock);
                if((dob = GetDiskObject(wba->wa_Name)))
                {
                    if(dob->do_ToolTypes)
                    {
                        int i;
                        for(i=0; dob->do_ToolTypes[i]; i++) {
                            if(AddWBArg(dob->do_ToolTypes[i])) {
                                status = -1;
                                break;
                            }
                        }
                    }
                    FreeDiskObject(dob);
                    dob = NULL;
                }
                CurrentDir(dir);
                dir = 0;
                CloseLibrary(IconBase);
                IconBase = NULL;
            }
            
            /* Return if we had an error */
            if (status != 0) {
                FreeWBArgs();
                return status;
            }
        }

        if (FullPathName(wba->wa_Lock, wba->wa_Name, buf, sizeof(buf)))
        {
            BPTR test=Lock(buf, ACCESS_READ);
            if(test)
            {
                UnLock(test);
                if(AddWBArg(buf)) {
                    FreeWBArgs();
                    return -1;
                }
            }
        }
    }

    /* Make sure _WBArgv is terminated with a NULL pointer like ANSI C argv lists are */
    _WBArgv[_WBArgc] = NULL;

    /* reorder the options */
    ReorderWBArgv(_WBArgc, _WBArgv);

    return 0;
} 