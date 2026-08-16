/* Minimal main program -- everything is loaded from the library */

#include "Python.h"

#ifdef __FreeBSD__
#include <fenv.h>
#endif

#ifdef _AMIGA
#include "../Amiga/wbconsole.h"
/* Amiga constructor function declarations */
extern int WBArgParse_constructor(void);
extern int dosio_init_constructor(void);
extern int locale_lib_init_constructor(void);
#endif

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

#ifdef _AMIGA
	/* Workbench: claim WBStartup (GetMsg) + open CON: before stdio use.
	 * Shell: amiga_wb_prepare is a no-op and keeps argc/argv.
	 */
	locale_lib_init_constructor();
	dosio_init_constructor();
	if (amiga_wb_prepare(&argc, &argv) != 0) {
		amiga_wb_cleanup();
		return 20;
	}
#endif

	{
		int st;

		st = Py_Main(argc, argv);
#ifdef _AMIGA
		amiga_wb_cleanup();
#endif
		return st;
	}
}
