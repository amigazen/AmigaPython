#ifndef Py_PROTOS_H
#define Py_PROTOS_H

#include <stdio.h>

/* Amiga-specific function prototypes for the Python port */

#ifdef __cplusplus
extern "C" {
#endif

/* Timer functions */
int init_timer(void);
void done_timer(void);

/* POSIX emulation - these are now provided by PosixLib */
/* int gettimeofday(struct timeval *tp, void *tzp); */
/* int usleep(unsigned long microseconds); */
/* int sleep(unsigned int seconds); */
/* int link(const char *to, const char *from); */
/* int symlink(const char *to, const char *from); */
/* int readlink(const char *path, char *buf, int bufsiz); */

/* Amiga-specific functions */
int my_mkdir(const char* path, int p);

/* File I/O */
int pclose(FILE *stream);

/* Process - these are now provided by PosixLib */
/* typedef long pid_t; */
/* pid_t getpid(void); */

/* Error conversion */
int __io2errno(int ioerr);

/* Networking */
#ifdef AMITCP
int checkusergrouplib(void);
#endif

/* AmiTCP-specific functions - these are now provided by PosixLib */
#ifdef AMITCP
/* struct utsname is now provided by PosixLib */
/* #define _SYS_NMLN 65 */
/* int uname(struct utsname *u); */
/* int gethostname(char *name, int namelen); */
#endif

#ifdef __cplusplus
}
#endif

#endif /* !Py_PROTOS_H */ 