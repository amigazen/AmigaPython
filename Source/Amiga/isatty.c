/*
 *      isatty.c - check is a file is a terminal (interactive) or not
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
#include <dos/dos.h>
#include <proto/dos.h>

/*
 * Check if a file descriptor refers to a terminal (interactive device)
 */
int
isatty(int fd)
{
  struct UFB *ufb;

  /*
   * find the ufb *
   */
  if ((ufb = __chkufb(fd)) != NULL &&
      !(ufb->ufbflg & UFB_SOCK)) { /* A socket is not a tty */
    /*
     * Convert DOSBOOL to BOOL
     */
    return (IsInteractive(ufb->ufbfh) & 0x1);
  }
  
  return 0;
} 