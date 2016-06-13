
#include "stdafx.h"

#include "tlog.h"

#include "talloc.h"
#include "terror.h"
#include "structure/tlist.h"

struct TLog {
	TStream *stream;
};

static TLog *mainLog = 0;

void TLogInit(TStream *stream) {
	if (!stream) {
		stream = TStreamFromFilePointer(stdout, 0);
	}
	mainLog = TLogNew(stream);
}

void TLogDestroy(void) {
	TLogFree(mainLog);
	mainLog = 0;
}

TLog *TLogNew(TStream *stream) {
	TLog *log;

	if (!stream) {
		TErrorZero(T_ERROR_INVALID_INPUT);
	}

	log = TAllocData(TLog);
	if (log) {
		log->stream = stream;
	}

	return log;
}

void TLogFree(TLog *context) {
	if (context) {
		TFree(context->stream);
		TFree(context);
	}
}

int TLogWrite(TLog *context, const char *format, ...) {
	int res;
	va_list ap;

	if (!context || !format) {
		TErrorSet(T_ERROR_INVALID_INPUT);
		return 1;
	}
	va_start(ap, format);
	res = TLogWriteV(context, format, ap);
	va_end(ap);

	return res;
}

int TLogWriteV(TLog *context, const char *format, va_list ap) {
	char buffer[TBUFSIZE];
	TInt32 size;

	if (!context || !format) {
		TErrorSet(T_ERROR_INVALID_INPUT);
		return 1;
	}

	size = vsprintf(buffer, format, ap);

	if (size < 0) {
		TErrorSet(T_ERROR_SIZE_EXCEEDED);
		return 1;
	}
	
	return TStreamWriteString(context->stream, buffer, size);
}

int TLogWriteMain(const char *format, ...) {
	int res;
	va_list ap;

	if (!mainLog || !format) {
		TErrorSet(T_ERROR_INVALID_INPUT);
		return 1;
	}

	va_start(ap, format);
	res = TLogWriteV(mainLog, format, ap);
	va_end(ap);

	return res;
}

int TLogWriteVMain(const char *format, va_list ap) {
	return TLogWriteV(mainLog, format, ap);
}
