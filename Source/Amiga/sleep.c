/*
 *      sleep.c - suspend execution for an interval
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 *
 *      DEPRECATED: This file is deprecated in Amiga Python 2.7.18 in favour of vbcc PosixLib
 */

#include <sys/param.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>

/*
 * Suspend process execution for the specified time in seconds
 */
void sleep(unsigned int secs)
{
  struct timeval tv;

  tv.tv_sec = secs;
  tv.tv_usec = 0;
  select(0, 0, 0, 0, &tv);
} 