
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
	T_ERROR_INITIALIZATION_FAILED,

	T_ERROR_AMOUNT,
};

void TErrorReset(void);

void TErrorSet(int code);
int TErrorGet(void);

void TErrorSetErrMsg(int code, const char *errmsg);
const char *TErrorGetErrMsg(int *code);

void TErrorClear(void);

#define TError(code) \
	TErrorSet(code); \
	return

#define TErrorZero(code) \
	TErrorSet(code); \
	return 0

#endif
