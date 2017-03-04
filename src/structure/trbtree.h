
#ifndef __included_terra_rbtree_h
#define __included_terra_rbtree_h

// Red-Black Tree

static TInt32 TRBTreePtrCompare(TCPtr p1, TCPtr p2) {
	if (p1 < p2) return -1;
	else if (p1 > p2) return 1;
	return 0;
}

typedef struct TRBTree TRBTree;

TRBTree *TRBTreeNew(TCompareFunc key_compare, TFreeFunc free_key, TFreeFunc free_data);
void TRBTreeFree(TRBTree *t);

void TRBTreeEmpty(TRBTree *t);

TSize TRBTreeSize(const TRBTree *t);

unsigned char TRBTreeInsert(TRBTree *t, TCPtr key, TCPtr data);
TCPtr TRBTreeReplace(TRBTree *t, TCPtr key, TCPtr data);
void TRBTreeErase(TRBTree *t, TCPtr key);

TCPtr TRBTreeFind(const TRBTree *t, TCPtr key);
unsigned char TRBTreeExists(const TRBTree *t, TCPtr key);

TSize TRBTreeMemUsage(const TRBTree *t);

void *TRBTreeDataTraverse(const TRBTree *t, TDataPairIterFunc f, TPtr udata);
void TRBTreeTraverse(const TRBTree *t, TPairIterFunc f);

// Red-Black Tree Iterator

typedef struct TRBTreeIterator TRBTreeIterator;

TRBTreeIterator *TRBTreeIteratorNew(const TRBTree *t);
void TRBTreeIteratorFree(TRBTreeIterator *iter);

int TRBTreeIteratorNext(TRBTreeIterator *iter, TCPtr *key, TCPtr *data);
int TRBTreeIteratorPrevious(TRBTreeIterator *iter, TCPtr *key, TCPtr *data);

#endif
