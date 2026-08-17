/*
 * Libc trampolines into the host (FastForward rule: no C runtime in LoadSeg).
 * VBCC/PosixLib string.h and ctype.h use macros / __asm_* forms — undef
 * those before defining our stubs so the plugin links these symbols.
 */
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "pyamiga_plugin.h"
#include "pyamiga_redir.h"

#undef malloc
#undef free
#undef memcpy
#undef memset
#undef memmove
#undef strlen
#undef strcpy
#undef strncpy
#undef strcmp
#undef sprintf
#undef sscanf
#undef strtoul
#undef isspace

void *
malloc(size_t n)
{
    return PyAmiga_Host->fn_malloc(n);
}

void
free(void *p)
{
    PyAmiga_Host->fn_free(p);
}

void *
memcpy(void *dst, const void *src, size_t n)
{
    return PyAmiga_Host->fn_memcpy(dst, src, n);
}

void *
memset(void *dst, int c, size_t n)
{
    return PyAmiga_Host->fn_memset(dst, c, n);
}

void *
memmove(void *dst, const void *src, size_t n)
{
    return PyAmiga_Host->fn_memmove(dst, src, n);
}

size_t
strlen(const char *s)
{
    return PyAmiga_Host->fn_strlen(s);
}

char *
strcpy(char *dst, const char *src)
{
    return PyAmiga_Host->fn_strcpy(dst, src);
}

char *
strncpy(char *dst, const char *src, size_t n)
{
    return PyAmiga_Host->fn_strncpy(dst, src, n);
}

int
strcmp(const char *a, const char *b)
{
    return PyAmiga_Host->fn_strcmp(a, b);
}

int
sprintf(char *s, const char *fmt, ...)
{
    int r;
    va_list v;

    va_start(v, fmt);
    r = PyAmiga_Host->fn_vsprintf(s, fmt, v);
    va_end(v);
    return r;
}

int
sscanf(const char *s, const char *fmt, ...)
{
    int r;
    va_list v;

    va_start(v, fmt);
    r = PyAmiga_Host->fn_vsscanf(s, fmt, v);
    va_end(v);
    return r;
}

unsigned long
strtoul(const char *s, char **end, int base)
{
    return PyAmiga_Host->fn_strtoul(s, end, base);
}

int
isspace(int c)
{
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r' ||
            c == '\f' || c == '\v');
}
