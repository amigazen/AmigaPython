/*
 *      syslog.c - syslog function stubs for Amiga
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 */

#include <sys/param.h>
#include <sys/syslog.h>

#include <bsdsocket.h>
#include <libraries/bsdsocket.h>

/*
 * Initialize the connection to the system logger
 * ident - string that precedes every message
 * logstat - logging options (LOG_PID, LOG_CONS, LOG_NDELAY, LOG_NOWAIT)
 * logfac - default facility (LOG_KERN, LOG_USER, LOG_MAIL, etc.)
 */
void
openlog(const char *ident, int logstat, int logfac)
{
  /*
   * Note: SocketBaseTags() does value checking for the arguments
   * This requires bsdsocket.library
   */
  SocketBaseTags(SBTM_SETVAL(SBTC_LOGTAGPTR), ident,
                 SBTM_SETVAL(SBTC_LOGSTAT), logstat,
                 SBTM_SETVAL(SBTC_LOGFACILITY), logfac,
                 TAG_END);
}

/*
 * Close the connection to the system logger
 */
void
closelog(void)
{
  SocketBaseTags(SBTM_SETVAL(SBTC_LOGTAGPTR), NULL,
                 TAG_END);
}

/*
 * Set the log mask level
 * Returns the previous log mask
 */
int
setlogmask(int pmask)
{
  ULONG taglist[5];

  taglist[0] = SBTM_GETVAL(SBTC_LOGMASK);
  taglist[2] = SBTM_SETVAL(SBTC_LOGMASK);
  taglist[3] = pmask;
  taglist[4] = TAG_END;

  SocketBaseTagList((struct TagItem *)taglist);
  return (int)taglist[1];
}

/*
 * syslog() function is provided by bsdsocket.library
 * We don't need to implement it here - it's a direct library call
 */ 