/* -*- C -*- ***********************************************
Copyright (c) 2000, BeOpen.com.
Copyright (c) 1995-2000, Corporation for National Research Initiatives.
Copyright (c) 1990-1995, Stichting Mathematisch Centrum.
All rights reserved.

See the file "Misc/COPYRIGHT" for information on usage and
redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
******************************************************************/

/* Module configuration */

/* This file contains the table of built-in modules.
   See init_builtin() in import.c. */

/**** Amiga Python 2.7.18 Configuration *****/
/* Based on Python 2.0 Amiga port by Irmen de Jong */
/* Updated for Python 2.7.18 */

#include "Python.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -- ADDMODULE MARKER 1 -- */

/* Core Python modules */
extern void PyMarshal_Init(void);
extern void initimp(void);
extern void initgc(void);
extern void init_ast(void);
extern void _PyWarnings_Init(void);

/* Standard modules */
extern void initarray(void);
extern void initmath(void);
extern void inittime(void);
extern void initbinascii(void);
extern void init_codecs(void);
extern void initcStringIO(void);
extern void initcPickle(void);
extern void initerrno(void);
/* extern void initposix(void); */
extern void initoperator(void);
extern void init_weakref(void);
extern void init_struct(void);
extern void initselect(void);
extern void initzlib(void);
extern void init_sre(void);
extern void init_md5(void);
extern void initcmath(void);
extern void init_sha(void);
/* extern void initunicodedata(void); */
extern void initenvironment(void);
extern void initstrop(void);

/* extern void initARexx(void); */
extern void initDoslib(void);

/* Amiga-specific modules */
extern void initamiga(void);
extern void initamigapath(void);
extern void init_socket(void);

#if defined(AMITCP) /* || defined(INET225) (/)
/* Network modules - only if networking is available */
extern void initpwd(void);
extern void initgrp(void);
extern void initcrypt(void);
extern void initsyslog(void);
#endif

struct _inittab _PyImport_Inittab[] = {

/* -- ADDMODULE MARKER 2 -- */

    /* Amiga-specific modules */
    {"amiga", initamiga},
    {"amigapath", initamigapath},

    /* Standard modules */
    {"array", initarray},
    {"math", initmath},
    {"time", inittime},
    {"binascii", initbinascii},
    {"_codecs", init_codecs},
    {"cStringIO", initcStringIO},
    {"cPickle", initcPickle},
    {"errno", initerrno},
    /* {"posix", initposix}, */
    {"operator", initoperator},
    {"_weakref", init_weakref},
    {"_struct", init_struct},
    {"select", initselect},
    {"zlib", initzlib},
    {"_sre", init_sre},
    {"md5", init_md5},
    {"cmath", initcmath},
    {"sha", init_sha},
    /* {"unicodedata", initunicodedata}, */
    {"environment", initenvironment},
    {"strop", initstrop},
    /* {"ARexx", initARexx}, */
    {"Doslib", initDoslib},

#if defined(AMITCP) /* || defined(INET225) */
    /* Network modules */
    {"_socket", init_socket},
    {"pwd", initpwd},
    {"grp", initgrp},
    {"crypt", initcrypt},
    {"syslog", initsyslog},
#endif

    /* This module lives in marshal.c */
    {"marshal", PyMarshal_Init},

    /* This lives in import.c */
    {"imp", initimp},

    /* This lives in Python/Python-ast.c */
    {"_ast", init_ast},

    /* These entries are here for sys.builtin_module_names */
    {"__main__", NULL},
    {"__builtin__", NULL},
    {"sys", NULL},
    {"exceptions", NULL},

    /* This lives in gcmodule.c */
    {"gc", initgc},

    /* This lives in _warnings.c */
    {"_warnings", _PyWarnings_Init},

    /* Sentinel */
    {0, 0}
};

#ifdef __cplusplus
}
#endif 