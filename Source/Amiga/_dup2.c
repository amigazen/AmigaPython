/*
 *      _dup2.c - duplicate a file descriptor to a specific number
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
 * Duplicate an existing file descriptor to a specific new file descriptor.
 * The current UFB implementation for SAS C allows only sockets to be duplicated.
 */
int
__dup2(int old_fd, int new_fd)
{
  struct UFB *ufb;
  int ufbflg;

  /*
   * Check if there is nothing to do
   */
  if (old_fd == new_fd)
    return old_fd;

  /*
   * Check for the break signals
   */
  __chkabort();

  __close(new_fd);

  /*
   * Find the ufb * for the old FD
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
    return Dup2Socket(old_fd, new_fd);
  } else {
    errno = EBADF;
    return -1;
  }
} 