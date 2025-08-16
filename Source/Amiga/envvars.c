/*
 *      envvars.c - Environment variable functions for Amiga
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 *
 *      getenv, putenv, setenv, unsetenv implementations for Amiga.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <proto/dos.h>
#include <dos/dos.h>

#define BUFFER_SIZE 256

/*
 * Helper function to fix environment variable names ending with '='
 * (for Unix compatibility)
 */
static const char *fixEqualEnd(const char *str, char *buf)
{
    LONG len=strlen(str);

    if(str[len-1]=='=')
    {
        strcpy(buf,str);
        buf[len-1]=0; /* unix compatibility */
        return buf;
    }
    return str;
}

/*
 * Helper function to fix environment variable values starting with '='
 * (for Unix compatibility)
 */
static const char *fixEqualStart(const char *str)
{
    if(str[0]=='=') return str+1;   /* unix compatibility */
    return str;
}

/*
 * Get environment variable value
 * Returns allocated string that must be freed by the caller
 */
char *getenv(const char *var)
{
    char *val;
    LONG len;
    char buf[BUFFER_SIZE];

    var=fixEqualEnd(var,buf);

    len=GetVar((STRPTR)var,(STRPTR)buf,200,GVF_GLOBAL_ONLY);  /* ENV: ONLY!! */
    if(len>=0)
    {
        if((val=malloc(len+1)))
        {
            strcpy(val,buf);
            return val;
        }
        else errno=ENOMEM;
    }

    return NULL;
}

#ifdef getenv
#undef getenv
char *getenv(const char* var) { return __getenv(var); } /* other getenv */
#endif

/*
 * Set environment variable, with option to overwrite existing value
 * Returns 0 on success, -1 on failure
 */
int setenv(const char *name, const char *value, int overwrite)
{
    char lbuf[20];
    char buf[BUFFER_SIZE];

    name=fixEqualEnd(name,buf);
    value=fixEqualStart(value);

    if(!name || !value)
    {
        errno=EINVAL;
        return -1;
    }

    if(overwrite)
    {
        /* Delete global instance first */
        DeleteVar((STRPTR)name,GVF_GLOBAL_ONLY);
        if(SetVar((STRPTR)name,(STRPTR)value,-1,GVF_GLOBAL_ONLY)) return 0;
        errno=ENOMEM;
        return -1;
    }
    
    /* Don't overwrite if var is already set */
    if(GetVar((STRPTR)name,(STRPTR)lbuf,20,GVF_GLOBAL_ONLY)>=0) return 0;

    /* Not set, do it now */
    if(SetVar((STRPTR)name,(STRPTR)value,-1,GVF_GLOBAL_ONLY)) return 0;
    errno=ENOMEM;
    return -1;
}

/*
 * Set environment variable in format "NAME=VALUE"
 * Returns 0 on success, -1 on failure
 */
int putenv(const char *string)
{
    char buf[BUFFER_SIZE];
    char *value;
    
    if(strlen(string)>=BUFFER_SIZE)
    {
        errno=ENOMEM;
        return -1;
    }

    strcpy(buf,string);
    if((value = strchr(buf,'=')))
    {
        *value=0;
        return setenv(buf,value+1,1);
    }
    errno=EINVAL;
    return -1;
}

/*
 * Remove environment variable
 */
void unsetenv(const char *name)
{
    char buf[BUFFER_SIZE];
    name=fixEqualEnd(name,buf);
    DeleteVar((STRPTR)name,GVF_GLOBAL_ONLY);  /* ENV: ONLY! */
} 