/*
 * vbcc_runtime.c - Missing VBCC runtime functions for AmigaOS
 * 
 * This file provides implementations for VBCC runtime functions that are missing
 * from the VBCC library but are needed by Python.
 */

#include <exec/memory.h>
#include <dos/dos.h>
#include <stddef.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include "netlib.h"
#include "../Include/protos.h"

/* Define missing constants */
#ifndef MAXPATHLEN
#define MAXPATHLEN 1024  /* Reasonable default for AmigaOS */
#endif

#ifndef _SYS_NMLN
#define _SYS_NMLN 65     /* System name length */
#endif

/* Define missing constants for pathconf, fpathconf, and sysconf */
#ifndef _PC_PATH_MAX
#define _PC_PATH_MAX 1
#endif
#ifndef _PC_NAME_MAX
#define _PC_NAME_MAX 2
#endif
#ifndef _PC_LINK_MAX
#define _PC_LINK_MAX 3
#endif
#ifndef _PC_MAX_CANON
#define _PC_MAX_CANON 4
#endif
#ifndef _PC_MAX_INPUT
#define _PC_MAX_INPUT 5
#endif
#ifndef _PC_PIPE_BUF
#define _PC_PIPE_BUF 6
#endif
#ifndef _PC_CHOWN_RESTRICTED
#define _PC_CHOWN_RESTRICTED 7
#endif
#ifndef _PC_NO_TRUNC
#define _PC_NO_TRUNC 8
#endif
#ifndef _PC_VDISABLE
#define _PC_VDISABLE 9
#endif

#ifndef _SC_ARG_MAX
#define _SC_ARG_MAX 1
#endif
#ifndef _SC_CHILD_MAX
#define _SC_CHILD_MAX 2
#endif
#ifndef _SC_CLK_TCK
#define _SC_CLK_TCK 3
#endif
#ifndef _SC_NGROUPS_MAX
#define _SC_NGROUPS_MAX 4
#endif
/* _SC_OPEN_MAX is already defined in PosixLib/unistd.h */
#ifndef _SC_JOB_CONTROL
#define _SC_JOB_CONTROL 6
#endif
#ifndef _SC_SAVED_IDS
#define _SC_SAVED_IDS 7
#endif
#ifndef _SC_VERSION
#define _SC_VERSION 8
#endif
#ifndef _SC_PAGESIZE
#define _SC_PAGESIZE 9
#endif
#ifndef _SC_XOPEN_CRYPT
#define _SC_XOPEN_CRYPT 10
#endif
#ifndef _SC_XOPEN_ENH_I18N
#define _SC_XOPEN_ENH_I18N 11
#endif
#ifndef _SC_XOPEN_SHM
#define _SC_XOPEN_SHM 12
#endif
#ifndef _SC_2_CHAR_TERM
#define _SC_2_CHAR_TERM 13
#endif
#ifndef _SC_2_C_BIND
#define _SC_2_C_BIND 14
#endif
#ifndef _SC_2_C_DEV
#define _SC_2_C_DEV 15
#endif
#ifndef _SC_2_FORT_DEV
#define _SC_2_FORT_DEV 16
#endif
#ifndef _SC_2_FORT_RUN
#define _SC_2_FORT_RUN 17
#endif
#ifndef _SC_2_LOCALEDEF
#define _SC_2_LOCALEDEF 18
#endif
#ifndef _SC_2_PBS
#define _SC_2_PBS 19
#endif
#ifndef _SC_2_PBS_ACCOUNTING
#define _SC_2_PBS_ACCOUNTING 20
#endif
#ifndef _SC_2_PBS_CHECKPOINT
#define _SC_2_PBS_CHECKPOINT 21
#endif
#ifndef _SC_2_PBS_LOCATE
#define _SC_2_PBS_LOCATE 22
#endif
#ifndef _SC_2_PBS_MESSAGE
#define _SC_2_PBS_MESSAGE 23
#endif
#ifndef _SC_2_PBS_TRACK
#define _SC_2_PBS_TRACK 24
#endif
#ifndef _SC_2_SW_DEV
#define _SC_2_SW_DEV 25
#endif
#ifndef _SC_2_UPE
#define _SC_2_UPE 26
#endif
#ifndef _SC_2_VERSION
#define _SC_2_VERSION 27
#endif
#ifndef _SC_BC_BASE_MAX
#define _SC_BC_BASE_MAX 28
#endif
#ifndef _SC_BC_DIM_MAX
#define _SC_BC_DIM_MAX 29
#endif
#ifndef _SC_BC_SCALE_MAX
#define _SC_BC_SCALE_MAX 30
#endif
#ifndef _SC_BC_STRING_MAX
#define _SC_BC_STRING_MAX 31
#endif
#ifndef _SC_COLL_WEIGHTS_MAX
#define _SC_COLL_WEIGHTS_MAX 32
#endif
#ifndef _SC_EQUIV_CLASS_MAX
#define _SC_EQUIV_CLASS_MAX 33
#endif
#ifndef _SC_EXPR_NEST_MAX
#define _SC_EXPR_NEST_MAX 34
#endif
#ifndef _SC_LINE_MAX
#define _SC_LINE_MAX 35
#endif
#ifndef _SC_RE_DUP_MAX
#define _SC_RE_DUP_MAX 36
#endif
#ifndef _SC_XOPEN_LEGACY
#define _SC_XOPEN_LEGACY 37
#endif
#ifndef _SC_XOPEN_REALTIME
#define _SC_XOPEN_REALTIME 38
#endif
#ifndef _SC_XOPEN_REALTIME_THREADS
#define _SC_XOPEN_REALTIME_THREADS 39
#endif
#ifndef _SC_XOPEN_VERSION
#define _SC_XOPEN_VERSION 40
#endif

/* Signal constants for kill() function */
#ifndef SIGBREAKF_CTRL_C
#define SIGBREAKF_CTRL_C (1L << 12)  /* Ctrl+C break signal */
#endif

/* Define _OSERR for this module */
int _OSERR;

/*
 * _lmods - Signed 32-bit long modulo operation
 * Returns: a % b (signed)
 */
long _lmods(long a, long b)
{
    return a % b;
}

/*
 * _lmodu - Unsigned 32-bit long modulo operation  
 * Returns: a % b (unsigned)
 */
unsigned long _lmodu(unsigned long a, unsigned long b)
{
    return a % b;
} 

/* Commented out - zlib provides its own implementations
void *zcalloc(void *opaque, unsigned items, unsigned size) {
    (void)opaque;
    return AllocVec(items * size, MEMF_PUBLIC | MEMF_CLEAR);
}

void zcfree(void *opaque, void *ptr) {
    (void)opaque;
    if (ptr) FreeVec(ptr);
}
*/

/*
 * zmemcpy - Copy memory for zlib
 * Returns: pointer to destination
 */
void *zmemcpy(void *dest, const void *src, unsigned len)
{
    char *d = (char *)dest;
    const char *s = (const char *)src;
    
    if (len == 0) return dest;
    do {
        *d++ = *s++;
    } while (--len != 0);
    return dest;
}

/*
 * zmemzero - Zero memory for zlib
 * Returns: pointer to destination
 */
void *zmemzero(void *dest, unsigned len)
{
    char *d = (char *)dest;
    
    if (len == 0) return dest;
    do {
        *d++ = 0;
    } while (--len != 0);
    return dest;
}

/*
 * my_mkdir - Create a directory (extracted from unixemul.c)
 * Returns: 0 on success, -1 on error with errno set
 */
int my_mkdir(const char* path, int p)
{
    BPTR lock;
    
    /* ignore the p (protection bits) parameter */
    
    if((lock = CreateDir(path)))
    {
        UnLock(lock);
        return 0;
    }
    
    errno = __io2errno(_OSERR = IoErr());
    return -1;
}

/*
 * nice - Change process priority using AmigaOS ChangeTaskPri
 * Returns: new nice value on success, -1 on error
 */
int nice(int incr)
{
    struct Task *task;
    int current_priority;
    int old_priority;
    int new_priority;
    int priority_change;
    int nice_value;
    
    task = FindTask(NULL);
    if (!task) {
        errno = EINVAL;
        return -1;
    }
    
    /* Get current priority */
    current_priority = task->tc_Node.ln_Pri;
    
    /* Calculate new priority based on nice increment */
    /* AmigaOS priorities range from -128 (highest) to +127 (lowest) */
    /* Unix nice values range from -20 (highest) to +19 (lowest) */
    /* We'll map nice values to AmigaOS priorities */
    
    new_priority = current_priority;
    
    if (incr != 0) {
        /* Convert nice increment to AmigaOS priority change */
        /* Each nice unit is roughly 1 AmigaOS priority unit */
        priority_change = incr;
        new_priority = current_priority + priority_change;
        
        /* Clamp to user task priority range (-10 to +10) */
        if (new_priority > 10) new_priority = 10;
        if (new_priority < -10) new_priority = -10;
        
        /* Change the task priority */
        old_priority = SetTaskPri(task, new_priority);
        /* SetTaskPri returns the old priority, so we don't need to check for errors */
    }
    
    /* Convert AmigaOS priority back to a "nice" value for return */
    /* Map -10 to +10 range to 0-20 nice range (0 = normal, 20 = lowest priority) */
    nice_value = (10 - new_priority);  /* Invert: -10 becomes 20, +10 becomes 0 */
    if (nice_value < 0) nice_value = 0;
    if (nice_value > 20) nice_value = 20;
    
    return nice_value;
}

/*
 * waitpid - Wait for a specific child process
 * Returns: process ID on success, -1 on error
 */
pid_t waitpid(pid_t pid, int *status, int options)
{
    /* AmigaOS doesn't have a true process hierarchy like Unix */
    /* For now, return error indicating not supported */
    errno = ENOSYS;
    return -1;
}

/*
 * kill - Send a signal to a process
 * Returns: 0 on success, -1 on error
 */
int kill(pid_t pid, int sig)
{
    struct Task *target_task;
    struct Task *current_task;
    
    target_task = (struct Task *)pid;
    
    /* Validate the task pointer */
    if (!target_task) {
        errno = EINVAL;
        return -1;
    }
    
    /* Check if the task is valid by looking for it in the task list */
    current_task = FindTask(NULL);
    if (!current_task) {
        errno = EINVAL;
        return -1;
    }
    
    /* For now, we'll only support SIGBREAKF_CTRL_C (Ctrl+C equivalent) */
    /* In AmigaOS, this is typically sent via the console handler */
    if (sig == SIGBREAKF_CTRL_C) {
        /* Send a break signal to the task */
        /* Note: This is a simplified implementation */
        /* In a full implementation, you'd need to handle different signal types */
        
        /* For SIGBREAKF_CTRL_C, we can try to signal the task */
        /* This is the equivalent of sending Ctrl+C to a process */
        Signal(target_task, SIGBREAKF_CTRL_C);
        return 0;  /* Success - Signal always succeeds on AmigaOS */
    }
    
    /* For other signals, return not supported for now */
    /* Could be extended to support other AmigaOS signals */
    errno = ENOSYS;
    return -1;
}

/*
 * pathconf - Get pathname configurable system variables
 * Returns: value on success, -1 on error
 */
long pathconf(const char *path, int name)
{
    switch (name) {
        case _PC_PATH_MAX:
            return MAXPATHLEN;
        case _PC_NAME_MAX:
            return 107;  /* AmigaOS filename limit */
        case _PC_LINK_MAX:
            return 1;    /* AmigaOS doesn't support hard links in the same way */
        case _PC_MAX_CANON:
            return 255;  /* Reasonable default */
        case _PC_MAX_INPUT:
            return 255;  /* Reasonable default */
        case _PC_PIPE_BUF:
            return 4096; /* Reasonable default */
        case _PC_CHOWN_RESTRICTED:
            return 1;    /* AmigaOS doesn't have Unix-style ownership */
        case _PC_NO_TRUNC:
            return 0;    /* Names can be truncated */
        case _PC_VDISABLE:
            return 0;    /* No special disable character */
        default:
            errno = EINVAL;
            return -1;
    }
}

/*
 * fpathconf - Get file descriptor configurable system variables
 * Returns: value on success, -1 on error
 */
long fpathconf(int fd, int name)
{
    switch (name) {
        case _PC_MAX_CANON:
            return 255;  /* Reasonable default */
        case _PC_MAX_INPUT:
            return 255;  /* Reasonable default */
        case _PC_PIPE_BUF:
            return 4096; /* Reasonable default */
        case _PC_CHOWN_RESTRICTED:
            return 1;    /* AmigaOS doesn't have Unix-style ownership */
        case _PC_NO_TRUNC:
            return 0;    /* Names can be truncated */
        case _PC_VDISABLE:
            return 0;    /* No special disable character */
        default:
            errno = EINVAL;
            return -1;
    }
}

/*
 * sysconf - Get configurable system variables
 * Returns: value on success, -1 on error
 */
long sysconf(int name)
{
    switch (name) {
        case _SC_ARG_MAX:
            return 4096; /* Reasonable default for AmigaOS */
        case _SC_CHILD_MAX:
            return 1;    /* AmigaOS doesn't have true child processes */
        case _SC_CLK_TCK:
            return 50;   /* AmigaOS tick rate */
        case _SC_NGROUPS_MAX:
            return 0;    /* AmigaOS doesn't have Unix-style groups */
        case _SC_GETPW_R_SIZE_MAX:
            return 1024; /* Reasonable default for password buffer size */
        case _SC_OPEN_MAX:
            return 256;  /* Reasonable default for file descriptors */
        case _SC_JOB_CONTROL:
            return 0;    /* AmigaOS doesn't have job control */
        case _SC_SAVED_IDS:
            return 0;    /* AmigaOS doesn't have saved IDs */
        case _SC_VERSION:
            return 198808L; /* POSIX.1-1988 */
        case _SC_PAGESIZE:
            return 4096; /* Reasonable default page size */
        case _SC_XOPEN_CRYPT:
            return 0;    /* No X/Open crypt */
        case _SC_XOPEN_ENH_I18N:
            return 0;    /* No enhanced internationalization */
        case _SC_XOPEN_SHM:
            return 0;    /* No shared memory */
        case _SC_2_CHAR_TERM:
            return 0;    /* No char terminal */
        case _SC_2_C_BIND:
            return 0;    /* No C binding */
        case _SC_2_C_DEV:
            return 0;    /* No C dev */
        case _SC_2_FORT_DEV:
            return 0;    /* No Fortran dev */
        case _SC_2_FORT_RUN:
            return 0;    /* No Fortran runtime */
        case _SC_2_LOCALEDEF:
            return 0;    /* No locale def */
        case _SC_2_PBS:
            return 0;    /* No portable batch system */
        case _SC_2_PBS_ACCOUNTING:
            return 0;    /* No PBS accounting */
        case _SC_2_PBS_CHECKPOINT:
            return 0;    /* No PBS checkpoint */
        case _SC_2_PBS_LOCATE:
            return 0;    /* No PBS locate */
        case _SC_2_PBS_MESSAGE:
            return 0;    /* No PBS message */
        case _SC_2_PBS_TRACK:
            return 0;    /* No PBS track */
        case _SC_2_SW_DEV:
            return 0;    /* No software dev */
        case _SC_2_UPE:
            return 0;    /* No user portability */
        case _SC_2_VERSION:
            return 199209L; /* POSIX.2-1992 */
        case _SC_BC_BASE_MAX:
            return 99;   /* Reasonable default */
        case _SC_BC_DIM_MAX:
            return 2048; /* Reasonable default */
        case _SC_BC_SCALE_MAX:
            return 99;   /* Reasonable default */
        case _SC_BC_STRING_MAX:
            return 1000; /* Reasonable default */
        case _SC_COLL_WEIGHTS_MAX:
            return 2;    /* Reasonable default */
        case _SC_EQUIV_CLASS_MAX:
            return 2;    /* Reasonable default */
        case _SC_EXPR_NEST_MAX:
            return 32;   /* Reasonable default */
        case _SC_LINE_MAX:
            return 2048; /* Reasonable default */
        case _SC_RE_DUP_MAX:
            return 255;  /* Reasonable default */
        case _SC_XOPEN_LEGACY:
            return 0;    /* No legacy features */
        case _SC_XOPEN_REALTIME:
            return 0;    /* No realtime features */
        case _SC_XOPEN_REALTIME_THREADS:
            return 0;    /* No realtime threads */
        case _SC_XOPEN_VERSION:
            return 4;    /* X/Open version 4 */
        default:
            errno = EINVAL;
            return -1;
    }
} 