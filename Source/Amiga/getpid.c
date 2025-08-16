/*
 *      getpid.c - get process ID (stub for FindTask(NULL))
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 */

#include <sys/types.h>
#include <exec/execbase.h>
extern struct ExecBase *SysBase;

/*
 * Get the process ID - on Amiga this is the pointer to the current task
 */
pid_t
getpid(void)
{
  return (pid_t)SysBase->ThisTask;
} 