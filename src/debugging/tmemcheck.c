
#include "stdafx.h"

#include "io/tstream.h"

#include "tmemleak.h"

#include "structure/tlist.h"
#include "structure/trbtree.h"
#include "utility/tstring.h"
#include "utility/tinteger.h"
#include "utility/tfilesys.h"

#include "talloc.h"

#ifdef _DEBUG
#ifdef _WINDOWS

#include <Windows.h>

typedef struct TMemLeakInput {
	char *source;
	int combined;
	int frequency;
} TMemLeakInput;

static int TMemLeakParseInput(const char *_inputFilename, TRBTree *input)
{
	FILE *memoryfile = fopen(_inputFilename,"r");
	int unrecognised = 0;

	while(!feof(memoryfile)) {
		char thisline [256];
		fgets(thisline,sizeof(thisline),memoryfile);

		if(!strncmp(thisline, " Data:", 6) == 0 && strchr(thisline, ':')) {
			char *sourcelocation, *colon;
			char *lowercasesourcelocation;
			TMemLeakInput *data;

			// Get the size
			char *lastcomma = strrchr(thisline, ',');
			char *ssize = lastcomma+2;
			int size;
			sscanf(ssize, "%d %*s", &size);

			// Get the source file name
			sourcelocation = thisline;
			colon = strrchr(thisline, ':');
			*(colon-1) = '\x0';
			lowercasesourcelocation = TStringLowerCaseO(sourcelocation);

			// Put the result into our BTree
			data = (TMemLeakInput *) TRBTreeFind(input, lowercasesourcelocation);
			if(!data) {
				data = (TMemLeakInput *) TAlloc(sizeof(TMemLeakInput));
				data->combined = data->frequency = 0;
				data->source = strdup(lowercasesourcelocation);
				TRBTreeInsert(input, data->source, data);
			}

			data->combined += size;
			data->frequency += 1;

			free(lowercasesourcelocation);
		} else {
			char *lastcomma = strrchr(thisline, ',');
			if(lastcomma) {
				char *ssize = lastcomma+2;
				int size;
				sscanf(ssize, "%d %*s", &size);

				unrecognised += size;
			}
		}
	}

	fclose(memoryfile);

	return unrecognised;
}

static int TMemLeakSortInput(TRBTree *input, TSList *sorted)
{
	int totalsize = 0;
	char *key;
	TMemLeakInput *data;
	TRBTreeIterator *iter = TRBTreeIteratorNew(input);

	while(TRBTreeIteratorNext(iter,(void **) &key,(void **) &data)) {
		unsigned char inserted = 0;
		int i = 0;
		TMemLeakInput *t = (TMemLeakInput *) TSListFirst(sorted);
		
		while(t) {
			if(data->combined <= t->combined) {
				TSListInsert(sorted,data,i);
				inserted = 1;
				break;
			}

			t = (TMemLeakInput *) TSListNext(sorted);
			i++;
		}

		if(!inserted) TSListAppend(sorted,data);

		totalsize += data->combined;
	}
	TRBTreeIteratorFree(iter);

	return totalsize;
}

static void TMemLeakOuput(const char *_outputFilename, TSList *sorted, int total, int unrecognised)
{
	// Open the output file
	FILE *output = fopen(_outputFilename, "wt");
	TMemLeakInput *t = (TMemLeakInput *) TSListPop(sorted);

	// Print out our sorted list
	fprintf(output, "Total recognised memory leaks   : %d Kbytes\n", (int)(total/1024));
	fprintf(output, "Total unrecognised memory leaks : %d Kbytes\n\n", (int)(unrecognised/1024));

	while(t) {
		char *source = t->source;
		int size = t->combined;
		int freq = t->frequency;

		if(size > 2048)	size = (int)(size/1024);
		
		fprintf(output, "%-95s(%d  bytes in %d leaks)\n", source, size, freq);

		free(source);
		free(t);

		t = (TMemLeakInput *) TSListPop(sorted);
	}

	// Clear up
	fclose(output);
}

void TMemLeakParseFile(const char *_inputFilename,const char *_outputFilename)
{
	int total;

	// Start up
	TRBTree *input = TRBTreeNew((TCompareFunc) strcmp,0,0);

	// Open the file and start parsing
	int unrecognised = TMemLeakParseInput(_inputFilename,input);

	// Sort the results into a list
	TSList sorted;
	TSListInit(&sorted);
	total = TMemLeakSortInput(input,&sorted);

	TRBTreeFree(input);
	
	//output the data and clear the list
	TMemLeakOuput(_outputFilename, &sorted, total, unrecognised);
}

void TMemLeakPrint(TStream *stream)
{
	const char tmpFilename[] = "memleak.tmp";
	OFSTRUCT ofstruct;
	HFILE file;

	// Print all raw memory leak data to a temporary file
	file = OpenFile(tmpFilename, &ofstruct, OF_CREATE);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN,(void *) file);
	_CrtDumpMemoryLeaks();
	_lclose(file);

	// Parse the temp file into a sensible format
	TMemLeakParseFile(tmpFilename, tmpFilename);

	// Delete the temporary file
	TFileSysDelete(tmpFilename);
}
#else
void TMemLeakPrint(char *_filename)
{
}
#endif
#else
void TMemLeakPrint(char *_filename)
{
}
#endif
