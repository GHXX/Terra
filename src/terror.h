
#ifndef __included_terra_error_h
#define __included_terra_error_h

enum T_ERROR_CODES {
	T_ERROR_NONE,
	T_ERROR_UNKNOWN,
	T_ERROR_NULL_POINTER,
	T_ERROR_OUT_OF_MEMORY,
	T_ERROR_PERMISSION_DENIED,
	T_ERROR_INVALID_INPUT,
	T_ERROR_OPERATION_NOT_SUPPORTED,
	T_ERROR_SIZE_EXCEEDED,
	T_ERROR_NOT_INITIALIZED,

	T_ERROR_AMOUNT,
};

void TErrorReport(int code);
void TErrorReportFull(int code, const char *message);

int TErrorGetCode(void);
const char *TErrorGetString(void);

void TErrorClear(void);

#define TError(code) \
	TErrorReport(code); \
	return;

#define TErrorFull(code, message) \
	TErrorReportFull(code, message); \
	return;

extern const char *TErrorMessages[];

#endif
