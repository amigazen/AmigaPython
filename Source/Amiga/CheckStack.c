#include "protos.h"

#ifdef __SASC
int PyOS_CheckStack (void)
{
	/* Amiga SAS/C: Explicit check of available stack */
	extern unsigned long stackavail(void);
	extern long __STKNEED;
	if(stackavail()<__STKNEED) return -1;
	return 0;
}
#endif /* __SASC */

