#include "stdafx.h"

#include "terror.h"

#include "talloc.h"

#include "utility/tstring.h"

static int TErrorCode = 0;
static char *TErrorMsg = 0;

void TErrorSet(int code) {
	TErrorCode = code;
	TFree(TErrorMsg);
}

int TErrorGet(void) {
	return TErrorCode;
}

void TErrorSetErrMsg(int code, const char *errmsg) {
	TFree(TErrorMsg);
	TErrorMsg = TStringCopy(errmsg);
}

const char *TErrorGetErrMsg(int *code) {
	if (code) *code = TErrorCode;
	return TErrorMsg;
}

void TErrorClear(void) {
	TErrorCode = 0;
	TFree(TErrorMsg);
	TErrorMsg = 0;
}
