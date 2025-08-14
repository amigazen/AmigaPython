/**************************************************************\
**                                                            **
**  UNIX 'emulation' functions for AmigaDOS                   **
**                                                            **
**  Made by Irmen de Jong (irmen@bigfoot.com)                 **
**                                                            **
**  21-jan-98: Created. Moved some functions from             **
**             Modules/Amigamodule.c to here.                 **
**  25-dec-98: Added I-Net 225 support                        **
**             Added ftruncate (but not used due to bugs)     **
**                                                            **
**  NOTE: Don't forget __io2errno conversion!!!!!!!!!!!!!!!!  **
**                                                            **
\**************************************************************/

#ifndef INET225
#include <dos.h>
#endif
#include <sys/stat.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <dos/dostags.h>
#include <exec/execbase.h>
#ifdef AMITCP
#include <proto/usergroup.h>
#include <proto/socket.h>
#endif
#ifdef INET225
#include <proto/socket.h>
static int _OSERR;
#endif
#include "Python.h"
#include "protos.h"

/************ Utility functions *************/

/*** checkLink: check for link loops and other errors ***/
static BOOL checkLink(char *from, BPTR to, BOOL root)
{
	struct FileInfoBlock __aligned fib;

	if(Examine(to,&fib))
	{
		if(fib.fib_EntryType>0)
		{
			// directory! Check some things (loops etc)
			char* pp;
			char p;
			BPTR fromLock,temp;

			// only superuser may link directories
			if(!root)
			{
				errno=EPERM; return FALSE;
			}

			pp = PathPart(from);
			p = *pp;
			*pp = 0;
			fromLock=Lock(from,SHARED_LOCK);
			*pp=p;

			if(fromLock)
			{
				do {
					if(SameLock(fromLock,to)==LOCK_SAME)
					{
						UnLock(fromLock);
#ifdef ELOOP
						errno = ELOOP;
#else
						errno = EMLINK;
#endif
						return FALSE;       // link loop
					}

					temp = fromLock;
					fromLock = ParentDir(fromLock);
					UnLock(temp);
				} while (fromLock);

				return TRUE;       // dir, OK.
			}
			else errno=__io2errno(_OSERR=IoErr());
		}
		else return TRUE;      // file, OK.
	}
	else errno=__io2errno(_OSERR=IoErr());

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

#if defined(AMITCP) || defined(INET225)
	/* are we superuser? */
#ifdef AMITCP
	if (!checkusergrouplib())
#else /* INET */
	if (!checksocketlib())
#endif
	{
		PyErr_Clear();
		root=TRUE;  /* can't tell... so be root */
	}
	else if(getuid()==0) root = TRUE;
	else root=FALSE;
#endif /* AMITCP or INET */

	if(toLock=Lock(to,SHARED_LOCK))
	{
		if(checkLink(from,toLock,root))
		{
			if(MakeLink(from,(LONG)toLock,FALSE))
			{
				UnLock(toLock);
				return 0;
			}
			else errno=__io2errno(_OSERR=IoErr());
		}
		UnLock(toLock);
	}
	else errno=__io2errno(_OSERR=IoErr());
	
	return -1;
}

/************** symlink(2): create symbolic (soft) link ********/
int symlink(const char *to, const char *from)
{
	/* symbolic link 'from' is created to 'to' */
	/* 0=ok, else -1 + errno */

	BPTR toLock;

	if(toLock=Lock(to,SHARED_LOCK))
	{
		if(checkLink(from,toLock,TRUE))
		{
			UnLock(toLock);
			if(MakeLink(from,(LONG)to,TRUE)) return 0;
			else errno=__io2errno(_OSERR=IoErr());
		}
		else UnLock(toLock);
	}
	else errno=__io2errno(_OSERR=IoErr());
	
	return -1;
}



/************** readlink(2): read value of a symbolic link ***********/

int readlink(const char *path, char *buf, int bufsiz)
{
	struct MsgPort *port;
	struct stat st;

	if(!(port=DeviceProc(path)))
	{
		errno=EIO; return -1;
	}

	buf[bufsiz-1]=0;
	errno=0;

	if(lstat(path,&st)>=0)
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
			c = *p; *p='\0';
			dirlock=Lock(path,ACCESS_READ); *p=c;
			if(dirlock)
			{
				olddir=CurrentDir(dirlock);

				if(!ReadLink(port,dirlock,link,buf,bufsiz))
					errno=__io2errno(_OSERR=IoErr());

				dirlock=CurrentDir(olddir);
				UnLock(dirlock);
			}   
			else errno=__io2errno(_OSERR=IoErr());
#ifdef S_ISLNK
		}
		else errno=EINVAL;
#endif
	}

	if(errno!=0) return -1;

	if(buf[bufsiz-1]==0) return strlen(buf);
	else return bufsiz;
}



/** custom mkdir() implementation **/
/** This version actually sets protection bits **/
#ifndef INET225
int my_mkdir(const char* path, int p)
{
#ifdef AMITCP
	if(checkusergrouplib()) p &= ~getumask();
	else PyErr_Clear();
#endif

	if(0==mkdir(path))
	{
		return chmod(path,p);
	}
	return -1;
}
#endif /* !INET225 */

int uname(struct utsname *u)
{
	int res;

	strcpy(u->sysname,"AmigaDOS");
	strcpy(u->machine,"m68k");
#if defined(AMITCP) || defined(INET225)
	if (!checksocketlib())
#endif
	{
		char *v;
		PyErr_Clear();
		res=0; v=getenv("HOSTNAME");
		if(v) strcpy(u->nodename, v);
		else strcpy(u->nodename, "localhost");
	}
#if defined(AMITCP) || defined(INET225)
	else res = gethostname(u->nodename, _UNAME_BUFLEN-1);
#endif
	if(res>=0)
	{
		LONG ver_major = SysBase->LibNode.lib_Version;
		LONG ver_minor = SysBase->SoftVer;
		sprintf (u->release, "%d.%d", ver_major,ver_minor);
		if(ver_major<36)
			strcpy(u->version,"1");
		else if(ver_major<39)
			strcpy(u->version,"2");
		else 
			strcpy(u->version,"3");
	}
	return 0;
}


#ifndef INET225
FILE *popen(const char *command, const char *type)
{
	char file[50];

	FILE *fh;

	static int num = 1;

	if((type[0]!='r') && (type[0]!='w'))
	{
		errno=EINVAL;
		return 0;
	}
		
	sprintf(file,"PIPE:Py_%ld_%ld",num++,FindTask(0));

	if(fh=fopen(file,type))
	{
		BPTR fh2;
		LONG mode=MODE_NEWFILE; /* 'r'-peer must write */

		if(type[0]=='w') mode=MODE_OLDFILE; /* peer must read */

		if(fh2=Open(file,mode))
		{
			BPTR fh3;
			if(type[0]=='r')
			{
				/* execute command with output to fh */
				fh3=Open("*",MODE_OLDFILE); /* should use CONSOLE: */
				if(fh3 && (0==SystemTags(command,SYS_Asynch,TRUE,SYS_Output,fh2,
									SYS_Input,fh3,TAG_DONE)))
				{
					return fh;
				}
			}
			else /** if(type[0]=='w') **/
			{
				/* execute command with input from fh */
				fh3=Open("*",MODE_NEWFILE); /* should use CONSOLE: */
				if(fh3 && (0==SystemTags(command,SYS_Asynch,TRUE,SYS_Input,fh2,
									SYS_Output,fh3,TAG_DONE)))
				{
					return fh;
				}
			}
			fclose(fh); Close(fh2); if(fh3) Close(fh3);
			errno=EAGAIN;
			return 0;
		}
		fclose(fh);
	}
	errno=ENOENT;
	return 0;
}

int pclose(FILE *stream)
{
	if(stream)
	{
		fclose(stream);
		return 0;
	}
	errno=EINVAL;
	return -1;  
}

#endif /* INET225 */


/*************** ftruncate is not yet used because of bugs in the OS FileSystem :-( ***/

#if 0

int ftruncate(int fd, long newlength)
{
  struct UFB *ufb;

  /*
   * find the ufb *
   */
  if ((ufb = __chkufb(fd)) != NULL && !(ufb->ufbflg & UFB_SOCK))
  {
	if(-1==SetFileSize(ufb->ufbfh,newlength,OFFSET_BEGINNING))
    {
		set_errno(IoErr());
		return -1;
	}
	return 0;
  }
  
  errno = EINVAL;
  return -1;
}

#endif


/********************* getpid ***********************/

pid_t getpid(void)
{
	return (pid_t)SysBase->ThisTask;
}
