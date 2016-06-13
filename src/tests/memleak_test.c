#include "stdafx.h"

#include "test.h"

#include "debugging/tmemcheck.h"

#include "test_utils.h"

void memleak_test(void)
{
	TPtr data = TAlloc(sizeof(int)+ sizeof(float) + 5);

	TStream *stream = TIOGetFile("leak.txt", "w");

	TLogWriteMain("Testing Memory Leak...\n");

	TMemLeakPrint(stream);

	TLogWriteMain(" Memory Leak tests completed.\n");

	TStreamFree(stream);
	TFree(data);
}
