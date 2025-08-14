
/* Portable fmod(x, y) implementation for systems that don't have it */

/* This version adapted to Amiga FFP/IEEE roundoff errors */

#include <errno.h>
#include "config.h"


/* Only use this function for FFP or IEEE math */

#if defined(_FFP) || (defined(_IEEE) && !defined(_M68881))

double fmod(double x, double y)
{
        double f;

        if (y == 0.0) {
                errno = EDOM;
                return 0.0;
        }

        /* return f such that x = i*y + f for some integer i
           such that |f| < |y| and f has the same sign as x */

        f = x-y*floor(x/y);
                if((f!=0.0) && ((x<0.0)!=(y<0.0))) f-=y;
        return f;
}

#endif
