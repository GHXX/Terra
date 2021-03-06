
#ifndef __included_terra_TList_h
#define __included_terra_TList_h

#include "tdefine.h"

//------------- Doubly-Linked List ---------------//

typedef struct _TListNode {
	struct _TListNode *next;
	struct _TListNode *previous;
	TPtr data;
} TListNode;

typedef struct _TList {
	TListNode *head;
	TListNode *end;
	TSize len;

	TListNode *previous;
	TSize previousindex;
} TList;

#define TListLength(l) (l)->len

TList *TListNew(void);
void TListInit(TList *list);
void TListFree(TList *list, TFreeFunc func);
void TListEmpty(TList *list, TFreeFunc func);

int TListInsert(TList *list, TPtr data, TSize index);
static inline int TListPrepend(TList *list, TPtr data) { return TListInsert(list, data, 0); }
static inline int TListAppend(TList *list, TPtr data) { return TListInsert(list, data, TListLength(list)); }

void TListForeach(TList *list, TIterFunc func);
TPtr TListForeachData(TList *list, TDataIterFunc func, TPtr userData);
TPtr TListGet(TList *list, TSize index);
int TListFind(TList *list, TPtr data);

void TListSort(TList *list);

TPtr TListPopIndex(TList *list, TSize index);
void TListRemove(TList *list, TPtr data);
void TListRemovePtr(TList *list, TListNode *ptr);
void TListRemoveIndex(TList *list, TSize index);

#define TListPush(l,d) TListPrepend(l, d)
#define TListPop(l) TListPopIndex(l, 0)

//------------- Single-Linked List ---------------//

typedef struct _TSListNode {
	struct _TSListNode *next;
	TCPtr data;
} TSListNode;

typedef struct _TSList {
	TSListNode *head;
	TSListNode *end;
	TSize len;

	TSListNode *previous;
	TSize previousindex;
} TSList;

#define TSListLength(l) (l)->len

static inline int TSListValid(const TSList *list, TSize index) { return index < TSListLength(list); }

TSList *TSListNew(void);
void TSListInit(TSList *list);
void TSListFree(TSList *list, TFreeFunc func);
void TSListEmpty(TSList *list, TFreeFunc func);

int TSListInsert(TSList *list, TCPtr data, TSize index);
static inline int TSListPrepend(TSList *list, TCPtr data) { return TSListInsert(list, data, 0); }
static inline int TSListAppend(TSList *list, TCPtr data) { return TSListInsert(list, data, TSListLength(list)); }

void TSListReplace(TSList *list, TCPtr data, TSize index);

void TSListConcat(TSList *list, const TSList *list2);

TSize TSListFind(TSList *list, TCPtr data);
void TSListForeach(const TSList *list, TIterFunc func);
TPtr TSListForeachData(const TSList *list, TDataIterFunc func, TPtr userData);

TPtr TSListGet(TSList *list, TSize index);

#define TSListFirst(l) TSListGet(l, 0)
static inline TPtr TSListLast(TSList *l) { return TSListGet(l, TSListLength(l) - 1); }
TPtr TSListNext(TSList *list);

void TSListSort(TSList *list, TCompareFunc func);

TPtr TSListPopIndex(TSList *list, TSize index);

int TSListRemove(TSList *list, TCPtr data);
void TSListRemovePtr(TSList *list, TSListNode *ptr);
void TSListRemoveIndex(TSList *list, TSize index);
void TSListRemoveIndexes(TSList *list, TSize start, TSize range);
void TSListRemovePtrFrom(TSList *list, TSListNode *origin);

#define TSListPush(l,d) TSListPrepend(l, d)
#define TSListPop(l) TSListPopIndex(l, 0)

#endif
