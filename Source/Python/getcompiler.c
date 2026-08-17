
/* Return the compiler identification, if possible. */

#include "Python.h"

#ifndef COMPILER

#ifdef __GNUC__
#define COMPILER "\n[GCC " __VERSION__ "]"
#endif

#endif /* !COMPILER */

#ifndef COMPILER

/* Amiga shell splits -DCOMPILER="[VBCC standard]" on the space and globs [].
 * Stamp the build here; vmakefile.python passes -DPYAMIGA_SLIM for SlimPython. */
#ifdef __VBCC__
#ifdef PYAMIGA_SLIM
#define COMPILER "[VBCC-SlimPython]"
#else
#define COMPILER "[VBCC-standard]"
#endif
#endif

#endif /* !COMPILER */

#ifndef COMPILER

#ifdef __cplusplus
#define COMPILER "[C++]"
#else
#define COMPILER "[C]"
#endif

#endif /* !COMPILER */

const char *
Py_GetCompiler(void)
{
    return COMPILER;
}
