
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

TList *TListNew(void);
void TListInit(TList *list);
void TListFree(TList *list, TFreeFunc func);
void TListEmpty(TList *list, TFreeFunc func);

int TListInsert(TList *list, TPtr data, TSize index);
static inline int TListPrepend(TList *list, TPtr data) { return TListInsert(list, data, 0); }
static inline int TListAppend(TList *list, TPtr data) { return TListInsert(list, data, list->len); }

void TListForeach(TList *list, TIterFunc func);
TPtr TListForeachData(TList *list, TDataIterFunc func, TPtr userData);
TPtr TListGet(TList *list, TSize index);
int TListFind(TList *list, TPtr data);

void TListSort(TList *list);

TPtr TListPopIndex(TList *list, TSize index);
void TListRemove(TList *list, TPtr data);
void TListRemovePtr(TList *list, TListNode *ptr);
void TListRemoveIndex(TList *list, TSize index);

#define TListPush(l,d) TListPrepend(l,d)
#define TListPop(l) TListPopIndex(l,0)

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

static inline int TSListValid(const TSList *list, TSize index) { return index < list->len; }

TSList *TSListNew(void);
void TSListInit(TSList *list);
void TSListFree(TSList *list, TFreeFunc func);
void TSListEmpty(TSList *list, TFreeFunc func);

int TSListInsert(TSList *list, TCPtr data, TSize index);
static inline int TSListPrepend(TSList *list, TCPtr data) { return TSListInsert(list, data, 0); }
static inline int TSListAppend(TSList *list, TCPtr data) { return TSListInsert(list, data, list->len); }

void TSListReplace(TSList *list, TCPtr data, TSize index);

void TSListConcat(TSList *list, const TSList *list2);

int TSListFind(const TList *list, TCPtr data);
void TSListForeach(const TSList *list, TIterFunc func);
TPtr TSListForeachData(const TSList *list, TDataIterFunc func, TPtr userData);

TPtr TSListGet(TSList *list, TSize index);

static inline TPtr TSListFirst(TSList *list) { return TSListGet(list, 0); }
static inline TPtr TSListLast(TSList *l) { return TSListGet(l, l->len - 1); }
TPtr TSListNext(TSList *list);

void TSListSort(TSList *list, TCompareFunc func);

TPtr TSListPopIndex(TSList *list, TSize index);

void TSListRemove(TSList *list, TCPtr data);
void TSListRemovePtr(TSList *list, TSListNode *ptr);
void TSListRemoveIndex(TSList *list, TSize index);
void TSListRemoveIndexes(TSList *list, TSize start, TSize range);
void TSListRemovePtrFrom(TSList *list, TSListNode *origin);

#define TSListPush(l,d) TSListPrepend(l,d)
#define TSListPop(l) TSListPopIndex(l, 0)

#endif
