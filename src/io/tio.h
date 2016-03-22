
#ifndef __included_terra_io_h
#define __included_terra_io_h

#include "tstream.h"

void TIOInitialize(void);
void TIODestroy(void);

void TIOParseArchive(const char *filename);
//TSList *TIOListArchive(const char *_dir, const char *_filter, unsigned char fullFilename);

TStream *TIOGetFile(const char *filename, const char *mode);
unsigned char *TIOGetBufferedFile(const char *filename, const char *mode, TSize *size);

void TIOAddSearchPath(const char *path);
void TIORemoveLastSearchPath(void);
void TIOClearSearchPath(void);

static inline const char *TIOGetApplicationPath(void) { return "."; }

#endif
