/*
 *      _dup.c - duplicate a file descriptor
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 *
 *      DEPRECATED: This file is deprecated in Amiga Python 2.7.18 in favour of vbcc PosixLib
 */

#include <ios1.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dos.h>
#define USE_BUILTIN_MATH
#include <string.h>
#include <errno.h>
#include <dos/dos.h>
#include <proto/dos.h>

#include <bsdsocket.h>
#include "libcheck.h"

/*
 * Duplicate an existing file descriptor.
 * The current UFB implementation for SAS C allows only sockets to be duplicated.
 */
int
dup(int old_fd)
{
  struct UFB *ufb;
  int ufbflg;
  
  /*
   * Check for the break signals
   */
  __chkabort();

  /*
   * Find the ufb * for the given FD
   */
  if ((ufb = __chkufb(old_fd)) == NULL) {
    errno = EBADF;
    return -1;
  }
  
  ufbflg = ufb->ufbflg;

  /* 
   * The UFB system won't allow duplicating ordinary files
   */
  if ((ufbflg & UFB_SOCK) == UFB_SOCK) {
    /* needs bsdsocket.library */
    if(!checksocketlib()) return -1;
    return Dup2Socket(old_fd, -1);
  } else {
    errno = EBADF;
    return -1;
  }
} 