/*
 * int64_ops.c - Implementation of 64-bit integer operations for AmigaOS
 * 
 * This file provides the implementation of arithmetic operations for our
 * custom amiga_int64_t type, which is used to emulate long long support on
 * AmigaOS systems that don't have native 64-bit integer support.
 */

#include "Python.h"

#ifdef _AMIGA

/* Add two amiga_int64_t values */
amiga_int64_t amiga_int64_add(amiga_int64_t a, amiga_int64_t b)
{
    amiga_int64_t result;
    unsigned long carry = 0;
    
    result.low = a.low + b.low;
    if (result.low < a.low) carry = 1;
    result.high = a.high + b.high + carry;
    
    return result;
}

/* Subtract two amiga_int64_t values */
amiga_int64_t amiga_int64_sub(amiga_int64_t a, amiga_int64_t b)
{
    amiga_int64_t result;
    unsigned long borrow = 0;
    
    result.low = a.low - b.low;
    if (result.low > a.low) borrow = 1;
    result.high = a.high - b.high - borrow;
    
    return result;
}

/* Compare two amiga_int64_t values */
int amiga_int64_cmp(amiga_int64_t a, amiga_int64_t b)
{
    if (a.high < b.high) return -1;
    if (a.high > b.high) return 1;
    if (a.low < b.low) return -1;
    if (a.low > b.low) return 1;
    return 0;
}

/* Convert long to amiga_int64_t */
amiga_int64_t amiga_long_to_int64(long val)
{
    amiga_int64_t result;
    if (val >= 0) {
        result.high = 0;
        result.low = (unsigned long)val;
    } else {
        result.high = 0xFFFFFFFF;
        result.low = (unsigned long)val;
    }
    return result;
}

/* Convert amiga_int64_t to long */
long amiga_int64_to_long(amiga_int64_t val)
{
    return (long)val.low;
}

/* Create amiga_int64_t from high and low parts */
amiga_int64_t amiga_make_int64(unsigned long high, unsigned long low)
{
    amiga_int64_t result;
    result.high = high;
    result.low = low;
    return result;
}

/* Shift right amiga_int64_t by specified number of bits */
amiga_int64_t amiga_int64_shr(amiga_int64_t val, int shift)
{
    amiga_int64_t result;
    if (shift >= 64) {
        result.high = 0;
        result.low = 0;
    } else if (shift >= 32) {
        result.high = 0;
        result.low = val.high >> (shift - 32);
    } else {
        result.high = val.high >> shift;
        result.low = (val.high << (32 - shift)) | (val.low >> shift);
    }
    return result;
}

/* Shift left amiga_int64_t by specified number of bits */
amiga_int64_t amiga_int64_shl(amiga_int64_t val, int shift)
{
    amiga_int64_t result;
    if (shift >= 64) {
        result.high = 0;
        result.low = 0;
    } else if (shift >= 32) {
        result.high = val.low << (shift - 32);
        result.low = 0;
    } else {
        result.high = (val.high << shift) | (val.low >> (32 - shift));
        result.low = val.low << shift;
    }
    return result;
}

#endif /* _AMIGA */ 