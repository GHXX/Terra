
#include "stdafx.h"

#include "theap.h"
#include "tarray.h"

#include "talloc.h"

#define DCMP(a,b) (a->priority >= b->priority)
#define CMP(a,b) (a->priority <= b->priority)

typedef struct {
	int priority;
	void *data;
} HeapNode;

HeapNode *THeapNodeNew(int priority, TPtr data)
{
	HeapNode *node = TAllocData(HeapNode);
	node->priority = priority;
	node->data = data;
	return node;
}

void THeapNodeFree(HeapNode *node, TFreeFunc func)
{
	if(node) {
		if(func) func(node->data);
		TFree(node);
	}
}

struct _THeap {
	TArray data;
	TSize count;
	int type;
};

THeap *THeapNew(int type)
{
	THeap *h = TAllocData(THeap);
	if(!h) return 0;

	TArrayInit(&h->data,0);
	h->count = 0;
	h->type = type;

	return h;
}

void THeapFree(THeap *h, TFreeFunc func)
{
	if(h) {
		THeapEmpty(h,func);
		TFree(h);
	}
}

void THeapEmpty(THeap *h, TFreeFunc func)
{
	TArrayEmpty(&h->data,func);
	h->count = 0;
}

void THeapPush(THeap *h, int priority, TPtr data)
{
	HeapNode *node = THeapNodeNew(priority, data), *p;
	unsigned int index, parent;

	for (index = h->count++; index; index = parent)
	{
		parent = (index - 1) >> 1;
		p = (HeapNode *)TArrayGet(&h->data, parent);
		if (h->type ? CMP(p, node) : DCMP(p, node)) break;
		TArrayInsert(&h->data, p, index);
	}
	TArrayInsert(&h->data, node, index);
}

TPtr THeapPop(THeap *h)
{
	unsigned int index = 0, swap, other;
	
	HeapNode *temp = (HeapNode *)TArrayGet(&h->data, 0), *s, *o;
	void *data = temp->data;

	THeapNodeFree(temp, 0);
 
	// Reorder the elements
	while(1) {
		// Find the child to swap with
		swap = (index << 1) + 1;
		if (swap >= h->count) break; // If there are no children, the heap is reordered
		s = (HeapNode *)TArrayGet(&h->data, swap);

		other = swap + 1;
		if (other < h->count) {
			o = (HeapNode *)TArrayGet(&h->data, other);
			if (h->type ? CMP(o, s) : DCMP(o, s)) {
				swap = other;
				s = o;
			}
		}
 
		TArrayInsert(&h->data, s, index);
		index = swap;
	}
	TArrayRemove(&h->data, index);

	h->count--;

	return data;
}

TSize THeapNumElements(THeap *h)
{
	return h->count;
}

void THeapPrint(THeap *h, TIterFunc func)
{
	TSize i = 0, limit = h->data.len;
	HeapNode *data;

	printf("THeap content: ");

	for(; i < limit; ++i) {
		data = (HeapNode *) h->data.data[i];
		func(data->data);
		printf(" ");
	}

	printf("\n");
}
