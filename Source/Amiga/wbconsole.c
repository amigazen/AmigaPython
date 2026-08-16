/*
 * wbconsole.c - Workbench CON: setup for VBCC + PosixLib
 *
 * SAS/C used __stdiowin to open one CON: for WB apps. PosixLib does not.
 * Workbench leaves pr_CIS/pr_COS ZERO; we Open CON:, SelectInput/Output,
 * SetConsoleTask, and retarget PosixLib fds.
 *
 * Exit / Close (WB-only panic):
 * PosixLib replaces stdin/stdout/stderr with new FILE objects. vclib's
 * fclose() only skips dos.library Close() for the original __stdin /
 * __stdout / __stderr symbols. With -lvc before -lposix, vclib fclose
 * wins and Close()s FILE->filehandle for each PosixLib std stream. We
 * put the same CON: BPTR in all three, so exit Close()s it 2-3 times
 * (Shell survives because CIS/COS are already distinct BPTRs).
 * Link -lposix before -lvc so PosixLib fclose + FDFL_STDIO is used.
 * We still clear PRF_CLOSE* and Close the CON: FH once in cleanup.
 *
 * Never open "*" or CONSOLE: as a stand-in for CON:; use the CON: path only.
 */

#include "wbconsole.h"

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <intuition/intuition.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/utility.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#ifndef MAX_FDESC
#define MAX_FDESC 64
#endif
#ifndef FDFL_STDIO
#define FDFL_STDIO       (1<<0)
#define FDFL_INTERACTIVE (1<<1)
#endif
struct __fd_s {
    long file;
    int nestcnt;
    int open_flags;
    unsigned long flags;
    unsigned long amode;
};
extern struct __fd_s *__fdesc[MAX_FDESC];

/* Amiga/dosio_init.c — keep in sync after SelectInput/Output. */
extern BPTR __dosio_files[3];

extern int WBArgParse_constructor(void);
extern struct WBStartup *WBenchMsg;
extern int _WBArgc;
extern char **_WBArgv;

/* Match classic SAS/C __stdiowin geometry/title. CLOSE gadget during run;
 * no WAIT — window must go away when we Close the FH (RKR M 13.2.1). */
static const char amiga_con_spec[] =
    "CON:0/11/640/200/Amiga Python 2.7/CLOSE/SMART";

#ifndef ZERO
#define ZERO ((BPTR)0L)
#endif

static BPTR wb_con_fh = 0;
static int wb_active = 0;
static char **wb_argv_owned = NULL;
/* Project launch: CD switched to sm_ArgList[1].wa_Lock (WB-owned; never UnLock). */
static BPTR wb_saved_cd = 0;
static int wb_cd_set = 0;
static APTR wb_saved_winptr = NULL;
static int wb_winptr_set = 0;

static char *
wb_strdup(const char *s)
{
    char *d;
    size_t n;

    if (s == NULL)
        return NULL;
    n = strlen(s) + 1;
    d = (char *)malloc(n);
    if (d == NULL)
        return NULL;
    memcpy(d, s, n);
    return d;
}

static void
wb_easy_error(CONST_STRPTR text)
{
    struct EasyStruct es;

    if (IntuitionBase == NULL) {
        IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library",
                                                            36L);
        if (IntuitionBase == NULL)
            return;
    }
    es.es_StructSize = sizeof(es);
    es.es_Flags = 0;
    es.es_Title = (CONST_STRPTR)"Amiga Python";
    es.es_TextFormat = text;
    es.es_GadgetFormat = (CONST_STRPTR)"OK";
    EasyRequestArgs(NULL, &es, NULL, NULL);
}

static struct WBStartup *
wb_get_startup(void)
{
    struct Process *pr;
    struct Message *msg;

    if (WBenchMsg != NULL)
        return WBenchMsg;

    /* RKR M: DOS I/O while WBStartup is still on pr_MsgPort deadlocks. */
    pr = (struct Process *)FindTask(NULL);
    msg = GetMsg(&pr->pr_MsgPort);
    if (msg != NULL) {
        WBenchMsg = (struct WBStartup *)msg;
        return WBenchMsg;
    }
    return NULL;
}

int
amiga_from_workbench(void)
{
    struct Process *pr;

    pr = (struct Process *)FindTask(NULL);
    if (pr == NULL)
        return 0;
    return (pr->pr_CLI == ZERO) ? 1 : 0;
}

static int
ends_with_py(const char *name)
{
    size_t n;

    if (name == NULL)
        return 0;
    n = strlen(name);
    if (n < 4)
        return 0;
    if (Stricmp((STRPTR)(name + n - 3), (STRPTR)".py") == 0)
        return 1;
    if (n >= 5 && Stricmp((STRPTR)(name + n - 4), (STRPTR)".pyw") == 0)
        return 1;
    return 0;
}

static int
full_path(BPTR lock, char *name, char *buf, int len)
{
    int i;
    int namelen;

    if (lock == ZERO || name == NULL || buf == NULL || len <= 0)
        return 0;
    if (!NameFromLock(lock, buf, len - 1))
        return 0;
    i = (int)strlen(buf);
    if (i > 0 && buf[i - 1] != ':' && buf[i - 1] != '/') {
        if (i >= len - 2)
            return 0;
        buf[i++] = '/';
        buf[i] = '\0';
    }
    namelen = (int)strlen(name);
    if (i + namelen >= len)
        return 0;
    strcpy(buf + i, name);
    return 1;
}

/* Lock without "Please insert volume ..." requesters (RKR M pr_WindowPtr). */
static BPTR
lock_quiet(STRPTR name, LONG mode)
{
    struct Process *pr;
    APTR oldwin;
    BPTR lock;

    pr = (struct Process *)FindTask(NULL);
    oldwin = NULL;
    if (pr != NULL) {
        oldwin = pr->pr_WindowPtr;
        pr->pr_WindowPtr = (APTR)-1L;
    }
    lock = Lock(name, mode);
    if (pr != NULL)
        pr->pr_WindowPtr = oldwin;
    return lock;
}

/* True if release/dev libs are next to the WB tool, or Python: exists. */
static int
libraries_reachable(struct WBStartup *wb)
{
    char buf[512];
    char dir[512];
    BPTR test;
    struct WBArg *wba;
    int n;

    if (wb != NULL && wb->sm_ArgList != NULL) {
        wba = wb->sm_ArgList;
        if (NameFromLock(wba->wa_Lock, dir, (LONG)sizeof(dir) - 1)) {
            n = (int)strlen(dir);
            if (n > 0 && dir[n - 1] != ':' && dir[n - 1] != '/') {
                if (n < (int)sizeof(dir) - 2) {
                    dir[n++] = '/';
                    dir[n] = '\0';
                }
            }
            strcpy(buf, dir);
            strcat(buf, "lib/python27.zip");
            test = lock_quiet(buf, ACCESS_READ);
            if (test != ZERO) {
                UnLock(test);
                return 1;
            }
            strcpy(buf, dir);
            strcat(buf, "Lib/os.py");
            test = lock_quiet(buf, ACCESS_READ);
            if (test != ZERO) {
                UnLock(test);
                return 1;
            }
            strcpy(buf, dir);
            strcat(buf, "lib/os.py");
            test = lock_quiet(buf, ACCESS_READ);
            if (test != ZERO) {
                UnLock(test);
                return 1;
            }
        }
    }

    test = lock_quiet((STRPTR)"Python:", ACCESS_READ);
    if (test != ZERO) {
        UnLock(test);
        return 1;
    }
    return 0;
}

static void
patch_stdio_file(FILE *f, BPTR fh)
{
    if (f == NULL || fh == ZERO)
        return;
    f->filehandle = (char *)fh;
}

static void
retarget_posix_stdio(BPTR fh)
{
    struct __fd_s *fp;

    if (fh == ZERO)
        return;

    fp = __fdesc[0];
    if (fp != NULL) {
        fp->file = (long)fh;
        fp->flags = FDFL_STDIO | FDFL_INTERACTIVE;
        fp->open_flags = O_RDWR;
    }
    fp = __fdesc[1];
    if (fp != NULL) {
        fp->file = (long)fh;
        fp->flags = FDFL_STDIO | FDFL_INTERACTIVE;
        fp->open_flags = O_RDWR;
    }
    if (__fdesc[2] == NULL && __fdesc[1] != NULL) {
        __fdesc[2] = __fdesc[1];
        __fdesc[1]->nestcnt++;
    } else if (__fdesc[2] != NULL && __fdesc[2] != __fdesc[1]) {
        __fdesc[2]->file = (long)fh;
        __fdesc[2]->flags = FDFL_STDIO | FDFL_INTERACTIVE;
        __fdesc[2]->open_flags = O_RDWR;
    }

    patch_stdio_file(stdin, fh);
    patch_stdio_file(stdout, fh);
    patch_stdio_file(stderr, fh);

    __dosio_files[0] = fh;
    __dosio_files[1] = fh;
    __dosio_files[2] = fh;
}

static int
build_wb_argv(struct WBStartup *wb, int *argc_out, char ***argv_out)
{
    char buf[512];
    char **argv;
    int argc;
    int i;
    int maxa;
    struct WBArg *wba;
    char *script;

    maxa = 8;
    argv = (char **)calloc((size_t)maxa, sizeof(char *));
    if (argv == NULL)
        return -1;
    argc = 0;
    script = NULL;

    wba = wb->sm_ArgList;
    if (wba != NULL && full_path(wba->wa_Lock, wba->wa_Name, buf, sizeof(buf)))
        argv[argc] = wb_strdup(buf);
    else if (wba != NULL && wba->wa_Name != NULL)
        argv[argc] = wb_strdup(wba->wa_Name);
    else
        argv[argc] = wb_strdup("Python");
    if (argv[argc] == NULL)
        goto fail;
    argc++;

    /*
     * Workbench project launch: sm_ArgList[0] is the tool, [1] is the
     * project (Default Tool). RKRM: CurrentDir(wa_Lock) then open
     * wa_Name — never Lock()/GetDiskObject()/absolute Open on the
     * project while WB still owns the startup locks. PosixLib opening
     * a NameFromLock absolute path can put a DOS requester on Workbench
     * while WB waits for ReplyMsg → whole-system deadlock (CON already
     * open). Pass the leaf name only; CD is the project drawer.
     */
    if (wb->sm_NumArgs >= 2) {
        wba = &wb->sm_ArgList[1];
        if (wba != NULL && wba->wa_Lock != ZERO && wba->wa_Name != NULL) {
            if (!wb_cd_set) {
                wb_saved_cd = CurrentDir(wba->wa_Lock);
                wb_cd_set = 1;
            }
            script = wb_strdup(wba->wa_Name);
        }
    }

    if (script == NULL && _WBArgv != NULL) {
        for (i = 1; i < _WBArgc; i++) {
            if (_WBArgv[i] == NULL)
                continue;
            if (Strnicmp(_WBArgv[i], (STRPTR)"PYTHONSCRIPT=", 13) == 0) {
                script = wb_strdup(_WBArgv[i] + 13);
                break;
            }
            if (ends_with_py(_WBArgv[i])) {
                script = wb_strdup(_WBArgv[i]);
                break;
            }
        }
    }

    if (script != NULL) {
        if (argc + 2 >= maxa) {
            char **nav;
            maxa += 8;
            nav = (char **)realloc(argv, (size_t)maxa * sizeof(char *));
            if (nav == NULL) {
                free(script);
                goto fail;
            }
            argv = nav;
        }
        argv[argc++] = script;
    }

    argv[argc] = NULL;
    *argc_out = argc;
    *argv_out = argv;
    wb_argv_owned = argv;
    return 0;

fail:
    for (i = 0; i < argc; i++)
        free(argv[i]);
    free(argv);
    return -1;
}

int
amiga_wb_prepare(int *argc, char ***argv)
{
    struct WBStartup *wb;
    struct FileHandle *cfh;
    struct Process *pr;
    int rc;

    if (!amiga_from_workbench())
        return 0;

    wb = wb_get_startup();
    if (wb == NULL) {
        wb_easy_error((CONST_STRPTR)
            "Could not read Workbench startup message.");
        return -1;
    }

    if (!libraries_reachable(wb)) {
        wb_easy_error((CONST_STRPTR)
            "Python libraries not found.\n\n"
            "Either Assign Python: to your install drawer,\n"
            "or run Python from a release folder that contains\n"
            "lib/python27.zip (and lib/lib-dynload).");
        return -1;
    }

    /* CON: first — then tooltypes / argv (never Lock the project). */
    wb_con_fh = Open((STRPTR)amiga_con_spec, MODE_NEWFILE);
    if (wb_con_fh == ZERO) {
        wb_easy_error((CONST_STRPTR)"Could not open CON: for Python.");
        return -1;
    }

    cfh = (struct FileHandle *)BADDR(wb_con_fh);
    if (cfh == NULL || cfh->fh_Type == NULL) {
        Close(wb_con_fh);
        wb_con_fh = ZERO;
        wb_easy_error((CONST_STRPTR)"CON: filehandle has no handler port.");
        return -1;
    }

    /* Process console port for this CON: (RKR M 10.2.11). */
    SetConsoleTask(cfh->fh_Type);

    /*
     * Own the CON: FH completely. DOS process teardown must not Close
     * pr_CIS/pr_COS/pr_CES — with one shared FH that is a double Close
     * panic (RKR M 5.3.2). We Close once in amiga_wb_cleanup().
     */
    pr = (struct Process *)FindTask(NULL);
    if (pr != NULL) {
        pr->pr_Flags &= ~(ULONG)(PRF_CLOSEINPUT | PRF_CLOSEOUTPUT |
                                 PRF_CLOSEERROR);
    }

    SelectInput(wb_con_fh);
    SelectOutput(wb_con_fh);

    retarget_posix_stdio(wb_con_fh);

    /*
     * Kill DOS volume requesters for this process. With pr_WindowPtr
     * NULL they go to Workbench; WB is blocked on our unreplied
     * WBStartup → invisible requester = system deadlock.
     */
    if (pr != NULL) {
        wb_saved_winptr = pr->pr_WindowPtr;
        pr->pr_WindowPtr = (APTR)-1L;
        wb_winptr_set = 1;
    }

    Write(wb_con_fh, (APTR)"Amiga Python 2.7.18\n", 19);

    /*
     * Tooltypes only for tool-alone launches. Project launch must not
     * touch icon.library / NameFromLock on WB args after CON is up.
     */
    if (wb->sm_NumArgs < 2)
        WBArgParse_constructor();

    rc = build_wb_argv(wb, argc, argv);
    if (rc != 0) {
        wb_easy_error((CONST_STRPTR)"Could not build Workbench argv.");
        return -1;
    }

    wb_active = 1;
    /* Py_Exit() never returns to main(); still run CON teardown once. */
    atexit(amiga_wb_cleanup);
    return 0;
}

void
amiga_wb_cleanup(void)
{
    struct __fd_s *fp;
    struct Process *pr;
    static int cleaned;

    /* atexit + main both call us; only act once. */
    if (cleaned)
        return;
    if (wb_con_fh == ZERO) {
        wb_active = 0;
        return;
    }
    cleaned = 1;
    wb_active = 0;

    /* Restore CD / window ptr before tearing down CON (do not UnLock WB). */
    if (wb_cd_set) {
        CurrentDir(wb_saved_cd);
        wb_saved_cd = ZERO;
        wb_cd_set = 0;
    }
    if (wb_winptr_set) {
        pr = (struct Process *)FindTask(NULL);
        if (pr != NULL)
            pr->pr_WindowPtr = wb_saved_winptr;
        wb_winptr_set = 0;
    }

    /* Flush C stdio before the Amiga FH goes away. */
    fflush(stdout);
    fflush(stderr);

    /*
     * Detach from the process streams first so DOS exit cannot Close this
     * FH (flags already clear; CIS/COS must not still reference it).
     */
    if (Input() == wb_con_fh)
        SelectInput(ZERO);
    if (Output() == wb_con_fh)
        SelectOutput(ZERO);

    /* Drop Amiga BPTRs from PosixLib so EXIT fclose/close cannot Close. */
    fp = __fdesc[0];
    if (fp != NULL && (BPTR)fp->file == wb_con_fh)
        fp->file = (long)ZERO;
    fp = __fdesc[1];
    if (fp != NULL && (BPTR)fp->file == wb_con_fh)
        fp->file = (long)ZERO;
    fp = __fdesc[2];
    if (fp != NULL && (BPTR)fp->file == wb_con_fh)
        fp->file = (long)ZERO;

    patch_stdio_file(stdin, ZERO);
    patch_stdio_file(stdout, ZERO);
    patch_stdio_file(stderr, ZERO);

    __dosio_files[0] = ZERO;
    __dosio_files[1] = ZERO;
    __dosio_files[2] = ZERO;

    /* Exactly one Close. Without WAIT, CON-Handler shuts the window. */
    Close(wb_con_fh);
    wb_con_fh = ZERO;
}
