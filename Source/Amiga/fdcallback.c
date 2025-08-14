/*
 *      fdcallback.c - AmiTCP socket callback handling for Amiga
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 */

#include <ios1.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <bsdsocket.h>
#include <sys/cdefs.h>
#include <amitcp/socketbasetags.h>
#include <syslog.h>
#include <constructor.h>

#include "Python.h"
#include "libcheck.h"

extern unsigned long __fmask;
extern int (*__closefunc)(int);

/* Forward declaration */
long ASM fdCallback(REG(d0) int fd, REG(d1) int action);

/*
 * Install the AmiTCP file descriptor callback
 * This must be called after the Python interpreter has been initialized
 *
 * Returns:
 *   0 on success
 *   1 if the callback could not be installed 
 *   2 if bsdsocket.library is not available (non-fatal if networking not required)
 */
long _install_AmiTCP_callback(void)
{
    /* First check if bsdsocket.library is available */
    if(checksocketlib())
    {
        /* Install the callback */
        if (SocketBaseTags(SBTM_SETVAL(SBTC_FDCALLBACK), &fdCallback, TAG_END)) {
            syslog(LOG_ERR, "Cannot install fdCallback!");
            return 1;
        }
        
        /*
         * Set up __closefunc (used at stdio cleanup)
         */
        if (__closefunc == NULL) {
            __closefunc = __close;
        }

        /*
         * Set default file mask to UNIX style
         */
        __fmask = 0644; 

        return 0;  /* Success */
    }
    else
    {
        PyErr_Clear();  /* Don't report error if socketlib not found */
        return 2;       /* Socket library not available (non-fatal) */
    }
}

/*
 * Callback function for AmiTCP socket operations
 * This function is called by AmiTCP when certain operations are performed
 * on sockets. It allows us to properly manage our custom file descriptor table.
 *
 * Parameters:
 *   fd     - File descriptor being operated on
 *   action - Operation being performed (e.g., FDCB_FREE)
 *
 * Returns:
 *   0 on success
 *  -1 on failure (invalid file descriptor)
 */
long ASM SAVEDS
fdCallback(REG(d0) int fd, REG(d1) int action)
{
    struct UFB *ufb;

    if ((ufb = __chkufb(fd)) == NULL)
        return -1;

    switch (action) {
    case FDCB_FREE:
        /* Socket is closing, free the UFB */
        ufb->ufbflg &= ~UFB_ALLOC;
        if (ufb->ufbfn) {
            free(ufb->ufbfn);
            ufb->ufbfn = NULL;
        }
        break;
        
    case FDCB_GETID:
        /* Return the socket identifier */
        return ufb->ufbfh;
        
    case FDCB_SETID:
        /* Set the socket identifier - not supported here */
        return -1;
        
    default:
        /* Unknown action - ignore it */
        break;
    }
    return 0;
} 