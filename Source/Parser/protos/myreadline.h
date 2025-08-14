/* Prototypes for use with the Amiga SAS/C Compiler. */

#ifdef __SASC

/* myreadline.c */
static int my_fgets ( char *buf , int len , FILE *fp );
char *PyOS_StdioReadline ( char *prompt );

#endif
