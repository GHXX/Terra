#include "stdafx.h"

#include "structure/tarray.h"
#include "utility/tinteger.h"

#include "test.h"

#include "test_utils.h"

static TArray testarray;
//static TIArray iarr;
static char *yay = "yay";

void array_test_init(void)
{
	TArrayInit(&testarray,0);
}

void array_test_insert(void)
{
	TArrayAppend(&testarray,TIntegerToPtr(1));

	TAssert(*((int *) TArrayGet(&testarray,0)) == 1);

	TArrayInsert(&testarray,TIntegerToPtr(5),4);

	TAssert(*((int *) TArrayGet(&testarray,4)) == 5);
	TAssert(TArrayGet(&testarray,2) == 0);
}

void array_test_move(void)
{
	//array_move(&testarray,0,5,-2);
}

static void *test_loop_1(void *data,void *udata)
{
	if(data) {
		*((int *) udata) = *((int *) data);
		printf("%d",*((int *) data));
	}
	else printf("<null>");

	return 0;
}

static void *test_loop_2(void *data,void *udata)
{
	if(data && *((int *) data) == 5) return yay;

	return 0;
}

void array_test_foreach(void)
{
	int temp;

	printf("test array content : ");
	TArrayForeachData(&testarray,test_loop_1,&temp);

	TAssert(temp == 5);

	TAssert(TArrayForeachData(&testarray,test_loop_2,0) == yay);
}

void array_test_remove(void)
{
	TArrayRemove(&testarray,0);

	TAssert(*((int *) TArrayGet(&testarray,3)) == 5);

	TArrayInsert(&testarray,TIntegerToPtr(1),0);

	TArrayRemoveFast(&testarray,0);

	TAssert(*((int *) TArrayGet(&testarray,0)) == 5);

	TArrayRemoveClear(&testarray,0);

	TAssert(!TArrayGet(&testarray,0));

	TAssert(!TArrayPopIndex(&testarray,0));

	TArrayRemove(&testarray,0);
}

void array_test_free(void)
{
	TArrayEmptyFull(&testarray,free);
}

/*static void *intarr_print(int *v, void *udata)
{
	printf("%d",*v);

	return 0;
}*/

void array_test(void)
{
	TLogReport(T_LOG_PROGRESS,0,"Testing array...\n");

	array_test_init();

	array_test_insert();

	array_test_move();

	array_test_foreach();

	array_test_remove();

	array_test_free();

	TLogReport(T_LOG_PROGRESS,0,"Array tests completed.\n");
}
