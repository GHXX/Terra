
#ifndef __included_tlog_h
#define __included_tlog_h

#ifdef __cplusplus
extern "C" {
#endif

#include "io/tstream.h"

typedef struct TLog TLog;

enum T_LOG_EVENT {
	T_LOG_PROGRESS = 0,
	T_LOG_WARNING,
	T_LOG_ERROR,
};

TLog *TLogNew(const char *path);
TLog *TLogNewStream(TStream *stream);
void TLogFree(TLog *context);

int TLogWrite(TLog *context, const char *format, ...);
int TLogWriteV(TLog *context, const char *format, va_list ap);

#ifdef __cplusplus
}
#endif

#endif
