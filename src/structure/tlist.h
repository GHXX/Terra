
#ifndef __included_terra_TList_h
#define __included_terra_TList_h

#include "tdefine.h"

//------------- Doubly-Linked List ---------------//

typedef struct _TListNode {
	struct _TListNode *next;
	struct _TListNode *previous;
	void *data;
} TListNode;

typedef struct _TList {
	TListNode *head;
	TListNode *end;
	size_t len;

	TListNode *previous;
	size_t previousindex;
} TList;

TList *TListNew(void);
void TListInit(TList *list);
void TListFree(TList *list,TFreeFunc func);
void TListEmpty(TList *list,TFreeFunc func);

int TListInsert(TList *list,void *data, size_t index);
static inline int TListPrepend(TList *list,void *data) { return TListInsert(list,data,0); }
static inline int TListAppend(TList *list,void *data) { return TListInsert(list,data,list->len); }

void TListForeach(TList *list, TIterFunc func);
void *TListForeachData(TList *list, TDataIterFunc func, void *user_data);
void *TListGet(TList *list,size_t index);
int TListFind(TList *list,void *data);

void TListSort(TList *list);

void *TListPopIndex(TList *list, size_t index);
void TListRemove(TList *list, void *data);
void TListRemovePtr(TList *list, TListNode *ptr);
void TListRemoveIndex(TList *list, size_t index);

#define TListPush(l,d) TListPrepend(l,d)
static inline void *TListPop(TList *l) { return TListPopIndex(l,l->len-1);}

//------------- Single-Linked List ---------------//

typedef struct _TSListNode {
	struct _TSListNode *next;
	const void *data;
} TSListNode;


typedef struct _TSList {
	TSListNode *head;
	TSListNode *end;
	size_t len;

	TSListNode *previous;
	size_t previousindex;
} TSList;

static inline int TSListValidU(const TSList *list, size_t index) { return index < list->len; }

TSList *TSListNew(void);
void TSListInit(TSList *list);
void TSListFree(TSList *list,TFreeFunc func);
void TSListEmpty(TSList *list,TFreeFunc func);

int TSListInsert(TSList *list,const void *data, size_t index);
static inline int TSListPrepend(TSList *list,const void *data) { return TSListInsert(list,data,0); }
static inline int TSListAppend(TSList *list,const void *data) { return TSListInsert(list,data,list->len); }

void TSListReplace(TSList *list,const void *data, size_t index);

void TSListConcat(TSList *list,const TSList *list2);

int TSListFind(const TList *list, const void *data);
void TSListForeach(const TSList *list,TIterFunc func);
void *TSListForeachData(const TSList *list,TDataIterFunc func,void *user_data);

void *TSListGet(TSList *list,size_t index);

static inline void *TSListFirst(TSList *list) { return TSListGet(list, 0); }
void *TSListNext(TSList *list);

void TSListSort(TSList *list,TCompareFunc func);

void *TSListPopIndex(TSList *list, size_t index);

void TSListRemove(TSList *list,const void *data);
void TSListRemovePtr(TSList *list, TSListNode *ptr);
void TSListRemoveIndex(TSList *list, size_t index);
void TSListRemoveIndexes(TSList *list, size_t start, size_t range);
void TSListRemovePtrFrom(TSList *list, TSListNode *origin);

#define TSListPush(l,d) TSListPrepend(l,d)
static inline void *TSListPop(TSList *l) { return TSListPopIndex(l, 0); }

#endif
