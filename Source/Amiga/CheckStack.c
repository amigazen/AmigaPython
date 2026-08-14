/*
 *      CheckStack.c - Check available stack space for Amiga
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18 / VBCC
 */

#include "Python.h"

/* AmigaOS version string (visible to version command). */
static const char version[] = "$VER: Amiga Python 2.7.18b2 (16/07/2025)";

/*
 * AmigaOS stack cookie. CLI/Workbench read this from the load file and
 * give the process at least this many bytes of stack.
 *
 * 40000 was enough for AmigaPython 2.0-era scripts but Python 2.7 imports
 * (collections, re, etc.) blow that C stack and abort with no traceback.
 * 256KiB matches common "stack 262144" advice for ported interpreters.
 *
 * Keep as a char array (not pointer) and touch it below so the linker
 * cannot strip the cookie from the binary.
 */
static const char stack_req[] = "$STACK:262144";

/*
 * Return 0 if enough stack is available, -1 if not.
 *
 * SAS/C exposes stackavail()/__STKNEED.  VBCC does not.  Do not rely on
 * VBCC -stack-check for the interpreter: it aborts with no Python
 * traceback when the CLI stack is too small.  Use this $STACK cookie
 * (and Stack 262144 in the shell if the cookie is ignored).
 */
int
PyOS_CheckStack(void)
{
    /* Prevent dead-strip of version / stack cookies. */
    if (version[0] == '\0' || stack_req[0] == '\0')
        return -1;

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
