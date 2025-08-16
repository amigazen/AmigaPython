/*
 *      dostat.c - *stat() function common part for Amiga
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 */

#include <proto/dos.h>
#include <proto/utility.h>
#include <libraries/usergroup.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <string.h>
#include <stdlib.h>

/* DOS 3.0 and MuFS extensions to file info block */
#include "fibex.h"

/*
 * Conversion table from Amiga filetypes to Unix filetypes
 */
const static mode_t ftype[ST_LINKDIR - ST_PIPEFILE + 1] = {
  S_IFIFO,
  S_IFREG,
  S_IFREG,
  S_IFREG,
  S_IFREG,
  S_IFREG,
  S_IFDIR,
  S_IFDIR,
  S_IFLNK,
  S_IFDIR,
};

/*
 * Conversion table from Amiga protections to Unix protections
 * rwed -> rwx
 */
const static UBYTE fbits[16] =
{
  00, 02, 01, 03, 02, 02, 03, 03,
  04, 06, 05, 07, 06, 06, 07, 07,
};

/*
 * MU2UG (MuFS to UserGroup) conversion macro
 * Converts from MuFS UID/GID to UserGroup library UID/GID format
 */
#define MU2UG(id) ((id == 0) ? (unsigned short)-1 : (unsigned short)(id))

/*
 * Fill in a stat structure from file information
 */
void __dostat(struct FileInfoBlock *fib, struct stat *st)
{
  ULONG pbits = fib->fib_Protection ^ 0xf;
  short fibtype = fib->fib_DirEntryType - ST_PIPEFILE;
  mode_t mode;

  if (fibtype < 0)
    fibtype = 0;
  else if (fibtype > ST_LINKDIR - ST_PIPEFILE)
    fibtype = ST_LINKDIR - ST_PIPEFILE;

  bzero(st, sizeof(*st));

  mode = ftype[fibtype] | (fbits[pbits & 0xf] << 6)
    | (fbits[(pbits >> FIBB_GRP_DELETE) & 0xf] << 3)
      | fbits[(pbits >> FIBB_OTR_DELETE) & 0xf];

  if ((pbits & FIBF_PURE) != 0)
    mode |= S_ISVTX;
  if ((pbits & FIBF_SUID) != 0)
    mode |= S_ISUID;
  if ((pbits & FIBF_SGID) != 0)
    mode |= S_ISGID;

  st->st_ino = fib->fib_DiskKey;
  st->st_mode = mode;
  st->st_nlink = 1;
  st->st_uid = MU2UG(fib->fib_OwnerUID);
  st->st_gid = MU2UG(fib->fib_OwnerGID);
  st->st_rdev = 0;
  st->st_size = fib->fib_Size;

  /* 
   * Calculatory time since Jan 1 1970, UCT 
   * (in reality there are an odd number of leap seconds, 
   * which are not included)
   */
  st->st_atime = st->st_ctime = st->st_mtime =
    60 * ((fib->fib_Date.ds_Days + (8*365+2)) * 24 * 60
        + fib->fib_Date.ds_Minute)
      + fib->fib_Date.ds_Tick / 50;

  st->st_blksize = 512;
  st->st_blocks = fib->fib_NumBlocks;
  st->st_dosmode = fib->fib_Protection;
  st->st_type = fib->fib_DirEntryType;
  st->st_comment = fib->fib_Comment;
}

/* Static FIB for use with various stat functions */
struct FileInfoBlock __dostat_fib[1]; 