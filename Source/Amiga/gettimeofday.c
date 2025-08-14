/*
 *      gettimeofday.c - get time of the day
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 *
 *      DEPRECATED: This file is deprecated in Amiga Python 2.7.18 in favour of vbcc PosixLib
 */

#include <sys/param.h>
#include <time.h>
#include <sys/time.h>

#undef PROTO_TIMER_H
#include <proto/timer.h>

/*
 * See timerinit.c for comments on these
 */
extern struct timezone __time_zone;
extern long __local_to_GMT;

/*
 * Get the current Greenwich time and timezone
 */
int 
gettimeofday(struct timeval *tp, struct timezone *tzp)
{
  if (tp) {
    GetSysTime(tp);
    tp->tv_sec += __local_to_GMT;
  }
  if (tzp) {
    /*
     * __time_zone is set up in the timerinit.c
     */
    *tzp = __time_zone;
  }

  return 0;
} 