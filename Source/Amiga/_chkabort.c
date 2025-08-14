/*
 *      _chkabort.c - check for break signal on Amiga
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 */

#include <errno.h>

extern int interrupted;

/*
 * Check if user pressed CTRL-C
 * Sets the error condition to EINTR if interrupted
 */
void __regargs __chkabort(void)
{
  /* Check if user pressed CTRL-C */
  if (interrupted) {
    interrupted = 0;
    errno = EINTR;
  }
} 