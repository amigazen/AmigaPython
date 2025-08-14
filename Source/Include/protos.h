#ifndef _PROTOS_H
#define _PROTOS_H

#include <stdio.h>

/* from Modules/main.c: */
int Py_Main(int argc, char **argv);
void Py_GetArgcArgv(int *argc, char ***argv);


#ifdef AMITCP
extern int checkusergrouplib(void); /* in main.c */
extern int checksocketlib(void); /* in main.c */
#endif

#ifdef INET225
extern int checksocketlib(void); /* in main.c */
extern int checkusergrouplib(void); /* in main.c */
#endif

extern double hypot (double x, double y);


#ifdef _AMIGA
/*********** UNIX 'emulation' functions ************/
/***** (implemented in Amiga/.../unixemul.c ********/
int link(const char *to, const char *from);
int symlink(const char *to, const char *from);
int readlink(const char *path, char *buf, int bufsiz);
int my_mkdir(const char* path, int p);	/* working mkdir(2) */

#define _UNAME_BUFLEN 32
struct utsname {
		char    sysname[_UNAME_BUFLEN];
		char    nodename[_UNAME_BUFLEN];
		char    release[_UNAME_BUFLEN];
		char    version[_UNAME_BUFLEN];
		char    machine[_UNAME_BUFLEN];
};
int uname(struct utsname *u);

FILE *popen(const char *command, const char *type);
int pclose(FILE *stream);

/** implemented in Amiga/.../environment.c: **/
char *getenv(const char *var);
int setenv(const char *name, const char *value, int overwrite);
int putenv(const char *string);
void unsetenv(const char *name);


#endif

#ifndef AMITCP
int gettimeofday(struct timeval *, struct __timezone *);
int utime(const char *name, const struct utimbuf *times);

extern int opterr;
extern int optind;
extern int optopt;
extern char *optarg;
int getopt(int argc, char * const argv[], char const *opts);

#endif /* !AMITCP */

/**** all initfuncs for the modules ****/
extern void PyMarshal_Init (void);
extern void initimp (void);
extern void initamiga (void);
extern void initARexx (void);
extern void initamiga_exec (void);
extern void initarray (void);
extern void initbinascii (void);
extern void initcmath (void);
extern void initcrypt (void);
extern void initDoslib (void);
extern void initExeclib (void);
extern void initenvironment (void);
extern void initerrno (void);
extern void initgetpath (void);
extern void initgrp (void);
extern void initmath (void);
extern void initmd5 (void);
extern void initnew (void);
extern void initoperator (void);
extern void initpwd (void);
extern void initregex (void);
extern void initrotor (void);
extern void initselect (void);
extern void initsha (void);
extern void init_socket (void);
extern void initsoundex (void);
extern void initstrop (void);
extern void initstruct (void);
extern void initsyslog (void);
extern void inittime (void);
extern void inittiming (void);
extern void initurlop (void);
extern void initavl (void);
extern void initsimplegfx (void);
extern void initcStringIO (void);
extern void initcPickle (void);
extern void initpcre (void);
extern void init_codecs (void);
extern void init_sre (void);
extern void initzlib (void);
extern void initunicodedata (void);
extern void initpyexpat (void);

int PyOS_CheckStack (void);

#endif
