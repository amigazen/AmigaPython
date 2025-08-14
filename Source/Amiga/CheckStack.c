/*
 *      CheckStack.c - Check available stack space for Amiga
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 */

/* AmigaOS version and stack requirements */
static const char *version = "$VER: Amiga Python 2.7.18b2 (16/07/2025)";
static const char *stack_req = "$STACK: 40000";

#ifdef __SASC
/*
 * Check if enough stack space is available
 * Returns 0 if enough stack is available, -1 if not
 */
int PyOS_CheckStack(void)
{
    /* Amiga SAS/C: Explicit check of available stack */
    extern unsigned long stackavail(void);
    extern long __STKNEED;
    
    if(stackavail() < __STKNEED) 
        return -1;
    return 0;
}
#elif defined(__VBCC__)
/*
 * Check if enough stack space is available 
 * Returns 0 if enough stack is available, -1 if not
 */
int PyOS_CheckStack(void)
{
    /* VBCC: Use __stack_check function and __stack_usage variable */
    /* The __stack_check function is called by VBCC when -stack-check is used */
    /* We can also check __stack_usage to see current stack usage */
    extern size_t __stack_usage;
    extern void __stack_check(size_t size);
    
    /* For VBCC, we check if we're approaching stack limits */
    /* Leave a safety margin of 8KB to prevent stack overflow */
    if (__stack_usage > (40000 - 8192))  /* 40KB stack - 8KB safety margin */
        return -1;
    return 0;
}
#else
/*
 * Generic stack check for other compilers
 * Returns 0 (assume enough stack space)
 */
int PyOS_CheckStack(void)
{
    return 0;
}
#endif 