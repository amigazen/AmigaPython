/*
 *      io2errno.c - AmigaDOS I/O error to UNIX errno conversion
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 */

#include <errno.h>
#include <dos/dos.h>
#include <proto/dos.h>

/* Define missing error codes if not already defined */
#ifndef ERROR_DIR_NOT_EMPTY
#define ERROR_DIR_NOT_EMPTY 205
#endif

#ifndef ERROR_BAD_ARGUMENTS
#define ERROR_BAD_ARGUMENTS 209
#endif

#ifndef ERROR_INVALID_PACKET
#define ERROR_INVALID_PACKET 210
#endif

/* Define missing POSIX error codes if not already defined */
#ifndef EOVERFLOW
#define EOVERFLOW 75
#endif

#ifndef EBADMSG
#define EBADMSG 74
#endif

/*
 * Convert AmigaDOS I/O error to UNIX errno
 * 
 * Maps Amiga DOS I/O error codes to their closest UNIX errno equivalents
 * 
 * Parameters:
 *   ioerr - The AmigaDOS error code to convert
 * 
 * Returns:
 *   The equivalent UNIX errno value
 */
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
        case ERROR_DIRECTORY_NOT_EMPTY:
            return ENOTEMPTY;
        case ERROR_TOO_MANY_LEVELS:
            return ELOOP;
        case ERROR_NO_MORE_ENTRIES:
            return ENOENT;
        case ERROR_LOCK_COLLISION:
            return EBUSY;
        case ERROR_BUFFER_OVERFLOW:
            return EOVERFLOW;
        case ERROR_BREAK:
            return EINTR;
        case ERROR_NOT_A_DOS_DISK:
            return ENOTBLK;
        case ERROR_NO_DISK:
            return ENXIO;
        case ERROR_OBJECT_NOT_FOUND:
            return ENOENT;
        case ERROR_BAD_TEMPLATE:
        case ERROR_BAD_ARGUMENTS:
        case ERROR_BAD_HUNK:
        case ERROR_ACTION_NOT_KNOWN:
            return EINVAL;
        case ERROR_INVALID_PACKET:
            return EBADMSG;

        /* Default case for unhandled errors */
        default:
            return EIO;
    }
} 