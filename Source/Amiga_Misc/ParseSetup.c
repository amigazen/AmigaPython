/******************************************************************

	Processor for Modules/Setup.in & Modules/Config.c.in,
	to generate appropriate Modules/Config.c file.
	Part of AmigaPython source distribution.

	Sorry but this program cannot be written in Python!
	(because people building Python need this program,
	and don't yet have a Python executable ofcourse).


	Written June 1998 by Irmen de Jong. (irmen@bigfoot.com)

******************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SETUPFILENAME "Setup"
#define CONFIGFILENAME_IN "config.c.in"
#define CONFIGFILENAME_OUT "config.c"

typedef enum { L_SKIP, L_MODULE, L_DEFINE, L_OPTION } lineType;

struct Node
{
	char *string;
	int line;
	struct Node *next;
};

static struct Node *moduleList = NULL;
static struct Node *moduleTail = NULL;
static struct Node *defineList = NULL;
static struct Node *defineTail = NULL;

static struct Node *protoList = NULL;
static struct Node *protoTail = NULL;
static struct Node *configentryList = NULL;
static struct Node *configentryTail = NULL;

static int lineNumber;

static char *setup_filename=SETUPFILENAME;
static char *configin_filename=CONFIGFILENAME_IN;
static char *configout_filename=CONFIGFILENAME_OUT;


/// Utility functions
static void addTail(struct Node **listhead, struct Node **listtail, struct Node *node)
{
	node->next = NULL;
	if(*listhead)
	{
		(*listtail)->next=node;
	}
	else
	{
		(*listhead)=node;
	}
	(*listtail)=node;
}

static void addStrToList(struct Node **listHead, struct Node **listTail, const char *string)
{
	struct Node *node;

	if(node=malloc(sizeof(struct Node)))
	{
		if(node->string = strdup(string))
		{
			node->line = lineNumber;
			addTail(listHead,listTail,node);
			return;
		}
		free(node);
	}

	puts("NO MEM");
}


static lineType classifyLine(char *string)
{
	switch (string[0])
	{
		case '#':
		case '\0':
		case '\n':
				return L_SKIP;
		case '*':
				return L_OPTION;
		default:
			{
				if(strchr(string,'='))
					return L_DEFINE;
				else
					return L_MODULE;
			}
	}
}

static void stripNL(char *string)
{
	int l;

	l=strlen(string)-1;
	if(l>=0)
	{
		if(string[l]=='\n')
			string[l]='\0';
	}
}

static void stripComment(char *moduleline)
{
	char *pos;

	pos=strchr(moduleline,'#');
	if(pos)
	{
		// strip comment and preceding tabs/spaces
		*pos=' ';
		while(pos>=moduleline && *pos==' ' || *pos=='\t')
		{
			*pos='\0';
			pos--;
		}
	}
}
///

/// Process functions
static int processModules(void)
{
	struct Node *node;
	char *moduleName;
	char moduleSources[BUFSIZ];
	char buf[BUFSIZ];

	moduleSources[0]='\0';
	node=moduleList;
	while(node)
	{
		char *ptr;
		moduleSources[0]='\0';
		ptr = strtok(node->string," \t");
		if(ptr)
		{
			moduleName=ptr;
			ptr=strtok(NULL," \t");
			while(ptr)
			{
				int i;
				i=strlen(ptr);
				// if it is a .c file, add it to the sources string
				// otherwise, break the loop
				if(i>2 && (ptr[i-2]=='.' && (ptr[i-1]=='c' || ptr[i-1]=='C')))
				{
					strcat(moduleSources," ");
					strcat(moduleSources,ptr);
					ptr=strtok(NULL," \t");
				}
				else break;
			}

			// generate prototype
			sprintf(buf,"void init%s(void);\n",moduleName);
			addStrToList(&protoList, &protoTail, buf);
			// generate configentry
			sprintf(buf,"\t{ \"%s\", init%s },\n", moduleName, moduleName);
			addStrToList(&configentryList, &configentryTail, buf);
		}

		node=node->next;
	}

	// Read config and add lines

	// open CONFIGFILENAME_IN as source
	// open CONFIGFILENAME_OUT as destination
	// copy lines until MARKER 1
	// write all protos:
/*
	node=protoList;
	while(node)
	{
		fprintf(outfile,node->string);
		node=node->next;
	}
*/
	// copy lines until MARKER 2
	// write all configentries:
/*
	node=configentryList;
	while(node)
	{
		fprintf(outfile,node->string);
		node=node->next;
	}
*/
	// copy remaining lines

	return 1;
}

static int processDefines(void)
{
	struct Node *node;

	node=defineList;
	while(node)
	{
		printf("%3d:DEFINE: %s\n",node->line,node->string);
		node=node->next;
	}

	return 1;
}
///

/// Main
int main(int argc, char **argv)
{
	FILE *fh;
	int RC=10;
	char readbuf[BUFSIZ];

	argc--; argv++;
	if(argc>0)
	{
		setup_filename = *argv;
		argc--; argv++;
	}
	if(argc>0)
	{
		configin_filename = *argv;
		argc--; argv++;
	}
	if(argc>0)
	{
		configout_filename = *argv;
		argc--; argv++;
	}

	printf("Files: Setup=%s, Config in=%s, Config out=%s\n",
		setup_filename,configin_filename,configout_filename);

	if(fh=fopen(setup_filename,"r"))
	{
		int skip=0;

		lineNumber=1;

		while(fgets(readbuf,BUFSIZ,fh)!=NULL)
		{
			stripNL(readbuf);
			switch (classifyLine(readbuf))
			{
			case L_SKIP:
				// skip this line
				break;
			case L_MODULE:
				if(!skip)
				{
					stripComment(readbuf);
					addStrToList(&moduleList, &moduleTail, readbuf);
				}
				break;
			case L_DEFINE:
				if(!skip)
				{
					addStrToList(&defineList, &defineTail, readbuf);
				}
				break;
			case L_OPTION:
				if(strcmp(readbuf,"*static*")==NULL) skip=0;
				if(strcmp(readbuf,"*doconfig*")==NULL) skip=0;
				if(strcmp(readbuf,"*shared*")==NULL) skip=1;
				if(strcmp(readbuf,"*noconfig*")==NULL) skip=1;
				break;
			}

			lineNumber++;
		}

		fclose(fh);

		if(processDefines() && processModules())
			RC=0;
	}
	else
	{
		printf("Cannot read `%s'\n",setup_filename);
	}

	return RC;
}
///

