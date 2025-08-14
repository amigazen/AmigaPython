/* ------------------------------------------------------------------------

   unicodedatabase -- The Unicode 3.0 data base.

   Data was extracted from the Unicode 3.0 UnicodeData.txt file.

Written by Marc-Andre Lemburg (mal@lemburg.com).

Copyright (c) Corporation for National Research Initiatives.

   ------------------------------------------------------------------------ */

#include <exec/types.h>
#include <proto/dos.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stat.h>
#include "unicodedatabase.h"

/* --- Unicode category names --------------------------------------------- */

const char *_PyUnicode_CategoryNames[32] = {

    "Cn", /*  Not assigned (also see below) */

    /* Normative Categories */

    "Lu", /* Letter, Uppercase */
    "Ll", /* Letter, Lowercase */
    "Lt", /* Letter, Titlecase */
    "Mn", /* Mark, Non-Spacing */
    "Mc", /* Mark, Spacing Combining */
    "Me", /* Mark, Enclosing */
    "Nd", /* Number, Decimal Digit */
    "Nl", /* Number, Letter */
    "No", /* Number, Other */
    "Zs", /* Separator, Space */
    "Zl", /* Separator, Line */
    "Zp", /* Separator, Paragraph */
    "Cc", /* Other, Control */
    "Cf", /* Other, Format */
    "Cs", /* Other, Surrogate */
    "Co", /* Other, Private Use */
    "Cn", /* Other, Not Assigned (no characters in the file
	     have this property) */

     /* Informative Categories */

    "Lm", /* Letter, Modifier */
    "Lo", /* Letter, Other */
    "Pc", /* Punctuation, Connector */
    "Pd", /* Punctuation, Dash */
    "Ps", /* Punctuation, Open */
    "Pe", /* Punctuation, Close */
    "Pi", /* Punctuation, Initial quote (may behave like Ps or Pe 
	     depending on usage) */
    "Pf", /* Punctuation, Final quote (may behave like Ps or Pe 
	     depending on usage) */
    "Po", /* Punctuation, Other */
    "Sm", /* Symbol, Math */
    "Sc", /* Symbol, Currency */
    "Sk", /* Symbol, Modifier */
    "So", /* Symbol, Other */

    0  /* Sentinel */
};

const char *_PyUnicode_BidirectionalNames[21] = {

    "",	 /*  Undefined (added so that the 0 index maps to undefined) */
    
    "L", /* Left-to-Right */
    "LRE", /* Left-to-Right Embedding */
    "LRO", /* Left-to-Right Override */
    "R", /* Right-to-Left */
    "AL", /* Right-to-Left Arabic */
    "RLE", /* Right-to-Left Embedding */
    "RLO", /* Right-to-Left Override */
    "PDF", /* Pop Directional Format */
    "EN", /* European Number */
    "ES", /* European Number Separator */
    "ET", /* European Number Terminator */
    "AN", /* Arabic Number */
    "CS", /* Common Number Separator */
    "NSM", /* Non-Spacing Mark */
    "BN", /* Boundary Neutral */
    "B", /* Paragraph Separator */
    "S", /* Segment Separator */
    "WS", /* Whitespace */
    "ON", /* Other Neutrals */
    
    0  /* Sentinel */
};


/*** ORIGINAL:
const _PyUnicode_DatabaseRecord *_PyUnicode_Database[16] = {
    _PyUnicode_Database_0,
    _PyUnicode_Database_1,
    _PyUnicode_Database_2,
    _PyUnicode_Database_3,
    _PyUnicode_Database_4,
    _PyUnicode_Database_5,
    _PyUnicode_Database_6,
    _PyUnicode_Database_7,
    _PyUnicode_Database_8,
    _PyUnicode_Database_9,
    _PyUnicode_Database_10,
    _PyUnicode_Database_11,
    _PyUnicode_Database_12,
    _PyUnicode_Database_13,
    _PyUnicode_Database_14,
    _PyUnicode_Database_15,
};
***/

/*** On AMIGA: fill in later... ***/
const _PyUnicode_DatabaseRecord *_PyUnicode_Database[16];

int init_Amiga_unicodedatabase(const char *path)
{
	static _PyUnicode_DatabaseRecord *database = NULL;
	static char *strtable = NULL;

	if(database==NULL)
	{
		if(database = malloc(16 * 4096 * sizeof(_PyUnicode_DatabaseRecord)))
		{
			FILE *fh;
			char file[512];
			strcpy(file, path);
			AddPart(file, "Unicode_DB.data", sizeof(file));

			fh = fopen(file, "rb");
			if(fh)
			{
				int siz = fread(database, 1, sizeof(_PyUnicode_DatabaseRecord) * 4096 * 16, fh);
				fclose(fh);
				if(siz>0)
				{
					int i;

					for(i=0; i<16; i++)
					{
						_PyUnicode_Database[i] = 
							&database[i*4096];
					}

					
					strcpy(file, path);
					AddPart(file, "Unicode_DB.strings", sizeof(file));
					fh = fopen(file,"rb");
					if(fh)
					{
						struct stat s;
						int siz;
						stat(file, &s);
						if(strtable = malloc(s.st_size))
						{
							siz=fread(strtable,1,s.st_size,fh);
							fclose(fh);
							if(siz==s.st_size)
							{
								// FIXUP string pointers
								for(i=0; i<16; i++)
								{
									int j;
									for(j=0; j<4096; j++)
									{
										_PyUnicode_DatabaseRecord* rec = _PyUnicode_Database[i];
										rec += j;
										if(rec->decomposition)
											rec->decomposition = strtable + (unsigned int)rec->decomposition;
									}
								}
								return TRUE;
							}
						}
						else
							fclose(fh);
					}
				}
			}
			free(database);
			database=NULL;
		}
		return FALSE;
	}
	return TRUE;	
}
