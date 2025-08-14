
/* Module configuration */

/* This file contains the table of built-in modules.
   See init_builtin() in import.c. */

/**** I.J. 11/11/1995 ***/
/* On the Amiga, you'll have to edit this file by hand to set up the *
/* modules configuration (sorry)                       */
/* Updated  8-dec-96 for Python 1.4                    */
/* Updated 12-jan-98 for Python 1.5                    */
/* Updated 25-dec-98 for I-Net225                      */
/* Updated 25-apr-99 for Python 1.5.2                  */
/*                (added sha, removed timing)          */
/* Updated 16-okt-99 for 'network free' version        */
/*******************************************************/


#include "Python.h"

#if defined(AMITCP) || defined(INET225)

/* special 'safe' init functions - they check library availabilities */

static void initpwd_check(void)
{
	if(!checkusergrouplib()) return;
	initpwd();
}

static void initgrp_check(void)
{
	if(!checkusergrouplib()) return;
	initgrp();
}

static void initcrypt_check(void)
{
	if(!checkusergrouplib()) return;
	initcrypt();	
}

static void initsyslog_check(void)
{
	if(!checksocketlib()) return;
	initsyslog();
}

static void initsocket_check(void)
{
	if(!checksocketlib()) return;
	initsocket();
}

static void initselect_check(void)
{
	if(!checksocketlib()) return;
	initselect();
}

#endif /* AmiTCP or Inet225 */


struct _inittab _PyImport_Inittab[] = {

/************ HERE YOU NEED TO INSTALL ALL DESIRED MODULES!!!! **************/

	{"amiga",initamiga},
	{"ARexxll",initARexx},
//	{"Doslib", initDoslib },
	{"amiga_exec", initamiga_exec },
//	{"simplegfx", initsimplegfx },  // XXX experimental

//	{"array",initarray},
//	{"binascii",initbinascii}, 
//	{"cmath",initcmath},
	{"math",initmath},
//	{"new",initnew},
	{"errno",initerrno},
	{"environment",initenvironment},
//	{"regex",initregex},
//	{"pcre",initpcre},
	{"strop",initstrop},
//	{"struct",initstruct},
	{"time",inittime},
//	{"timing",inittiming},		// XXX obsolete
//	{"md5",initmd5}, 
//	{"soundex",initsoundex},
//	{"rotor",initrotor},
	{"operator",initoperator},
//	{"cStringIO",initcStringIO},
//	{"cPickle",initcPickle},
//	{"sha",initsha},
//	{"zlib",PyInit_zlib},

#if defined(AMITCP) || defined(INET225)
	/* Use the lib-checking init functions defined above */
	{"pwd",initpwd_check},
	{"grp",initgrp_check},
	{"crypt",initcrypt_check},
	{"select",initselect_check},
	{"socket",initsocket_check},
	{"syslog",initsyslog_check},
#endif

/*  {"signal",initsignal}, */

/****************************************************** I.J. 10/12/1996 *****/

	/* This module "lives in" with marshal.c */
	{"marshal", PyMarshal_Init},

	/* This lives in with import.c */
	{"imp", initimp},

	/* These entries are here for sys.builtin_module_names */
	{"__main__", NULL},
	{"__builtin__", NULL},
	{"sys", NULL},

	/* Sentinel */
	{0, 0}
};
