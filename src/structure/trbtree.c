
#include "stdafx.h"

#include "trbtree.h"

#include "talloc.h"

enum color_t { Red, Black };

typedef struct TRBTreeNode {
	TCPtr key;
	TCPtr data;
	TUInt8 color;

	struct TRBTreeNode *left, *right, *parent;
} TRBTreeNode;

TRBTreeNode *TRBTreeNodeNew(void) {
	TRBTreeNode *n = TAllocData(TRBTreeNode);

	if (n) {
		n->left = n->right = n->parent = 0;
		n->color = Red;
		n->key = n->data = 0;
	}

	return n;
}

void TRBTreeNodeFree(TRBTreeNode *n, TFreeFunc fk, TFreeFunc fd) {
	if (n) {
		if (fk) fk((TPtr)n->key);
		if (fd) fd((TPtr)n->data);
		TRBTreeNodeFree(n->left, fk, fd);
		TRBTreeNodeFree(n->right, fk, fd);
		TFree(n);
	}
}

TSize TRBTreeNodeMemUsage(const TRBTreeNode *n) {
	if (n) return sizeof(*n) + sizeof(*n->left) + sizeof(*n->right);

	return 0;
}

struct TRBTree {
	TRBTreeNode *root;

	TSize size;

	TCompareFunc kcomp;

	TFreeFunc fk;
	TFreeFunc fd;
};

int ptr_comp(TCPtr d1, TCPtr d2) {
	if (d1 > d2) return 1;
	else if (d1 < d2) return -1;
	return 0;
}

TRBTree *TRBTreeNew(TCompareFunc key_compare, TFreeFunc free_key, TFreeFunc free_data) {
	TRBTree *t = TAllocData(TRBTree);

	if (t) {
		t->root = 0;
		t->size = 0;
		t->kcomp = key_compare ? key_compare : ptr_comp;
		t->fk = free_key;
		t->fd = free_data;
	}

	return t;
}

void TRBTreeFree(TRBTree *t) {
	if (t) {
		TRBTreeNodeFree(t->root, t->fk, t->fd);
		free(t);
	}
}

void TRBTreeEmpty(TRBTree *t) {
	TRBTreeNodeFree(t->root, t->fk, t->fd);
	t->root = 0;
	t->size = 0;
}

TSize TRBTreeSize(const TRBTree *t) {
	return t->size;
}

inline const TRBTreeNode *TRBTreeLeftMost(const TRBTree *t, const TRBTreeNode *c) {
	if (!c) {
		if (!t->root) return 0;
		c = t->root;
	}

	while (c->left) c = c->left;

	return c;
}

inline const TRBTreeNode *TRBTreeRightMost(const TRBTree *t, const TRBTreeNode *c) {
	if (!c) {
		if (!t->root) return 0;
		c = t->root;
	}

	while (c->right) c = c->right;

	return c;
}

void TRBTreeRotateLeft(TRBTree *t, TRBTreeNode *x) {
	TRBTreeNode *y = x->right;

	/* establish x->right link */
	x->right = y->left;
	if (y->left)
		y->left->parent = x;

	/* establish y->parent link */
	y->parent = x->parent;

	if (x->parent) {
		if (x == x->parent->left)
			x->parent->left = y;
		else
			x->parent->right = y;
	} else {
		t->root = y;
	}

	/* link x and y */
	y->left = x;
	x->parent = y;
}

void TRBTreeRotateRight(TRBTree *t, TRBTreeNode *x) {
	TRBTreeNode *y = x->left;

	/* establish x->left link */
	x->left = y->right;
	if (y->right)
		y->right->parent = x;

	/* establish y->parent link */
	y->parent = x->parent;

	if (x->parent) {
		if (x == x->parent->right)
			x->parent->right = y;
		else
			x->parent->left = y;
	} else {
		t->root = y;
	}

	/* link x and y */
	y->right = x;
	x->parent = y;
}

void TRBTreeInsertFixup(TRBTree *t, TRBTreeNode *x) {
	/* check Red-Black properties */
	while (x != t->root && x->parent->color == Red) {
		/* we have a violation */
		if (x->parent == x->parent->parent->left) {
			TRBTreeNode *y = x->parent->parent->right;

			if (y && y->color == Red) {
				/* uncle is RED */
				y->color = x->parent->color = Black;
				x->parent->parent->color = Red;
				x = x->parent->parent;
			} else {
				/* uncle is BLACK */
				if (x == x->parent->right) {
					/* make x a left child */
					x = x->parent;
					TRBTreeRotateLeft(t, x);
				}

				/* recolor and rotate */
				x->parent->color = Black;
				x->parent->parent->color = Red;
				TRBTreeRotateRight(t, x->parent->parent);
			}
		} else {
			/* mirror image of above code */
			TRBTreeNode *y = x->parent->parent->left;

			if (y && y->color == Red) {
				/* uncle is RED */
				y->color = x->parent->color = Black;
				x->parent->parent->color = Red;
				x = x->parent->parent;
			} else {
				/* uncle is BLACK */
				if (x == x->parent->left) {
					x = x->parent;
					TRBTreeRotateRight(t, x);
				}

				x->parent->color = Black;
				x->parent->parent->color = Red;
				TRBTreeRotateLeft(t, x->parent->parent);
			}
		}
	}
	t->root->color = Black;
}

inline TRBTreeNode *TRBTreeFindNode(const TRBTree *t, TCPtr key) {
	TRBTreeNode *current = t->root;

	while (current) {
		int cmp = t->kcomp(key, current->key);
		if (cmp < 0)
			current = current->left;
		else if (cmp > 0)
			current = current->right;
		else
			return current;
	}

	return 0;
}

TCPtr TRBTreeReplace(TRBTree *t, TCPtr key, TCPtr data) {
	TRBTreeNode *current = TRBTreeFindNode(t, key);
	if (!current) return 0;

	TSWAPT(current->data, data, TCPtr);
	return data;
}

unsigned char TRBTreeInsert(TRBTree *t, TCPtr key, TCPtr data) {
	TRBTreeNode *current = t->root, *parent = 0, *x = 0;
	int ret;

	x = TRBTreeNodeNew();
	if (!x) return 1;

	/* find future parent */
	while (current) {
		parent = current;

		ret = t->kcomp(key, current->key);
		if (ret < 0)
			current = current->left;
		else if (ret > 0)
			current = current->right;
		else
			return 1;
	}

	/* setup new node */
	x->parent = parent;
	x->key = key;
	x->data = data;

	/* insert node in tree */
	if (parent) {
		if (ret <= 0)
			parent->left = x;
		else
			parent->right = x;
	} else {
		t->root = x;
	}

	t->size++;

	TRBTreeInsertFixup(t, x);

	return 0;
}

void TRBTreeDeleteFixup(TRBTree *t, TRBTreeNode *x) {
	if (!x) return;

	while (x != t->root && x->color == Black) {
		if (x == x->parent->left) {
			TRBTreeNode *w = x->parent->right;

			if (w->color == Red) {
				w->color = Black;
				x->parent->color = Red;
				TRBTreeRotateLeft(t, x->parent);
				w = x->parent->right;
			}

			if (w->left->color == Black && w->right->color == Black) {
				w->color = Red;
				x = x->parent;
			} else {
				if (w->right->color == Black) {
					w->left->color = Black;
					w->color = Red;
					TRBTreeRotateRight(t, w);
					w = x->parent->right;
				}

				w->color = x->parent->color;
				x->parent->color = w->right->color = Black;
				TRBTreeRotateLeft(t, x->parent);
				x = t->root;
			}
		} else {
			TRBTreeNode *w = x->parent->left;

			if (w->color == Red) {
				w->color = Black;
				x->parent->color = Red;
				TRBTreeRotateRight(t, x->parent);
				w = x->parent->left;
			}

			if (w->right->color == Black && w->left->color == Black) {
				w->color = Red;
				x = x->parent;
			} else {
				if (w->left->color == Black) {
					w->right->color = Black;
					w->color = Red;
					TRBTreeRotateLeft(t, w);
					w = x->parent->left;
				}

				w->color = x->parent->color;
				x->parent->color = w->left->color = Black;
				TRBTreeRotateRight(t, x->parent);
				x = t->root;
			}
		}
	}
	x->color = Black;
}

void TRBTreeKillNode(TRBTree *t, TRBTreeNode *z) {
	TRBTreeNode *x, *y;

	if (!z->left || !z->right) {
		/* y has a null node as a child */
		y = z;
	} else {
		/* find tree successor with a null node as a child */
		y = z->right;

		while (y->left)
			y = y->left;
	}

	/* x is y's only child */
	x = y->left ? y->left : y->right;

	/* remove y from the parent chain */
	if (x) x->parent = y->parent;

	if (y->parent) {
		if (y == y->parent->left)
			y->parent->left = x;
		else
			y->parent->right = x;
	} else
		t->root = x;

	if (t->fk) t->fk((TPtr)z->key);
	if (y != z) {
		z->key = y->key;
		z->data = y->data;
	}

	if (y->color == Black)
		TRBTreeDeleteFixup(t, x);

	t->size--;

	y->left = y->right = 0;
	TFree(y);
}

void TRBTreeErase(TRBTree *t, TCPtr key) {
	TRBTreeNode *z = TRBTreeFindNode(t, key);

	/*  delete node z from tree */

	/* find node in tree */
	if (z) TRBTreeKillNode(t, z);
}

TCPtr TRBTreeFind(const TRBTree *t, TCPtr key) {
	TRBTreeNode *current = TRBTreeFindNode(t, key);

	return current ? current->data : 0;
}

unsigned char TRBTreeExists(const TRBTree *t, TCPtr key) {
	TRBTreeNode *current = TRBTreeFindNode(t, key);

	return current != 0;
}

TSize TRBTreeMemUsage(const TRBTree *t) {
	TSize ret = sizeof(*t);
	if (t->root) ret += TRBTreeNodeMemUsage(t->root);

	return ret;
}

TPtr TRBTreeDataTraverse(const TRBTree *t, TDataPairIterFunc f, TPtr udata) {
	TRBTreeIterator *iter;
	TCPtr key, data;
	TPtr d = 0;

	if (!f) return 0;

	iter = TRBTreeIteratorNew(t);

	while (!d && TRBTreeIteratorNext(iter, &key, &data)) d = f((TPtr)key, (TPtr)data, udata);

	TRBTreeIteratorFree(iter);

	return d;
}

void TRBTreeTraverse(const TRBTree *t, TPairIterFunc f) {
	TRBTreeIterator *iter;
	TCPtr key, data;

	if (!f) return;

	iter = TRBTreeIteratorNew(t);

	while (TRBTreeIteratorNext(iter, &key, &data)) f((TPtr)key, (TPtr)data);

	TRBTreeIteratorFree(iter);
}

struct TRBTreeIterator {
	const TRBTree *t;
	const TRBTreeNode *c;
	unsigned char beginning, end;
};

TRBTreeIterator *TRBTreeIteratorNew(const TRBTree *t) {
	TRBTreeIterator *iter = TAllocData(TRBTreeIterator);

	iter->t = t;
	iter->c = 0;
	iter->beginning = iter->end = 0;

	return iter;
}

void TRBTreeIteratorFree(TRBTreeIterator *iter) {
	TFree(iter);
}

int TRBTreeIteratorNext(TRBTreeIterator *iter, TCPtr *key, TCPtr *data) {
	iter->beginning = 0;
	if (iter->end) return 0;

	if (!iter->c) {
		if (!iter->t->root) return 0;

		iter->c = TRBTreeLeftMost(iter->t, 0);
		iter->beginning = 1;
	} else if (iter->c->right) {
		iter->c = iter->c->right;
		iter->c = TRBTreeLeftMost(iter->t, iter->c);
	} else {
		unsigned char didbreak = 0;
		const TRBTreeNode *c = iter->c;

		while (iter->c->parent) {
			const TRBTreeNode *l = iter->c;
			iter->c = iter->c->parent;
			if (iter->c->left == l) {
				didbreak = 1;
				break;
			}
		}

		if (!didbreak) {
			iter->end = 1;
			iter->c = c;
			return 0;
		}
	}
	if (key) *key = iter->c->key;
	if (data) *data = iter->c->data;

	return 1;
}

int TRBTreeIteratorPrevious(TRBTreeIterator *iter, TCPtr *key, TCPtr *data) {
	iter->end = 0;
	if (iter->beginning) {
		return 0;
	}

	if (!iter->c) {
		if (!iter->t->root) return 0;

		iter->c = TRBTreeRightMost(iter->t, 0);
		iter->end = 1;
	} else if (iter->c->left) {
		iter->c = iter->c->left;
		iter->c = TRBTreeRightMost(iter->t, iter->c);
	} else {
		unsigned char didbreak = 0;
		const TRBTreeNode *c = iter->c;

		while (iter->c->parent) {
			const TRBTreeNode *l = iter->c;
			iter->c = iter->c->parent;
			if (iter->c->right == l) {
				didbreak = 1;
				break;
			}
		}

		if (!didbreak) {
			iter->beginning = 1;
			iter->c = c;
			return 0;
		}
	}

	if (key) *key = iter->c->key;
	if (data) *data = iter->c->data;

	return 1;
}
