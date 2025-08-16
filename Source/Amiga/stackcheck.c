/*
 *      stackcheck.c - Check available stack space for Amiga
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 */

#include "Python.h"

/*
 * Check if a Python operation would cause a stack overflow
 * based on the Amiga SAS/C stack settings.
 * Returns 0 if there's enough stack space, -1 if not.
 */
int PyOS_CheckStack(void)
{
#ifdef __SASC
    /* Amiga SAS/C: Explicit check of available stack */
    extern unsigned long stackavail(void);
    extern long __STKNEED;
    
    if(stackavail() < __STKNEED) 
        return -1;
#endif /* __SASC */
    return 0;
} 