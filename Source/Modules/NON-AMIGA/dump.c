#include <stdio.h>
#include <string.h>

typedef struct {
	const unsigned char category;   /* index into
					   _PyUnicode_CategoryNames */
	const unsigned char combining;  /* combining class value 0 - 255 */
	const unsigned char bidirectional;  /* index into
					   _PyUnicode_BidirectionalNames */
	const unsigned char mirrored;   /* true if mirrored in bidir mode */
	const char *decomposition;      /* pointer to the decomposition
					   string or NULL */
} _PyUnicode_DatabaseRecord;


extern const _PyUnicode_DatabaseRecord *_PyUnicode_Database[16];


static char stringtable[200000];
static char *stringOffset = stringtable;

static void DumpBlock(FILE* fh, const _PyUnicode_DatabaseRecord * block)
{
	int i;
	static int recordNum = 0;

	for(i=0; i< 4096; i++)
	{
		_PyUnicode_DatabaseRecord copy = block[i];
		if(copy.decomposition)
		{
			strcpy(stringOffset, copy.decomposition);
			copy.decomposition = (const char*)(stringOffset - stringtable);
			stringOffset += strlen(stringOffset)+1;
		}
		fwrite(&copy, sizeof(_PyUnicode_DatabaseRecord), 1, fh);
		recordNum++;
	}
}

int main(void)
{
	int i;
	FILE *fh;
	unsigned int blocklen = 4096 * sizeof(_PyUnicode_DatabaseRecord);

	/* write 16 data blocks: */
	puts("Writing data blocks");
	fh=fopen("Unicode_DB.data","wb");
	for(i=0; i<16; i++)
	{
		unsigned long crc;
		char filename[200];

		DumpBlock(fh, _PyUnicode_Database[i]);
	}
	fclose(fh);

	/* write strings */
	puts("Writing string table");
	blocklen = stringOffset - stringtable;
	printf("Stringtable = %d\n", blocklen);
	fh=fopen("Unicode_DB.strings","wb");
	fwrite(stringtable, blocklen, 1, fh);
	fclose(fh);

	return 0;
}
