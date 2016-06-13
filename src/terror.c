#include "stdafx.h"

#include "terror.h"

static int TErrorCode = 0;

void TErrorSet(int code)
{
	TErrorCode = code;
}

int TErrorGet(void)
{
	return TErrorCode;
}

void TErrorClear(void)
{
	TErrorCode = 0;
}
