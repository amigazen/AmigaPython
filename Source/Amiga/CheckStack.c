/*
 *      CheckStack.c - Check available stack space for Amiga
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18 / VBCC
 */

#include "Python.h"

/* AmigaOS version and stack requirements */
static const char *version = "$VER: Amiga Python 2.7.18b2 (16/07/2025)";
static const char *stack_req = "$STACK: 40000";

/*
 * Return 0 if enough stack is available, -1 if not.
 *
 * SAS/C exposes stackavail()/__STKNEED.  VBCC does not — stack safety
 * there comes from the compiler's -stack-check flag, so we report OK.
 */
int
PyOS_CheckStack(void)
{
#ifdef __SASC
    {
        extern unsigned long stackavail(void);
        extern long __STKNEED;

        if (stackavail() < __STKNEED)
            return -1;
    }
#endif
    return 0;
}
