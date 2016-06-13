#include "stdafx.h"

#include "talloc.h"

#include "terror.h"

TPtr allocDef(TSize size) {
	TPtr d = malloc(size);
	if (!d) TErrorSet(T_ERROR_OUT_OF_MEMORY);
	return d;
}

TPtr rAllocDef(TPtr ptr, TSize size) {
	TPtr d = realloc(ptr, size);
	if (!d) TErrorSet(T_ERROR_OUT_OF_MEMORY);
	return d;
}

TPtr(*TAlloc)(TSize size) = allocDef;
TPtr(*TRAlloc)(TPtr ptr, TSize size) = rAllocDef;
void(*TFree)(TPtr ptr) = free;
