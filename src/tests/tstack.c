#include "stdafx.h"

#include "test.h"

#include "structure/tstack.h"
#include "utility/tinteger.h"
#include "talloc.h"

#include "ttest.h"

int TStackTestAll(void) {
	int i;

	//init
	TStack *stack = TStackNew();
	TTestMustValidate(stack);

	for (i = 0; i < 1000; ++i)
		TStackPush(stack, TIntegerToPtr(i));

	if(!(TStackCount(stack) == 1000)) goto stack_all_error;

	if (!(*((int *)TStackPeek(stack)))) goto stack_all_error;

	for (i = 999; i >= 0; i--) {
		int *v = (int *)TStackPop(stack);
		if (!(*v == i)) goto stack_all_error;
		TFree(v);
	}

	if (TStackPop(stack)) goto stack_all_error;

	TStackFree(stack, TFree);

	return TTestSuccess;

stack_all_error:
	TStackFree(stack, TFree);
	return TTestFailAndStop;
}

void TStackTest(void) {
	TestFunc tests[] = {
		TStackTestAll

	};

	TTestRun("Stack", tests, sizeof(tests) / sizeof(TestFunc));
}
