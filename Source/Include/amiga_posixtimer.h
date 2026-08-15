/*
 * PosixLib blocks Amiga devices/timer.h (struct timeval clash).
 * Headers that pull intuition/workbench/datatypes still need the
 * timer device types (inputevent.h, prtbase.h). Include this before
 * Python.h / Amiga GUI headers.
 *
 * Matches NDK devices/timer.h under -D__USE_NEW_TIMEVAL__, plus a
 * legacy alias: older headers still write "struct timerequest".
 */
#ifndef AMIGA_POSIXTIMER_H
#define AMIGA_POSIXTIMER_H

#ifndef DEVICES_TIMER_H
#define DEVICES_TIMER_H
#endif

#include <exec/types.h>
#include <exec/io.h>

#ifndef __TIME_TYPES_DEFINED__
#define __TIME_TYPES_DEFINED__

struct TimeVal {
	ULONG tv_secs;
	ULONG tv_micro;
};

struct TimeRequest {
	struct IORequest tr_node;
	struct TimeVal tr_time;
};

typedef struct TimeVal TimeVal_Type;
typedef struct TimeRequest TimeRequest_Type;

#endif /* __TIME_TYPES_DEFINED__ */

/* prtbase.h and friends still use the pre-__USE_NEW_TIMEVAL__ name. */
#ifndef timerequest
#define timerequest TimeRequest
#endif

#endif /* AMIGA_POSIXTIMER_H */
