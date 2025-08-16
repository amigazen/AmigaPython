/*
 *      chown.c - chown() for Amiga
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 *
 *      DEPRECATED: This file is deprecated in Amiga Python 2.7.18 in favour of vbcc PosixLib
 */

#include <libraries/usergroup.h>
#include <proto/dos.h>
#include <dos/dosextens.h>
#include <errno.h>

#ifndef ACTION_SET_OWNER
#define ACTION_SET_OWNER 1036
#endif

/*
 * UG2MU (UserGroup to MuFS) conversion macro
 * Converts from UserGroup library UID/GID to MuFS UID/GID format
 */
#define UG2MU(id) ((id == (unsigned short)-1) ? (unsigned short)0 : (unsigned short)(id))

/*
 * Change owner and group of a file.
 * One of the owner or group id's may be left unchanged by specifying it as -1.
 */
int chown(const char *name, uid_t uid, gid_t gid)
{
  BPTR lock;
  short rc = 0;

  if ((lock = Lock((STRPTR)name, ACCESS_READ))) {
    if (uid == -1 || gid == -1) {
      /* XXX We are supposed to do stat() and find out the suitable value */
      
    }
    rc = DoPkt(((struct FileLock *)BADDR(lock))->fl_Task,
               ACTION_SET_OWNER, 
               0, lock, MKBADDR(""), 
               (UG2MU(uid) << 16) | UG2MU(gid), 0);
    UnLock(lock);
  }

  if (!rc) {
    errno = __io2errno(IoErr());
    return -1;
  } else {
    return 0;
  }
} 