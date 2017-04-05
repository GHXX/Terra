#include "stdafx.h"

#include "test.h"

#include "debugging/tdebug.h"
#include "structure/trbtree.h"
#include "utility/tinteger.h"

#include "ttest.h"

void rbtree_test_iteration(const TRBTree *tree) {
	TRBTreeIterator *iter;
	int *key, *data;
	int i = 0;

	// init
	iter = TRBTreeIteratorNew(tree);
	if (!TTestNotNull(iter, "Initializing iterator\t\t")) return;

	// next
	while (TRBTreeIteratorNext(iter, (const void **)&key, (const void **)&data)) {
		TAssert(*key == *data && *key == i++);
	}

	// previous
	--i;
	while (TRBTreeIteratorPrevious(iter, (const void **)&key, (const void **)&data)) {
		TAssert(*key == *data && *key == --i);
	}

	// free
	TRBTreeIteratorFree(iter);
}

int rbtree_test_all(void) {
	TRBTree *tree;
	int i;
	unsigned char f = 0;
	int testv = 50;

	// init
	tree = TRBTreeNew((TCompareFunc)TIntegerCompare, free, free);
	if (!TTestNotNull(tree, "Initializing tree\t\t")) return 0;

	// insert
	for (i = 0; !f && i < 100; ++i) f = TRBTreeInsert(tree, TIntegerToPtr(i), TIntegerToPtr(i));
	TTestReport(!f, "Testing Insertion\t\t");

	// size
	TTestReport(TRBTreeSize(tree) == 100, "Testing Size\t\t");

	// find
	TTestReport(*(int *)TRBTreeFind(tree, &testv) == 50, "Testing Find function\t\t");

	// exists
	testv = 34;
	TTestReport(TRBTreeExists(tree, &testv), "Testing Exists function\t\t");

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
	for (i = 0; i < 94; ++i) TRBTreeErase(tree, &i);
	TTestReport(TRBTreeSize(tree) == 6, "Testing Erase Function\t\t");;

	// empty
	TRBTreeEmpty(tree);
	TTestReport(TRBTreeSize(tree) == 0, "Ensuring that the tree is empty\t\t");;

	// free
	TRBTreeFree(tree);

	return 0;
}

void rbtree_test(void) {

	TestFunc tests[] = {
		rbtree_test_all

	};

	TTestRun("RBTree", tests, sizeof(tests) / sizeof(TestFunc));
}
