/*
 *      strftime.c - date/time formatting for Amiga
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 *
 *      Originally based on Berkeley code:
 *      Copyright (c) 1989 The Regents of the University of California.
 *      All rights reserved.
 */

#define TM_YEAR_BASE 1900

#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <exec/types.h>
#include <libraries/locale.h>
#include <proto/locale.h>
#include <proto/exec.h>
#include <stdlib.h>

char *tzname[2] = { "GMT", "GMT" };  /* Defaults */

/* Timezone variables for compatibility with timemodule.c */
long timezone = 0;  /* Timezone offset in seconds */
int daylight = 0;   /* Daylight saving time flag */

/* PosixLib timezone variables */
extern long __gmtoffset;
extern int __dstflag;

static void amiga_init_tzname(void) {
    struct Locale *locale;
    struct Library *LocaleBase = NULL;

    /* Initialize PosixLib timezone variables */
    tzset();
    
    /* Initialize timezone variables from PosixLib */
    timezone = __gmtoffset;
    daylight = __dstflag;

    LocaleBase = OpenLibrary("locale.library", 38);
    if (LocaleBase) {
        locale = OpenLocale(NULL);
        if (locale) {
            if (locale->loc_LocaleName && *locale->loc_LocaleName)
                tzname[0] = (char *)locale->loc_LocaleName;
            else if (locale->loc_LanguageName && *locale->loc_LanguageName)
                tzname[0] = (char *)locale->loc_LanguageName;
            tzname[1] = tzname[0];
            CloseLocale(locale);
        }
        CloseLibrary(LocaleBase);
    }
}

/* Month and day names */
static char *afmt[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
};
static char *Afmt[] = {
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday",
    "Saturday",
};
static char *bfmt[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep",
    "Oct", "Nov", "Dec",
};
static char *Bfmt[] = {
    "January", "February", "March", "April", "May", "June", "July",
    "August", "September", "October", "November", "December",
};

/* Forward declarations */
static size_t _fmt(const char *, const struct tm *);
static int _conv(int, int, char);
static int _add(char *);

/* User data for formatter */
static struct user
{
    char *u_pt;
    size_t u_gsize;
} u;

/*
 * Format a time value according to the specified format string
 * and time value. Return number of characters placed in the buffer.
 */
size_t strftime(char *s, size_t maxsize, const char *format, const struct tm *t)
{
    size_t res = 0;
    
    amiga_init_tzname();

    u.u_pt = s;
    if ((u.u_gsize = maxsize) < 1)
        return 0;
    if (_fmt(format, t)) {
        *u.u_pt = '\0';
        res = maxsize - u.u_gsize;
    }
    return res;
}

/*
 * Internal format function that interprets format strings
 */
static size_t _fmt(const char *format, const struct tm *t)
{
    for (; *format; ++format) {
        if (*format == '%')
            switch (*++format) {
            case '\0':
                --format;
                break;
            case 'A':
                if (t->tm_wday < 0 || t->tm_wday > 6)
                    return(0);
                if (!_add(Afmt[t->tm_wday]))
                    return(0);
                continue;
            case 'a':
                if (t->tm_wday < 0 || t->tm_wday > 6)
                    return(0);
                if (!_add(afmt[t->tm_wday]))
                    return(0);
                continue;
            case 'B':
                if (t->tm_mon < 0 || t->tm_mon > 11)
                    return(0);
                if (!_add(Bfmt[t->tm_mon]))
                    return(0);
                continue;
            case 'b':
            case 'h':
                if (t->tm_mon < 0 || t->tm_mon > 11)
                    return(0);
                if (!_add(bfmt[t->tm_mon]))
                    return(0);
                continue;
            case 'C':
                if (!_fmt("%a %b %e %H:%M:%S %Y", t))
                    return(0);
                continue;
            case 'c':
                if (!_fmt("%m/%d/%y %H:%M:%S", t))
                    return(0);
                continue;
            case 'e':
                if (!_conv(t->tm_mday, 2, ' '))
                    return(0);
                continue;
            case 'D':
                if (!_fmt("%m/%d/%y", t))
                    return(0);
                continue;
            case 'd':
                if (!_conv(t->tm_mday, 2, '0'))
                    return(0);
                continue;
            case 'H':
                if (!_conv(t->tm_hour, 2, '0'))
                    return(0);
                continue;
            case 'I':
                if (!_conv(t->tm_hour % 12 ?
                    t->tm_hour % 12 : 12, 2, '0'))
                    return(0);
                continue;
            case 'j':
                if (!_conv(t->tm_yday + 1, 3, '0'))
                    return(0);
                continue;
            case 'k':
                if (!_conv(t->tm_hour, 2, ' '))
                    return(0);
                continue;
            case 'l':
                if (!_conv(t->tm_hour % 12 ?
                    t->tm_hour % 12 : 12, 2, ' '))
                    return(0);
                continue;
            case 'M':
                if (!_conv(t->tm_min, 2, '0'))
                    return(0);
                continue;
            case 'm':
                if (!_conv(t->tm_mon + 1, 2, '0'))
                    return(0);
                continue;
            case 'n':
                if (!_add("\n"))
                    return(0);
                continue;
            case 'p':
                if (!_add(t->tm_hour >= 12 ? "PM" : "AM"))
                    return(0);
                continue;
            case 'R':
                if (!_fmt("%H:%M", t))
                    return(0);
                continue;
            case 'r':
                if (!_fmt("%I:%M:%S %p", t))
                    return(0);
                continue;
            case 'S':
                if (!_conv(t->tm_sec, 2, '0'))
                    return(0);
                continue;
            case 'T':
            case 'X':
                if (!_fmt("%H:%M:%S", t))
                    return(0);
                continue;
            case 't':
                if (!_add("\t"))
                    return(0);
                continue;
            case 'U':
                if (!_conv((t->tm_yday + 7 - t->tm_wday) / 7,
                    2, '0'))
                    return(0);
                continue;
            case 'W':
                if (!_conv((t->tm_yday + 7 -
                    (t->tm_wday ? (t->tm_wday - 1) : 6))
                    / 7, 2, '0'))
                    return(0);
                continue;
            case 'w':
                if (!_conv(t->tm_wday, 1, '0'))
                    return(0);
                continue;
            case 'x':
                if (!_fmt("%m/%d/%y", t))
                    return(0);
                continue;
            case 'y':
                if (!_conv((t->tm_year + TM_YEAR_BASE)
                    % 100, 2, '0'))
                    return(0);
                continue;
            case 'Y':
                if (!_conv(t->tm_year + TM_YEAR_BASE, 4, '0'))
                    return(0);
                continue;
            case 'Z':
                if (!_add(tzname[0]))
                    return(0);
                continue;
            case '%':
            default:
                break;
        }
        if (!u.u_gsize--)
            return(0);
        *u.u_pt++ = *format;
    }
    return(u.u_gsize);
}

/*
 * Convert value to string with specified digits and padding
 */
static int _conv(int n, int digits, char pad)
{
    char buf[10];
    register char *p;

    for (p = buf + sizeof(buf) - 2; n > 0 && p > buf; n /= 10, --digits)
        *p-- = n % 10 + '0';
    while (p > buf && digits-- > 0)
        *p-- = pad;
    return(_add(++p));
}

/*
 * Add a string to the output buffer
 */
static int _add(char *str)
{
    for (; *str; ++str, --u.u_gsize) {
        if (!u.u_gsize)
            return(0);
        *u.u_pt++ = *str;
    }
    return(1);
} 