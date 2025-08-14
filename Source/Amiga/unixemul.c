/**************************************************************\
**                                                            **
**  UNIX 'emulation' functions for AmigaDOS                   **
**                                                            **
**  Based on Irmen de Jong's original Amiga port              **
**  Updated for Python 2.7.18                                 **
**                                                            **
**  NOTE: Don't forget __io2errno conversion!                 **
**                                                            **
\**************************************************************/

#include <dos/dos.h>
#include <sys/stat.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <dos/dostags.h>
#include <exec/execbase.h>
#ifdef AMITCP
#include <proto/usergroup.h>
#include <proto/socket.h>
#endif
#include "Python.h"
#include "protos.h"

/************ Utility functions *************/

/*** checkLink: check for link loops and other errors ***/
static BOOL checkLink(char *from, BPTR to, BOOL root)
{
    struct FileInfoBlock __aligned fib;

    if(Examine(to, &fib))
    {
        if(fib.fib_EntryType > 0)
        {
            // directory! Check some things (loops etc)
            char* pp;
            char p;
            BPTR fromLock, temp;

            // only superuser may link directories
            if(!root)
            {
                errno = EPERM; 
                return FALSE;
            }

            pp = PathPart(from);
            p = *pp;
            *pp = 0;
            fromLock = Lock(from, SHARED_LOCK);
            *pp = p;

            if(fromLock)
            {
                do {
                    if(SameLock(fromLock, to) == LOCK_SAME)
                    {
                        UnLock(fromLock);
#ifdef ELOOP
                        errno = ELOOP;
#else
                        errno = EMLINK;
#endif
                        return FALSE;   // link loop
                    }

                    temp = fromLock;
                    fromLock = ParentDir(fromLock);
                    UnLock(temp);
                } while (fromLock);

                return TRUE;   // dir, OK.
            }
            else errno = __io2errno(_OSERR = IoErr());
        }
        else return TRUE;  // file, OK.
    }
    else errno = __io2errno(_OSERR = IoErr());

    return FALSE;
}


/************ link(2) : make a hard link ************/

/* LINK: make hardlink from 'from' to 'to' (to must exist, from is new) */
/* 'from' may not be a directory if you are not the super-user. */
/* 0=ok, -1=err */
int link(const char *to, const char *from)
{
    BOOL root = TRUE;
    BPTR toLock;

#ifdef AMITCP
    /* are we superuser? */
    if (!checkusergrouplib())
    {
        PyErr_Clear();
        root = TRUE;  /* can't tell... so be root */
    }
    else if(getuid() == 0) root = TRUE;
    else root = FALSE;
#endif

    if((toLock = Lock(to, SHARED_LOCK)))
    {
        if(checkLink((char *)from, toLock, root))
        {
            if(MakeLink((char *)from, (LONG)toLock, FALSE))
            {
                UnLock(toLock);
                return 0;
            }
            else errno = __io2errno(_OSERR = IoErr());
        }
        UnLock(toLock);
    }
    else errno = __io2errno(_OSERR = IoErr());
    
    return -1;
}

/************** symlink(2): create symbolic (soft) link ********/
int symlink(const char *to, const char *from)
{
    /* symbolic link 'from' is created to 'to' */
    /* 0=ok, else -1 + errno */

    BPTR toLock;

    if((toLock = Lock(to, SHARED_LOCK)))
    {
        if(checkLink((char *)from, toLock, TRUE))
        {
            UnLock(toLock);
            if(MakeLink((char *)from, (LONG)to, TRUE)) return 0;
            else errno = __io2errno(_OSERR = IoErr());
        }
        else UnLock(toLock);
    }
    else errno = __io2errno(_OSERR = IoErr());
    
    return -1;
}

/************** readlink(2): read value of a symbolic link ***********/
int readlink(const char *path, char *buf, int bufsiz)
{
    struct MsgPort *port;
    struct stat st;

    if(!(port = DeviceProc(path)))
    {
        errno = EIO; 
        return -1;
    }

    buf[bufsiz-1] = 0;
    errno = 0;

    if(lstat(path, &st) >= 0)
    {
#ifdef S_ISLNK
        if(S_ISLNK(st.st_mode))
        {
#endif
            char c;
            BPTR dirlock;
            BPTR olddir;
            char *p;
            char *link;

            p = PathPart(path);
            link = FilePart(path);
            c = *p; *p = '\0';
            dirlock = Lock(path, ACCESS_READ); *p = c;
            if(dirlock)
            {
                olddir = CurrentDir(dirlock);

                if(!ReadLink(port, dirlock, link, buf, bufsiz))
                    errno = __io2errno(_OSERR = IoErr());

                dirlock = CurrentDir(olddir);
                UnLock(dirlock);
                return strlen(buf);
            }
            else errno = __io2errno(_OSERR = IoErr());
#ifdef S_ISLNK
        }
        else errno = EINVAL;
#endif
    }

    return -1;
}

/************** mkdir(2): create a directory ***********/
int my_mkdir(const char* path, int p)
{
    BPTR lock;
    
    /* ignore the p (protection bits) parameter */
    
    if((lock = CreateDir(path)))
    {
        UnLock(lock);
        return 0;
    }
    
    errno = __io2errno(_OSERR = IoErr());
    return -1;
}

/************** uname(2): get system information ***********/
int uname(struct utsname *u)
{
    strcpy(u->sysname, "AmigaOS");
    
    /* Get the processor type */
    {
        ULONG processor = 0;
        struct Library *SysBase = *(struct Library **)4;
        
        if (SysBase)
        {
            processor = (SysBase->lib_Version >= 37) ? 
                GetProcInfo(NULL) : PROCESSOR_68000;
        }
        
        switch(processor)
        {
            case PROCESSOR_68000:
                strcpy(u->machine, "m68k");
                break;
            case PROCESSOR_68010:
                strcpy(u->machine, "m68010");
                break;
            case PROCESSOR_68020:
                strcpy(u->machine, "m68020");
                break;
            case PROCESSOR_68030:
                strcpy(u->machine, "m68030");
                break;
            case PROCESSOR_68040:
                strcpy(u->machine, "m68040");
                break;
            case PROCESSOR_68060:
                strcpy(u->machine, "m68060");
                break;
            default:
                strcpy(u->machine, "unknown");
        }
    }
    
    /* Get the Amiga OS version */
    {
        struct Library *SysBase = *(struct Library **)4;
        
        if (SysBase)
        {
            char version[16];
            sprintf(version, "%d.%d", SysBase->lib_Version, SysBase->lib_Revision);
            strcpy(u->version, version);
            
            /* Determine the OS release */
            if (SysBase->lib_Version >= 39)
                strcpy(u->release, "3.5+");
            else if (SysBase->lib_Version >= 37)
                strcpy(u->release, "3.0");
            else if (SysBase->lib_Version >= 36)
                strcpy(u->release, "2.1");
            else if (SysBase->lib_Version >= 34)
                strcpy(u->release, "2.0");
            else
                strcpy(u->release, "1.3-");
        }
        else
        {
            strcpy(u->version, "Unknown");
            strcpy(u->release, "Unknown");
        }
    }
    
    gethostname(u->nodename, _SYS_NMLN);
    return 0;
}

/************** pclose(3): close a process stream ***********/
int pclose(FILE *stream)
{
    int ret = -1;
    
    if (stream)
    {
        ret = fclose(stream);
    }
    
    return ret;
}

/************** getpid(2): get process ID ***********/
pid_t getpid(void)
{
    struct Task *task = FindTask(NULL);
    return (pid_t)task;   /* Use task pointer as "pid" */
} 