/*
**  Environment functions. 
**
**  getenv, putenv, setenv, unsetenv.
**
**  Replace the SAS/C implementations of these functions by shorter,
**  better ones. (IMHO)
**
**  June 12, 1996: fixed zero-length variables
**  November 7, 1996: replaced static buffer by buffer on stack
**
**      © Irmen de Jong
**      <irmen@bigfoot.com>
*/

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <proto/dos.h>
#include <dos/dos.h>

#define BUFFER_SIZE 256

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

static const char *fixEqualStart(const char *str)
{
	if(str[0]=='=') return str+1;   /* unix compatibility */
	return str;
}

char *getenv(const char *var)
{
	char *val;
	LONG len;
	char buf[BUFFER_SIZE];

	var=fixEqualEnd(var,buf);

	len=GetVar(var,buf,200,GVF_GLOBAL_ONLY);        /* ENV: ONLY!! */
	if(len>=0)
	{
		if(val=malloc(len+1))
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
		DeleteVar(name,GVF_GLOBAL_ONLY);
		if(SetVar(name,value,-1,GVF_GLOBAL_ONLY)) return 0;
		errno=ENOMEM;
		return -1;
	}
	
	/* Don't overwrite if var is already set */
	if(GetVar(name,lbuf,20,GVF_GLOBAL_ONLY)>=0) return 0;

	/* Not set, do it now */
	if(SetVar(name,value,-1,GVF_GLOBAL_ONLY)) return 0;
	errno=ENOMEM;
	return -1;
}

/* returns 0 if OK, nonzero if fail */
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
	if(value = strchr(buf,'='))
	{
		*value=0;
		return setenv(buf,value+1,1);
	}
	errno=EINVAL;
	return -1;
}

/* silently remove environment var name */
void unsetenv(const char *name)
{
	char buf[BUFFER_SIZE];
	name=fixEqualEnd(name,buf);
	DeleteVar(name,GVF_GLOBAL_ONLY);        /* ENV: ONLY! */
}

