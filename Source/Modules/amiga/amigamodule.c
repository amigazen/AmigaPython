/*
 * AMIGA module implementation for Python 2.7.18
 *
 * Original by Irmen de Jong (irmen@bigfoot.com)
 * Enhanced for modern AmigaOS with PosixLib integration
 *
 * Key Changes from Original:
 * - Integrated PosixLib for enhanced POSIX compatibility
 * - Added comprehensive POSIX functions (access, isatty,
 *   closerange, nice, kill, waitpid, pathconf, etc.)
 * - Fixed SAS/C syntax issues for VBCC compatibility
 * - Removed SAS/C-specific features (__aligned, __asm)
 * - Added proper header guards to prevent conflicts
 * - Enhanced error handling and AmigaOS integration
 * - Added missing POSIX constants and defines
 * - Improved environment variable handling
 *
 * Build Requirements:
 * - VBCC compiler with aos68kr_posix configuration
 * - PosixLib for POSIX function implementations
 *
 * Features:
 * - Full AmigaOS integration with dos.library, exec.library
 * - POSIX compatibility through PosixLib
 * - Environment variable management (global, local, both)
 * - File and directory operations
 * - Process management and system calls
 * - Network support via PosixLib
 * - CRC32 calculation for data integrity
 *
 */


#include "Python.h"
#include "osdefs.h"

#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <dirent.h>
#define NAMLEN(dirent) strlen((dirent)->d_name)

#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>

/* #include <termios.h>        For tcgetpgrp/tcsetpgrp - not available in PosixLib */

/* Define times() structures and functions since they're not in PosixLib */
/*
#ifndef _SYS_TIMES_H_
#define _SYS_TIMES_H_

#ifndef clock_t
typedef long clock_t;
#endif
/*
struct tms {
    clock_t tms_utime;  
    clock_t tms_stime;  
    clock_t tms_cutime;
    clock_t tms_cstime;
};
*/

/* AmigaOS implementation of times() */

/*
static clock_t amiga_times_func(struct tms *buffer)
{
    struct Task *task = FindTask(NULL);
    if (!task || !buffer) {
        return (clock_t)-1;
    }
    
    buffer->tms_utime = (clock_t)task->tc_ETime;   
    buffer->tms_stime = 0;                       
    buffer->tms_cutime = 0;                       
    buffer->tms_cstime = 0;         
    
    return buffer->tms_utime;
}

#define times amiga_times_func

#endif _SYS_TIMES_H_
*/

/* Prevent timer.h from being included to avoid struct timeval conflicts */
#define DEVICES_TIMER_H

/* AmigaOS-specific headers - only include what we actually need */
#include <exec/types.h>
#include <dos/dosextens.h>
#include <dos/var.h>
#include <dos/dostags.h>
#include <exec/execbase.h>
#include <proto/exec.h>
#include <proto/dos.h>

#ifdef HAVE_UTIME_H
#include <utime.h>
#endif

/* 
#ifdef INET225
#include <proto/socket.h>
#define getegid() getgid()
#define geteuid() getuid()
static __inline int dup(int oldsd) { return s_dup(oldsd); }
static __inline int dup2(int oldsd, int newsd) { return s_dup2(oldsd, newsd); }
#endif
*/

#include "protos.h"

/* Return a dictionary corresponding to the AmigaDOS environment table. */
/* That is, scan ENV: for global environment variables.                 */
/* The local shell environment variables are put into another table.    */

static int
convertenviron(PyObject **glob, PyObject **loc,
			   PyObject **both, PyObject **aliases)
{
	BPTR dlok;
	struct FileInfoBlock fib;
	PyObject *v;
	char *dynbuf;
	struct LocalVar *lvar;
	struct List *localvars;

	*glob=PyDict_New();
	*loc=PyDict_New();
	*both=PyDict_New();
	*aliases=PyDict_New();

	if(!*glob || !*loc || !*both || !*aliases)
	{
		if(*glob) Py_DECREF(*glob);
		if(*loc) Py_DECREF(*loc);
		if(*both) Py_DECREF(*both);
		if(*aliases) Py_DECREF(*aliases);
		return 0;
	}

	/* Read global vars from ENV: */
	/* Put them in 'glob' and in 'both'. */

	if(dlok=Lock("ENV:",ACCESS_READ))
	{
		if(Examine(dlok,&fib))
		{
			while(ExNext(dlok,&fib))
			{
				if(fib.fib_DirEntryType<0)
				{
					if(dynbuf=malloc(fib.fib_Size+1))
					{
						int len=GetVar(fib.fib_FileName,dynbuf,fib.fib_Size+1,GVF_GLOBAL_ONLY);
						if(len>=0 && (v=PyString_FromString(dynbuf)))
						{
							PyDict_SetItemString(*glob,fib.fib_FileName,v);
							PyDict_SetItemString(*both,fib.fib_FileName,v);
							Py_DECREF(v);
						}
						free(dynbuf);
					}
				}
			}
		}
	}

	if(dlok) UnLock(dlok);

	/* Scan the local shell environment, including "RC" and "Result2"!   */
	/* Put shell vars in 'loc' and 'both', and aliases in 'aliases'. */
	/* Because of the fact that the inserting of local vars into 'both' */
	/* happens AFTER the insertion of global vars, the formor overwrite */
	/* the latter, and thus have higher priority (as it should be). */

	localvars = (struct List*) &((struct Process*)FindTask(0))->pr_LocalVars;

	if(!IsListEmpty(localvars))
	{
		lvar = (struct LocalVar*) localvars->lh_Head;
		do {
			if(dynbuf=malloc(lvar->lv_Len+1))
			{
				strncpy(dynbuf,lvar->lv_Value,lvar->lv_Len);
				dynbuf[lvar->lv_Len]=0;

				if(v=PyString_FromString(dynbuf))
				{
					if(lvar->lv_Node.ln_Type==LV_VAR)
					{
						PyDict_SetItemString(*loc,lvar->lv_Node.ln_Name,v);
						PyDict_SetItemString(*both,lvar->lv_Node.ln_Name,v);
					}
					else if(lvar->lv_Node.ln_Type==LV_ALIAS)
						PyDict_SetItemString(*aliases,lvar->lv_Node.ln_Name,v);

					Py_DECREF(v);
				}
				free(dynbuf);
			}
		} while((lvar=(struct LocalVar*)lvar->lv_Node.ln_Succ)->lv_Node.ln_Succ);
	}


	return 1;
}


/* Set a Amiga-specific error from errno, and return NULL */

static PyObject * amiga_error(void)
{
	return PyErr_SetFromErrno(PyExc_OSError);
}
static PyObject * amiga_error_with_filename(char *name)
{
	return PyErr_SetFromErrnoWithFilename(PyExc_OSError, name);
}

/* POSIX-style error functions for file I/O */
static PyObject *
posix_error(void)
{
    return PyErr_SetFromErrno(PyExc_OSError);
}

static PyObject *
posix_error_with_allocated_filename(char* name)
{
    PyObject *rc = PyErr_SetFromErrnoWithFilename(PyExc_OSError, name);
    PyMem_Free(name);
    return rc;
}

/* Helper function to convert struct stat to Python stat tuple */
static PyObject*
_pystat_fromstructstat(struct stat *st)
{
    PyObject *v = Py_BuildValue("(llllllllll)",
            (long)st->st_mode,
            (long)st->st_ino,
            (long)st->st_dev,
            (long)st->st_nlink,
            (long)st->st_uid,
            (long)st->st_gid,
            (long)st->st_size,
            (long)st->st_atime,
            (long)st->st_mtime,
            (long)st->st_ctime);
    return v;
}

/* File descriptor verification (simplified for Amiga, clean version) */
/* static int _PyVerify_fd(int fd)
{
    if (fd < 0 || fd > 1000) {
        errno = EBADF;
        return 0;
    }
    return 1;
} */


/* AMIGA generic methods */

static PyObject *
amiga_1str(PyObject *args, int (*func)(const char *))
{
	char *path1;
	int res;
	if (!PyArg_Parse(args, "s", &path1))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = (*func)(path1);
	Py_END_ALLOW_THREADS
	if (res < 0)
		return amiga_error_with_filename(path1);
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
amiga_2str(PyObject *args, int (*func)(const char *, const char *))
{
	char *path1, *path2;
	int res;
	if (!PyArg_Parse(args, "(ss)", &path1, &path2))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = (*func)(path1, path2);
	Py_END_ALLOW_THREADS
	if (res < 0)
		return amiga_error();
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
amiga_strint(PyObject *args, int (*func)(const char *, int))
{
	char *path;
	int i;
	int res;
	if (!PyArg_Parse(args, "(si)", &path, &i))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = (*func)(path, i);
	Py_END_ALLOW_THREADS
	if (res < 0)
		return amiga_error_with_filename(path);
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
amiga_strintint(PyObject *args, int (*func)(const char *, int, int))
{
	char *path;
	int i,i2;
	int res;
	if (!PyArg_Parse(args, "(sii)", &path, &i, &i2))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = (*func)(path, i, i2);
	Py_END_ALLOW_THREADS
	if (res < 0)
		return amiga_error_with_filename(path);
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
amiga_do_stat(PyObject *self, PyObject *args, int (*statfunc)(const char *, struct stat *))
{
	struct stat st;
	char *path;
	int res;
	
	/* Check if args is None or empty */
	if (args == NULL || args == Py_None) {
		PyErr_SetString(PyExc_TypeError, "stat() argument must be string, not None");
		return NULL;
	}
	
	if (!PyArg_Parse(args, "s", &path))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = (*statfunc)(path, &st);
	Py_END_ALLOW_THREADS
	if (res != 0)
		return amiga_error_with_filename(path);
	return Py_BuildValue("(llllllllll)",
			(long)st.st_mode,
			(long)st.st_ino,
			(long)st.st_dev,
			(long)st.st_nlink,
			(long)st.st_uid,
			(long)st.st_gid,
			(long)st.st_size,
			(long)st.st_atime,
			(long)st.st_mtime,
			(long)st.st_ctime);
}


/* AMIGA methods */

static PyObject *
amiga_chdir(PyObject *self, PyObject *args)
{
	return amiga_1str(args, chdir);
}

static PyObject *
amiga_chmod(PyObject *self, PyObject *args)
{
    /* Explicit cast to match expected function pointer type */
    return amiga_strint(args, (int (*)(const char *, int))chmod);
}

#ifdef HAVE_CHOWN
static PyObject *
amiga_chown(PyObject *self, PyObject *args)
{
	return amiga_strintint(args, chown);
}
#endif /* HAVE_CHOWN */

#ifdef HAVE_GETCWD
static PyObject *
amiga_getcwd(PyObject *self, PyObject *args)
{
	char buf[MAXPATHLEN];
	char *res;
	if (!PyArg_Parse(args,""))
			return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = getcwd(buf, sizeof buf);
	Py_END_ALLOW_THREADS
	if (res == NULL)
			return amiga_error();
	return PyString_FromString(buf);
}
#endif

#ifdef HAVE_LINK
static PyObject *
amiga_link(PyObject *self, PyObject *args)
{
	return amiga_2str(args, link);
}
#endif

static PyObject *
amiga_listdir(PyObject *self, PyObject *args)
{
	BPTR dlok;
	char *name;
	struct FileInfoBlock fib;
	PyObject *d;

	if (!PyArg_Parse(args, "s", &name)) return NULL;

	if ((d = PyList_New(0)) == NULL) return NULL;

	if(dlok=Lock(name,ACCESS_READ))
	{
		if(Examine(dlok,&fib))
		{
			while(ExNext(dlok,&fib))
			{
				PyObject *v = PyString_FromString(fib.fib_FileName);
				if(v==NULL)
				{
					Py_DECREF(d); d=NULL; break;
				}

				if(PyList_Append(d,v)!=0)
				{
					Py_DECREF(v); Py_DECREF(d); d=NULL; break;
				}
				Py_DECREF(v);
			}
		}
		UnLock(dlok);
	}

	if(IoErr()==ERROR_NO_MORE_ENTRIES) return d;

	Py_DECREF(d);
	errno=__io2errno(IoErr());
	return amiga_error();
}

static PyObject *
amiga_mkdir(PyObject *self, PyObject *args)
{
	int res;
	char *path;
	int mode = 0777;
	if (!PyArg_ParseTuple(args, "s|i", &path, &mode)) return NULL;
	Py_BEGIN_ALLOW_THREADS
#ifdef INET225
	res = mkdir(path, mode);
#else
	res = my_mkdir(path, mode);
#endif
	Py_END_ALLOW_THREADS
	if (res < 0) return amiga_error_with_filename(path);
	Py_INCREF(Py_None); return Py_None;
}

static PyObject *
amiga_rename(PyObject *self, PyObject *args)
{
	return amiga_2str(args, rename);
}

static PyObject *
amiga_rmdir(PyObject *self, PyObject *args)
{
	return amiga_1str(args, rmdir);
}

static PyObject *
amiga_stat(PyObject *self, PyObject *args)
{
	return amiga_do_stat(self, args, stat);
}

#ifdef HAVE_SYSTEM
static PyObject *
amiga_system(PyObject *self, PyObject *args)
{
	char *command;
	long sts;
	if (!PyArg_Parse(args, "s", &command))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	sts = system(command);
	Py_END_ALLOW_THREADS
	return PyInt_FromLong(sts);
}
#endif

#if defined(AMITCP) || defined(INET225)
static PyObject *
amiga_umask(PyObject *self, PyObject *args)
{
	int i;
	if (!PyArg_Parse(args,"i",&i))
		return NULL;
	i = umask(i);
	if (i < 0)
		return amiga_error();
	return PyInt_FromLong((long)i);
}
#endif

#ifdef HAVE_UNAME
static PyObject *
amiga_uname(PyObject *self, PyObject *args)
{
        struct utsname u;
        int res;
        if (!PyArg_NoArgs(args))
                return NULL;
        Py_BEGIN_ALLOW_THREADS
        res = uname(&u);
        Py_END_ALLOW_THREADS
        if (res < 0)
                return amiga_error();
        return Py_BuildValue("(sssss)",
                             u.sysname,
                             u.nodename,
                             u.release,
                             u.version,
                             u.machine);
}
#endif

static PyObject *
amiga_unlink(PyObject *self, PyObject *args)
{
	return amiga_1str(args, unlink);
}

#if defined(AMITCP) || defined(INET225)
static PyObject *
amiga_utime(PyObject *self, PyObject *args)
{
	char *path;
	long atime, mtime;
	int res;

#ifdef HAVE_UTIME_H
	struct utimbuf buf;
#define ATIME buf.actime
#define MTIME buf.modtime
#define UTIME_ARG &buf
#else /* HAVE_UTIME_H */
	time_t buf[2];
#define ATIME buf[0]
#define MTIME buf[1]
#define UTIME_ARG buf
#endif /* HAVE_UTIME_H */

	if (!PyArg_Parse(args, "(s(ll))", &path, &atime, &mtime))
		return NULL;
	ATIME = atime;
	MTIME = mtime;
	Py_BEGIN_ALLOW_THREADS
	res = utime(path, UTIME_ARG);
	Py_END_ALLOW_THREADS
	if (res < 0)
		return amiga_error_with_filename(path);
	Py_INCREF(Py_None);
	return Py_None;
#undef UTIME_ARG
#undef ATIME
#undef MTIME
}
#endif


/* Process operations */

/* XXX Removed _exit. You are VERY STUPID if you used this. (2-apr-96) */

/* XXX Removed execv. You must use system/exit combination instead. */
/*     Maybe one day I'll implement a REAL execv ?? */



#ifdef HAVE_GETEGID
static PyObject *
amiga_getegid(PyObject *self, PyObject *args)
{
	if (!PyArg_Parse(args,""))
		return NULL;
	return PyInt_FromLong((long)getegid());
}
#endif

#ifdef HAVE_GETEUID
static PyObject *
amiga_geteuid(PyObject *self, PyObject *args)
{
	if (!PyArg_Parse(args,""))
		return NULL;
	return PyInt_FromLong((long)geteuid());
}
#endif

#ifdef HAVE_GETGID
static PyObject *
amiga_getgid(PyObject *self, PyObject *args)
{
	if (!PyArg_Parse(args,""))
		return NULL;
	return PyInt_FromLong((long)getgid());
}
#endif

static PyObject *
amiga_getpid(PyObject *self, PyObject *args)
{
	if (!PyArg_Parse(args,""))
		return NULL;
	return PyInt_FromLong((long)FindTask(0));
}

#ifdef HAVE_GETPGRP
static PyObject *
amiga_getpgrp(PyObject *self, PyObject *args)
{
	if (!PyArg_Parse(args,""))
		return NULL;
#ifdef GETPGRP_HAVE_ARG
	return PyInt_FromLong((long)getpgrp(0));
#else /* GETPGRP_HAVE_ARG */
	return PyInt_FromLong((long)getpgrp());
#endif /* GETPGRP_HAVE_ARG */
}
#endif /* HAVE_GETPGRP */

#ifdef HAVE_SETPGRP
static PyObject *
amiga_setpgrp(PyObject *self, PyObject *args)
{
	if (!PyArg_Parse(args,""))
		return NULL;
#ifdef SETPGRP_HAVE_ARG
	if (setpgrp(0, 0) < 0)
#else /* SETPGRP_HAVE_ARG */
	if (setpgrp() < 0)
#endif /* SETPGRP_HAVE_ARG */
		return amiga_error();
	Py_INCREF(Py_None);
	return Py_None;
}

#endif /* HAVE_SETPGRP */

#ifdef HAVE_GETPPID
static PyObject *
amiga_getppid(PyObject *self, PyObject *args)
{
	if (!PyArg_Parse(args,""))
		return NULL;
	return PyInt_FromLong((long)getppid());
}
#endif

#ifdef HAVE_GETUID
static PyObject *
amiga_getuid(PyObject *self, PyObject *args)
{
	if (!PyArg_Parse(args,""))
		return NULL;
	return PyInt_FromLong((long)getuid());
}
#endif

#ifdef HAVE_POPEN
static PyObject *
amiga_popen(PyObject *self, PyObject *args)
{
	char *name;
	char *mode = "r";
	int bufsize = -1;
	FILE *fp;
	PyObject *f;
	if (!PyArg_ParseTuple(args, "s|si", &name, &mode, &bufsize))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	fp = popen(name, mode);
	Py_END_ALLOW_THREADS
	if (fp == NULL)
		return amiga_error();
	f = PyFile_FromFile(fp, name, mode, pclose);
	if (f != NULL)
		PyFile_SetBufSize(f, bufsize);
	return f;
}
#endif

#ifdef HAVE_SETUID
static PyObject *
amiga_setuid(PyObject *self, PyObject *args)
{
	int uid;
	if (!PyArg_Parse(args, "i", &uid))
		return NULL;
	if (setuid(uid) < 0)
		return amiga_error();
	Py_INCREF(Py_None);
	return Py_None;
}
#endif /* HAVE_SETUID */

#ifdef HAVE_SETGID
static PyObject *
amiga_setgid(PyObject *self, PyObject *args)
{
	int gid;
	if (!PyArg_Parse(args, "i", &gid))
		return NULL;
	if (setgid(gid) < 0)
		return amiga_error();
	Py_INCREF(Py_None);
	return Py_None;
}
#endif /* HAVE_SETGID */

static PyObject *
amiga_lstat(PyObject *self, PyObject *args)
{
#ifdef HAVE_LSTAT
	return amiga_do_stat(self, args, lstat);
#else /* !HAVE_LSTAT */
	return amiga_do_stat(self, args, stat);
#endif /* !HAVE_LSTAT */
}

#ifdef HAVE_READLINK
static PyObject *
amiga_readlink(PyObject *self, PyObject *args)
{
	char buf[MAXPATHLEN];
	char *path;
	int n;
	if (!PyArg_Parse(args, "s", &path))
	        return NULL;
	Py_BEGIN_ALLOW_THREADS
	n = readlink(path, buf, (int) sizeof buf);
	Py_END_ALLOW_THREADS
	if (n < 0)
	        return amiga_error_with_filename(path);
	return PyString_FromStringAndSize(buf, n);
}
#endif

#ifdef HAVE_SYMLINK
static PyObject *
amiga_symlink(PyObject *self, PyObject *args)
{
	return amiga_2str(args, symlink);
}
#endif

#ifdef HAVE_SETSID
static PyObject *
amiga_setsid(PyObject *self, PyObject *args)
{
	if (!PyArg_Parse(args,""))
		return NULL;
	if ((int)setsid() < 0)
		return amiga_error();
	Py_INCREF(Py_None);
	return Py_None;
}
#endif /* HAVE_SETSID */

#ifdef HAVE_SETPGID
static PyObject *
amiga_setpgid(PyObject *self, PyObject *args)
{
	int pid, pgrp;
	if (!PyArg_Parse(args, "(ii)", &pid, &pgrp))
		return NULL;
	if (setpgid(pid, pgrp) < 0)
		return amiga_error();
	Py_INCREF(Py_None);
	return Py_None;
}
#endif /* HAVE_SETPGID */

/* Functions acting on file descriptors */

static PyObject *
amiga_open(PyObject *self, PyObject *args)
{
    char *file = NULL;
    int flag;
    int mode = 0777;
    int fd;

    if (!PyArg_ParseTuple(args, "eti|i",
                          Py_FileSystemDefaultEncoding, &file,
                          &flag, &mode))
        return NULL;

    Py_BEGIN_ALLOW_THREADS
    fd = open(file, flag, mode);
    Py_END_ALLOW_THREADS
    if (fd < 0)
        return posix_error_with_allocated_filename(file);
    PyMem_Free(file);
    return PyInt_FromLong((long)fd);
}

static PyObject *
amiga_close(PyObject *self, PyObject *args)
{
    int fd, res;
    if (!PyArg_ParseTuple(args, "i:close", &fd))
        return NULL;
    if (!_PyVerify_fd(fd))
        return posix_error();
    Py_BEGIN_ALLOW_THREADS
    res = close(fd);
    Py_END_ALLOW_THREADS
    if (res < 0)
        return posix_error();
    Py_INCREF(Py_None);
    return Py_None;
}

#if defined(AMITCP) || defined(INET225)
static PyObject *
amiga_dup(PyObject *self, PyObject *args)
{
    int fd;
    if (!PyArg_ParseTuple(args, "i:dup", &fd))
        return NULL;
    if (!_PyVerify_fd(fd))
        return posix_error();
    Py_BEGIN_ALLOW_THREADS
    fd = dup(fd);
    Py_END_ALLOW_THREADS
    if (fd < 0)
        return posix_error();
    return PyInt_FromLong((long)fd);
}

static PyObject *
amiga_dup2(PyObject *self, PyObject *args)
{
    int fd, fd2, res;
    if (!PyArg_ParseTuple(args, "ii:dup2", &fd, &fd2))
        return NULL;
    if (!_PyVerify_fd(fd))
        return posix_error();
    Py_BEGIN_ALLOW_THREADS
    res = dup2(fd, fd2);
    Py_END_ALLOW_THREADS
    if (res < 0)
        return posix_error();
    Py_INCREF(Py_None);
    return Py_None;
}
#endif

static PyObject *
amiga_lseek(PyObject *self, PyObject *args)
{
    int fd, how;
    off_t pos, res;
    PyObject *posobj;
    if (!PyArg_ParseTuple(args, "iOi:lseek", &fd, &posobj, &how))
        return NULL;
#ifdef SEEK_SET
    /* Turn 0, 1, 2 into SEEK_{SET,CUR,END} */
    switch (how) {
    case 0: how = SEEK_SET; break;
    case 1: how = SEEK_CUR; break;
    case 2: how = SEEK_END; break;
    }
#endif /* SEEK_END */

    pos = PyInt_AsLong(posobj);
    if (PyErr_Occurred())
        return NULL;

    if (!_PyVerify_fd(fd))
        return posix_error();
    Py_BEGIN_ALLOW_THREADS
    res = lseek(fd, pos, how);
    Py_END_ALLOW_THREADS
    if (res < 0)
        return posix_error();

    return PyInt_FromLong(res);
}

static PyObject *
amiga_read(PyObject *self, PyObject *args)
{
    int fd, size, n;
    PyObject *buffer;
    if (!PyArg_ParseTuple(args, "ii:read", &fd, &size))
        return NULL;
    if (size < 0) {
        errno = EINVAL;
        return posix_error();
    }
    buffer = PyString_FromStringAndSize((char *)NULL, size);
    if (buffer == NULL)
        return NULL;
    if (!_PyVerify_fd(fd)) {
        Py_DECREF(buffer);
        return posix_error();
    }
    Py_BEGIN_ALLOW_THREADS
    n = read(fd, PyString_AsString(buffer), size);
    Py_END_ALLOW_THREADS
    if (n < 0) {
        Py_DECREF(buffer);
        return posix_error();
    }
    if (n != size)
        _PyString_Resize(&buffer, n);
    return buffer;
}

static PyObject *
amiga_write(PyObject *self, PyObject *args)
{
    Py_buffer pbuf;
    int fd;
    Py_ssize_t size, len;

    if (!PyArg_ParseTuple(args, "is*:write", &fd, &pbuf))
        return NULL;
    if (!_PyVerify_fd(fd)) {
        PyBuffer_Release(&pbuf);
        return posix_error();
    }
    len = pbuf.len;
    Py_BEGIN_ALLOW_THREADS
    size = write(fd, pbuf.buf, len);
    Py_END_ALLOW_THREADS
    PyBuffer_Release(&pbuf);
    if (size < 0)
        return posix_error();
    return PyInt_FromSsize_t(size);
}

static PyObject *
amiga_fstat(PyObject *self, PyObject *args)
{
    int fd;
    struct stat st;
    int res;
    if (!PyArg_ParseTuple(args, "i:fstat", &fd))
        return NULL;
    if (!_PyVerify_fd(fd))
        return posix_error();
    Py_BEGIN_ALLOW_THREADS
    res = fstat(fd, &st);
    Py_END_ALLOW_THREADS
    if (res != 0) {
        return posix_error();
    }

    return _pystat_fromstructstat(&st);
}

static PyObject *
amiga_fdopen(PyObject *self, PyObject *args)
{
    int fd;
    char *orgmode = "r";
    int bufsize = -1;
    FILE *fp;
    PyObject *f;
    char *mode;
    if (!PyArg_ParseTuple(args, "i|si", &fd, &orgmode, &bufsize))
        return NULL;

    /* Sanitize mode.  See fileobject.c */
    mode = PyMem_MALLOC(strlen(orgmode)+3);
    if (!mode) {
        PyErr_NoMemory();
        return NULL;
    }
    strcpy(mode, orgmode);
    if (_PyFile_SanitizeMode(mode)) {
        PyMem_FREE(mode);
        return NULL;
    }
    if (!_PyVerify_fd(fd)) {
        PyMem_FREE(mode);
        return posix_error();
    }
    /* The dummy filename used here must be kept in sync with the value
       tested against in gzip.GzipFile.__init__() - see issue #13781. */
    f = PyFile_FromFile(NULL, "<fdopen>", orgmode, fclose);
    if (f == NULL) {
        PyMem_FREE(mode);
        return NULL;
    }
    Py_BEGIN_ALLOW_THREADS
    fp = fdopen(fd, mode);
    Py_END_ALLOW_THREADS
    PyMem_FREE(mode);
    if (fp == NULL) {
        Py_DECREF(f);
        return posix_error();
    }
    /* We now know we will succeed, so initialize the file object. */
    ((PyFileObject *)f)->f_fp = fp;
    PyFile_SetBufSize(f, bufsize);
    return f;
}

#if 0
/*** XXX pipe() is useless without fork() or threads ***/
/***     TODO: guess what.. implement threads! ***/
static int pipe(int *fildes)
{
	/* 0=ok, -1=err, errno=EMFILE,ENFILE,EFAULT */
	char buf[50];
	static int num = 1;
	
	sprintf(buf,"PIPE:Py%ld_%ld",FindTask(0),num++);
	fildes[0]=open(buf,O_RDONLY,0);
	if(fildes[0]>0)
	{
		fildes[1]=open(buf,O_WRONLY|O_CREAT,FIBF_OTR_READ|FIBF_OTR_WRITE);
		if(fildes[1]>0)
		{
			return 0;
		}
		close(fildes[0]);
	}
	return -1;
}

static PyObject *
amiga_pipe(PyObject *self, PyObject *args)
{
	int fds[2];
	int res;
	if (!PyArg_Parse(args, ""))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = pipe(fds);
	Py_END_ALLOW_THREADS
	if (res != 0)
		return amiga_error();
	return Py_BuildValue("(ii)", fds[0], fds[1]);
}
#endif


static PyObject *
amiga_fullpath(PyObject *self, PyObject *args)
{
	BOOL ok=FALSE;
	BPTR lk;
	char *path;
	char buf[MAXPATHLEN];

	if (!PyArg_ParseTuple(args, "s", &path)) return NULL;

	Py_BEGIN_ALLOW_THREADS
	if(lk=Lock(path,SHARED_LOCK))
	{
		ok=NameFromLock(lk,buf,sizeof(buf));
		UnLock(lk);
	}
	Py_END_ALLOW_THREADS

	if(!ok)
	{
		errno=__io2errno(IoErr());
		return amiga_error();
	}
	else return PyString_FromString(buf);
}

static PyObject *amiga_putenv(PyObject *self, PyObject *args)
{
	char *s1, *s2;

	if (!PyArg_ParseTuple(args, "ss", &s1, &s2)) return NULL;
	if(setenv(s1,s2,1))
	{
		amiga_error(); return NULL;
	}
	
	Py_INCREF(Py_None); return Py_None;
}

#ifdef HAVE_STRERROR
static char posix_strerror__doc__[] =
"strerror(code) -> string\n\
Translate an error code to a message string.";

PyObject *
amiga_strerror(PyObject *self, PyObject *args)
{
	int errnum;
	if (!PyArg_ParseTuple(args, "i:strerror", &errnum))
		return NULL;
	return PyString_FromString(strerror(errnum));
}
#endif /* strerror */

/* New functions using PosixLib */

static PyObject *
amiga_access(PyObject *self, PyObject *args)
{
	char *path;
	int mode;
	int res;
	if (!PyArg_ParseTuple(args, "si:access", &path, &mode))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = access(path, mode);
	Py_END_ALLOW_THREADS
	if (res < 0)
		return amiga_error_with_filename(path);
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
amiga_isatty(PyObject *self, PyObject *args)
{
	int fd;
	int res;
	if (!PyArg_ParseTuple(args, "i:isatty", &fd))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = isatty(fd);
	Py_END_ALLOW_THREADS
	return PyInt_FromLong((long)res);
}

static PyObject *
amiga_closerange(PyObject *self, PyObject *args)
{
	int fd_low, fd_high;
	int i;
	if (!PyArg_ParseTuple(args, "ii:closerange", &fd_low, &fd_high))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	for (i = fd_low; i < fd_high; i++) {
		close(i);  /* Ignore errors */
	}
	Py_END_ALLOW_THREADS
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
amiga_nice(PyObject *self, PyObject *args)
{
	int incr;
	int res;
	if (!PyArg_ParseTuple(args, "i:nice", &incr))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = nice(incr);
	Py_END_ALLOW_THREADS
	if (res < 0)
		return amiga_error();
	return PyInt_FromLong((long)res);
}

static PyObject *
amiga_kill(PyObject *self, PyObject *args)
{
	int pid, sig;
	int res;
	if (!PyArg_ParseTuple(args, "ii:kill", &pid, &sig))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = kill(pid, sig);
	Py_END_ALLOW_THREADS
	if (res < 0)
		return amiga_error();
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
amiga_waitpid(PyObject *self, PyObject *args)
{
	int pid, options;
	pid_t result;
	int status;
	if (!PyArg_ParseTuple(args, "ii:waitpid", &pid, &options))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	result = waitpid(pid, &status, options);
	Py_END_ALLOW_THREADS
	if (result < 0)
		return amiga_error();
	return Py_BuildValue("Ni", PyInt_FromLong((long)result), status);
}

static PyObject *
amiga_pathconf(PyObject *self, PyObject *args)
{
	char *path;
	int name;
	long result;
	if (!PyArg_ParseTuple(args, "si:pathconf", &path, &name))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	result = pathconf(path, name);
	Py_END_ALLOW_THREADS
	if (result < 0)
		return amiga_error_with_filename(path);
	return PyInt_FromLong(result);
}

static PyObject *
amiga_fpathconf(PyObject *self, PyObject *args)
{
	int fd;
	int name;
	long result;
	if (!PyArg_ParseTuple(args, "ii:fpathconf", &fd, &name))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	result = fpathconf(fd, name);
	Py_END_ALLOW_THREADS
	if (result < 0)
		return amiga_error();
	return PyInt_FromLong(result);
}

static PyObject *
amiga_sysconf(PyObject *self, PyObject *args)
{
	int name;
	long result;
	if (!PyArg_ParseTuple(args, "i:sysconf", &name))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	result = sysconf(name);
	Py_END_ALLOW_THREADS
	if (result < 0)
		return amiga_error();
	return PyInt_FromLong(result);
}

static PyObject *
amiga_tempnam(PyObject *self, PyObject *args)
{
	/* tempnam not available in PosixLib, return error for now */
	PyErr_SetString(PyExc_NotImplementedError, "tempnam not implemented on Amiga");
	return NULL;
}

static PyObject *
amiga_urandom(PyObject *self, PyObject *args)
{
	int size;
	char *buf;
	int res;
	int i;
	PyObject *ret;
	if (!PyArg_ParseTuple(args, "i:urandom", &size))
		return NULL;
	if (size < 0) {
		PyErr_SetString(PyExc_ValueError, "size must be non-negative");
		return NULL;
	}
	buf = malloc(size);
	if (buf == NULL)
		return PyErr_NoMemory();
	
	/* On Amiga, we'll use a simple random generator for now */
	/* In AmigaOS 4, you might want to use RANDOM: device */
	Py_BEGIN_ALLOW_THREADS
	for (i = 0; i < size; i++) {
		buf[i] = (char)(rand() & 0xFF);
	}
	Py_END_ALLOW_THREADS
	
	ret = PyString_FromStringAndSize(buf, size);
	free(buf);
	return ret;
}

static PyObject *
amiga_getdtablesize(PyObject *self, PyObject *args)
{
	int size;
	if (!PyArg_ParseTuple(args, ":getdtablesize"))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	size = getdtablesize();
	Py_END_ALLOW_THREADS
	return PyInt_FromLong((long)size);
}

static PyObject *
amiga_fsync(PyObject *self, PyObject *args)
{
	int fd;
	int res;
	if (!PyArg_ParseTuple(args, "i:fsync", &fd))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = fsync(fd);
	Py_END_ALLOW_THREADS
	if (res < 0)
		return amiga_error();
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
amiga_sleep(PyObject *self, PyObject *args)
{
	unsigned int seconds;
	unsigned int result;
	if (!PyArg_ParseTuple(args, "I:sleep", &seconds))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	result = sleep(seconds);
	Py_END_ALLOW_THREADS
	return PyInt_FromLong((long)result);
}

static PyObject *
amiga_usleep(PyObject *self, PyObject *args)
{
	unsigned int useconds;
	int res;
	if (!PyArg_ParseTuple(args, "I:usleep", &useconds))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = usleep(useconds);
	Py_END_ALLOW_THREADS
	if (res < 0)
		return amiga_error();
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
amiga_gettimeofday(PyObject *self, PyObject *args)
{
	struct timeval tv;
	int res;
	if (!PyArg_ParseTuple(args, ":gettimeofday"))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = gettimeofday(&tv, NULL);
	Py_END_ALLOW_THREADS
	if (res < 0)
		return amiga_error();
	return Py_BuildValue("(ll)", (long)tv.tv_sec, (long)tv.tv_usec);
}

static PyObject *
amiga_settimeofday(PyObject *self, PyObject *args)
{
	struct timeval tv;
	int res;
	if (!PyArg_ParseTuple(args, "(ll):settimeofday", &tv.tv_sec, &tv.tv_usec))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = settimeofday(&tv, NULL);
	Py_END_ALLOW_THREADS
	if (res < 0)
		return amiga_error();
	Py_INCREF(Py_None);
	return Py_None;
}

/* external function prototype: */
unsigned long CalcCRC32(unsigned long startcrc, const void *data, unsigned long size);

PyObject *
amiga_crc32(PyObject *self, PyObject *args)
{
	PyObject *py_str;
	int startcrc = 0;
	if(!PyArg_ParseTuple(args,"S|i",&py_str,&startcrc)) return NULL;
	return PyInt_FromLong(CalcCRC32(startcrc,PyString_AsString(py_str), PyString_Size(py_str)));
}

static PyObject *
amiga_abort(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":abort"))
		return NULL;
	
	/* Abort the interpreter immediately */
	abort();
	/* This should never return */
	return NULL;
}

static PyObject *
amiga_exit(PyObject *self, PyObject *args)
{
	int status = 0;
	
	if (!PyArg_ParseTuple(args, "|i:_exit", &status))
		return NULL;
	
	/* On AmigaOS, we can implement _exit by sending ourselves a signal */
	/* PosixLib kill() handles self-signaling by calling __exit() directly */
	Py_BEGIN_ALLOW_THREADS
	kill(getpid(), SIGKILL);  /* Use SIGKILL which is available in PosixLib */
	Py_END_ALLOW_THREADS
	
	/* If kill() doesn't work, fall back to exit() */
	exit(status);
	/* This should never return */
	return NULL;
}

/* times() function commented out due to compilation issues */
/*
static PyObject *
amiga_times(PyObject *self, PyObject *args)
{
	struct tms tms;
	clock_t result;
	
	if (!PyArg_ParseTuple(args, ":times"))
		return NULL;
	
	Py_BEGIN_ALLOW_THREADS
	result = amiga_times_func(&tms);
	Py_END_ALLOW_THREADS
	
	if (result == (clock_t)-1)
		return amiga_error();
	
	return Py_BuildValue("(lllll)",
		(long)result,
		(long)tms.tms_utime,
		(long)tms.tms_stime,
		(long)tms.tms_cutime,
		(long)tms.tms_cstime);
}
*/

/* mkfifo() not implemented in PosixLib */
/*
static PyObject *
amiga_mkfifo(PyObject *self, PyObject *args)
{
	char *path;
	int mode = 0666;
	int res;
	
	if (!PyArg_ParseTuple(args, "s|i:mkfifo", &path, &mode))
		return NULL;
	
	Py_BEGIN_ALLOW_THREADS
	res = mkfifo(path, mode);
	Py_END_ALLOW_THREADS
	
	if (res < 0)
		return amiga_error_with_filename(path);
	
	Py_INCREF(Py_None);
	return Py_None;
}
*/

/* Terminal control functions not available on AmigaOS 3 - no termios.h support */
/*
static PyObject *
amiga_tcgetpgrp(PyObject *self, PyObject *args)
{
	int fd;
	pid_t pgrp;
	
	if (!PyArg_ParseTuple(args, "i:tcgetpgrp", &fd))
		return NULL;
	
	Py_BEGIN_ALLOW_THREADS
	pgrp = tcgetpgrp(fd);
	Py_END_ALLOW_THREADS
	
	if (pgrp < 0)
		return amiga_error();
	
	return PyInt_FromLong((long)pgrp);
}

static PyObject *
amiga_tcsetpgrp(PyObject *self, PyObject *args)
{
	int fd;
	int pgrp;
	int res;
	
	if (!PyArg_ParseTuple(args, "ii:tcsetpgrp", &fd, &pgrp))
		return NULL;
	
	Py_BEGIN_ALLOW_THREADS
	res = tcsetpgrp(fd, pgrp);
	Py_END_ALLOW_THREADS
	
	if (res < 0)
		return amiga_error();
	
	Py_INCREF(Py_None);
	return Py_None;
}
*/

#ifdef HAVE_FTRUNCATE
static PyObject *amiga_ftruncate(PyObject *, PyObject *);
#endif

static struct PyMethodDef amiga_methods[] = {
	{"chdir",   amiga_chdir},
	{"chmod",   amiga_chmod},
#ifdef HAVE_CHOWN
	{"chown",   amiga_chown},
#endif
#ifdef HAVE_GETCWD
	{"getcwd",  amiga_getcwd},
#endif
	{"fullpath", amiga_fullpath,1},
#ifdef HAVE_LINK
	{"link",    amiga_link},
#endif
	{"listdir", amiga_listdir},
	{"lstat",   amiga_lstat},
	{"mkdir",   amiga_mkdir , 1},
#ifdef HAVE_READLINK
	{"readlink",    amiga_readlink},
#endif
	{"rename",  amiga_rename},
	{"rmdir",   amiga_rmdir},
	{"stat",    amiga_stat},
#ifdef HAVE_SYMLINK
	{"symlink", amiga_symlink},
#endif
#ifdef HAVE_SYSTEM
	{"system",  amiga_system},
#endif
#if defined(AMITCP) || defined(INET225)
	{"umask",   amiga_umask},
#endif
#ifdef HAVE_UNAME
	{"uname",   amiga_uname},
#endif
	{"unlink",  amiga_unlink},
	{"remove",  amiga_unlink},
#if defined(AMITCP) || defined(INET225)
	{"utime",   amiga_utime},
#endif
	{"abort",   amiga_abort},
	{"_exit",   amiga_exit},
	/* {"times",   amiga_times}, */  
	/* execv/execve not supported on AmigaOS 3 */
#ifdef HAVE_GETEGID
	{"getegid", amiga_getegid},
#endif
#ifdef HAVE_GETEUID
	{"geteuid", amiga_geteuid},
#endif
#ifdef HAVE_GETGID
	{"getgid",  amiga_getgid},
#endif
	{"getpid",  amiga_getpid},
#ifdef HAVE_GETPGRP
	{"getpgrp", amiga_getpgrp},
#endif
#ifdef HAVE_GETPPID
	{"getppid", amiga_getppid},
#endif
#ifdef HAVE_GETUID
	{"getuid",  amiga_getuid},
#endif
#ifdef HAVE_POPEN
	{"popen",   amiga_popen,    1},
#endif
#ifdef HAVE_SETUID
	{"setuid",  amiga_setuid},
#endif
#ifdef HAVE_SETGID
	{"setgid",  amiga_setgid},
#endif
#ifdef HAVE_SETPGRP
	{"setpgrp", amiga_setpgrp},
#endif
#ifdef HAVE_SETSID
	{"setsid",  amiga_setsid},
#endif
#ifdef HAVE_SETPGID
	{"setpgid", amiga_setpgid},
#endif
	/* Terminal control functions not available on AmigaOS 3 - no termios.h support */
	/* {"tcgetpgrp",   amiga_tcgetpgrp}, */
	/* {"tcsetpgrp",   amiga_tcsetpgrp}, */
	{"open",    amiga_open},
	{"close",   amiga_close},
#if defined(AMITCP) || defined(INET225)
	{"dup",     amiga_dup},
	{"dup2",    amiga_dup2},
#endif
	{"lseek",   amiga_lseek},
	{"read",    amiga_read},
	{"write",   amiga_write},
	{"fstat",   amiga_fstat},
	{"fdopen",  amiga_fdopen,   1},
	/* {"mkfifo",	amiga_mkfifo, 1}, */  /* Not implemented in PosixLib */
#ifdef HAVE_FTRUNCATE
	{"ftruncate",	amiga_ftruncate, 1},
#endif
#ifdef HAVE_PUTENV
	{"putenv", amiga_putenv, 1},
#endif
#ifdef HAVE_STRERROR
	{"strerror",	amiga_strerror, 1},
#endif
#ifdef HAVE_ACCESS
	{"access", amiga_access},
#endif
#ifdef HAVE_ISATTY
	{"isatty", amiga_isatty},
#endif
#ifdef HAVE_CLOSERANGE
	{"closerange", amiga_closerange},
#endif
#ifdef HAVE_NICE
	{"nice", amiga_nice},
#endif
#ifdef HAVE_KILL
	{"kill", amiga_kill},
#endif
#ifdef HAVE_WAITPID
	{"waitpid", amiga_waitpid},
#endif
#ifdef HAVE_PATHCONF
	{"pathconf", amiga_pathconf},
#endif
#ifdef HAVE_FPATHCONF
	{"fpathconf", amiga_fpathconf},
#endif
#ifdef HAVE_SYSCONF
	{"sysconf", amiga_sysconf},
#endif
#ifdef HAVE_TEMPNAM
	{"tempnam", amiga_tempnam},
#endif
#ifdef HAVE_URANDOM
	{"urandom", amiga_urandom},
#endif
#ifdef HAVE_GETDTABLESIZE
	{"getdtablesize", amiga_getdtablesize},
#endif
#ifdef HAVE_FSYNC
	{"fsync", amiga_fsync},
#endif
#ifdef HAVE_SLEEP
	{"sleep", amiga_sleep},
#endif
#ifdef HAVE_USLEEP
	{"usleep", amiga_usleep},
#endif
#ifdef HAVE_GETTIMEOFDAY
	{"gettimeofday", amiga_gettimeofday},
#endif
#ifdef HAVE_SETTIMEOFDAY
	{"settimeofday", amiga_settimeofday},
#endif
#if 0
	/* AMIGA TODO: implement threads. Otherwise pipe() is useless. */
	{"pipe",    amiga_pipe},
#endif
	{"crc32",	amiga_crc32, 1},
	{NULL,      NULL}        /* Sentinel */
};


static int
ins(PyObject *d, char *symbol, long value)
{
        PyObject* v = PyInt_FromLong(value);
        if (!v || PyDict_SetItemString(d, symbol, v) < 0)
                return -1;                   /* triggers fatal error */

        Py_DECREF(v);
        return 0;
}

static int all_ins(PyObject *d)
{
#ifdef WNOHANG
        if (ins(d, "WNOHANG", (long)WNOHANG)) return -1;
#endif        
#ifdef O_RDONLY
        if (ins(d, "O_RDONLY", (long)O_RDONLY)) return -1;
#endif
#ifdef O_WRONLY
        if (ins(d, "O_WRONLY", (long)O_WRONLY)) return -1;
#endif
#ifdef O_RDWR
        if (ins(d, "O_RDWR", (long)O_RDWR)) return -1;
#endif
#ifdef O_NDELAY
        if (ins(d, "O_NDELAY", (long)O_NDELAY)) return -1;
#endif
#ifdef O_NONBLOCK
        if (ins(d, "O_NONBLOCK", (long)O_NONBLOCK)) return -1;
#endif
#ifdef O_APPEND
        if (ins(d, "O_APPEND", (long)O_APPEND)) return -1;
#endif
#ifdef O_DSYNC
        if (ins(d, "O_DSYNC", (long)O_DSYNC)) return -1;
#endif
#ifdef O_RSYNC
        if (ins(d, "O_RSYNC", (long)O_RSYNC)) return -1;
#endif
#ifdef O_SYNC
        if (ins(d, "O_SYNC", (long)O_SYNC)) return -1;
#endif
#ifdef O_NOCTTY
        if (ins(d, "O_NOCTTY", (long)O_NOCTTY)) return -1;
#endif
#ifdef O_CREAT
        if (ins(d, "O_CREAT", (long)O_CREAT)) return -1;
#endif
#ifdef O_EXCL
        if (ins(d, "O_EXCL", (long)O_EXCL)) return -1;
#endif
#ifdef O_TRUNC
        if (ins(d, "O_TRUNC", (long)O_TRUNC)) return -1;
#endif
#ifdef O_BINARY
        if (ins(d, "O_BINARY", (long)O_BINARY)) return -1;
#endif
#ifdef O_TEXT
        if (ins(d, "O_TEXT", (long)O_TEXT)) return -1;
#endif

/* Add constants for new POSIX functions */
#ifdef F_OK
        if (ins(d, "F_OK", (long)F_OK)) return -1;
#endif
#ifdef R_OK
        if (ins(d, "R_OK", (long)R_OK)) return -1;
#endif
#ifdef W_OK
        if (ins(d, "W_OK", (long)W_OK)) return -1;
#endif
#ifdef X_OK
        if (ins(d, "X_OK", (long)X_OK)) return -1;
#endif

#ifdef WNOHANG
        if (ins(d, "WNOHANG", (long)WNOHANG)) return -1;
#endif
#ifdef WUNTRACED
        if (ins(d, "WUNTRACED", (long)WUNTRACED)) return -1;
#endif

#ifdef _PC_PATH_MAX
        if (ins(d, "_PC_PATH_MAX", (long)_PC_PATH_MAX)) return -1;
#endif
#ifdef _PC_NAME_MAX
        if (ins(d, "_PC_NAME_MAX", (long)_PC_NAME_MAX)) return -1;
#endif
#ifdef _PC_LINK_MAX
        if (ins(d, "_PC_LINK_MAX", (long)_PC_LINK_MAX)) return -1;
#endif

#ifdef _SC_ARG_MAX
        if (ins(d, "_SC_ARG_MAX", (long)_SC_ARG_MAX)) return -1;
#endif
#ifdef _SC_CHILD_MAX
        if (ins(d, "_SC_CHILD_MAX", (long)_SC_CHILD_MAX)) return -1;
#endif
#ifdef _SC_OPEN_MAX
        if (ins(d, "_SC_OPEN_MAX", (long)_SC_OPEN_MAX)) return -1;
#endif
#ifdef _SC_PAGESIZE
        if (ins(d, "_SC_PAGESIZE", (long)_SC_PAGESIZE)) return -1;
#endif

/* Add missing constants for new functions */
#ifdef S_IFIFO
        if (ins(d, "S_IFIFO", (long)S_IFIFO)) return -1;
#endif

#ifdef WCONTINUED
        if (ins(d, "WCONTINUED", (long)WCONTINUED)) return -1;
#endif

#ifdef WEXITED
        if (ins(d, "WEXITED", (long)WEXITED)) return -1;
#endif

#ifdef WSTOPPED
        if (ins(d, "WSTOPPED", (long)WSTOPPED)) return -1;
#endif

/* Add signal constants for AmigaOS */
#ifdef SIGBREAKF_CTRL_C
        if (ins(d, "SIGBREAKF_CTRL_C", (long)SIGBREAKF_CTRL_C)) return -1;
#endif
#ifdef SIGKILL
        if (ins(d, "SIGKILL", (long)SIGKILL)) return -1;
#endif
#ifdef SIGQUIT
        if (ins(d, "SIGQUIT", (long)SIGQUIT)) return -1;
#endif

#if defined(PYOS_OS2)
        if (insertvalues(d)) return -1;
#endif
        return 0;
}

void
initamiga(void)
{
	PyObject *m, *d, *globv, *locv, *bothv, *aliases;

	m = Py_InitModule("amiga", amiga_methods);
	d = PyModule_GetDict(m);
	
	/* Initialize amiga.environ dictionary */
	if(!convertenviron(&globv, &locv, &bothv, &aliases))
		Py_FatalError("can't read environment");

	if (PyDict_SetItemString(d, "environ", bothv) != 0)
		Py_FatalError("can't define amiga.environ");
	Py_DECREF(bothv);
	if (PyDict_SetItemString(d, "globalvars", globv) != 0)
		Py_FatalError("can't define amiga.globalvars");
	Py_DECREF(globv);
	if (PyDict_SetItemString(d, "shellvars", locv) != 0)
		Py_FatalError("can't define amiga.shellvars");
	Py_DECREF(locv);
	if (PyDict_SetItemString(d, "shellaliases", aliases ) != 0)
		Py_FatalError("can't define amiga.shellaliases");
	Py_DECREF(aliases);

	if(all_ins(d)) return;

	/* Initialize exception */
	PyDict_SetItemString(d, "error", PyExc_OSError);
}

#ifdef HAVE_FTRUNCATE
static PyObject *
amiga_ftruncate(PyObject *self, PyObject *args)
{
    int fd;
    long length;
    int res;
    if (!PyArg_ParseTuple(args, "il", &fd, &length))
        return NULL;
    Py_BEGIN_ALLOW_THREADS
    res = ftruncate(fd, (off_t)length);
    Py_END_ALLOW_THREADS
    if (res < 0)
        return amiga_error();
    Py_INCREF(Py_None);
    return Py_None;
}
#endif
