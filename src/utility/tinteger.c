
#include "stdafx.h"

#include "tinteger.h"

#include "talloc.h"

int *TIntegerToPtr(int data)
{
	int *i = TAllocData(int);
	if(!i) return 0;
	*i = data;
	return i;
}

TSize *TIntegerToPtrU(TSize data)
{
	TSize *i = TAllocData(TSize);
	if(!i) return 0;
	*i = data;
	return i;
}
