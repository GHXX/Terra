
#ifndef __included_terra_io_h
#define __included_terra_io_h

#include "trw.h"

void TIOInitialize(void);
void TIODestroy(void);

void TIOParseArchive(const char *filename);
//TSList *TIOListArchive(const char *_dir, const char *_filter, unsigned char fullFilename);

FILE *TIOGetFile(const char *filename,const char *mode);
TRW *TIOGetRW(const char *filename,const char *mode);
char *TIOGetBufferedFile(const char *filename, const char *mode, size_t *size);

void TIOAddSearchPath(const char *path);
void TIOClearSearchPath(void);

static inline const char *TIOGetApplicationPath(void) { return "."; }

#endif
