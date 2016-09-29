
#ifndef __included_terra_array_h
#define __included_terra_array_h

//------------- Ptr Array ---------------//

#include "tdefine.h"

typedef struct _TArray {
	void **data;
	TSize size, used;
	TSize len;
} TArray;

TArray *TArrayNew(TSize size);
void TArrayInit(TArray *arr, TSize size);
void TArrayFree(TArray *arr, TFreeFunc func);

int TArrayResize(TArray *arr, TSize _size);

TArray *TArrayCopy(const TArray *arr, TCopyFunc data_cpy);
void TArrayCopyInplace(TArray *to, const TArray *arr, TCopyFunc data_cpy);

void TArrayEmpty(TArray *arr, TFreeFunc func);
void TArrayEmptyFull(TArray *arr, TFreeFunc func);

TSize TArrayAppend(TArray *arr, TPtr data);
int TArrayInsert(TArray *arr, TPtr data, TSize index);

void TArrayMove(TArray *arr, TSize start, TSize end, int space);

void TArrayForeach(TArray *arr, TIterFunc func);
TPtr TArrayForeachData(TArray *arr, TDataIterFunc func, TPtr user_data);

static inline unsigned char TArrayValid(const TArray *arr, TSize index) {
	return index < arr->len;
}

static inline TPtr TArrayGet(const TArray *arr, TSize index) {
	if (!TArrayValid(arr, index)) return 0;
	return arr->data[index];
}

TSize TArrayFind(TArray *arr, TCPtr data);

void TArraySort(TArray *arr);

void *TArrayPopIndex(TArray *arr, TSize index);

void TArrayRemove(TArray *arr, TSize index);
void TArrayRemoveFast(TArray *arr, TSize index); // grab end element and put it in place
void TArrayRemoveClear(TArray *arr, TSize index); // replace with 0

#define TArrayPush TArrayAppend
static inline TPtr TArrayPop(TArray *a) { return TArrayPopIndex(a, a->len - 1); }
static inline TPtr TArrayLast(TArray *a) { return TArrayGet(a, a->len - 1); }

//------------- Integer Array ---------------//

typedef struct _TIArray {
	int *data;
	TSize size;
	TSize len;
} TIArray;

TIArray *TIArrayNew(TSize size);
void TIArrayInit(TIArray *arr, TSize size);
void TIArrayFree(TIArray *arr);

int TIArrayResize(TIArray *arr, TSize _size);

TIArray *TIArrayCopy(const TIArray *arr);
void TIArrayCopyInplace(TIArray *to, const TIArray *arr);

void TIArrayEmpty(TIArray *arr);
void TIArrayEmptyFull(TIArray *arr);

TSize TIArrayAppend(TIArray *arr, int data);
void TIArrayInsert(TIArray *arr, int data, TSize index);

void TIArrayMove(TIArray *arr, TSize start, TSize end, int space);

void TIArrayForeach(TIArray *arr, TIterFunc func);
TPtr TIArrayForeachData(TIArray *arr, TDataIterFunc func, TPtr userData);

static inline unsigned char TIArrayValid(TIArray *arr, TSize index) {
	return index < arr->len;
}

static inline int TIntArrayGet(TIArray *arr, TSize index) {
	if (!TIArrayValid(arr, index)) return 0;
	return arr->data[index];
}

void TIArraySort(TIArray *arr);

int TIArrayPopIndex(TIArray *arr, TSize index);
void TIArrayRemove(TIArray *arr, TSize index);
void TIArrayRemoveFast(TIArray *arr, TSize index); // grab end element and put it in place

#define TIArrayPush TIArrayAppend
static inline int TIArrayPop(TIArray *a) { return TIArrayPopIndex(a, a->len - 1); }

#endif
