
#ifndef __included_terra_debug_h
#define __included_terra_debug_h

#include "tlog.h"
#include "tmemcheck.h"
#include "tprofiler.h"

typedef void(*TAbortFunc)(const char *, va_list);

void TDebugDumpData(TStream *stream, unsigned char *data, TSize dataLen); // Dumps data as a hex table

void TDebugAssertFailed(const char *fmt, ...);

void TDebugPrintStackTrace(TStream *stream);

void TDebugSetAbortFunction(TAbortFunc func);

#ifdef COMPILER_MICROSOFT
#define TDebugAssert(cond, ...) \
	if(!(cond)) \
		TDebugAssertFailed(__VA_ARGS__);
#else // GCC
#define TDebugAssert(cond, fmt, args...) \
	if(!(cond)) \
		TDebugAssertFailed(fmt, ## args);
#endif

#define TAssert(x)		TDebugAssert((x),               \
						"Assertion failed : '%s'\n\n"   \
						"%s\nline number %d",           \
						#x, __FILE__, __LINE__)

#define TAbort(x)		TDebugAssert(0,                 \
						"Abort : '%s'\n\n"              \
						"%s\nline number %d",           \
						x, __FILE__, __LINE__)

#endif
