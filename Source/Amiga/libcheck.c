/*
 *      libcheck.c - Check library availability for Amiga
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 *
 *      Never use a fake library base (e.g. pointer 1). AmiTCP / Roadshow
 *      LVO macros (proto/usergroup.h, proto/socket.h) jump through the
 *      base in A6 — a dummy pointer hard-crashes.
 *
 *      bsdsocket.library: do not OpenLibrary from checksocketlib(). Opening
 *      with no TCP stack can hang. Real open is PosixLib __init_bsdsocket()
 *      from pyamiga_init_socket_gated() on import _socket (or PosixLib
 *      wrappers that call __init_bsdsocket themselves). checksocketlib()
 *      only reports whether SocketBase is already live.
 *
 *      usergroup.library: OpenLibrary on demand. Missing lib returns NULL
 *      (no hang); callers must soft-fail.
 *
 *      Newer PosixLib defines SocketBase in posix.lib(bsdsocket.c); we
 *      must not define it here or the link fails.
 */

#include "Python.h"
#include "libcheck.h"

#include <libraries/locale.h>
#include <proto/locale.h>
#include <proto/exec.h>
#include <constructor.h>

/* global h_errno */
int h_errno = 0;

/* Real bases only — NULL means not open. */
struct Library *UserGroupBase = NULL;
struct Library *UtilityBase = NULL;

/* Owned by posix.lib when HAVE_POSIXLIB; otherwise we provide the base. */
#ifdef HAVE_POSIXLIB
extern struct Library *SocketBase;
#else
struct Library *SocketBase = NULL;
#endif

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

/*
 * Open usergroup.library if needed. Returns 1 on success, 0 if missing.
 * Does not set a Python exception (use checkusergrouplib for that).
 */
int
have_usergrouplib(void)
{
    if (UserGroupBase != NULL)
        return 1;

    UserGroupBase = OpenLibrary("usergroup.library", 4);
    /* Do not try AmiTCP:libs/... — a missing AmiTCP: assign waits on a
     * volume requester and looks like a hard lockup. */
    return (UserGroupBase != NULL) ? 1 : 0;
}

int
checkusergrouplib(void)
{
    if (have_usergrouplib())
        return 1;
    if (!PyErr_Occurred()) {
        PyErr_SetString(PyExc_SystemError,
                        "Couldn't open usergroup.library");
    }
    return 0;
}

int
checkutilitylib(void)
{
    if (UtilityBase != NULL)
        return 1;

    UtilityBase = OpenLibrary("utility.library", 37);
    if (UtilityBase != NULL)
        return 1;

    if (!PyErr_Occurred()) {
        PyErr_SetString(PyExc_SystemError,
                        "Couldn't open utility.library");
    }
    return 0;
}

/*
 * Report whether bsdsocket is already open. Does not OpenLibrary —
 * that belongs to the gated _socket path / PosixLib __init_bsdsocket.
 */
int
have_socketlib(void)
{
    return (SocketBase != NULL) ? 1 : 0;
}

int
checksocketlib(void)
{
    if (have_socketlib())
        return 1;
    if (!PyErr_Occurred()) {
        PyErr_SetString(PyExc_SystemError,
                        "bsdsocket.library not open "
                        "(import _socket or start a TCP/IP stack)");
    }
    return 0;
}

void
cleanup_libraries(void)
{
    if (UserGroupBase != NULL) {
        CloseLibrary(UserGroupBase);
        UserGroupBase = NULL;
    }
    if (UtilityBase != NULL) {
        CloseLibrary(UtilityBase);
        UtilityBase = NULL;
    }
    /*
     * PosixLib owns SocketBase (posix.lib bsdsocket.c) and closes it in
     * _EXIT_4_bsdsocket. Do not CloseLibrary here — that would double-close.
     */
#ifdef HAVE_POSIXLIB
    SocketBase = NULL;
#else
    if (SocketBase != NULL) {
        CloseLibrary(SocketBase);
        SocketBase = NULL;
    }
#endif
    if (LocaleBase != NULL) {
        CloseLibrary((struct Library *)LocaleBase);
        LocaleBase = NULL;
    }
    if (CRCBase != NULL) {
        CloseLibrary(CRCBase);
        CRCBase = NULL;
    }
}
