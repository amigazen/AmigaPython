#ifndef Py_PYCONFIG_H
#define Py_PYCONFIG_H

/* pyconfig_amiga.h - Python configuration for Amiga platforms */
/* Based on the older Amiga port by Irmen de Jong */
/* Updated for Python 2.7.18 */
/* Enhanced with vbcc PosixLib integration for enhanced POSIX compatibility */

/* Define if you want to enable thread support */
/* For now, Amiga lacks proper threading capabilities */
#undef WITH_THREAD

/* Define if you want to build an interpreter with many run-time checks  */
#undef Py_DEBUG

/* Define to force use of thread-safe errno, h_errno, and other functions */
#undef _REENTRANT

/* Define if setpgrp() must be called as setpgrp(0, 0). */
#undef SETPGRP_HAVE_ARG

/* Define to empty if the keyword does not work. */
#undef const

/* Define to `int' if <sys/types.h> doesn't define. */
#undef gid_t

/* Define if your struct tm has tm_zone. */
#undef HAVE_TM_ZONE 

/* Define if you don't have tm_zone but do have the external array
   tzname. */
#define HAVE_TZNAME 1

/* Define if on MINIX. */
#undef _MINIX

/* Define to `int' if <sys/types.h> doesn't define. */
#undef mode_t

/* Define to `long' if <sys/types.h> doesn't define. */
#undef off_t

/* Define to `int' if <sys/types.h> doesn't define. */
#undef pid_t

/* Define if the system does not provide POSIX.1 features except
   with this defined. */
#undef _POSIX_1_SOURCE

/* Define if you need to in order for stat and other things to work. */
#undef _POSIX_SOURCE

/* Define as the return type of signal handlers (int or void). */
#define RETSIGTYPE void

/* Define to `unsigned' if <sys/types.h> doesn't define. */
#undef size_t

/* Define if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>. */
#undef TIME_WITH_SYS_TIME

/* Define if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define if you have ssize_t type */
#define HAVE_SSIZE_T 1

/* Define if your <sys/time.h> declares struct tm. */
#undef TM_IN_SYS_TIME

/* Define to `int' if <sys/types.h> doesn't define. */
#undef uid_t

/* Define if your processor stores words with the most significant
   byte first (like Motorola and SPARC, unlike Intel and VAX). */
#define WORDS_BIGENDIAN 1

/* Define if your compiler botches static forward declarations */
#undef BAD_STATIC_FORWARD

/* Define to `long' if <time.h> doesn't define. */
#undef clock_t

/* Define if you have the Mach cthreads package */
#undef C_THREADS

/* Define to 1 if you have the <conio.h> header file. */
#undef HAVE_CONIO_H

/* Define if gettimeofday() does not have second (timezone) argument
   This is the case on Motorola V4 (R40V4.2) */
#undef GETTIMEOFDAY_NO_TZ

/* Define if your struct stat has st_blksize. */
#undef HAVE_ST_BLKSIZE

/* Define if your compiler supports function prototypes */
#define HAVE_PROTOTYPES 1

/* Define if your compiler supports variable length function prototypes */
#define HAVE_STDARG_PROTOTYPES 1

/* Define to 1 if you have the <sys/file.h> header file. */
#define HAVE_SYS_FILE_H 1

/* Define to 1 if you have the <sys/param.h> header file. */
#define HAVE_SYS_PARAM_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Defined if MALLOC_ZERO_RETURNS_NULL */
#define MALLOC_ZERO_RETURNS_NULL 1

/* Define if you have POSIX threads */
#undef _POSIX_THREADS

/* Define if  you can safely include both <sys/select.h> and <sys/time.h> */
#define SYS_SELECT_WITH_SYS_TIME 1

/* Define if you have the <sys/select.h> header file. */
#define HAVE_SYS_SELECT_H 1

/* Define if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define if you have the <time.h> header file. */
#define HAVE_TIME_H 1

/* Define if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* Define if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define if you have the <signal.h> header file. */
#define HAVE_SIGNAL_H 1

/* Define if you have the <math.h> header file. */
#define HAVE_MATH_H 1

/* Define if you have the <setjmp.h> header file. */
#define HAVE_SETJMP_H 1

/* Define if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define if you have the <float.h> header file. */
#define HAVE_FLOAT_H 1

/* Define if you have the <stdarg.h> header file. */
#define HAVE_STDARG_H 1

/* Define if you have the <stddef.h> header file. */
#define HAVE_STDDEF_H 1

/* Define if you have the <ctype.h> header file. */
#define HAVE_CTYPE_H 1

/* Define if you have the <assert.h> header file. */
#define HAVE_ASSERT_H 1

/* Define if you have the <locale.h> header file. */
#define HAVE_LOCALE_H 1

/* Define if you have the <stdio.h> header file. */
#define HAVE_STDIO_H 1

/* Additional headers provided by vbcc PosixLib */
#define HAVE_DIRENT_H 1
#define HAVE_TERMIOS_H 1
#define HAVE_SYS_SOCKET_H 1
#define HAVE_NETINET_IN_H 1
#define HAVE_ARPA_INET_H 1
#define HAVE_NETDB_H 1

/* Define if you have the getcwd function. */
#define HAVE_GETCWD 1

/* Define if you have the opendir function. */
#define HAVE_OPENDIR 1

/* Define if you have the readdir function. */
#define HAVE_READDIR 1

/* Define if you have the closedir function. */
#define HAVE_CLOSEDIR 1

/* Define if you have the strerror function. */
#define HAVE_STRERROR 1

/* Define if you have the strftime function. */
#define HAVE_STRFTIME 1

/* Define if you have the strtol function. */
#define HAVE_STRTOL 1

/* Define if you have the strtoul function. */
#define HAVE_STRTOUL 1

/* Define if you have the strtof function. */
#define HAVE_STRTOF 1

/* Define if you have the strtod function. */
#define HAVE_STRTOD 1

/* Define if you have the isnan function. */
#define HAVE_ISNAN 1

/* Define if you have the isinf function. */
#define HAVE_ISINF 1

/* Define if you have the finite function. */
#define HAVE_FINITE 1

/* Define if you have the copysign function. */
#define HAVE_COPYSIGN 1

/* Define if you have the round function. */
#define HAVE_ROUND 1

/* Define if you have the hypot function. */
#define HAVE_HYPOT 1

/* Define if you have the atanh function. */
#define HAVE_ATANH 1

/* Define if you have the asinh function. */
#define HAVE_ASINH 1

/* Define if you have the acosh function. */
#define HAVE_ACOSH 1

/* Additional functions provided by vbcc PosixLib */
/* File operations */
#define HAVE_DUP2 1
#define HAVE_FTRUNCATE 1
#define HAVE_FSYNC 1
#define HAVE_FSEEKO 1
#define HAVE_FTELLO 1
#define HAVE_FLOCK 1
#define HAVE_READLINK 1
#define HAVE_REALPATH 1
#define HAVE_LSTAT 1
#define HAVE_PREAD 1
#define HAVE_PWRITE 1
#define HAVE_ACCESS 1
#define HAVE_ISATTY 1
#define HAVE_CLOSERANGE 1
#define HAVE_GETDTABLESIZE 1

/* Process management */
#define HAVE_NICE 1
#define HAVE_KILL 1
/* #define HAVE_WAITPID 1 */ /* Amiga doesn't have proper process management */
#define HAVE_SLEEP 1
#define HAVE_USLEEP 1

/* System configuration */
#define HAVE_PATHCONF 1
#define HAVE_FPATHCONF 1
#define HAVE_SYSCONF 1

/* Time functions */
#define HAVE_GETTIMEOFDAY 1
#define HAVE_SETTIMEOFDAY 1

/* Utility functions */
#define HAVE_TEMPNAM 1
#define HAVE_URANDOM 1

/* Process and user functions */
#define HAVE_GETPPID 1
#define HAVE_KILLPG 1
#define HAVE_GETLOGIN 1
#define HAVE_GETLOGIN_R 1
#define HAVE_GETPWNAM 1
#define HAVE_GETPWUID 1
#define HAVE_GETPWUID_R 1
#define HAVE_GETGRNAM 1
#define HAVE_GETGRGID 1

/* Time functions */
#define HAVE_GMTIME_R 1
#define HAVE_LOCALTIME_R 1

/* String and utility functions */
#define HAVE_STRDUP 1
#define HAVE_STRNCPY 1
#define HAVE_STRLCAT 1
#define HAVE_STRLCPY 1
#define HAVE_STRSEP 1
#define HAVE_STRNLEN 1
#define HAVE_STRCOLL 1
#define HAVE_ASPRINTF 1
#define HAVE_VASPRINTF 1

/* Environment functions */
#define HAVE_SETENV 1
#define HAVE_UNSETENV 1

/* Directory functions */
#define HAVE_SCANDIR 1
#define HAVE_ALPHASORT 1

/* Pattern matching */
#define HAVE_GLOB 1
#define HAVE_FNMATCH 1

/* Terminal functions */
#define HAVE_TERMCAP 1
#define HAVE_TGETENT 1
#define HAVE_TGETSTR 1
#define HAVE_TGETFLAG 1
#define HAVE_TGETNUM 1
#define HAVE_TGOTO 1
#define HAVE_TPUTS 1

/* Network functions (via bsdsocket.library) */
#define HAVE_GETHOSTBYNAME 1
#define HAVE_GETHOSTBYADDR 1
#define HAVE_GETHOSTNAME 1
#define HAVE_GETSERVBYNAME 1
#define HAVE_GETSERVBYPORT 1
#define HAVE_INET_ADDR 1
#define HAVE_INET_NTOA 1

/* Define if your compiler supports the register keyword */
#define HAVE_REGISTER 1

/* Define if your compiler supports __attribute__ */
#define HAVE_ATTRIBUTE 1

/* Define if your compiler supports __attribute__((format)) */
#define HAVE_ATTRIBUTE_FORMAT 1

/* Define if your compiler supports __attribute__((unused)) */
#define HAVE_ATTRIBUTE_UNUSED 1

/* Define if your compiler supports __attribute__((noreturn)) */
#define HAVE_ATTRIBUTE_NORETURN 1

/* Define if you have the va_list type */
#define HAVE_VA_LIST 1

/* Define if you have the va_start macro */
#define HAVE_VA_START 1

/* Define if you have the va_end macro */
#define HAVE_VA_END 1

/* Define if you have the va_arg macro */
#define HAVE_VA_ARG 1

/* Define if you have the va_copy macro */
#define HAVE_VA_COPY 1

/* Define if you have the __va_copy macro */
#define HAVE___VA_COPY 1

/* Define if you have the __builtin_va_copy macro */
#define HAVE___BUILTIN_VA_COPY 1

/* Define if you have the __builtin_va_list type */
#define HAVE___BUILTIN_VA_LIST 1

/* Define if you have the __builtin_va_start macro */
#define HAVE___BUILTIN_VA_START 1

/* Define if you have the __builtin_va_end macro */
#define HAVE___BUILTIN_VA_END 1

/* Define if you have the __builtin_va_arg macro */
#define HAVE___BUILTIN_VA_ARG 1

/* Define if you have the __builtin_va_copy macro */
#define HAVE___BUILTIN_VA_COPY 1

/* Define if you have the __builtin_va_list type */
#define HAVE___BUILTIN_VA_LIST 1

/* Define if you have the __builtin_va_start macro */
#define HAVE___BUILTIN_VA_START 1

/* Define if you have the __builtin_va_end macro */
#define HAVE___BUILTIN_VA_END 1

/* Define if you have the __builtin_va_arg macro */
#define HAVE___BUILTIN_VA_ARG 1

/* Define if you want to use SGI (IRIX 4) dynamic linking */
#undef WITH_SGI_DL

/* Define if you want documentation strings in extension modules */
#define WITH_DOC_STRINGS 1

/* Define if you want to compile in rudimentary thread support */
#undef WITH_THREAD

/* The number of bytes in an off_t. */
#define SIZEOF_OFF_T 4

/* The number of bytes in a time_t. */
#define SIZEOF_TIME_T 4

/* The number of bytes in a va_list. */
#define SIZEOF_VA_LIST 4

/* The number of bytes in a void *. */
#define SIZEOF_VOID_P 4

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
#define WORDS_BIGENDIAN 1

/* Define for large files, on AIX-style hosts. */
#undef _LARGE_FILES

/* Define if your compiler has __bool__ type. */
#undef HAVE_BOOL

/* Define to 1 if the system has the type `long double'. */
#undef HAVE_LONG_DOUBLE

/* Define to 1 if the system has the type `long long'. */
#define HAVE_LONG_LONG 1

/* Define to 1 if the system has the type `signed char'. */
#define HAVE_SIGNED_CHAR 1

/* The size of a `char', as computed by sizeof. */
#define SIZEOF_CHAR 1

/* The size of a `double', as computed by sizeof. */
#define SIZEOF_DOUBLE 8

/* The size of a `float', as computed by sizeof. */
#define SIZEOF_FLOAT 4

/* The size of a `fpos_t', as computed by sizeof. */
#define SIZEOF_FPOS_T 4

/* The size of a `int', as computed by sizeof. */
#define SIZEOF_INT 4

/* The size of a `long', as computed by sizeof. */
#define SIZEOF_LONG 4

/* The size of a `long long', as computed by sizeof. */
#define SIZEOF_LONG_LONG 8

/* The size of a `short', as computed by sizeof. */
#define SIZEOF_SHORT 2

/* The size of a `wchar_t', as computed by sizeof. */
#define SIZEOF_WCHAR_T 4

/* The size of a `size_t', as computed by sizeof. */
#define SIZEOF_SIZE_T 4

/* The size of a `pid_t', as computed by sizeof. */
#define SIZEOF_PID_T 4

/* The size of a `uid_t', as computed by sizeof. */
#define SIZEOF_UID_T 4

/* The size of a `gid_t', as computed by sizeof. */
#define SIZEOF_GID_T 4

/* The size of a `mode_t', as computed by sizeof. */
#define SIZEOF_MODE_T 4

/* Amiga-specific options */
#define PREFIX "Python:"
#define EXEC_PREFIX "Python:"

/* Define to 1 if you have the function dlopen. */
#undef HAVE_DLOPEN

/* Defined if MALLOC_ZERO_RETURNS_NULL */
#define MALLOC_ZERO_RETURNS_NULL 1

/* Define if you have the 'select' function. */
#define HAVE_SELECT 1

/* Define if malloc(0) returns a NULL pointer */
#define MALLOC_ZERO_RETURNS_NULL 1

/* PY_FORMAT_SIZE_T is a platform-specific modifier for use in a printf
   format string for size_t values.  It is defined by configure. */
#define PY_FORMAT_SIZE_T "lu"

/* Define if you have <dirent.h> */
#define HAVE_DIRENT_H 1

/* Define if Python was built with Amiga support */
#define PLATFORM_AMIGA 1
#define _AMIGA 1

/* Define if you have bsdsocket.library networking */
#define AMITCP 1

/* Define if you have the socket module */
#define HAVE_SOCKET 1

/* Define printf format for long long on AmigaOS */
#define PY_FORMAT_LONG_LONG "ll"

/* Disable Unicode support for VBCC compatibility */
#undef Py_USING_UNICODE
#undef HAVE_USABLE_WCHAR_T
#undef HAVE_WCHAR_H
#undef WANT_WCTYPE_FUNCTIONS

#endif /* !Py_PYCONFIG_H */ 