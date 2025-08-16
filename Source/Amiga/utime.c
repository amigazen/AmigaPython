/*
 *      utime.c - set the modification date of the file
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 *
 *      DEPRECATED: This file is deprecated in Amiga Python 2.7.18 in favour of vbcc PosixLib
 */

#include <sys/param.h>
#include <sys/time.h>
#include <dos/dos.h>
#include <proto/dos.h>
#include <utime.h>
#include <errno.h>

#include <syslog.h>

#include "netlib.h"

/*
 * See timerinit.c for comments on this
 */
extern long __local_to_GMT; /* defined and initialized in timerinit.c */

/*
 * Set file access and modification times
 * 
 * Since AmigaDOS files have only one timestamp, both access and
 * modification times cannot be supported. Since the AmigaDOS file date
 * is the modification time, only the 'modtime' field of the 'times' is used.
 */
int
utime(const char *name, const struct utimbuf *times)
{
  struct DateStamp stamp;
  unsigned long days, secs;
  time_t time;

  if (times == NULL)
    DateStamp(&stamp);
  else {
    /*
     * AmigaDOS file date is the modification time
     */
    time = times->modtime;

    /*
     * Convert time (secs since 1.1.1970 GMT) to
     * AmigaDOS DateStamp (based on 1.1.1978 local time).
     */
    time -= __local_to_GMT; /* GMT to local */
    days = (unsigned long)time / (unsigned long)(24*60*60);
    secs = (unsigned long)time % (unsigned long)(24*60*60);
    stamp.ds_Days = (LONG)days;
    stamp.ds_Minute = (LONG)(secs / 60);
    stamp.ds_Tick = (LONG)((secs % 60) * TICKS_PER_SECOND);
  }

  if (!SetFileDate((STRPTR)name, &stamp)) {
    errno = __io2errno(IoErr());
    return -1;
  }

  return 0;
} 