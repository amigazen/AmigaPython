/*
 *      libcheck.c - Check library availability for Amiga
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 *
 *      SocketBase stays a PosixLib dummy (pointer 1) until the _socket
 *      LoadSeg plugin opens bsdsocket.library for real. Do not OpenLibrary
 *      from checksocketlib() — that can hang or block console I/O when no
 *      TCP stack is running, and the interpreter never reaches a prompt.
 */

#include "Python.h"
#include "libcheck.h"

#include <libraries/locale.h>
#include <proto/locale.h>
#include <proto/exec.h>
#include <constructor.h>

/* global h_errno */
int h_errno = 0;

/* Library bases - PosixLib: non-NULL dummy means "available" */
struct Library *UserGroupBase = (struct Library *)1;
struct Library *SocketBase = (struct Library *)1;
struct Library *UtilityBase = (struct Library *)1;

/* Opened on demand by Modules/_hashcrc.c (import _hashlib). */
struct Library *CRCBase = NULL;

/*
 * LocaleBase is declared extern in proto/locale.h. VBCC stubs and PosixLib
 * (tzset, strncmp, etc.) all need a real definition and an opened library.
 */
struct LocaleBase *LocaleBase = NULL;

CONSTRUCTOR_P(locale_lib_init, 5000)
{
    if (LocaleBase == NULL)
        LocaleBase = (struct LocaleBase *)OpenLibrary("locale.library", 38);
    return 0;
}

int
checkusergrouplib(void)
{
    return 1;
}

int
checkutilitylib(void)
{
    return 1;
}

/*
 * PosixLib path: always succeed. Real bsdsocket open is only in
 * pyamiga_init_socket_gated() when import _socket loads the plugin.
 */
int
checksocketlib(void)
{
    return 1;
}

void
cleanup_libraries(void)
{
    UserGroupBase = NULL;
    UtilityBase = NULL;
    /* Only CloseLibrary a real base, not the PosixLib dummy (1). */
    if (SocketBase != NULL && SocketBase != (struct Library *)1) {
        CloseLibrary(SocketBase);
    }
    SocketBase = NULL;
    if (LocaleBase != NULL) {
        CloseLibrary((struct Library *)LocaleBase);
        LocaleBase = NULL;
    }
    if (CRCBase != NULL) {
        CloseLibrary(CRCBase);
        CRCBase = NULL;
    }
}
