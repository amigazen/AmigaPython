/*
 *      fib.c - common fib buffer for stat() and other functions
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 */

/* DOS 3.0 and MuFS extensions to file info block */
#include "fibex.h"

/* Static FileInfoBlock used by various file functions */
struct FileInfoBlock __dostat_fib[1]; 