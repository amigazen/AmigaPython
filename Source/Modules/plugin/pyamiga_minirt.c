/*
 * Minimal runtime symbols for LoadSeg plugins linked -nostdlib.
 * +aos68k_posix still pulls -lmieee (soft float for socket timeouts);
 * that needs SysBase/exit. VBCC may emit __aprintf for sprintf-like calls.
 *
 * AmiSSL/OpenSSL 3 uses uint64_t SSL_OP_* flags; VBCC emits jsr
 * __rshuint64 / __lshint64. Those are assembler names. A C function
 * named __rshuint64 is exported as ___rshuint64 and does not match.
 * C names _rshuint64 / _lshint64 get the Amiga '_' prefix and match.
 * Implement with 32-bit words — do not shift a long long or the
 * compiler will recurse into these same symbols.
 */
#include <stdarg.h>
#include <exec/types.h>
#include <exec/execbase.h>

#include "pyamiga_plugin.h"

typedef union {
    unsigned long long ull;
    long long sll;
    unsigned long w[2]; /* 68k big-endian: w[0]=high, w[1]=low */
} pyamiga_u64;

/* Set from PyAmiga_InstallHost before any soft-float / init_socket work. */
struct ExecBase *SysBase = NULL;

void
abort(void)
{
    for (;;)
        ;
}

void
exit(int code)
{
    (void)code;
    for (;;)
        ;
}

/* VBCC may rewrite sprintf-family calls to __aprintf (symbol ___aprintf). */
int
__aprintf(char *s, const char *fmt, ...)
{
    int r;
    va_list v;

    va_start(v, fmt);
    r = PyAmiga_Host->fn_vsprintf(s, fmt, v);
    va_end(v);
    return r;
}

/*
 * VBCC libcalls jsr __rshuint64 (two underscores). A C name __rshuint64
 * is exported as ___rshuint64 and does not match. C name _rshuint64
 * plus the Amiga '_' prefix is the symbol the compiler emits.
 */
unsigned long long
_rshuint64(unsigned long long x, int n)
{
    pyamiga_u64 u;
    unsigned long hi;
    unsigned long lo;

    if (n <= 0)
        return x;
    if (n >= 64)
        return 0;
    u.ull = x;
    hi = u.w[0];
    lo = u.w[1];
    if (n >= 32) {
        lo = hi >> (unsigned)(n - 32);
        hi = 0;
    } else {
        lo = (lo >> (unsigned)n) | (hi << (unsigned)(32 - n));
        hi = hi >> (unsigned)n;
    }
    u.w[0] = hi;
    u.w[1] = lo;
    return u.ull;
}

long long
_rshsint64(long long x, int n)
{
    pyamiga_u64 u;
    unsigned long hi;
    unsigned long lo;
    unsigned long fill;
    unsigned s;

    if (n <= 0)
        return x;
    u.sll = x;
    hi = u.w[0];
    lo = u.w[1];
    fill = (hi & 0x80000000UL) ? 0xFFFFFFFFUL : 0UL;
    if (n >= 64) {
        u.w[0] = fill;
        u.w[1] = fill;
        return u.sll;
    }
    if (n >= 32) {
        s = (unsigned)(n - 32);
        if (s == 0)
            lo = hi;
        else
            lo = (hi >> s) | (fill << (unsigned)(32 - s));
        hi = fill;
    } else {
        lo = (lo >> (unsigned)n) | (hi << (unsigned)(32 - n));
        hi = (hi >> (unsigned)n) | (fill << (unsigned)(32 - n));
    }
    u.w[0] = hi;
    u.w[1] = lo;
    return u.sll;
}

long long
_lshint64(long long x, int n)
{
    pyamiga_u64 u;
    unsigned long hi;
    unsigned long lo;

    if (n <= 0)
        return x;
    if (n >= 64)
        return 0;
    u.sll = x;
    hi = u.w[0];
    lo = u.w[1];
    if (n >= 32) {
        hi = lo << (unsigned)(n - 32);
        lo = 0;
    } else {
        hi = (hi << (unsigned)n) | (lo >> (unsigned)(32 - n));
        lo = lo << (unsigned)n;
    }
    u.w[0] = hi;
    u.w[1] = lo;
    return u.sll;
}

unsigned long long
_lshuint64(unsigned long long x, int n)
{
    return (unsigned long long)_lshint64((long long)x, n);
}
