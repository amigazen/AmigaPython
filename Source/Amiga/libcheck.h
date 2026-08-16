#ifndef _LIBCHECK_H
#define _LIBCHECK_H

/* Library availability checks for Amiga */

/* Open on demand; return 1 if available. No Python exception. */
extern int have_usergrouplib(void);
extern int have_socketlib(void);

/* Same as have_*, but set SystemError on failure. */
extern int checkusergrouplib(void);
extern int checkutilitylib(void);
extern int checksocketlib(void);

extern void cleanup_libraries(void);

/* 1 if dos.library says Input() is an interactive console. */
extern int Py_Amiga_StdinInteractive(void);

#endif /* _LIBCHECK_H */
