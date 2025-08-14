/*
 * VBCC-compatible ios1.h header
 * 
 * This header provides the UFB (Unix File Block) structure and related
 * definitions that are normally provided by SAS/C's ios1.h but are
 * missing in VBCC.
 * 
 * For VBCC builds, we use a simplified UFB structure that provides
 * the basic functionality needed by the Amiga Python port.
 */

#ifndef IOS1_H
#define IOS1_H

#include <dos/dos.h>
#include <errno.h>

/* UFB (Unix File Block) structure for VBCC */
struct UFB {
    struct UFB *ufbnxt;    /* Next UFB in chain */
    int ufbflg;            /* File flags */
    BPTR ufbfh;            /* File handle */
    char *ufbfn;           /* File name */
    long ufbpos;           /* File position */
    int ufbsiz;            /* Buffer size */
    int ufbgrow;           /* Grow size */
};

/* UFB flags */
#define UFB_RA      0x0001  /* Read access */
#define UFB_WA      0x0002  /* Write access */
#define UFB_APP     0x0004  /* Append mode */
#define UFB_XLAT    0x0008  /* Translation mode (CR-LF <-> LF) */
#define UFB_TEMP    0x0010  /* Temporary file */
#define UFB_NC      0x0020  /* No close */
#define UFB_CLO     0x0040  /* Close on exec */
#define UFB_SOCK    0x0080  /* Socket file descriptor */
#define UFB_ALLOC   0x0100  /* UFB is allocated */

/* Global variables that should be provided by the runtime */
extern struct UFB *__ufbs;     /* First UFB in chain */
extern int __nufbs;            /* Number of UFBs */

/* Function prototypes */
struct UFB *__chkufb(int fd);
struct UFB *__allocufb(int *fdp);
void __chkabort(void);

/* Error handling */
extern int _OSERR;

#endif /* IOS1_H */ 