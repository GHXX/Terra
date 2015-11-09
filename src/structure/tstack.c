
#include "stdafx.h"

#include "tstack.h"

#include "utility/tinteger.h"

#include "talloc.h"

struct TStack {
	void **bottom, **top;
	TSize size, len;
};

void TStackGrow(TStack *stack)
{
	TSize newsize = 0;
	if(stack->size == 0) newsize = 2;
	else newsize = TIntegerUpperPowerOfTwo(stack->size+1);

	TStackResize(stack, newsize);
}

void TStackShrink(TStack *stack)
{
	if (stack->len >= 2)
		TStackResize(stack, TIntegerUpperPowerOfTwo(stack->len));
}

TStack *TStackNew(void)
{
	TStack *stack = TAllocData(TStack);

	if(stack) {
		stack->bottom = stack->top = 0;
		stack->size = 0;
		stack->len = 0;
	}

	return stack;
}

void TStackFree(TStack *stack, TFreeFunc func)
{
	if(stack) {
		TStackEmpty(stack, func);
		TFree(stack);
	}
}

void TStackEmpty(TStack *stack, TFreeFunc func)
{
	if (func) while (stack->len > 0) func(TStackPop(stack));

	TFree(stack->bottom);
}

void TStackPush(TStack *stack, TPtr data)
{
	if (stack->size <= stack->len) TStackGrow(stack);

	*stack->top = data;
	stack->top++;
	stack->len++;
}

TPtr TStackPop(TStack *stack)
{
	if(stack->len) {
		TPtr data = *(--stack->top);
		stack->len--;

		if (stack->len <= (stack->size >> 2)) TStackShrink(stack);

		return data;
	}

	return 0;
}

TPtr TStackPeek(TStack *stack)
{
	if(stack->len == 0) return 0;
	else return *(stack->top-1);
}

void TStackResize(TStack *stack, TSize _size)
{
	if(_size >stack->size) {
		void *nptr = TRAlloc(stack->bottom, sizeof(void *) * _size);
		if(nptr) {
			stack->bottom = nptr;
			stack->size = _size;
			stack->top = stack->bottom + stack->len;
		}
	} else if (_size < stack->size) {
		void *nptr;

		if(stack->len > _size) return;

		nptr = TRAlloc(stack->bottom, sizeof(void *) * _size);
		if(nptr) {
			stack->bottom = nptr;
			stack->size = _size;
		}
	}
}

TSize TStackCount(TStack *stack)
{
	return stack->len;
}
