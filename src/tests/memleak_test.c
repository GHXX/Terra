#include "stdafx.h"

#include "test.h"

#include "debugging/tmemleak.h"

#include "test_utils.h"

void memleak_test(void)
{
	void *data = malloc(sizeof(int)+ sizeof(float) + 5);

	TLogWriteMain("Testing Memory Leak...\n");

	TMemLeakPrint("leak.txt");

	TLogWriteMain(" Memory Leak tests completed.\n");

	free(data);
}
