#include "stdafx.h"

#include "test.h"

#include "debugging/tmemleak.h"

#include "test_utils.h"

void memleak_test(void)
{
	void *data = malloc(sizeof(int)+ sizeof(float) + 5);

	TLogReport(T_LOG_PROGRESS,0,"Testing Memory Leak...\n");

	TMemLeakPrint("leak.txt");

	TLogReport(T_LOG_PROGRESS,0," Memory Leak tests completed.\n");

	free(data);
}
