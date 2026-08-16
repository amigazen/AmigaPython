/*
 * wbargs.c - Workbench argument handling for Amiga
 *
 * Based on Irmen de Jong's original Amiga port, updated for VBCC/PosixLib.
 *
 * IMPORTANT (Workbench deadlock): While a project is launched via Default
 * Tool, Workbench keeps locks related to that icon/file and waits for
 * ReplyMsg(WBStartup). Calling Lock() or GetDiskObject() on the project
 * can block forever on those locks — whole-system deadlock. Only use the
 * wa_Lock/wa_Name already provided in the startup message; never Lock() or
 * GetDiskObject() the project. Tooltypes are read from the TOOL icon only.
 */

#include <workbench/startup.h>
#include <exec/execbase.h>
#include <string.h>
#include <dos/dos.h>
#include <stdlib.h>

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/utility.h>

#define GRAPHICS_GFXNODES_H
#define GRAPHICS_VIEW_H
#define INTUITION_SCREENS_H
#include <proto/icon.h>

extern struct WBStartup *WBenchMsg;

int _WBArgc;
char **_WBArgv;

static int _WBArgMax;
static int wbargs_done;

static int
FullPathName(BPTR parent, char *name, char *buf, int len)
{
    int i;
    int name_len;

    if (!parent || !name || !buf || len <= 0)
        return 0;

    if (!NameFromLock(parent, buf, len - 1))
        return 0;

    i = (int)strlen(buf);
    if (i > 0 && buf[i - 1] != ':' && buf[i - 1] != '/') {
        if (i >= len - 2)
            return 0;
        buf[i++] = '/';
        buf[i] = '\0';
    }

    name_len = (int)strlen(name);
    if (i + name_len >= len)
        return 0;

    strncpy(buf + i, name, (size_t)(len - i - 1));
    buf[len - 1] = '\0';
    return 1;
}

static void
FreeWBArgs(void)
{
    int i;

    if (_WBArgv) {
        for (i = 0; i < _WBArgc; i++) {
            if (_WBArgv[i]) {
                free(_WBArgv[i]);
                _WBArgv[i] = NULL;
            }
        }
        free(_WBArgv);
        _WBArgv = NULL;
    }
    _WBArgc = 0;
    _WBArgMax = 0;
}

static int
AddWBArg(char *arg)
{
    int new_max;
    char **new_argv;
    int arg_len;

    if (!arg)
        return -1;

    if (_WBArgc >= _WBArgMax - 1) {
        new_max = _WBArgMax + 10;
        new_argv = realloc(_WBArgv, (size_t)new_max * sizeof(char *));
        if (new_argv == NULL)
            return -1;
        _WBArgv = new_argv;
        _WBArgMax = new_max;
    }

    arg_len = (int)strlen(arg);
    _WBArgv[_WBArgc] = malloc((size_t)arg_len + 1);
    if (_WBArgv[_WBArgc] == NULL)
        return -1;

    strcpy(_WBArgv[_WBArgc], arg);
    _WBArgc++;
    return 0;
}

static void
ReorderWBArgv(int argc, char **argv)
{
    int i;

    if (!argv || argc <= 1)
        return;

    for (i = 1; i < argc; i++) {
        if (!argv[i])
            continue;

        if (Strnicmp((STRPTR)"PYTHONSCRIPT=", (STRPTR)argv[i], 13) == 0) {
            short j;
            char *thisarg;

            memmove(argv[i], argv[i] + 13, strlen(argv[i] + 13) + 1);
            thisarg = argv[i];
            for (j = (short)i; j > 1; )
                argv[j] = argv[--j];
            argv[1] = thisarg;
        }

        if (Strnicmp((STRPTR)"PYSCRIPTARG=", (STRPTR)argv[i], 12) == 0) {
            short j;
            char *thisarg;

            memmove(argv[i], argv[i] + 12, strlen(argv[i] + 12) + 1);
            thisarg = argv[i];
            for (j = (short)i; j < argc - 1; j++)
                argv[j] = argv[j + 1];
            argv[argc - 1] = thisarg;
            i--;
        }
    }
}

/*
 * Parse WBStartup into _WBArgc/_WBArgv. Safe to call more than once.
 * Must only run after WBenchMsg has been GetMsg'd off pr_MsgPort.
 */
int
WBArgParse_constructor(void)
{
    struct WBArg *wba;
    int nargs;
    int status;
    char buf[512];
    struct DiskObject *dob;
    struct Library *IconBase;
    BPTR dir;

    status = 0;
    dob = NULL;
    IconBase = NULL;
    dir = 0;

    if (WBenchMsg == NULL)
        return 0;

    /* Already parsed (prepare may call more than once). */
    if (wbargs_done && _WBArgv != NULL)
        return 0;

    FreeWBArgs();
    _WBArgMax = 10;
    _WBArgv = calloc((size_t)_WBArgMax, sizeof(char *));
    if (_WBArgv == NULL)
        return -1;

    wba = WBenchMsg->sm_ArgList;
    if (FullPathName(wba->wa_Lock, wba->wa_Name, buf, sizeof(buf))) {
        if (AddWBArg(buf)) {
            FreeWBArgs();
            return -1;
        }
    } else if (wba->wa_Name != NULL) {
        if (AddWBArg(wba->wa_Name)) {
            FreeWBArgs();
            return -1;
        }
    }

    /* Tooltypes from the TOOL icon only — never from project icons. */
    if ((IconBase = OpenLibrary("icon.library", 0L)) != NULL) {
        dir = CurrentDir(wba->wa_Lock);
        dob = GetDiskObject(wba->wa_Name);
        if (dob != NULL) {
            if (dob->do_ToolTypes != NULL) {
                for (nargs = 0; dob->do_ToolTypes[nargs]; nargs++) {
                    if (AddWBArg(dob->do_ToolTypes[nargs])) {
                        status = -1;
                        break;
                    }
                }
            }
            FreeDiskObject(dob);
            dob = NULL;
        }
        CurrentDir(dir);
        dir = 0;
        CloseLibrary(IconBase);
        IconBase = NULL;
    }

    if (status != 0) {
        FreeWBArgs();
        return status;
    }

    /*
     * Project / shift-click args: path from wa_Lock+wa_Name only.
     * No Lock(), no GetDiskObject() — see file header.
     */
    for (nargs = 1, wba++;
         nargs < WBenchMsg->sm_NumArgs;
         nargs++, wba++) {
        if (FullPathName(wba->wa_Lock, wba->wa_Name, buf, sizeof(buf))) {
            if (AddWBArg(buf)) {
                FreeWBArgs();
                return -1;
            }
        } else if (wba->wa_Name != NULL) {
            if (AddWBArg(wba->wa_Name)) {
                FreeWBArgs();
                return -1;
            }
        }
    }

    _WBArgv[_WBArgc] = NULL;
    ReorderWBArgv(_WBArgc, _WBArgv);
    wbargs_done = 1;
    return 0;
}

/*
 * CONSTRUCTOR_P(WBArgParse,…) expands to WBArgParse_constructor — do not
 * add a second definition. Parse only from amiga_wb_prepare() after CON:.
 */
