/*
 *      _close.c - close a file descriptor
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 *
 *      DEPRECATED: This file is deprecated in Amiga Python 2.7.18 in favour of vbcc PosixLib
 */

#include <ios1.h>
#include <stdlib.h>
#include <stdio.h>
#include <dos.h>
#include <proto/dos.h>
#include <errno.h>

#ifdef AMITCP
#include <proto/socket.h>
extern struct Library *SocketBase;
#endif

#include "Python.h"
#include "protos.h"
#include "libcheck.h"

int 
__close(int fd)
{
  struct UFB *ufb;

  /*
   * Check for the break signals
   */
  __chkabort();

  /*
   * Find the ufb
   */
  if ((ufb = __chkufb(fd)) == NULL) {
    /* __chkufb sets the errno to EBADF */
    return -1;
  }

  /*
   * Check if close is not needed
   */
  if ((ufb->ufbflg & (UFB_NC | UFB_CLO)) != UFB_NC) {

    /*
     * Empty flags mean empty ufb
     */
    if (ufb->ufbflg == 0) {
      errno = EBADF;
      return -1;
    }

    /*
     * Close the file
     */
    if (!(ufb->ufbflg & UFB_SOCK) && ufb->ufbfh != NULL) {
      Close((BPTR)ufb->ufbfh);
      
      /*
       * Remove the file if it was temporary
       */
      if (ufb->ufbflg & UFB_TEMP && ufb->ufbfn != NULL) 
        remove(ufb->ufbfn);
    }
  }

  /*
   * Free the file name
   */
  if (ufb->ufbfn != NULL) {
    free(ufb->ufbfn);
    ufb->ufbfn = NULL;
  }

  /*
   * Clear the flags to free this ufb
   */
  ufb->ufbflg = 0;
  ufb->ufbfh = NULL; /* just in case */

#ifdef AMITCP
  /* 
   * Close the socket if applicable
   * Can safely assume it is opened at the time we need to do a
   * CloseSocket() call (how could you create a socket otherwise?),
   * so a call to checksocketlib() is not needed.
   */
  if (SocketBase) 
    CloseSocket(fd);
#endif
  
  return 0;
} 