#include "config.h"
#include "myproto.h"

#ifdef macintosh
#include "macbuildno.h"
#endif

#include <stdio.h>

#ifndef DATE
#ifdef __DATE__
#define DATE __DATE__
#else
#define DATE "xx/xx/xx"
#endif
#endif

#ifndef TIME
#ifdef __TIME__
#define TIME __TIME__
#else
#define TIME "xx:xx:xx"
#endif
#endif

#ifndef BUILD
#define BUILD 0
#endif


const char *
Py_GetBuildInfo Py_PROTO((void))
{
	static char buildinfo[50];
	sprintf(buildinfo, "#%d, %.20s, %.9s", BUILD, DATE, TIME);
	return buildinfo;
}
