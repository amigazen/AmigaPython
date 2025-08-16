/*
 *      netlib.h - common Network Support Library definitions
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 */

#ifndef _NETLIB_H
#define _NETLIB_H

/*
 * This is supposed to be compiler-independent error setting interface
 */
#ifdef __SASC
extern int _OSERR;
extern int errno;
extern int __io2errno(int);
#define __set_errno(x) do { errno = __io2errno(_OSERR = (x)); } while (0)
#define SET_OSERR(code) do { _OSERR = (code); } while (0)
#else
void __set_errno(UBYTE code);
#define SET_OSERR(code) do { } while (0)
#endif

#define set_errno __set_errno
#define OSERR _OSERR

/*
 * Usergroup UID/GID converters for MuFS  
 */
#define UG2MU(id) ((id == (unsigned short)-1) ? (unsigned short)0 : (unsigned short)(id))
#define MU2UG(id) ((id == 0) ? (unsigned short)-1 : (unsigned short)(id))

#endif /* _NETLIB_H */ 