
#include "stdafx.h"

#include "tarray.h"

#include "utility/tinteger.h"

#include "talloc.h"
#include "terror.h"

static inline int TArrayGrow(TArray *arr, TSize minsize)
{
	TSize newSize;

	if (arr->size == SIZE_MAX) {
		TErrorReport(T_ERROR_SIZE_EXCEEDED);
		return T_ERROR_SIZE_EXCEEDED;
	}

	newSize = TIntegerUpperPowerOfTwo(minsize);
	if (newSize < arr->size) newSize = SIZE_MAX;

	TArrayResize(arr, newSize);
	return T_ERROR_NONE;
}

static inline void TArrayShrink(TArray *arr)
{
	if (arr->len >= 1)
		TArrayResize(arr, TIntegerUpperPowerOfTwo(arr->len));
}

TArray *TArrayNew(TSize size)
{
	TArray *arr = TAllocData(TArray);
	if (arr) TArrayInit(arr, size);

	return arr;
}

void TArrayInit(TArray *arr,TSize size)
{
	memset(arr, 0, sizeof(TArray));

	if (size) TArrayResize(arr, size);
}

void TArrayFree(TArray *arr,TFreeFunc func)
{
	if(arr) {
		TArrayEmpty(arr, func);

		TFree(arr->data);
		TFree(arr);
	}
}

int TArrayResize(TArray *arr, TSize _size)
{
	if (arr) {
		if (_size == 0) {
			TFree(arr->data);
			arr->size = arr->used = arr->len = 0;
			arr->data = 0;
		} else if(_size != arr->size) {
			void *nptr = TRAlloc(arr->data, sizeof(TPtr) * _size);
			if(!nptr) return T_ERROR_OUT_OF_MEMORY;
			arr->data = nptr;

			if (_size > arr->size) memset(arr->data + arr->size, 0, (size_t) sizeof(TPtr) *(_size - arr->size));
			else arr->len = TMIN(arr->len, _size);

			arr->size = _size;
		}
	}

	return T_ERROR_NONE;
}

TArray *TArrayCopy(const TArray *arr, TCopyFunc data_cpy)
{
	TArray *cpy;
	TSize i = 0;
	
	if(!arr) return 0;

	cpy = TArrayNew(arr->size);
	
	cpy->used = arr->used;
	cpy->len = arr->len;

	for(; i < arr->len; ++i) cpy->data[i] = data_cpy ? data_cpy(arr->data[i]) : arr->data[i];

	return cpy;
}

void TArrayCopyInplace(TArray *to, const TArray *arr, TCopyFunc data_cpy)
{
	TSize i = 0;
	if(!to || !arr) return;

	if (to->size < arr->size) TArrayResize(to, arr->len);

	to->used = arr->used;
	to->len = arr->len;

	for(; i < arr->len; ++i) to->data[i] = data_cpy ? data_cpy(arr->data[i]) : arr->data[i];
}

void TArrayEmpty(TArray *arr,TFreeFunc func)
{
	if(func) {
		TSize i = 0;
		for(; i < arr->len && arr->used; ++i) {
			if(arr->data[i]) {
				arr->used--;
				func(arr->data[i]);
				arr->data[i] = 0;
			}
		}
	}
	arr->len = 0;
}

void TArrayEmptyFull(TArray *arr,TFreeFunc func)
{
	TArrayEmpty(arr,func);
	TFree(arr->data);
	arr->data = 0;
	arr->size = 0;
}

TSize TArrayAppend(TArray *arr, TPtr data)
{
	TSize index = arr->len;
	if (TArrayInsert(arr, data, index))
		return (TSize)-1;
	return index;
}

int TArrayInsert(TArray *arr, TPtr data, TSize index)
{
	if (index >= arr->size) {
		// Grow the array to get enough space
		int error = TArrayGrow(arr, index + 1);
		if (error) return error;
	}
	if (!arr->data[index]) arr->used += 1;
	arr->data[index] = data;
	arr->len = TMAX(index + 1, arr->len);

	return T_ERROR_NONE;
}

void TArrayMove(TArray *arr, TSize start, TSize end, int space)
{
	if (!arr || start > end || !TArrayValid(arr, start) || !space) return;

	if(end > arr->len) end = arr->len;

	if(space > 0) {
		int i;
		int li = (int)end + space - 1;

		if((TSize) li > arr->size) TArrayGrow(arr,li);

		for(i = end-1; i >= (int)start; i--) {
			arr->data[i+space] = arr->data[i];
			arr->data[i] = 0;
		}

		arr->len = (TSize)(li + 1);
	} else {
		TSize i;

		if((int)start + space < 0) start = (TSize)(-space);

		for(i = start; i < end; ++i) {
			arr->data[i + space] = arr->data[i];
			arr->data[i] = 0;
		}

		if(arr->len <= (arr->size >> 2)) TArrayShrink(arr);

		arr->len = TMAX(end,arr->len);
	}

}

void TArrayForeach(TArray *arr, TIterFunc func)
{
	TSize i = 0;
	for(; i < arr->len; ++i) func(arr->data[i]);
}

TPtr TArrayForeachData(TArray *arr, TDataIterFunc func, TPtr user_data)
{
	TSize i = 0;
	for(; i < arr->len; ++i) {
		TPtr value = func(arr->data[i], user_data);
		if(value) return value;
	}

	return 0;
}

TSize TArrayFind(TArray *arr, TCPtr data)
{
	TSize i = 0;

	for(; i < arr->len; ++i)
		if(data == arr->data[i])
			return i;

	return i;
}

void TArraySort(TArray *arr)
{
	//TODO
}

TPtr TArrayPopIndex(TArray *arr,TSize index) {
	TPtr data;

	if(index >= arr->len) return 0;

	data = arr->data[index];
	if(data) arr->used -= 1;

	for(; index < arr->len-1; ++index) arr->data[index] = arr->data[index+1];
	arr->len -= 1;

	if(arr->len <= (arr->size >> 2)) TArrayShrink(arr);

	return data;
}

void TArrayRemove(TArray *arr,TSize index)
{
	if(index >= arr->len) return;

	if(arr->data[index]) arr->used -= 1;

	memcpy(arr->data[index], arr->data[index + 1], sizeof(TPtr) *(arr->len - (index + 1)));
	arr->len -= 1;

	if(arr->len <= (arr->size >> 2)) TArrayShrink(arr);
}

void TArrayRemoveFast(TArray *arr,TSize index)
{
	if(index >= arr->len) return;

	if(arr->data[index]) arr->used -= 1;

	arr->data[index] = arr->data[arr->len-1];
	arr->data[arr->len-1] = 0;
	arr->len -= 1;

	if(arr->len <= (arr->size >> 2)) TArrayShrink(arr);
}

void TArrayRemoveClear(TArray *arr,TSize index)
{
	if(index >= arr->len) return;

	if(arr->data[index]) arr->used -= 1;

	arr->data[index] = 0;
}

//------------- Integer TArray ---------------//

static inline void TIArrayGrow(TIArray *arr, TSize minsize)
{
	TIArrayResize(arr,TIntegerUpperPowerOfTwo(minsize));
}

static inline void TIArrayShrink(TIArray *arr)
{
	if(arr->len >= 1)
		TIArrayResize(arr,TIntegerUpperPowerOfTwo(arr->len));
}

TIArray *TIArrayNew(TSize size)
{
	TIArray *arr = TAllocData(TIArray);
	if(arr) TIArrayInit(arr,size);

	return arr;
}

void TIArrayInit(TIArray *arr, TSize size)
{
	arr->data = 0;
	arr->size = arr->len = 0;

	TIArrayResize(arr,size);
}

void TIArrayFree(TIArray *arr)
{
	if(arr) {
		TFree(arr->data);
		TFree(arr);
	}
}

int TIArrayResize(TIArray *arr, TSize _size)
{
	if(arr) {
		if(_size == 0) {
			TFree(arr->data);
			arr->data = 0;
			arr->len = arr->size = 0;
		} else {
			void *nptr = TRAlloc(arr->data,sizeof(int) * _size);
			if(!nptr) return 1;
			arr->data = (int *) nptr;

			if(_size > arr->size) memset(arr->data+arr->size,0,sizeof(int) *(_size-arr->size));
			else arr->len = TMIN(arr->len,_size);

			arr->size = _size;
		}
	}

	return 0;
}

TIArray *TIArrayCopy(const TIArray *arr)
{
	TIArray *cpy;
	TSize i = 0;
	
	if(!arr) return 0;

	cpy = TIArrayNew(arr->size);
	
	cpy->len = arr->len;

	for(; i < arr->len; ++i) cpy->data[i] = arr->data[i];

	return cpy;
}

void TIArrayCopyInplace(TIArray *to, const TIArray *arr)
{
	TSize i = 0;
	if(!to || !arr) return;

	if(to->size < arr->size) TIArrayGrow(to,arr->len);

	to->len = arr->len;

	for(; i < arr->len; ++i) to->data[i] = arr->data[i];
}

void TIArrayEmpty(TIArray *arr)
{
	arr->len = 0;
}

void TIArrayEmptyFull(TIArray *arr)
{
	arr->len = 0;
	TFree(arr->data);
	arr->data = 0;
	arr->size = 0;
}

TSize TIArrayAppend(TIArray *arr, int data)
{
	TIArrayInsert(arr,data,arr->len);
	return arr->len-1;
}

void TIArrayInsert(TIArray *arr, int data, TSize index)
{
	if(index >= arr->size) TIArrayGrow(arr,index+1);
	arr->data[index] = data;
	arr->len = TMAX(index+1,arr->len);
}

void TIArrayMove(TIArray *arr, TSize start, TSize end, int space)
{
	if(!arr || start > end || !TIArrayValid(arr,start) || !space) return;

	if(end > arr->len) end = arr->len;

	if(space > 0) {
		int i;
		int li = (int)end + space - 1;

		if((TSize) li > arr->size) TIArrayGrow(arr,li);

		for(i = end-1; i >= (int)start; i--) {
			arr->data[i+space] = arr->data[i];
			arr->data[i] = 0;
		}

		arr->len = (TSize)(li + 1);
	} else {
		TSize i;

		if((int)start + space < 0) start = (TSize)(-space);

		for(i = start; i < end; ++i) {
			arr->data[i+space] = arr->data[i];
			arr->data[i] = 0;
		}

		if(arr->len <= (arr->size >> 2)) TIArrayShrink(arr);

		arr->len = TMAX(end,arr->len);
	}

}

void TIArrayForeach(TArray *arr, TIterFunc func)
{
	TSize i = 0;
	for(; i < arr->len; ++i) func(&arr->data[i]);
}

TPtr TIArrayForeachData(TArray *arr, TDataIterFunc func, TPtr user_data)
{
	TSize i = 0;
	for(; i < arr->len; ++i) {
		void *value = func(&arr->data[i],user_data);
		if(value) return value;
	}

	return 0;
}

void TIArraySort(TIArray *arr)
{
	//TODO
}

int TIArrayPopIndex(TIArray *arr, TSize index)
{
	int data;

	if(index >= arr->len) return 0;

	data = arr->data[index];

	for(; index < arr->len-1; ++index) arr->data[index] = arr->data[index+1];
	arr->len -= 1;

	if(arr->len <= (arr->size >> 2)) TIArrayShrink(arr);

	return data;
}

void TIArrayRemove(TIArray *arr, TSize index)
{
	if(index >= arr->len) return;

	memcpy(arr->data + index,arr->data +(index+1),sizeof(int) *(arr->len - (index + 1)));
	arr->len -= 1;

	if(arr->len <= (arr->size >> 2)) TIArrayShrink(arr);
}

void TIArrayRemoveFast(TIArray *arr, TSize index)
{
	if(index >= arr->len) return;

	arr->data[index] = arr->data[arr->len-1];
	arr->data[arr->len-1] = 0;
	arr->len -= 1;

	if(arr->len <= (arr->size >> 2)) TIArrayShrink(arr);
}
