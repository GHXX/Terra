
#include "stdafx.h"

#include "tlog.h"

#include "talloc.h"
#include "terror.h"

#include "io/tio.h"

struct TLog {
	TStream *stream;
};

TLog *TLogNew(const char *path) {
	TStream *stream;

	if (!path) {
		TErrorZero(T_ERROR_INVALID_INPUT);
	}

	stream = TIOGetFile(path, "w");
	return TLogNewStream(stream);
}

TLog *TLogNewStream(TStream *stream) {
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

	va_start(ap, format);
	res = TLogWriteV(context, format, ap);
	va_end(ap);

	return res;
}

int TLogWriteV(TLog *context, const char *format, va_list ap) {
	unsigned char *buffer;
	TInt32 size;
	va_list copy;
	int res;

	if (!context || !format) {
		TErrorSet(T_ERROR_INVALID_INPUT);
		return 1;
	}

	va_copy(copy, ap);
	size = vsnprintf(0, 0, format, copy) + 1;
	va_end(copy);

	buffer = TAllocNData(unsigned char, size);
	vsprintf(buffer, format, ap);

	res = TStreamWriteBlock(context->stream, buffer, size -1);
	TFree(buffer);

	return res;
}
