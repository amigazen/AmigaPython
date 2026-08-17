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

/*
 * Two link builds. vmakefile compiles this file to config.o (standard)
 * and config_slim.o (-DPYAMIGA_SLIM). BUILD=compact|core remain aliases
 * for slim; BUILD=full remains an alias for standard.
 *
 * SlimPython -- Amiga scripting + stdlib boot (name after Irmen de Jong's
 *   SlimPython 1.5.2). Always includes the Amiga OS modules (amiga,
 *   GUI/libs, ARexx, environment), zipimport/_io/_sre, and the C
 *   accelerators Lib imports on a typical script (collections, itertools,
 *   pickle, select for the _socket plugin). XML, datetime, hashing,
 *   csv/bisect/heapq C accels, cmath, and pwd/grp/crypt/syslog stay out
 *   so the binary is smaller. Those modules cannot be LoadSeg plugins
 *   (PyTypeObject / Expat callbacks).
 *
 * standard -- SlimPython plus the extras above. Default BUILD=standard is
 *   the complete static C module set (former all-in-one interpreter).
 */

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

/* Modules in both builds */
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
extern void initzipimport(void);
extern void init_io(void);
extern void init_sre(void);
extern void init_collections(void);
extern void inititertools(void);
extern void init_functools(void);
extern void init_random(void);
extern void init_symtable(void);
extern void initenvironment(void);
extern void initstrop(void);

#ifndef PYAMIGA_SLIM
extern void init_md5(void);
extern void initcmath(void);
extern void init_sha(void);
extern void init_hashlib(void);
extern void initdatetime(void);
extern void init_bisect(void);
extern void init_heapq(void);
extern void init_csv(void);
extern void initpyexpat(void);
#endif
/* extern void initunicodedata(void); */

/* Amiga-specific modules (both builds) */
extern void initamiga(void);
extern void initamigagui(void);
extern void initamigalibs(void);
extern void init_arexx(void);
/* _socket / _ssl are LoadSeg plugins (lib-dynload/*.module), not builtin */
#if !defined(PYAMIGA_SLIM) && defined(AMITCP) /* || defined(INET225) */
extern void initpwd(void);
extern void initgrp(void);
extern void initcrypt(void);
extern void initsyslog(void);
#endif

struct _inittab _PyImport_Inittab[] = {

/* -- ADDMODULE MARKER 2 -- */

    /* Amiga-specific modules */
    {"amiga", initamiga},
    {"amigagui", initamigagui},
    {"amigalibs", initamigalibs},

    /* Modules in both builds */
    {"array", initarray},
    {"math", initmath},
    {"time", inittime},
    {"binascii", initbinascii},
    {"_codecs", init_codecs},
    {"cStringIO", initcStringIO},
    {"cPickle", initcPickle},
    {"errno", initerrno},
    /* {"posix", initposix}, */  /* not built; amiga is the OS builtin (+ sys.modules alias) */
    {"operator", initoperator},
    {"_weakref", init_weakref},
    {"_struct", init_struct},
    {"select", initselect},
    {"zlib", initzlib},
    /* Builtin (same as Unix Setup); needs zlib for deflated zip members. */
    {"zipimport", initzipimport},
    /* PEP 3116 io; text is 8-bit str when Py_USING_UNICODE is off. */
    {"_io", init_io},
    {"_sre", init_sre},
    {"_collections", init_collections},
    {"itertools", inititertools},
    {"_functools", init_functools},
    {"_random", init_random},
    /* Always on Unix Setup; small compiler tooling helper. */
    {"_symtable", init_symtable},
    {"environment", initenvironment},
    {"strop", initstrop},
    /* Low-level ARexx accelerator; use Lib/site-python/arexx.py for the public API. */
    {"_arexx", init_arexx},

#ifndef PYAMIGA_SLIM
    {"md5", init_md5},
    {"cmath", initcmath},
    {"sha", init_sha},
    {"_hashlib", init_hashlib},
    /* C-only in 2.7; needed by calendar/email/etc. */
    {"datetime", initdatetime},
    {"_bisect", init_bisect},
    {"_heapq", init_heapq},
    {"_csv", init_csv},
    /* XML: statically linked Modules/expat (see vmakefile.modules). */
    {"pyexpat", initpyexpat},
#endif
    /* {"unicodedata", initunicodedata}, */

#if !defined(PYAMIGA_SLIM) && defined(AMITCP) /* || defined(INET225) */
    /* Network / usergroup extras (not _socket — that is LoadSeg'd) */
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
