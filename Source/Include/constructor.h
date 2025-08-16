/*
 * constructor.h - Constructor macros for Amiga Python
 *
 * This file provides macros for defining constructor functions that
 * are called automatically when the program starts.
 *
 * Based on the original Amiga Python port constructor system
 * Updated for VBCC compatibility
 */

#ifndef CONSTRUCTOR_H
#define CONSTRUCTOR_H

/*
 * CONSTRUCTOR_P(name, stacksize) - Define a constructor function
 * 
 * This macro creates a function that will be called automatically
 * during program initialization. The stacksize parameter specifies
 * the stack size in bytes for the constructor function.
 * 
 * For VBCC compatibility, this creates a non-static function that can
 * be called from main(). The stacksize parameter is currently
 * ignored but kept for compatibility.
 */
#define CONSTRUCTOR_P(name, stacksize) \
    int name##_constructor(void)

/*
 * STDIO_CONSTRUCTOR(name) - Define a stdio constructor function
 * 
 * Similar to CONSTRUCTOR_P but specifically for stdio initialization.
 */
#define STDIO_CONSTRUCTOR(name) \
    int name##_constructor(void)

#endif /* CONSTRUCTOR_H */ 