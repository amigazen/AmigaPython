/*
 *      syslog.c - PosixLib-style syslog helpers (Amiga/ tree copy)
 *
 *      Prefer Modules/syslog.c in the VBCC link. Kept in sync for SMAKEFILE.
 */

#include <exec/types.h>
#include <utility/tagitem.h>
#include <proto/exec.h>
#include <libraries/bsdsocket.h>

#include "libcheck.h"

extern int __init_bsdsocket(int);
extern struct Library *SocketBase;

static LONG __SocketBaseTagList(__reg("a6") void *base,
                                __reg("a0") struct TagItem *tags)
    = "\tjsr\t-294(a6)";

static VOID __vsyslog(__reg("a6") void *base,
                      __reg("d0") LONG pri,
                      __reg("a0") STRPTR msg,
                      __reg("a1") APTR args)
    = "\tjsr\t-258(a6)";

static int
amiga_syslog_init(void)
{
    if (SocketBase != NULL)
        return 0;
    if (__init_bsdsocket(-1) != 0 || SocketBase == NULL)
        return -1;
    return 0;
}

void
openlog(const char *ident, int logstat, int logfac)
{
    struct TagItem tags[4];

    if (amiga_syslog_init() != 0)
        return;

    tags[0].ti_Tag = SBTM_SETVAL(SBTC_LOGTAGPTR);
    tags[0].ti_Data = (ULONG)ident;
    tags[1].ti_Tag = SBTM_SETVAL(SBTC_LOGSTAT);
    tags[1].ti_Data = (ULONG)logstat;
    tags[2].ti_Tag = SBTM_SETVAL(SBTC_LOGFACILITY);
    tags[2].ti_Data = (ULONG)logfac;
    tags[3].ti_Tag = TAG_DONE;

    (void)__SocketBaseTagList(SocketBase, tags);
}

void
closelog(void)
{
    struct TagItem tags[2];

    if (SocketBase == NULL)
        return;

    tags[0].ti_Tag = SBTM_SETVAL(SBTC_LOGTAGPTR);
    tags[0].ti_Data = (ULONG)NULL;
    tags[1].ti_Tag = TAG_DONE;

    (void)__SocketBaseTagList(SocketBase, tags);
}

int
setlogmask(int pmask)
{
    struct TagItem taglist[3];

    if (amiga_syslog_init() != 0)
        return 0;

    taglist[0].ti_Tag = SBTM_GETVAL(SBTC_LOGMASK);
    taglist[0].ti_Data = 0;
    taglist[1].ti_Tag = SBTM_SETVAL(SBTC_LOGMASK);
    taglist[1].ti_Data = (ULONG)pmask;
    taglist[2].ti_Tag = TAG_DONE;

    (void)__SocketBaseTagList(SocketBase, taglist);
    return (int)taglist[0].ti_Data;
}

void
amiga_syslog(int priority, const char *message)
{
    const char *msg;

    if (amiga_syslog_init() != 0 || message == NULL)
        return;

    msg = message;
    __vsyslog(SocketBase, (LONG)priority, (STRPTR)"%s", (APTR)&msg);
}

void
syslog(int priority, const char *format, ...)
{
    if (format == NULL)
        return;
    amiga_syslog(priority, format);
}
