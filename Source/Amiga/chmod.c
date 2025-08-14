/*
 *      chmod.c - chmod() for Amiga
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 *
 *      DEPRECATED: This file is deprecated in Amiga Python 2.7.18 in favour of vbcc PosixLib
 */

#include <libraries/usergroup.h>
#include <proto/dos.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "fibex.h"

/*
 * rwx -> rwed conversion table
 * - rwx are what Unix uses
 * - rwed are what AmigaOS uses
 */
const static BYTE pbits[8] = 
{ 
  0, 0X2, 0X5, 0X7, 0X8, 0XA, 0XD, 0XF, 
};

/*
 * Change mode of a file.
 * This call is provided for Unix compatibility. It does not know all
 * Amiga protection bits (Delete, Archive, Script). The archive and
 * script bits are cleared, Delete set according to the Write bit.
 */
int chmod(const char *path, int mode)
{
  LONG prot = 
    (pbits[mode & 7] << FIBB_OTR_DELETE) |
      (pbits[(mode >> 3) & 7] << FIBB_GRP_DELETE) |
	(pbits[(mode >> 6) & 7] ^ 0xf);

  if (mode & S_ISVTX)
    prot |= FIBF_PURE;
  if (mode & S_ISUID)
    prot |= FIBF_SUID;
  if (mode & S_ISGID)
    prot |= FIBF_SGID;

  if (!SetProtection((STRPTR)path, prot)) {
    errno = __io2errno(IoErr());
    return -1;
  } else {
    return 0;
  }
} 