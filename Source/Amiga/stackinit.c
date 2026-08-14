/*
 *      stackinit.c - Default process stack size for Amiga
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 *
 * SAS/C (and some POSIX startups) read __stack as the default stack.
 * Keep it in sync with the $STACK cookie in CheckStack.c (262144).
 */

#ifdef __SASC
__near long __stack = 262144;  /* Default stack size */
long __STKNEED = 4096;         /* Minimum free stack needed */
#else
/* VBCC / PosixLib: AmigaOS uses the $STACK cookie in CheckStack.c.
 * Still provide __stack for startups that honor it. */
long __stack = 262144;
#endif
