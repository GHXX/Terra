#include "stdafx.h"

#include "structure/theap.h"

#include "test.h"

#include "test_utils.h"

static THeap *test = 0;

char *strarr[] = {"bleh","blah","blih","bluh","blyh"};

void heap_test_init(void)
{
	test = THeapNew(0);
	TAssert(test);
}

void heap_test_init2(void)
{
	test = THeapNew(1);
	TAssert(test);
}

void heap_test_push(void)
{
	THeapPush(test,10,strarr[0]);
	THeapPush(test,5,strarr[1]);
	THeapPush(test,2,strarr[2]);
	THeapPush(test,12,strarr[3]);
	THeapPush(test,7,strarr[4]);

	TAssert(THeapNumElements(test) == 5);
}

#ifdef _DEBUG
void heap_test_print(void)
{
	THeapPrint(test,(TIterFunc) printf);
}
#endif

void heap_test_pop(void)
{
	TAssert(THeapPop(test) == strarr[3]);
	TAssert(THeapPop(test) == strarr[0]);
	TAssert(THeapPop(test) == strarr[4]);
	TAssert(THeapPop(test) == strarr[1]);
	TAssert(THeapPop(test) == strarr[2]);
}

void heap_test_pop2(void)
{
	TAssert(THeapPop(test) == strarr[2]);
	TAssert(THeapPop(test) == strarr[1]);
	TAssert(THeapPop(test) == strarr[4]);
	TAssert(THeapPop(test) == strarr[0]);
	TAssert(THeapPop(test) == strarr[3]);
}

void heap_test_free(void)
{
	THeapFree(test,0);
}

void heap_test(void)
{
	heap_test_init();

	heap_test_push();

#ifdef _DEBUG
	heap_test_print();
#endif

	heap_test_pop();

	heap_test_free();

	heap_test_init2();

	heap_test_push();

#ifdef _DEBUG
	heap_test_print();
#endif

	heap_test_pop2();

	heap_test_free();
}
