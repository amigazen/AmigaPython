/*
 *      usleep.c - suspend execution for microsecond intervals
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
#include <errno.h>
#include <dos.h>
#include <proto/dos.h>

/*
 * Suspend process execution for the specified time in microseconds
 * Uses select() with a timeout for longer delays and Delay() for short periods
 * 
 * Parameters:
 *   usecs - The number of microseconds to sleep
 */
void usleep(unsigned int usecs)
{
  /* For very small delays (less than 20000 microseconds), use Delay directly */
  if (usecs < 20000) {
    /* Convert to ticks (1/50th of a second) - minimum 1 tick */
    ULONG ticks = (usecs / 20000) + 1;
    Delay(ticks);
    return;
  }
  
  struct timeval tv;
  int ret;

  /* Setup the timeval structure */
  tv.tv_sec = 0;
  while (usecs >= 1000000) {
    usecs -= 1000000;
    tv.tv_sec++;
  }	
  tv.tv_usec = usecs;
  
  /* Using select with NULL file descriptor sets just for delay */
  ret = select(0, NULL, NULL, NULL, &tv);
  
  /* Check for errors, but don't do anything - we can't really restore lost time */
  if (ret < 0) {
    /* select() was interrupted or failed - just continue */
    /* We could handle error cases like EINTR by doing another select, 
       but for simplicity we just continue */
  }
} 