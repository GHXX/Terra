#include "stdafx.h"

#include "test.h"

#include "structure/tstack.h"
#include "utility/tinteger.h"

#include "ttest.h"

int stack_test_all(void) {
	int i;

	//init
	TStack *stack = TStackNew();
	TAssert(stack);

	for (i = 0; i < 1000; ++i)
		TStackPush(stack, TIntegerToPtr(i));

	TAssert(TStackCount(stack) == 1000);

	TAssert(*((int *)TStackPeek(stack)) == 999);

	for (i = 999; i >= 0; i--) {
		int *v = (int *)TStackPop(stack);
		TAssert(*v == i);
		TFree(v);
	}

	TAssert(TStackPop(stack) == 0);

	TStackFree(stack, free);

	return 0;
}

void stack_test(void) {
	TestFunc tests[] = {
		stack_test_all

	};

	TTestRun("Stack", tests, sizeof(tests) / sizeof(TestFunc));
}
