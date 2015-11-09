
#include "stdafx.h"

#include "utility/tconvert.h"

#include "tdata.h"

#include "talloc.h"
#include "terror.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TDATACPY(data, size, dest) { \
	void *d = TAlloc(size); \
	if(d) { \
		memcpy(d, &data, (size_t) size); \
		dest = d; \
	} \
}

struct _TData {
	TCPtr data;
	TSize size;

	TUInt16 type;
};

static inline TData *TDataNew(TUInt16 type)
{
	TData *content = TAllocData(TData);
	if (content) {
		content->data = 0;
		content->size = 0;
		content->type = type;
	}

	return content;
}

TData *TDataFromPtr(TPtr data, TSize size)
{
	TData *content = TDataNew(data && size ? T_DATA_UNKNOWN : T_DATA_NULL);
	if (content && data && size) {
		content->size = size;
		TDATACPY(data, content->size, content->data);
		if (!content->data) {
			content->type = T_DATA_NULL;
			content->size = 0;
		}
	}

	return content;
}

TData *TDataFromConstPtr(TCPtr data)
{
	TData *content = TDataNew(data ? T_DATA_CONST_UNKNOWN : T_DATA_NULL);
	if (content)
		content->data = data;

	return content;
}

TData *TDataFromInt8(TInt8 data)
{
	TData *content = TDataNew(T_DATA_INT8);
	if (content) {
		content->size = sizeof(TInt8);
		TDATACPY(data, content->size, content->data);
		if (!content->data) {
			content->type = T_DATA_NULL;
			content->size = 0;
		}
	}

	return content;
}

TData *TDataFromUInt8(TUInt8 data)
{
	TData *content = TDataNew(T_DATA_UINT8);
	if (content) {
		content->size = sizeof(TUInt8);
		TDATACPY(data, content->size, content->data);
		if (!content->data) {
			content->type = T_DATA_NULL;
			content->size = 0;
		}
	}

	return content;
}

TData *TDataFromInt16(TInt16 data)
{
	TData *content = TDataNew(T_DATA_INT16);
	if (content) {
		content->size = sizeof(TInt16);
		TDATACPY(data, content->size, content->data);
		if (!content->data) {
			content->type = T_DATA_NULL;
			content->size = 0;
		}
	}

	return content;
}

TData *TDataFromUInt16(TUInt16 data)
{
	TData *content = TDataNew(T_DATA_UINT16);
	if (content) {
		content->size = sizeof(TUInt16);
		TDATACPY(data, content->size, content->data);
		if (!content->data) {
			content->type = T_DATA_NULL;
			content->size = 0;
		}
	}

	return content;
}

TData *TDataFromInt32(TInt32 data)
{
	TData *content = TDataNew(T_DATA_INT32);
	if (content) {
		content->size = sizeof(TInt32);
		TDATACPY(data, content->size, content->data);
		if (!content->data) {
			content->type = T_DATA_NULL;
			content->size = 0;
		}
	}

	return content;
}

TData *TDataFromUInt32(TUInt32 data)
{
	TData *content = TDataNew(T_DATA_UINT32);
	if (content) {
		content->size = sizeof(TUInt32);
		TDATACPY(data, content->size, content->data);
		if (!content->data) {
			content->type = T_DATA_NULL;
			content->size = 0;
		}
	}

	return content;
}

#ifdef _X86_64
TData *TDataFromInt64(TInt64 data)
{
	TData *content = TDataNew(T_DATA_INT64);
	if (content) {
		content->size = sizeof(TInt64);
		TDATACPY(data, content->size, content->data);
		if (!content->data) {
			content->type = T_DATA_NULL;
			content->size = 0;
		}
	}

	return content;
}

TData *TDataFromUInt64(TUInt64 data)
{
	TData *content = TDataNew(T_DATA_UINT64);
	if (content) {
		content->size = sizeof(TUInt64);
		TDATACPY(data, content->size, content->data);
		if (!content->data) {
			content->type = T_DATA_NULL;
			content->size = 0;
		}
	}

	return content;
}
#endif

TData *TDataFromConstString(const char *data)
{
	TData *content = TDataNew(data ? T_DATA_CONST_STRING : T_DATA_NULL);
	if (content) {
		content->data = data;
	}

	return content;
}

TData *TDataFromString(char *data)
{
	TData *content = TDataNew(data ? T_DATA_STRING : T_DATA_NULL);
	if (content && data) {
		content->size = (strlen(data) + 1) * sizeof(char);
		TDATACPY(data, content->size, content->data);
		if (!content->data) {
			content->type = T_DATA_NULL;
			content->size = 0;
		}
	}

	return content;
}

TData *TDataFromChar(char data)
{
	TData *content = TDataNew(T_DATA_CHAR);
	if (content) {
		content->size = sizeof(char);
		TDATACPY(data, content->size, content->data);
		if (!content->data) {
			content->type = T_DATA_NULL;
			content->size = 0;
		}
	}

	return content;
}

TData *TDataFromFloat(float data)
{
	TData *content = TDataNew(T_DATA_FLOAT);
	if (content) {
		content->size = sizeof(float);
		TDATACPY(data, content->size, content->data);
		if (!content->data) {
			content->type = T_DATA_NULL;
			content->size = 0;
		}
	}

	return content;
}

TData *TDataFromDouble(double data)
{
	TData *content = TDataNew(T_DATA_DOUBLE);
	if (content) {
		content->size = sizeof(double);
		TDATACPY(data, content->size, content->data);
		if (!content->data) {
			content->type = T_DATA_NULL;
			content->size = 0;
		}
	}

	return content;
}

TUInt16 TDataGetType(const TData *context)
{
	if (context) return context->type;
	return T_DATA_NULL;
}

TCPtr TDataToConstPointer(const TData *context, TUInt16 *type)
{
	if (context) return convertToConstPointer(context->data, context->type);
	return 0;
}

TPtr TDataToPointer(const TData *context, TUInt16 *type)
{
	if (context) return convertToPointer(context->data, context->type);
	return 0;
}

const char *TDataToConstString(const TData *context)
{
	if (context) return convertToConstString(context->data, context->type);
	return 0;
}

char *TDataToString(const TData *context)
{
	if (context) return convertToString(context->data, context->type);
	return 0;
}

char TDataToChar(const TData *context)
{
	if (context) return convertToChar(context->data, context->type);
	return 0;
}

TInt8 TDataToInt8(const TData *context)
{
	if (context) return convertToInt8(context->data, context->type);
	return 0;
}

TUInt8 TDataToUInt8(const TData *context)
{
	if (context) return convertToUInt8(context->data, context->type);
	return 0;
}

TInt16 TDataToInt16(const TData *context)
{
	if (context) return convertToInt16(context->data, context->type);
	return 0;
}

TUInt16 TDataToUInt16(const TData *context)
{
	if (context) return convertToUInt16(context->data, context->type);
	return 0;
}

TInt32 TDataToInt32(const TData *context)
{
	if (context) return convertToInt32(context->data, context->type);
	return 0;
}

TUInt32 TDataToUInt32(const TData *context)
{
	if (context) return convertToUInt32(context->data, context->type);
	return 0;
}

#ifdef _X86_64
TInt64 TDataToInt64(const TData *context)
{
	if (context) return convertToInt64(context->data, context->type);
	return 0;
}

TUInt64 TDataToUInt64(const TData *context)
{
	if (context) return convertToUInt64(context->data, context->type);
	return 0;
}
#endif

float TDataToFloat(const TData *context)
{
	if (context) return convertToFloat(context->data, context->type);
	return 0;
}

double TDataToDouble(const TData *context)
{
	if (context) return convertToDouble(context->data, context->type);
	return 0;
}

void TDataFree(TData *context)
{
	if (context) {
		if (context->type != T_DATA_CONST_UNKNOWN &&
			context->type != T_DATA_CONST_STRING)
			TFree((TPtr) context->data);

		TFree(context);
	}
}

#ifdef __cplusplus
}
#endif
