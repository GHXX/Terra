#include "stdafx.h"

#include "test.h"

#include "debugging/tmemcheck.h"

#include "ttest.h"

void memleak_test(void)
{
	TPtr data = TAlloc(sizeof(int)+ sizeof(float) + 5);

	TStream *stream = TIOGetFile("leak.txt", "w");

	TLogWrite(testLog, "Testing Memory Leak...\n");

	TMemLeakPrint(stream);

	TLogWrite(testLog, " Memory Leak tests completed.\n");

	TStreamFree(stream);
	TFree(data);
}
