/*
 *      _fstat.c - get file status information
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 *
 *      DEPRECATED: This file is deprecated in Amiga Python 2.7.18 in favour of vbcc PosixLib
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>

#include <string.h>
#include <stdlib.h>

/* DOS 3.0 and MuFS extensions to file info block */
#include "fibex.h"
#include <proto/dos.h>
#include <proto/utility.h>

#include <ios1.h>
#include "libcheck.h"

int fstat(int fd, struct stat *st)
{
  struct UFB *ufb = __chkufb(fd);

  if (st == NULL || ((1 & (long)st) == 1)) {
    errno = EFAULT;
    return -1;
  }

  if (ufb == NULL || ufb->ufbflg == 0) {
    errno = EBADF;
    return -1;
  }

  if (ufb->ufbflg & UFB_SOCK) { /* a socket */
    long value;
    long size = sizeof(value);

    /* Needs both usergroup.library and bsdsocket.library */
    if(!checkusergrouplib() || !checksocketlib()) return -1;

    bzero(st, sizeof(*st));

    /* st->st_dev = ??? */
    st->st_mode = S_IFSOCK | S_IRUSR | S_IWUSR;
    st->st_uid = geteuid();
    st->st_gid = getegid();

    if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &value, &size) == 0)
      st->st_blksize = value;

    return 0;
  } else { /* ordinal file */
    /* test if it is a NIL: file handle (ExamineFH() crashes on those!) */
    if (((struct FileHandle *)BADDR((BPTR)ufb->ufbfh))->fh_Type == NULL) {
      /* It is NIL:, make up something... */
      bzero(st, sizeof(*st));
      st->st_mode = S_IFIFO | S_IRWXU | S_IRWXG | S_IRWXO;
      return 0;
    }
    else {
      if (ExamineFH(ufb->ufbfh, __dostat_fib)) {
        __dostat(__dostat_fib, st);
        st->st_dev = (dev_t)((struct FileHandle *)BADDR(ufb->ufbfh))->fh_Type;
        return 0;
      } else {
        errno = EIO;
        return -1;
      }
    }
  }
} 