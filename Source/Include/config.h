#ifndef Py_CONFIG_H
#define Py_CONFIG_H

/* config.h. Edited by hand for Amiga SAS/C by Irmen de Jong */
/* (adapted from: config.h.in) */

/* 27-mar-96: updated for AmiTCP (Irmen de Jong) */
/*  8-dec-96: now for Python 1.4 */
/* 11-jan-98: now for Python 1.5... for now, assumes AMITCP is used */
/*            Looks much more like original config.h.in now */
/* 24-dec-98: added some more #defines */
/* 25-dec-98: added support for I-Net 225 (at the bottom) */
/* 24-apr-99: added some defines for Python 1.5.2 */
/* 16-okt-99: added 'network-free' options */
/* 9-sep-00:  Python 1.6 */


/*************************** ADDED STUFF FOR AMIGA PYTHON ****************/

/*  BUILTIN LIBRARY SEARCH PATH */
/***#define PYTHONPATH "Python:Lib"***/

/************************* END ADDED STUFF FOR AMIGA PYTHON **************/

/* Must be defined if using UNSIGNED CHARACTERS: */
/* (_UNSCHAR is SAS/C's defined symbol if using unsigned chars) */

#ifdef _UNSCHAR
#define __CHAR_UNSIGNED__
#endif



#if defined(AMITCP) || defined(INET225)

/* Define to empty if the keyword does not work.  */
#undef const

/* Define to `int' if <sys/types.h> doesn't define.  */
#undef gid_t

/* Define if your struct tm has tm_zone.  */
#undef HAVE_TM_ZONE 

/* Define if you don't have tm_zone but do have the external array
   tzname.  */
#define HAVE_TZNAME 1

/* Define if on MINIX.  */
#undef _MINIX

/* Define to `int' if <sys/types.h> doesn't define.  */
#undef mode_t

/* Define to `long' if <sys/types.h> doesn't define.  */
#undef off_t

/* Define to `int' if <sys/types.h> doesn't define.  */
#undef pid_t

/* Define if the system does not provide POSIX.1 features except
   with this defined.  */
#undef _POSIX_1_SOURCE

/* Define if you need to in order for stat and other things to work.  */
#undef _POSIX_SOURCE

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
#undef size_t

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Define if your <sys/time.h> declares struct tm.  */
#undef TM_IN_SYS_TIME

/* Define to `int' if <sys/types.h> doesn't define.  */
#undef uid_t

/* Define if your processor stores words with the most significant
   byte first (like Motorola and SPARC, unlike Intel and VAX).  */
#define WORDS_BIGENDIAN 1

/* Define if your <unistd.h> contains bad prototypes for exec*()
   (as it does on SGI IRIX 4.x) */
#undef BAD_EXEC_PROTOTYPES

/* Define if your compiler botches static forward declarations
   (as it does on SCI ODT 3.0) */
#undef BAD_STATIC_FORWARD

/* Define if you have the Mach cthreads package */
#undef C_THREADS

/* Define to `long' if <time.h> doesn't define.  */
#undef clock_t

/* Define if gettimeofday() does not have second (timezone) argument
   This is the case on Motorola V4 (R40V4.2) */
#undef GETTIMEOFDAY_NO_TZ

/* Define this if your time.h defines altzone */
#undef HAVE_ALTZONE

/* Define this if you have a K&R style C preprocessor */
#undef HAVE_OLD_CPP

/* Define if your compiler supports function prototypes */
#define HAVE_PROTOTYPES 1

/* Define if your compiler supports variable length function prototypes
   (e.g. void fprintf(FILE *, char *, ...);) *and* <stdarg.h> */
#define HAVE_STDARG_PROTOTYPES 1

/* Define if malloc(0) returns a NULL pointer */
#define MALLOC_ZERO_RETURNS_NULL 1

/* Imports case sensitive? */
#define CHECK_IMPORT_CASE 1

/* Define if you have POSIX threads */
#undef _POSIX_THREADS

/* Define to force use of thread-safe errno, h_errno, and other functions */
#undef _REENTRANT

/* Define if setpgrp() must be called as setpgrp(0, 0). */
#undef SETPGRP_HAVE_ARG

/* Define to empty if the keyword does not work.  */
#undef signed

/* Define if  you can safely include both <sys/select.h> and <sys/time.h>
   (which you can't on SCO ODT 3.0). */
#define SYS_SELECT_WITH_SYS_TIME 1

/* Define if a va_list is an array of some kind */
#undef VA_LIST_IS_ARRAY

/* Define to empty if the keyword does not work.  */
#undef volatile

/* Define if you want SIGFPE handled (see Include/pyfpe.h). */
#undef WANT_SIGFPE_HANDLER

/* Define if you want to use SGI (IRIX 4) dynamic linking.
   This requires the "dl" library by Jack Jansen,
   ftp://ftp.cwi.nl/pub/dynload/dl-1.6.tar.Z.
   Don't bother on IRIX 5, it already has dynamic linking using SunOS
   style shared libraries */ 
#undef WITH_SGI_DL

/* Define if you want to emulate SGI (IRIX 4) dynamic linking.
   This is rumoured to work on VAX (Ultrix), Sun3 (SunOS 3.4),
   Sequent Symmetry (Dynix), and Atari ST.
   This requires the "dl-dld" library,
   ftp://ftp.cwi.nl/pub/dynload/dl-dld-1.1.tar.Z,
   as well as the "GNU dld" library,
   ftp://ftp.cwi.nl/pub/dynload/dld-3.2.3.tar.Z.
   Don't bother on SunOS 4 or 5, they already have dynamic linking using
   shared libraries */ 
#undef WITH_DL_DLD

/* Define if you want to compile in rudimentary thread support */
#undef WITH_THREAD

/* Define this if you have BeOS threads */
#undef BEOS_THREADS

/* The number of bytes in a int.  */
#define SIZEOF_INT 4

/* The number of bytes in a long.  */
#define SIZEOF_LONG 4

/* The number of bytes in an off_t. */
#define SIZEOF_OFF_T 4

/* The number of bytes in a double.  */
#define SIZEOF_DOUBLE 8

/* The number of bytes in a float.  */
#define SIZEOF_FLOAT 4

/* The number of bytes in a long long.  */
#undef SIZEOF_LONG_LONG

/* The number of bytes in a short.  */
#define SIZEOF_SHORT 2

/* The number of bytes in a void *.  */
#define SIZEOF_VOID_P 4

/* Define if you have the alarm function.  */
#undef HAVE_ALARM

/* Define if you have the clock function.  */
#define HAVE_CLOCK 1

/* Define if you have the dlopen function.  */
#undef HAVE_DLOPEN

/* Define if you have the dup2 function.  */
#undef HAVE_DUP2

/* Define if you have the execv function.  */
#undef HAVE_EXECV

/* Define if you have the flock function.  */
#undef HAVE_FLOCK

/* Define if you have the fork function.  */
#undef HAVE_FORK

/* Define if you have the ftime function.  */
#undef HAVE_FTIME

/* Define if you have the ftruncate function.  */
#undef HAVE_FTRUNCATE

/* Define if you have the getcwd function.  */
#define HAVE_GETCWD 1

/* Define if you have the gethostname_r function.  */
#undef HAVE_GETHOSTNAME_R

/* Define if you have the getpeername function.  */
#define HAVE_GETPEERNAME 1

/* Define if you have the getpid function.  */
#define HAVE_GETPID 1

/* Define if you have the getpwent function.  */
#define HAVE_GETPWENT 1

/* Define if you have the gettimeofday function.  */
#define HAVE_GETTIMEOFDAY 1

/* Define if you have the getwd function.  */
#undef HAVE_GETWD

/* Define if you have the hypot function.  */
#undef HAVE_HYPOT

/* Define if you have the kill function.  */
#undef HAVE_KILL

/* Define if you have the link function.  */
#define HAVE_LINK 1

/* Define if you have the lstat function.  */
#define HAVE_LSTAT 1

/* Define if you have the memmove function.  */
#define HAVE_MEMMOVE 1

/* Define if you have the mkfifo function.  */
#undef HAVE_MKFIFO

/* Define if you have the mktime function.  */
#define HAVE_MKTIME 1

/* Define if you have the nice function.  */
#undef HAVE_NICE

/* Define if you have the pause function.  */
#undef HAVE_PAUSE

/* Define if you have the plock function.  */
#undef HAVE_PLOCK

/* Define if you have the putenv function.  */
#define HAVE_PUTENV 1

/* Define if you have the readlink function.  */
#define HAVE_READLINK 1

/* Define if you have the select function.  */
#define HAVE_SELECT 1

/* Define if you have the setgid function.  */
#define HAVE_SETGID 1

/* Define if you have the setlocale function.  */
#define HAVE_SETLOCALE 1

/* Define if you have the setpgid function.  */
#undef HAVE_SETPGID

/* Define if you have the setpgrp function.  */
#undef HAVE_SETPGRP

/* Define if you have the setuid function.  */
#define HAVE_SETUID 1

/* Define if you have the setvbuf function.  */
#define HAVE_SETVBUF 1

/* Define if you have the sigaction function.  */
#undef HAVE_SIGACTION

/* Define if you have the siginterrupt function.  */
#undef HAVE_SIGINTERRUPT

/* Define if you have the sigrelse function.  */
#undef HAVE_SIGRELSE

/* Define if you have the strdup function.  */
#define HAVE_STRDUP 1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR 1

/* Define if you have the strftime function.  */
#define HAVE_STRFTIME 1

/* Define if you have the strptime function.  */
#undef HAVE_STRPTIME

/* Define if you have the symlink function.  */
#define HAVE_SYMLINK 1

/* Define if you have the tcgetpgrp function.  */
#undef HAVE_TCGETPGRP

/* Define if you have the tcsetpgrp function.  */
#undef HAVE_TCSETPGRP

/* Define if you have the times function.  */
#undef HAVE_TIMES

/* Define if you have the truncate function.  */
#undef HAVE_TRUNCATE

/* Define if you have the uname function.  */
#define HAVE_UNAME 1

/* Define if you have the waitpid function.  */
#undef HAVE_WAITPID

/* Define if you have the fseek64 function.  */
#undef HAVE_FSEEK64

/* Define if you have the fseeko function.  */
#undef HAVE_FSEEKO

/* Define if you have the fstatvfs function.  */
#undef HAVE_FSTATVFS

/* Define if you have the fsync function.  */
#undef HAVE_FSYNC

/* Define if you have the ftell64 function.  */
#undef HAVE_FTELL64

/* Define if you have the ftello function.  */
#undef HAVE_FTELLO

/* Define if you have the statvfs function.  */
#undef HAVE_STATVFS

/* Define if you have the timegm function.  */
#undef HAVE_TIMEGM

/* Define this if you have some version of gethostbyname_r() */
#undef HAVE_GETHOSTBYNAME_R

/* Define this if you have the 3-arg version of gethostbyname_r() */
#undef HAVE_GETHOSTBYNAME_R_3_ARG

/* Define this if you have the 5-arg version of gethostbyname_r() */
#undef HAVE_GETHOSTBYNAME_R_5_ARG

/* Define this if you have the 6-arg version of gethostbyname_r() */
#undef HAVE_GETHOSTBYNAME_R_6_ARG

/* Define this if you have the type long long */
#undef HAVE_LONG_LONG

/* execv function? */
#undef HAVE_EXECV

/* system function? */
#define HAVE_SYSTEM 1

/* wait function? */
#undef HAVE_WAIT

/* kill function? */
#undef HAVE_KILL

/* pipe function? */
#define HAVE_PIPE 1

/* popen function? */
#define HAVE_POPEN 1

/* fork function? */
#undef HAVE_FORK

/* getegid function? */
#define HAVE_GETEGID 1

/* geteuid function? */
#define HAVE_GETEUID 1

/* getgid function? */
#define HAVE_GETGID 1

/* getppid function? */
#undef HAVE_GETPPID

/* getuid function? */
#define HAVE_GETUID 1

/* opendir function? */
#define HAVE_OPENDIR 1


/* Define if you have the <dirent.h> header file.  */
#undef HAVE_DIRENT_H

/* Define if you have the <dlfcn.h> header file.  */
#undef HAVE_DLFCN_H

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <limits.h> header file.  */
#define HAVE_LIMITS_H 1

/* Define if you have the <locale.h> header file.  */
#define HAVE_LOCALE_H 1

/* Define if you have the <ncurses.h> header file.  */
#undef HAVE_NCURSES_H

/* Define if you have the <ndir.h> header file.  */
#undef HAVE_NDIR_H

/* Define if you have the <pthread.h> header file.  */
#undef HAVE_PTHREAD_H

/* Define if you have the <signal.h> header file.  */
#define HAVE_SIGNAL_H 1

/* Define if you have the <stdarg.h> header file.  */
#define HAVE_STDARG_H 1

/* Define if you have the <stddef.h> header file.  */
#define HAVE_STDDEF_H 1

/* Define if you have the <stdlib.h> header file.  */
#define HAVE_STDLIB_H 1

/* Define if you have the <sys/audioio.h> header file.  */
#undef HAVE_SYS_AUDIOIO_H

/* Define if you have the <sys/dir.h> header file.  */
#define HAVE_SYS_DIR_H 1

/* Define if you have the <sys/file.h> header file.  */
#define HAVE_SYS_FILE_H 1

/* Define if you have the <sys/lock.h> header file.  */
#undef HAVE_SYS_LOCK_H

/* Define if you have the <sys/ndir.h> header file.  */
#undef HAVE_SYS_NDIR_H

/* Define if you have the <sys/param.h> header file.  */
#define HAVE_SYS_PARAM_H 1

/* Define if you have the <sys/select.h> header file.  */
#undef HAVE_SYS_SELECT_H

/* Define if you have the <sys/time.h> header file.  */
#define HAVE_SYS_TIME_H 1

/* Define if you have the <sys/times.h> header file.  */
#undef HAVE_SYS_TIMES_H

/* Define if you have the <sys/un.h> header file.  */
#undef HAVE_SYS_UN_H

/* Define if you have the <sys/utsname.h> header file.  */
#undef HAVE_SYS_UTSNAME_H

/* Define if you have the <thread.h> header file.  */
#undef HAVE_THREAD_H

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* Define if you have the <utime.h> header file.  */
#define HAVE_UTIME_H 1

/* Define if you have the dl library (-ldl).  */
#undef HAVE_LIBDL

/* Define if you have the dld library (-ldld).  */
#undef HAVE_LIBDLD

/* Define if you have the ieee library (-lieee).  */
#undef HAVE_LIBIEEE

/* Define if you have the sun library (-lsun).  */
#undef HAVE_LIBSUN


#endif /* BOTH AMITCP OR INET */


/************************* FROM HERE:   AMITCP stuff ********************/

#ifdef AMITCP

/* Define if you have the chown function.  */
#define HAVE_CHOWN 1

/* Define if getpgrp() must be called as getpgrp(0). */
/* #define GETPGRP_HAVE_ARG 1 */

/* Define if you have the getpgrp function.  */
#define HAVE_GETPGRP 1

/* Define if you have the setsid function.  */
#define HAVE_SETSID 1

#define HAVE_SYS_WAIT_H 1


#endif /* AmiTCP only */

/************************* FROM HERE:   INET225 stuff ********************/

#ifdef INET225

/* Define if you have the <sys/wait.h> header file.  */
#undef HAVE_SYS_WAIT_H

/* Actually we do have unistd.h but Inet chokes too often on it */
#undef HAVE_UNISTD_H

#endif /* INET225 only */


/************************* FROM HERE:   No-network stuff ********************/

#if !defined(AMITCP) && !defined(INET225)

#define HAVE_TZNAME 1
#define RETSIGTYPE void
#define STDC_HEADERS 1
#define HAVE_PROTOTYPES 1
#define HAVE_STDARG_PROTOTYPES 1
#define MALLOC_ZERO_RETURNS_NULL 1
#define SIZEOF_INT 4 
#define SIZEOF_LONG 4
#define SIZEOF_OFF_T 4
#undef SIZEOF_LONG_LONG
#define SIZEOF_VOID_P 4
#define HAVE_CLOCK 1
#define HAVE_GETCWD 1
#define HAVE_GETPID 1
#define HAVE_LINK 1
#define HAVE_LSTAT 1
#define HAVE_MEMMOVE 1
#define HAVE_MKTIME 1
#define HAVE_PUTENV 1
#define HAVE_READLINK 1
#define HAVE_SETLOCALE 1
#define HAVE_SETVBUF 1
#define HAVE_STRDUP 1
#define HAVE_STRERROR 1
#define HAVE_STRFTIME 1
#define HAVE_SYMLINK 1
#define HAVE_UNAME 1
#define HAVE_SYSTEM 1
#define HAVE_PIPE 1
#define HAVE_POPEN 1
#define HAVE_OPENDIR 1
#define HAVE_FCNTL_H 1
#define HAVE_LIMITS_H 1
#define HAVE_LOCALE_H 1
#define HAVE_SIGNAL_H 1
#define HAVE_STDARG_H 1
#define HAVE_STDDEF_H 1
#define HAVE_STDLIB_H 1
#define HAVE_SYS_DIR_H 1
#define HAVE_SYS_FILE_H 1
#define HAVE_UNISTD_H 1

#endif /* No networking */

#endif
