/*
 *      _lseek.c - reposition read/write file offset
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
#include <errno.h>
#include <dos/dos.h>
#include <proto/dos.h>

#include "Python.h"
#include "protos.h"

#ifdef HAVE_LARGEFILE_SUPPORT
off_t __lseek(int fd, off_t rpos, int mode)
#else
long __lseek(int fd, long rpos, int mode)
#endif
{
  struct UFB *ufb;
  long        apos;
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
  
  _OSERR = 0;

  /* Socket cannot seek */
  if (ufb->ufbflg & UFB_SOCK) {
    errno = ESPIPE; /* illegal seek */
    return -1;
  }

  if ((apos = Seek((BPTR)ufb->ufbfh, (LONG)rpos, mode - 1)) == -1) {
    _OSERR = IoErr();
    errno = EIO;
    return -1;
  }
  
  switch (mode) {
  case SEEK_SET: /* 0 */
    return rpos;
  case SEEK_CUR: /* 1 */
    return apos + rpos;
  case SEEK_END: /* 2 */
    return Seek((BPTR)ufb->ufbfh, 0, 0);
  default:
    errno = EINVAL;
    return -1;
  }
} 