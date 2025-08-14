/*
 *      perror.c - print error message
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>

/*
 * Print an error message to stderr corresponding to the current errno value
 * If banner is not NULL, it is printed before the error message, separated by a colon
 */
void 
perror(const char *banner)
{
  const char *err = strerror(errno);

  if (banner != NULL) {
    fputs(banner, stderr);
    fputs(": ", stderr);
  }
  fputs(err, stderr);
  fputc('\n', stderr);
  fflush(stderr);
} 