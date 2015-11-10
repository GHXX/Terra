#include "stdafx.h"

#include "talloc.h"

#include "terror.h"

TPtr allocDef(TSize size)
{
	TPtr d = malloc(size);
	if(!d) TErrorReport(T_ERROR_OUT_OF_MEMORY);
	return d;
}

TPtr rAllocDef(TPtr ptr, TSize size)
{
	TPtr d = realloc(ptr, size);
	if(!d) TErrorReport(T_ERROR_OUT_OF_MEMORY);
	return d;
}

static TPtr (*talloc)(TSize) = allocDef;
static TPtr (*tralloc)(TPtr , TSize) = rAllocDef;
static void (*tfree)(TPtr ) = free;

TPtr TAlloc(TSize size)
{
	return talloc(size);
}

TPtr TRAlloc(TPtr ptr, TSize size)
{
	return tralloc(ptr,size);
}

void TFree(TPtr ptr)
{
	tfree(ptr);
}

void TAllocSet(TPtr (*allocFunc)(TSize), TPtr (*rallocFunc)(TPtr , TSize),void (*freeFunc) (TPtr ))
{
	if(!allocFunc) allocFunc = allocDef;
	if(!rallocFunc) rallocFunc = rAllocDef;
	if(!freeFunc) freeFunc = free;

	talloc = allocFunc;
	tralloc = rallocFunc;
	tfree = freeFunc;
}
