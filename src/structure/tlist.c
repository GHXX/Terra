

#include "stdafx.h"

#include "tlist.h"

#include "talloc.h"

TListNode *TListNodeNew(TPtr data) {
	TListNode *node = TAllocData(TListNode);
	if (!node) return 0;

	node->next = node->previous = 0;
	node->data = data;
	return node;
}

static TListNode *TListFetch(TList *list, TSize index) {
	if (index >= list->len) return 0;

	if (index >= list->previousindex) {
		index -= list->previousindex;
		list->previousindex += index;
	} else {
		list->previousindex = index;
		list->previous = list->head;
	}

	while (list->previous && index) {
		list->previous = list->previous->next;
		index -= 1;
	}

	return list->previous;
}

TList *TListNew(void) {
	TList *list = TAllocData(TList);
	if (!list) return 0;

	TListInit(list);
	return list;
}

void TListInit(TList *list) {
	list->previous = list->head = list->end = 0;
	list->previousindex = list->len = 0;
}

void TListFree(TList *list, TFreeFunc func) {
	if (list) {
		TListEmpty(list, func);
		TFree(list);
	}
}

void TListEmpty(TList *list, TFreeFunc func) {
	while (list->head) {
		TListNode *remove = list->head;
		if (func) func(list->head->data);
		list->head = list->head->next;
		TFree(remove);
	}
	TListInit(list);
}

int TListInsert(TList *list, TPtr data, TSize index) {
	TListNode *newnode = TListNodeNew(data);
	if (!newnode) return 1;

	if (!list->head) {
		list->head = list->end = newnode;
		list->previousindex = 0;
	} else if (index >= list->len) {
		list->end->next = newnode;
		newnode->previous = list->end;
		list->end = newnode;
		list->previousindex = list->len;
	} else if (index == 0) {
		newnode->next = list->head;
		list->head->previous = newnode;
		list->head = newnode;
		list->previousindex = 0;
	} else {
		TListNode *location = TListFetch(list, index);
		newnode->next = location;
		newnode->previous = location->previous;
		location->previous = newnode->previous->next = newnode;
		list->previousindex = index;
	}
	list->previous = newnode;
	list->len += 1;

	return 0;
}

void TListForeach(TList *list, TIterFunc func) {
	TListNode *cur = list->head;

	while (cur) {
		func(cur->data);
		cur = cur->next;
	}
}

TPtr TListForeachData(TList *list, TDataIterFunc func, TPtr userData) {
	TListNode *cur = list->head;

	while (cur) {
		TPtr value = func(cur->data, userData);
		if (value) return value;
		cur = cur->next;
	}

	return 0;
}

TPtr TListGet(TList *list, TSize index) {
	TListNode *n = TListFetch(list, index);

	return n ? n->data : 0;
}

int TListFind(TList *list, TPtr data) {
	TSize idx = 0;
	TListNode *n = list->head;

	while (n) {
		if (n->data == data) break;
		n = n->next; idx += 1;
	}

	return n ? idx : -1;
}

void TListSort(TList *list) {
	//TODO
}

TPtr TListPopIndex(TList *list, TSize index) {
	TListNode *n = TListFetch(list, index);
	TPtr data = 0;

	if (n) {
		data = n->data;
		TListRemovePtr(list, n);
	}

	return data;
}

void TListRemove(TList *list, TPtr data) {
	TListNode *n = list->head;

	while (n) {
		if (n->data == data) break;
		n = n->next;
	}

	if (n) TListRemovePtr(list, n);
}

void TListRemoveIndex(TList *list, TSize index) {
	TListNode *n = TListFetch(list, index);

	if (n) TListRemovePtr(list, n);
}

void TListRemovePtr(TList *list, TListNode *ptr) {
	if (!list || !list->head || !ptr) return;

	if (ptr->next) ptr->next->previous = ptr->previous;
	else list->end = ptr->previous;

	if (ptr->previous) ptr->previous->next = ptr->next;
	else list->head = ptr->next;

	list->len -= 1;
	list->previousindex = 0;
	list->previous = list->head;
}

//------------- Single-Linked TList ---------------//

TSListNode *TSListNodeNew(TCPtr  data) {
	TSListNode *node = TAllocData(TSListNode);
	if (!node) return 0;

	node->next = 0;
	node->data = data;
	return node;
}

static TSListNode *TSListFetch(TSList *list, TSize index) {
	if (index >= list->len) return 0;

	if (index >= list->previousindex) {
		index -= list->previousindex;
		list->previousindex += index;
	} else {
		list->previousindex = index;
		list->previous = list->head;
	}

	while (list->previous && index) {
		list->previous = list->previous->next;
		index -= 1;
	}

	return list->previous;
}

TSList *TSListNew(void) {
	TSList *list = (TSList *)TAlloc(sizeof(TSList));
	if (list) TSListInit(list);
	return list;
}

void TSListInit(TSList *list) {
	if (list) {
		list->head = list->end = 0;
		list->len = list->previousindex = 0;
		list->previous = 0;
	}
}

void TSListFree(TSList *list, TFreeFunc func) {
	if (list) {
		TSListEmpty(list, func);
		free(list);
	}
}

void TSListEmpty(TSList *list, TFreeFunc func) {
	if (list) {
		while (list->head) {
			TSListNode *remove = list->head;
			if (func) func((TPtr)list->head->data);
			list->head = list->head->next;
			free(remove);
		}
		TSListInit(list);
	}
}

int TSListInsert(TSList *list, TCPtr  data, TSize index) {
	TSListNode *newnode = TSListNodeNew(data);
	if (!newnode) return 1;

	if (!list->head) {
		list->head = list->end = newnode;
		list->previousindex = 0;
	} else if (index >= list->len) {
		list->end = list->end->next = newnode;
		list->previousindex = list->len;
	} else if (!index) {
		newnode->next = list->head;
		list->head = newnode;
		list->previousindex = 0;
	} else {
		TSListFetch(list, index - 1);

		newnode->next = list->previous->next;
		list->previous->next = newnode;
		list->previousindex += 1;
	}
	list->previous = newnode;
	list->len += 1;

	return 0;
}

void TSListReplace(TSList *list, TCPtr  data, TSize index) {
	if (TSListFetch(list, index)) return;

	list->previous->data = data;
}

void TSListConcat(TSList *list, const TSList *list2) {
	TSListNode *cur = list2->head;

	while (cur) {
		TSListAppend(list, cur->data);
		cur = cur->next;
	}
}

TSize TSListFind(TSList *list, TCPtr data) {
	list->previous = list->head;
	list->previousindex = 0;

	while (list->previous) {
		if (list->previous->data == data) break;
		list->previous = list->previous->next;
		list->previousindex++;
	}

	if (list->previous) return list->previousindex;

	list->previous = list->head;
	list->previousindex = 0;
	return -1;
}

void TSListForeach(const TSList *list, TIterFunc func) {
	if (list && func) {
		TSListNode *cur = list->head;

		while (cur) {
			func((TPtr)cur->data);
			cur = cur->next;
		}
	}
}

TPtr TSListForeachData(const TSList *list, TDataIterFunc func, TPtr userData) {
	if (list && func) {
		TSListNode *cur = list->head;

		while (cur) {
			TPtr value = func((TPtr)cur->data, userData);
			if (value) return value;
			cur = cur->next;
		}
	}

	return 0;
}

TPtr TSListGet(TSList *list, TSize index) {
	TSListNode *n = TSListFetch(list, index);

	return n ? (TPtr)n->data : 0;
}

TPtr TSListNext(TSList *list) {
	return TSListGet(list, list->previousindex + 1);
}

static inline void TSListMergeInsert(TSList *mlist, TSList *sublist) {
	if (mlist->previous) {
		mlist->previous = mlist->end = mlist->previous->next = sublist->head;
		mlist->previousindex += 1;
	} else {
		mlist->previous = mlist->end = mlist->head = sublist->head;
	}
	mlist->len += 1;

	sublist->head = sublist->head->next;
	sublist->len -= 1;
}

void TSListMerge(TSList *mlist, TSList *left, TSList *right, TCompareFunc func) {
	while (left->len || right->len) {
		if (left->len && right->len)
			TSListMergeInsert(mlist, func(left->head->data, right->head->data) ? right : left);
		else if (left->len)
			TSListMergeInsert(mlist, left);
		else
			TSListMergeInsert(mlist, right);
	}
}

void TSListSort(TSList *list, TCompareFunc func) {
	TSList left, right;
	TSize middle;

	if (!list || list->len <= 1) return;

	TSListInit(&left);
	TSListInit(&right);
	middle = list->len / 2;

	TSListFetch(list, middle - 1);

	//divide the list in two
	left.head = list->head;
	left.end = list->previous;
	left.len = middle;
	left.previous = left.head;

	right.head = list->previous->next;
	right.end = list->end;
	right.len = middle;
	right.previous = right.head;

	left.end->next = 0;
	TSListInit(list);

	TSListSort(&left, func);
	TSListSort(&right, func);

	TSListMerge(list, &left, &right, func);
}

TPtr TSListPopIndex(TSList *list, TSize index) {
	TCPtr data = 0;

	TSListNode *n = TSListFetch(list, index - 1);
	if (n) {
		if (n->next) {
			data = n->next->data;
			TSListRemovePtrFrom(list, n);
		}
	} else if (index == 0) {
		if (list->head) {
			data = list->head->data;
			TSListRemovePtrFrom(list, 0);
		}
	}

	return (TPtr)data;
}

int TSListRemove(TSList *list, TCPtr data) {
	if (list) {
		TSListNode *cur = list->head, *origin = 0;

		while (cur && cur->data != data) {
			origin = cur;
			cur = cur->next;
		}

		if (cur) {
			TSListRemovePtrFrom(list, origin);
			return 1;
		}
	}

	return 0;
}

void TSListRemovePtr(TSList *list, TSListNode *ptr) {
	if (list && ptr) {
		TSListNode *cur = list->head, *origin = 0;

		while (cur && cur != ptr) {
			origin = cur;
			cur = cur->next;
		}

		if (cur) TSListRemovePtrFrom(list, origin);
	}
}

void TSListRemoveIndex(TSList *list, TSize index) {
	if (list) {
		TSListNode *n = TSListFetch(list, index - 1);

		if (n) if (n->next) TSListRemovePtrFrom(list, n);
	}
}

void TSListRemoveIndexes(TSList *list, TSize start, TSize range) {
	if (list && range) {
		TSListNode *n = TSListFetch(list, start - 1);
		while (n && range) {
			if (!n->next) break;
			TSListRemovePtrFrom(list, n);
			range--;
		}
	}
}

void TSListRemovePtrFrom(TSList *list, TSListNode *origin) {
	if (!origin) {
		if (list->end == list->head) list->end = 0;
		list->head = list->head->next;
		list->len -= 1;
		if (list->previousindex == 0) list->previous = list->head;
		else list->previousindex -= 1;
		return;
	}

	if (origin->next) {
		if (list->end == origin->next) list->end = origin;
		origin->next = origin->next->next;
		list->len -= 1;
		list->previousindex = 0;
		list->previous = list->head;
	}
}
