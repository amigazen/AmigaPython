
/* Sigcheck is similar to intrcheck() but sets an exception when an
   interrupt occurs.  It can't be in the intrcheck.c file since that
   file (and the whole directory it is in) doesn't know about objects
   or exceptions.  It can't be in errors.c because it can be
   overridden (at link time) by a more powerful version implemented in
   signalmodule.c. */

#include "Python.h"

#ifdef __SASC
/* Replacement stack overflow routine for SAS/C */
void __stdargs _CXOVF(void)
{
	PyErr_SetString(PyExc_MemoryError,"too deep recursion");
}
#endif /* __SASC */

/* ARGSUSED */
int
PyErr_CheckSignals(void)
{
#ifdef __SASC
/* Amiga SAS/C: Explicit check of available stack */
extern unsigned long stackavail(void);
extern long __STKNEED;
if(stackavail()<__STKNEED) return _CXOVF(),-1;
#endif /* __SASC */
	if (!PyOS_InterruptOccurred())
		return 0;
	PyErr_SetNone(PyExc_KeyboardInterrupt);
	return -1;
}
