/*
 * Expat configuration for python. This file is not part of the expat
 * distribution.
 */
#ifndef EXPAT_CONFIG_H
#define EXPAT_CONFIG_H

#include <pyconfig.h>

/*
 * pyconfig.h defines PREFIX/EXEC_PREFIX for the Python install path.
 * Expat uses PREFIX as a typedef name (xmlparse.c) — drop the clash.
 */
#ifdef PREFIX
#undef PREFIX
#endif
#ifdef EXEC_PREFIX
#undef EXEC_PREFIX
#endif

#ifdef WORDS_BIGENDIAN
#define BYTEORDER 4321
#else
#define BYTEORDER 1234
#endif

#define XML_NS 1
#define XML_DTD 1
#define XML_CONTEXT_BYTES 1024

/* Amiga/VBCC: no getrandom; allow weak entropy for hash salt. */
#ifdef _AMIGA
#define XML_POOR_ENTROPY 1
/* xmltok.c includes <stdbool.h>; provide a minimal fallback. */
#ifndef __bool_true_false_are_defined
typedef int _Bool;
#define bool _Bool
#define true 1
#define false 0
#define __bool_true_false_are_defined 1
#endif
/*
 * SipHash needs uint64_t / unsigned long long. Compile Modules/expat
 * with -c99 (see EXPAT_CFLAGS) so PosixLib <stdint.h> provides them.
 */
#endif

#endif /* EXPAT_CONFIG_H */
