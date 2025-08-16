#ifndef _LIBCHECK_H
#define _LIBCHECK_H

/* Library availability checks for Amiga */

#ifdef AMITCP
extern int checkusergrouplib(void);
extern int checkutilitylib(void);
extern int checksocketlib(void);
#endif

extern void PyErr_Clear(void);

#endif /* _LIBCHECK_H */ 