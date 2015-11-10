#include "stdafx.h"

#include "stack_test.h"

#include "structure/tstack.h"
#include "utility/tinteger.h"

#include "test_utils.h"

void stack_test_all(void)
{
	int i;

	//init
	TStack *stack = TStackNew();
	TAssert(stack);

	for(i = 0; i < 1000; ++i)
		TStackPush(stack,TIntegerToPtr(i));

	TAssert(TStackCount(stack) == 1000);

	TAssert(*((int *) TStackPeek(stack)) == 999);

	for(i = 999; i >= 0; i--) {
		int *v = (int *) TStackPop(stack);
		TAssert(*v == i);
		free(v);
	}

	TAssert(TStackPop(stack) == 0);

	TStackFree(stack,free);
}

void stack_test(void)
{
	TLogReport(T_LOG_PROGRESS,0,"Testing Stack...\n");

	stack_test_all();

	TLogReport(T_LOG_PROGRESS,0,"Stack tests completed.\n");
}
