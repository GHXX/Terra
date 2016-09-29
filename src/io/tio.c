
#include "stdafx.h"

#include "tio.h"

#ifdef _WINDOWS
#include <shlobj.h>
#endif

#include "talloc.h"

#include "tencoding.h"

#include "utility/tstring.h"
#include "utility/tfilesys.h"

#include "debugging/tdebug.h"

#include "structure/tlist.h"

static TSList *searchPaths = 0;
static char *savePath = 0;
static char *appPath = 0;

static inline int TIOInitSearchPath(void) {
	searchPaths = TSListNew();
	if (!searchPaths) return 1;
	if (appPath) if (TSListInsert(searchPaths, TStringCopy(appPath), 0)) return 1;
	if (savePath) if (TSListInsert(searchPaths, TStringCopy(savePath), 1)) return 1;

	return 0;
}

void TIOInitialize(const char *argv) {
	searchPaths = 0;
	savePath = 0;

	if (argv) {
		appPath = TFileSysGetDirectory(argv);
		TIOInitSearchPath();
	}
}

void TIODestroy(void) {
	TSListFree(searchPaths, TFree);
	searchPaths = 0;
	TFree(savePath);
	TFree(appPath);
}

void TIOParseArchive(const char *path) {
	TAbort("File System: parse archive function not implemented");
}

TSList *TIOListArchive(const char *_dir, const char *_filter, unsigned char fullFilename) {
	TAbort("File System: list archive function not implemented");
	return 0;
}

TPtr TIOTestPath(const char *searchPath, const char *filePath) {
	char *fullFilename = TFileSysConcat(searchPath, filePath, 0);
	if (TFileSysFileExists(fullFilename)) {
		return fullFilename;
	}

	TFree(fullFilename);
	return 0;
}

char *TIOGetFilePath(const char *path, const char *mode) {
	char *fullPath;

	if (!path) return 0;

	if (TFileSysIsFullPath(path)) return TStringCopy(path);

	if (!mode) mode = "rb";
	else if (strchr(mode, 'w')) {
		return savePath ? TStringConcat(savePath, path, 0) : TStringCopy(path);
	}

	fullPath = (char *)TSListForeachData(searchPaths, (TDataIterFunc)TIOTestPath, (TPtr)path);
	if (fullPath) return fullPath;

	//TODO : check in archives

	return 0;
}

static FILE *TIOGetFileInternal(const char *path, const char *mode) {
	char *fullPath;

	if (!mode) mode = "rb";
	fullPath = TIOGetFilePath(path, mode);

	if (fullPath) {
		FILE *f = TFileSysOpen(fullPath, mode);
		TFree(fullPath);

		return f;
	}

	return 0;
}

TStream *TIOGetFile(const char *path, const char *mode) {
	TStream *stream = 0;

	FILE *f = TIOGetFileInternal(path, mode);
	if (f) stream = TStreamFromFilePointer(f, 1);

	return stream;
}

unsigned char *TIOGetBufferedFile(const char *path, const char *mode, TSize *size) {
	unsigned char *buffer = 0;
	unsigned int finalsize = 0;

	TStream *stream = TIOGetFile(path, mode);
	if (!stream) return 0;

	finalsize = TStreamSize(stream);
	buffer = TAllocNData(unsigned char, finalsize);
	*size = TStreamReadBlock(stream, buffer, finalsize);

	TStreamFree(stream);

	return buffer;
}

void TIOAddSearchPath(const char *path) {
	char *cpy;

	if (!path) return;
	if (!searchPaths) if (TIOInitSearchPath()) return;

	cpy = TStringCopy(path);
	TSListAppend(searchPaths, cpy);
}

void TIORemoveLastSearchPath(void) {
	TFree(TSListPop(searchPaths));
}

void TIOClearSearchPath(void) {
	TSListFree(searchPaths, TFree);
	TIOAddSearchPath(TIOGetApplicationPath());
}

const char *TIOGetApplicationPath(void) {
	return appPath;
}

void TIOSetSavePath(const char *path) {
	if (savePath) {
		TFree(TSListPopIndex(searchPaths, 1));
		TFree(savePath);
	}

	savePath = TStringCopy(path);
	if (savePath) TSListInsert(searchPaths, TStringCopy(savePath), 1);
}

const char *TIOGetSavePath(void) {
	return savePath;
}
