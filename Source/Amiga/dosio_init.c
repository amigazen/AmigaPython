/*
 *      dosio_init.c - SAS C auto initialization functions for DOSIO
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 */

#include <exec/execbase.h>
extern struct ExecBase *SysBase;

#include <dos/dos.h>
#include <dos/dosextens.h>
#include <proto/dos.h>
#include <constructor.h>

/*
 * File table used by the stdio look-a-like macros
 */
BPTR __dosio_files[3];

/*
 * Initialize the file table used by the stdio look-a-like macros.
 * The stdio macros provided are suitable for stdin, stdout and
 * stderr usage.
 */
STDIO_CONSTRUCTOR(dosio_init)
{
  struct Process *p = (struct Process *)SysBase->ThisTask;

  __dosio_files[0] = p->pr_CIS;	/* stdin */
  __dosio_files[1] = p->pr_COS;	/* stdout */
  __dosio_files[2] = p->pr_CES;	/* stderr */

  if (__dosio_files[2] == 0)
    __dosio_files[2] = __dosio_files[1];

  return 0;
} 