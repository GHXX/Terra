#include "stdafx.h"

#include "test.h"

#include "structure/trbtree.h"
#include "utility/tinteger.h"

#include "test_utils.h"

void rbtree_test_iteration(const TRBTree *tree)
{
	TRBTreeIterator *iter;
	int *key, *data;
	int i = 0;

	// init
	iter = TRBTreeIteratorNew(tree);
	if(!testNotNull(iter,"Initializing iterator\t\t")) return;

	// next
	while(TRBTreeIteratorNext(iter,(const void **) &key,(const void **) &data))
	{
		TAssert(*key == *data && *key == i++);
	}

	// previous
	--i;
	while(TRBTreeIteratorPrevious(iter,(const void **) &key,(const void **) &data))
	{
		TAssert(*key == *data && *key == --i);
	}

	// free
	TRBTreeIteratorFree(iter);
}

void rbtree_test_all(void)
{
	TRBTree *tree;
	int i;
	unsigned char f = 0;
	int testv = 50;

	// init
	tree = TRBTreeNew((TCompareFunc) TIntegerCompare,free,free);
	if(!testNotNull(tree,"Initializing tree\t\t")) return;

	// insert
	for(i = 0;!f && i < 100; ++i) f = TRBTreeInsert(tree,TIntegerToPtr(i),TIntegerToPtr(i));
	testReport(!f,"Testing Insertion\t\t");

	// size
	testReport(TRBTreeSize(tree) == 100,"Testing Size\t\t");

	// find
	testReport(*(int *)TRBTreeFind(tree,&testv) == 50,"Testing Find function\t\t");

	// exists
	testv = 34;
	testReport(TRBTreeExists(tree,&testv),"Testing Exists function\t\t");

	// mem
	// TODO

	// data traversal
	// TODO

	// traversal
	// TODO

	// iteration
	rbtree_test_iteration(tree);

	// replace
	// TODO

	// removal
	for(i = 0; i < 94; ++i) TRBTreeErase(tree,&i);
	testReport(TRBTreeSize(tree) == 6,"Testing Erase Function\t\t");;

	// empty
	TRBTreeEmpty(tree);
	testReport(TRBTreeSize(tree) == 0,"Ensuring that the tree is empty\t\t");;

	// free
	TRBTreeFree(tree);
}

void rbtree_test(void)
{
	TLogReport(T_LOG_PROGRESS,0,"Testing Red Black Tree...\n");

	rbtree_test_all();

	TLogReport(T_LOG_PROGRESS,0,"Red Black Tree tests completed.\n");
}
