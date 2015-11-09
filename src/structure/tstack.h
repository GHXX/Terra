
#ifndef __included_terra_stack_h
#define __included_terra_stack_h

typedef struct TStack TStack;

TStack *TStackNew(void);
void TStackFree(TStack *stack, TFreeFunc func);

void TStackEmpty(TStack *stack, TFreeFunc func);

void TStackPush(TStack *stack, TPtr data);
TPtr TStackPop(TStack *stack);

TPtr TStackPeek(TStack *stack);

void TStackResize(TStack *stack, TSize _size);

TSize TStackCount(TStack *stack);

#endif
