/*
 *      _allocufb.c - get a free ufb (Unix File Block) for Amiga
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 */

#include <ios1.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

/*
 * Allocate new ufb, which is returned as return value. The corresponding fd
 * is returned via fdp.
 *
 * Parameters:
 *   fdp - Pointer to store the file descriptor
 *
 * Returns:
 *   Pointer to the allocated UFB structure or NULL on error
 */
struct UFB *
__allocufb(int *fdp)
{
  struct UFB *ufb, *last_ufb;
  int         last_fd = 0;
  
  /* Check input parameters */
  if (fdp == NULL) {
    errno = EINVAL;
    return NULL;
  }

  /*
   * find first free ufb
   */
  last_ufb = ufb = __ufbs;
  while (ufb != NULL && ufb->ufbflg != 0) {
    last_ufb = ufb;
    
    /* Check for file descriptor overflow */
    if (last_fd == INT_MAX) {
      errno = EMFILE;  /* Too many open files */
      return NULL;
    }
    
    last_fd++;
    ufb = last_ufb->ufbnxt;
  }
  
  /*
   * Check if need to create one
   */
  if (ufb == NULL) {
    if ((ufb = calloc(1, sizeof(*ufb))) == NULL) {
      errno = ENOMEM;
      return NULL;
    }
    
    /* Initialize all fields of the new UFB structure */
    ufb->ufbnxt = NULL;
    ufb->ufbflg = 0;    /* => unused ufb */
    ufb->ufbfh = 0;     /* Initialize file handle */
    ufb->ufbfn = NULL;  /* Initialize file name */
    ufb->ufbpos = 0;    /* Initialize file position */
    ufb->ufbsiz = 0;    /* Initialize buffer size */
    ufb->ufbgrow = 0;   /* Initialize grow size */
    
    /* Link it into the UFB chain */
    if (last_ufb == NULL)
      __ufbs = ufb;
    else
      last_ufb->ufbnxt = ufb;
    
    *fdp = __nufbs++;
  }
  else
    *fdp = last_fd;
  
  return ufb;
} 