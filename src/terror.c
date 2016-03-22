#include "stdafx.h"

#include "terror.h"

static int TErrorCode = 0;
static const char *TErrorString = 0;

const char *TErrorMessages[] = {
	"No error",
	"Unknown error",
	"NULL Pointer",
	"Out of memory",
	"Permission denied",
	"Invalid Input",
	"Operation not supported",
	"Size exceeded",
	"Not Initialized",
};

void TErrorReport(int code, const char *message)
{
	TErrorCode = code;
	TErrorString = message;
}

void TErrorReportDefault(int code)
{
	if (code < 0 || code >= T_ERROR_AMOUNT) code = 1;
	TErrorReport(code, TErrorMessages[code]);
}

int TErrorGetCode(void)
{
	return TErrorCode;
}

const char *TErrorGetString(void)
{
	return TErrorString;
}

void TErrorClear(void)
{
	TErrorString = TErrorMessages[0];
	TErrorCode = 0;
}
