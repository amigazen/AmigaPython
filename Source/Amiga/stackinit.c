/*
 *      stackinit.c - Stack settings for Amiga SAS/C
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 */

/* Amiga SAS/C stack settings */
__near long __stack = 40000;   /* Default stack size */
long __STKNEED = 1000;         /* Minimum free stack needed */ 