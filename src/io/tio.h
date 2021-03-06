
#ifndef __included_terra_io_h
#define __included_terra_io_h

#include "tstream.h"
#include "tevent.h"
#include "tconsole.h"
#include "tclipboard.h"

void TIOInitialize(const char *argv);
void TIODestroy(void);

void TIOParseArchive(const char *path);
//TSList *TIOListArchive(const char *_dir, const char *_filter, unsigned char fullFilename);

char *TIOGetFilePath(const char *path, const char *mode);
TStream *TIOGetFile(const char *path, const char *mode);
unsigned char *TIOGetBufferedFile(const char *path, const char *mode, TSize *size);

unsigned char TIOIsFile(const char *path);

const char *TIOMatchPath(const char *path);

void TIOAddSearchPath(const char *path);
void TIORemoveLastSearchPath(void);
void TIOClearSearchPath(void);

const char *TIOGetApplicationPath(void);

void TIOSetSavePath(const char *path);
const char *TIOGetSavePath(void);

#endif
