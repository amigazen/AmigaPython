#ifndef _LIBCHECK_H
#define _LIBCHECK_H

/* Library availability checks for Amiga */

extern int checkusergrouplib(void);
extern int checkutilitylib(void);
extern int checksocketlib(void);
extern void cleanup_libraries(void);

/* 1 if dos.library says Input() is an interactive console. */
extern int Py_Amiga_StdinInteractive(void);

#endif /* _LIBCHECK_H */
