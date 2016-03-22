
#include "stdafx.h"

#include "tio.h"

#ifdef _WINDOWS
#include <shlobj.h>
#endif

#include "talloc.h"
#include "tthread.h"

#include "utility/tstring.h"
#include "utility/tfilesys.h"

#include "debugging/tdebug.h"

#include "structure/tlist.h"

static TSList *searchpaths = 0;

void TIOInitialize(void)
{
	searchpaths = 0;
	TIOAddSearchPath(TIOGetApplicationPath());
}

void TIODestroy(void)
{
	TSListFree(searchpaths, TFree);
	searchpaths = 0;
}

void TIOParseArchive(const char *filename)
{
	TAbort("File System: parse archive function not implemented");
}

TSList *TIOListArchive(const char *_dir, const char *_filter, unsigned char fullFilename)
{
	TAbort("File System: list archive function not implemented");
	return 0;
}

void *testpath(const char *searchpath, const char *filename)
{
	char *fullFilename = TFileSysConcatPaths(searchpath, filename, NULL);
	if (TFileSysFileExists(fullFilename)) return fullFilename;

	TFree(fullFilename);

	return 0;
}

static FILE *TIOGetFileInternal(const char *filename, const char *mode)
{
	char *found = 0;
	if(!filename) return 0;

	if(!mode) mode = "rb";
	else if (strchr(mode, 'w')) {
		return fopen(filename, mode);
	}

	found = (char *)TSListForeachData(searchpaths, (TDataIterFunc)testpath, (void *)filename);
	if(found) {
		FILE *f = fopen(found, mode);
		TFree(found);
		return f;
	}

	//TODO : check in archives

	return 0;
}

TStream *TIOGetFile(const char *filename, const char *mode)
{
	TStream *trw = 0;

	FILE *f = TIOGetFileInternal(filename, mode);
	if (f) trw = TStreamFromFilePointer(f, 1);

	return trw;
}

unsigned char *TIOGetBufferedFile(const char *filename, const char *mode, TSize *size)
{
	unsigned char *buffer = 0;
	unsigned int finalsize = 0;

	TStream *trw = TIOGetFile(filename, mode);
	if (!trw) return 0;

	finalsize = TStreamSize(trw);
	buffer = TAlloc(sizeof(unsigned char) * finalsize);
	*size = TStreamReadBlock(trw, buffer, finalsize);

	TStreamFree(trw);

	return buffer;
}

void TIOAddSearchPath(const char *path)
{
	if (!searchpaths) searchpaths = TSListNew();

	TSListAppend(searchpaths, TStringCopy(path));
}

void TIORemoveLastSearchPath(void)
{
	TSListRemoveIndex(searchpaths, searchpaths->len - 1);
}

void TIOClearSearchPath(void)
{
	TSListFree(searchpaths, TFree);
	TIOAddSearchPath(TIOGetApplicationPath());
}

