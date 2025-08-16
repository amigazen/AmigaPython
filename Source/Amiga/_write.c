/*
 *      _write.c - write to a file descriptor
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 *
 *      DEPRECATED: This file is deprecated in Amiga Python 2.7.18 in favour of vbcc PosixLib
 */

#include <ios1.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <dos.h>
#include <string.h>
#include <errno.h>
#include <dos/dos.h>
#include <proto/dos.h>

#include <bsdsocket.h>
#include "libcheck.h"

int
__write(int fd, const void *buffer, unsigned int length)
{
  struct UFB *ufb;
  int         count, totcount;
  char       *ptr;
  int         _OSERR;

  /*
   * Check for the break signals
   */
  __chkabort();
  
  /*
   * find the ufb
   */
  if ((ufb = __chkufb(fd)) == NULL) {
    errno = EINVAL;
    return -1;
  }
  
  /*
   * Check if write is allowed
   */
  if (!(ufb->ufbflg & UFB_WA)) {
    _OSERR = ERROR_WRITE_PROTECTED;
    errno = EIO;
    return -1;
  }

  /*
   * Seek to end of the file if necessary
   */
  if (ufb->ufbflg & UFB_APP)
    __lseek(fd, 0, 2);

  /* check if socket, then this function needs bsdsocket.library */
  if (ufb->ufbflg & UFB_SOCK)
    if(!checksocketlib()) return -1;

  /*
   * Check if translation is not needed
   */
  if (!(ufb->ufbflg & UFB_XLAT) ||
      (ptr = memchr(buffer, 0x0A, length)) == NULL) {
    if (ufb->ufbflg & UFB_SOCK) {
      if ((count = send(fd, (char *)buffer, length, 0)) < 0)
        return -1;
    }
    else {
      if ((count = Write((BPTR)ufb->ufbfh, (void *)buffer, length)) == -1)
        goto osfail;
    }
    return count;
  }

  totcount = length;

  /*
   * Translate, ie., append CR before each LF
   */
  do {
    count = ptr - (char *)buffer;
    if (ufb->ufbflg & UFB_SOCK) {
      if (send(fd, (char *)buffer, count, 0) < 0)
        return -1;
      if (send(fd, "\015"/* CR */, 1, 0) < 0)
        return -1;
    }
    else {
      if (Write((BPTR)ufb->ufbfh, (void *)buffer, count) == -1)
        goto osfail;
      if (Write((BPTR)ufb->ufbfh, "\015"/* CR */, 1) == -1)
        goto osfail;
    }
    length -= count;
    
    buffer = ptr;
  } while ((ptr = memchr((char *)buffer + 1, 0x0A, length)) != NULL);
  
  if (ufb->ufbflg & UFB_SOCK) {
    if ((count = send(fd, (char *)buffer, length, 0)) < 0)
      return -1;
  }
  else {
    if (Write((BPTR)ufb->ufbfh, (void *)buffer, length) == -1)
      goto osfail;
  }

  return totcount;

osfail:
  errno = __io2errno(_OSERR = IoErr());
  return -1;
} 