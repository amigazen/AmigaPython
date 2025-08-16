/**************************************************************\
**                                                            **
**  UNIX 'errno' mapping for the Amiga                        **
**                                                            **
**  Based on Irmen de Jong's original Amiga port              **
**  Updated for Python 2.7.18                                 **
**                                                            **
\**************************************************************/

#include <errno.h>
#include <string.h>
#include <dos/dos.h>
#include <proto/dos.h>
#include <bsdsocket.h>
#include <amitcp/socketbasetags.h>

#include "Python.h"
#include "libcheck.h"

/* Convert AmigaDOS I/O error to UNIX errno */
int __io2errno(int ioerr)
{
    switch (ioerr) 
    {
        case 0:
            return 0;
        case ERROR_NO_FREE_STORE:
            return ENOMEM;
        case ERROR_TASK_TABLE_FULL:
            return EAGAIN;
        case ERROR_LINE_TOO_LONG:
            return E2BIG;
        case ERROR_FILE_NOT_OBJECT:
            return EACCES;
        case ERROR_OBJECT_WRONG_TYPE:
            return ENOTDIR;
        case ERROR_INVALID_RESIDENT_LIBRARY:
        case ERROR_NO_DEFAULT_DIR:
        case ERROR_OBJECT_IN_USE:
        case ERROR_OBJECT_EXISTS:
        case ERROR_DIR_NOT_EMPTY:
            return EBUSY;
        case ERROR_BAD_NUMBER:
            return EINVAL;
        case ERROR_BAD_STREAM_NAME:
        case ERROR_INVALID_LOCK:
        case ERROR_INVALID_COMPONENT_NAME:
            return ENOENT;
        case ERROR_DEVICE_NOT_MOUNTED:
            return ENXIO;
        case ERROR_NOT_IMPLEMENTED:
            return ENOSYS;
        case ERROR_IS_SOFT_LINK:
        case ERROR_OBJECT_LINKED:
            return EMLINK;
        case ERROR_WRITE_PROTECTED:
        case ERROR_DISK_WRITE_PROTECTED:
        case ERROR_READ_PROTECTED:
        case ERROR_DELETE_PROTECTED:
            return EACCES;
        case ERROR_SEEK_ERROR:
            return ESPIPE;
        case ERROR_DISK_FULL:
            return ENOSPC;
        case ERROR_RENAME_ACROSS_DEVICES:
            return EXDEV;
        case ERROR_DIRECTORY_NOT_EMPTY:
            return ENOTEMPTY;
        case ERROR_TOO_MANY_LEVELS:
            return EMLINK;
        default:
            return EIO;
    }
}

/*
 * Return the text for a given error number
 *
 * This function returns a pointer to the (English) string describing the
 * error code given as argument. The error strings are defined for the
 * error codes defined in <sys/errno.h>.
 *
 * The string pointer returned should not be modified by the program, 
 * but may be overwritten by a subsequent call to this function.
 */

extern char *__sys_errlist[];

char *
strerror(int error)
{
  ULONG taglist[3];

  if(!checksocketlib())
  {
    /* cannot use bsdsocket.lib's error strings, use those from SAS */
    PyErr_Clear();
    if(error>=0 && error<=34) return __sys_errlist[error];
    if(error==ELOOP) return "Too many levels of links"; /* link loop */
    else return __sys_errlist[0];
  }

  taglist[0] = SBTM_GETVAL(SBTC_ERRNOSTRPTR);
  taglist[1] = error;
  taglist[2] = TAG_END;

  SocketBaseTagList((struct TagItem *)taglist);
  return (char *)taglist[1];
} 