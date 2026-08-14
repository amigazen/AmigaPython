/*
 *      libcheck.c - Check library availability for Amiga
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 *      Simplified for vbcc PosixLib usage
 */

#include "Python.h"
#include "libcheck.h"

#include <libraries/locale.h>
#include <proto/locale.h>
#include <proto/exec.h>
#include <constructor.h>

/* global h_errno */
int h_errno = 0;

/* Library bases - simplified for PosixLib */
struct Library *UserGroupBase = (struct Library *)1;  /* Dummy pointer to indicate available */
struct Library *SocketBase = (struct Library *)1;     /* Dummy pointer to indicate available */
struct Library *UtilityBase = (struct Library *)1;    /* Dummy pointer to indicate available */

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
 * Check if usergroup.library is available
 * With PosixLib, user/group functions should be available directly
 * Returns 1 if available, 0 if not
 */
int checkusergrouplib(void)
{
    /* With PosixLib, user/group functionality should be available directly */
    return 1;
}

/*
 * Check if utility.library is available
 * With PosixLib, utility functions should be available directly
 * Returns 1 if available, 0 if not
 */
int checkutilitylib(void)
{
    /* With PosixLib, utility functionality should be available directly */
    return 1;
}

/*
 * Check if socket functionality is available
 * With PosixLib, socket functions should be available directly
 * Returns 1 if available, 0 if not
 */
int checksocketlib(void)
{
    /* With PosixLib, socket functionality should be available directly */
    return 1;
}

/*
 * Clean up library resources
 * Called at program exit
 * Simplified for PosixLib - no actual cleanup needed
 */
void cleanup_libraries(void)
{
    /* With PosixLib, no manual cleanup is needed */
    UserGroupBase = NULL;
    SocketBase = NULL;
    UtilityBase = NULL;
    if (LocaleBase != NULL) {
        CloseLibrary((struct Library *)LocaleBase);
        LocaleBase = NULL;
    }
} 