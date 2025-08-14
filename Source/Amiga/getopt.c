/*
 *      getopt.c - Unix compatible command line option parsing
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 *
 *      This is AT&T public domain source for getopt(3) from the 1985
 *      UNIFORUM conference in Dallas.
 *
 *      DEPRECATED: This file is deprecated in Amiga Python 2.7.18 in favour of vbcc PosixLib
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * Error reporting macro
 */
#define ERR(s, c)\
  if(opterr) { fprintf(stderr, "%s%s%c\n", argv[0], s, c); }

/* Global variables used by getopt */
int	opterr = 1;
int	optind = 1;
int	optopt;
char	*optarg;

/*
 * Parse command-line options
 * 
 * argc - argument count
 * argv - argument vector
 * opts - string of valid option characters, colons indicate options with arguments
 * 
 * Returns the next option character, EOF when done, or '?' for errors
 */
int
getopt(int argc, char *argv[], const char *opts)
{
	static int sp = 1;
	register int c;
	register char *cp;

	if(sp == 1)
		if(optind >= argc ||
		   argv[optind][0] != '-' || argv[optind][1] == '\0')
			return(EOF);
		else if(strcmp(argv[optind], "--") == NULL) {
			optind++;
			return(EOF);
		}
	optopt = c = argv[optind][sp];
	if(c == ':' || (cp=strchr(opts, c)) == NULL) {
		ERR(": illegal option -- ", c);
		if(argv[optind][++sp] == '\0') {
			optind++;
			sp = 1;
		}
		return('?');
	}
	if(*++cp == ':') {
		if(argv[optind][sp+1] != '\0')
			optarg = &argv[optind++][sp+1];
		else if(++optind >= argc) {
			ERR(": option requires an argument -- ", c);
			sp = 1;
			return('?');
		} else
			optarg = argv[optind++];
		sp = 1;
	} else {
		if(argv[optind][++sp] == '\0') {
			sp = 1;
			optind++;
		}
		optarg = NULL;
	}
	return(c);
} 