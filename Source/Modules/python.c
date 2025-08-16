/* Minimal main program -- everything is loaded from the library */

#include "Python.h"

#ifdef __FreeBSD__
#include <fenv.h>
#endif

/* Amiga constructor function declarations */
extern int WBArgParse_constructor(void);
extern int dosio_init_constructor(void);

int
main(int argc, char **argv)
{
	/* 754 requires that FP exceptions run in "no stop" mode by default,
	 * and until C vendors implement C99's ways to control FP exceptions,
	 * Python requires non-stop mode.  Alas, some platforms enable FP
	 * exceptions by default.  Here we disable them.
	 */
#ifdef __FreeBSD__
	fedisableexcept(FE_OVERFLOW);
#endif

	/* Call Amiga constructor functions before Python initialization */
#ifdef _AMIGA
	WBArgParse_constructor();
	dosio_init_constructor();
#endif

	return Py_Main(argc, argv);
}
