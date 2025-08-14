#ifndef _PROTOS_H
#define _PROTOS_H

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

extern double hypot Py_PROTO((double x, double y));


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
extern void PyMarshal_Init Py_PROTO((void));
extern void initimp Py_PROTO((void));
extern void initamiga Py_PROTO((void));
extern void initARexx Py_PROTO((void));
extern void initamiga_exec Py_PROTO((void));
extern void initarray Py_PROTO((void));
extern void initbinascii Py_PROTO((void));
extern void initcmath Py_PROTO((void));
extern void initcrypt Py_PROTO((void));
extern void initDoslib Py_PROTO((void));
extern void initexeclib Py_PROTO((void));
extern void initenvironment Py_PROTO((void));
extern void initerrno Py_PROTO((void));
extern void initgetpath Py_PROTO((void));
extern void initgrp Py_PROTO((void));
extern void initmath Py_PROTO((void));
extern void initmd5 Py_PROTO((void));
extern void initnew Py_PROTO((void));
extern void initoperator Py_PROTO((void));
extern void initpwd Py_PROTO((void));
extern void initregex Py_PROTO((void));
extern void initrotor Py_PROTO((void));
extern void initselect Py_PROTO((void));
extern void initsha Py_PROTO((void));
extern void initsocket Py_PROTO((void));
extern void initsoundex Py_PROTO((void));
extern void initstrop Py_PROTO((void));
extern void initstruct Py_PROTO((void));
extern void initsyslog Py_PROTO((void));
extern void inittime Py_PROTO((void));
extern void inittiming Py_PROTO((void));
extern void initurlop Py_PROTO((void));
extern void initavl Py_PROTO((void));
extern void initsimplegfx Py_PROTO((void));
extern void initcStringIO Py_PROTO((void));
extern void initcPickle Py_PROTO((void));
extern void initpcre Py_PROTO((void));
extern void init_codecs Py_PROTO((void));
extern void init_sre Py_PROTO((void));
extern void initzlib Py_PROTO((void));
extern void initunicodedata Py_PROTO((void));

int PyOS_CheckStack Py_PROTO((void));

#endif
