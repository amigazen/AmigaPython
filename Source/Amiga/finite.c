/*
 * _finite.c - Implementation of _finite function for AmigaOS
 * 
 * The _finite function returns true if a number is finite (not infinite or NaN).
 * Since _finite is not available in VBCC, we implement it using _isinf and _isnan.
 */

#include <math.h>

/*
 * _finite - Check if a double is finite
 * Returns: 1 if finite, 0 if infinite or NaN
 * 
 * This function is called by Py_IS_FINITE macro in PC/pyconfig.h
 */
int finite(double x)
{
    return !isinf(x) && !isnan(x);
}

/*
 * _finitef - Check if a float is finite (for completeness)
 * Returns: 1 if finite, 0 if infinite or NaN
 */
int finitef(float x)
{
    return !isinf(x) && !isnan(x);
} 