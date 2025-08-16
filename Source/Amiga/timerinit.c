/***********************************************************************
 *
 * Timer init and timer related functions for Amiga. Timer is important
 * for Python's time module and other time-related functions.
 *
 * Based on the original Amiga port by Irmen de Jong.
 * Updated for Python 2.7.18.
 *
 ***********************************************************************/

#include <proto/timer.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <devices/timer.h>
#include <exec/types.h>
#include <time.h>

/* Keep track of timer state */
static struct MsgPort *TimerMP = NULL;
static struct timerequest *TimerIO = NULL;
static int timer_open = 0;

/* Open the timer device */
int init_timer(void)
{
    if (timer_open) 
        return 1;  /* Already initialized */

    /* Create the message port */
    TimerMP = CreateMsgPort();
    if (!TimerMP)
        return 0;

    /* Create the timerequest */
    TimerIO = (struct timerequest *)CreateIORequest(TimerMP, sizeof(struct timerequest));
    if (!TimerIO) {
        DeleteMsgPort(TimerMP);
        TimerMP = NULL;
        return 0;
    }

    /* Open the timer device */
    if (OpenDevice(TIMERNAME, UNIT_MICROHZ, (struct IORequest *)TimerIO, 0) != 0) {
        DeleteIORequest((struct IORequest *)TimerIO);
        TimerIO = NULL;
        DeleteMsgPort(TimerMP);
        TimerMP = NULL;
        return 0;
    }

    timer_open = 1;
    return 1;
}

/* Close the timer device */
void done_timer(void)
{
    if (!timer_open)
        return;

    CloseDevice((struct IORequest *)TimerIO);
    DeleteIORequest((struct IORequest *)TimerIO);
    DeleteMsgPort(TimerMP);

    TimerIO = NULL;
    TimerMP = NULL;
    timer_open = 0;
}

/* Get current system time in seconds and microseconds */
int gettimeofday(struct timeval *tp, void *tzp)
{
    if (!timer_open && !init_timer())
        return -1;

    /* Get the current time */
    TimerIO->tr_node.io_Command = TR_GETSYSTIME;
    DoIO((struct IORequest *)TimerIO);
    
    tp->tv_secs = TimerIO->tr_time.tv_secs;
    tp->tv_micro = TimerIO->tr_time.tv_micro;

    /* Ignore timezone information */
    if (tzp) {
        struct timezone *tz = (struct timezone *)tzp;
        tz->tz_minuteswest = 0;
        tz->tz_dsttime = 0;
    }

    return 0;
}

/* Sleep for a specified number of seconds */
int sleep(unsigned int seconds)
{
    if (!timer_open && !init_timer())
        return -1;

    /* Set up delay time */
    TimerIO->tr_node.io_Command = TR_ADDREQUEST;
    TimerIO->tr_time.tv_secs = seconds;
    TimerIO->tr_time.tv_micro = 0;

    /* Wait for the timer to complete */
    DoIO((struct IORequest *)TimerIO);

    return 0;
}

/* Sleep for a specified number of microseconds */
int usleep(unsigned long microseconds)
{
    if (!timer_open && !init_timer())
        return -1;

    /* Set up delay time */
    TimerIO->tr_node.io_Command = TR_ADDREQUEST;
    TimerIO->tr_time.tv_secs = microseconds / 1000000;
    TimerIO->tr_time.tv_micro = microseconds % 1000000;

    /* Wait for the timer to complete */
    DoIO((struct IORequest *)TimerIO);

    return 0;
} 