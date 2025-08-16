/* 
 *      printuserfault.c - Print a usergroup error message for Amiga
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 */

#include <errno.h>

#include <exec/execbase.h>
extern struct ExecBase *SysBase;

#include <dos/dos.h>
#include <dos/dosextens.h>
#include <proto/dos.h>
#include <proto/usergroup.h>

/*
 * Print a detailed error message to the standard error output
 * If banner is not NULL, it is printed before the error message, separated by a colon
 */
void PrintUserFault(LONG code, const UBYTE *banner)
{
  struct Process *p = (struct Process *)SysBase->ThisTask;
  BPTR Stderr = p->pr_CES ? p->pr_CES : p->pr_COS;

  if (banner != NULL) {
    FPuts(Stderr, (STRPTR)banner);
    FPuts(Stderr, ": ");
  }
  
  /* Use usergroup library's ug_StrError to get detailed error messages */
  FPuts(Stderr, (STRPTR)ug_StrError(code));
  FPuts(Stderr, "\n");
  Flush(Stderr);
} 