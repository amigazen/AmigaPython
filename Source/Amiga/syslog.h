/*
 * syslog.h - Additional syslog constants for AmigaOS
 * 
 * This file provides the missing constants that aren't in netinclude:sys/syslog.h
 * Based on Irmen de Jong's original Amiga port
 * Updated for Python 2.7.18 and VBCC compatibility
 */

#ifndef _SYSLOG_H
#define _SYSLOG_H

/* Facility codes - not provided by netinclude */
#define LOG_KERN     (0<<3)  /* kernel messages */
#define LOG_USER     (1<<3)  /* random user-level messages */
#define LOG_MAIL     (2<<3)  /* mail system */
#define LOG_DAEMON   (3<<3)  /* system daemons */
#define LOG_AUTH     (4<<3)  /* security/authorization messages */
#define LOG_SYSLOG   (5<<3)  /* messages generated internally by syslogd */
#define LOG_LPR      (6<<3)  /* line printer subsystem */
#define LOG_NEWS     (7<<3)  /* network news subsystem */
#define LOG_UUCP     (8<<3)  /* UUCP subsystem */
#define LOG_CRON     (9<<3)  /* clock daemon */
#define LOG_AUTHPRIV (10<<3) /* security/authorization messages (private) */
#define LOG_FTP      (11<<3) /* ftp daemon */

/* Reserved for local use */
#define LOG_LOCAL0   (16<<3) /* reserved for local use */
#define LOG_LOCAL1   (17<<3) /* reserved for local use */
#define LOG_LOCAL2   (18<<3) /* reserved for local use */
#define LOG_LOCAL3   (19<<3) /* reserved for local use */
#define LOG_LOCAL4   (20<<3) /* reserved for local use */
#define LOG_LOCAL5   (21<<3) /* reserved for local use */
#define LOG_LOCAL6   (22<<3) /* reserved for local use */
#define LOG_LOCAL7   (23<<3) /* reserved for local use */

/* Option flags for openlog() - not provided by netinclude */
#define LOG_PID      0x01    /* log the pid with each message */
#define LOG_CONS     0x02    /* log on the console if errors in sending */
#define LOG_ODELAY   0x04    /* delay open until first syslog() (default) */
#define LOG_NDELAY   0x08    /* don't delay open */
#define LOG_NOWAIT   0x10    /* don't wait for console forks: DEPRECATED */
#define LOG_PERROR   0x20    /* log to stderr as well */

/* Macros for constructing priority masks - not provided by netinclude */
#define LOG_MASK(pri)    (1 << (pri))           /* mask for one priority */
#define LOG_UPTO(pri)    ((1 << ((pri)+1)) - 1) /* all priorities through pri */

/* Function declarations */
void openlog(const char *ident, int logopt, int facility);
void closelog(void);
int setlogmask(int maskpri);
void syslog(int priority, const char *format, ...);

#endif /* _SYSLOG_H */ 