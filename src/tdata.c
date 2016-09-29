
#include "stdafx.h"

#include "utility/tconvert.h"

#include "tdata.h"

#include "talloc.h"
#include "terror.h"
#include "utility/tstring.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TDATACPY(data, size, dest) { \
	TPtr d = TAlloc(size); \
	if(d) { \
		memcpy(d, &data, (size_t) size); \
		dest = d; \
	} \
}

struct _TData {
	TCPtr data;
	TSize size;

	TUInt8 type;
};

static inline TData *TDataNew(TUInt8 type) {
	TData *content = TAllocData(TData);
	if (content) {
		content->data = 0;
		content->size = 0;
		content->type = type;
	}

	return content;
}

TData *TDataCopy(TData *data) {
	TData *cpy;

	if (!data) return 0;

	cpy = TDataNew(data->type);
	if (cpy && data) {
		cpy->size = data->size;
		TDATACPY(data->data, cpy->size, cpy->data);
		if (!cpy->data) {
			cpy->type = T_DATA_NULL;
			cpy->size = 0;
		}
	}

	return cpy;
}

TData *TDataFromPtr(TPtr data, TSize size) {
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

TData *TDataFromConstPtr(TCPtr data) {
	TData *content = TDataNew(data ? T_DATA_CONST_UNKNOWN : T_DATA_NULL);
	if (content)
		content->data = data;

	return content;
}

TData *TDataFromInt8(TInt8 data) {
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

TData *TDataFromUInt8(TUInt8 data) {
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

TData *TDataFromInt16(TInt16 data) {
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

TData *TDataFromUInt16(TUInt16 data) {
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

TData *TDataFromInt32(TInt32 data) {
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

TData *TDataFromUInt32(TUInt32 data) {
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

TData *TDataFromInt64(TInt64 data) {
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

TData *TDataFromUInt64(TUInt64 data) {
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

TData *TDataFromString(const char *data) {
	TData *content = TDataNew(data ? T_DATA_STRING : T_DATA_NULL);
	if (content && data) {
		content->size = TStringSize(data);
		TDATACPY(data, content->size, content->data);
		if (!content->data) {
			content->type = T_DATA_NULL;
			content->size = 0;
		}
	}

	return content;
}

TData *TDataFromChar(char data) {
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

TData *TDataFromFloat(float data) {
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

TData *TDataFromDouble(double data) {
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

TUInt8 TDataGetType(const TData *context) {
	if (context) return context->type;
	return T_DATA_NULL;
}

TCPtr TDataToConstPointer(const TData *context, TUInt16 *type) {
	if (context) return TConvertToConstPointer(context->data, context->type);
	return 0;
}

TPtr TDataToPointer(const TData *context, TUInt16 *type) {
	if (context) return TConvertToPointer(context->data, context->type);
	return 0;
}

char *TDataToString(const TData *context) {
	if (context) return TConvertToString(context->data, context->type);
	return 0;
}

char TDataToChar(const TData *context) {
	if (context) return TConvertToChar(context->data, context->type);
	return 0;
}

TInt8 TDataToInt8(const TData *context) {
	if (context) return TConvertToInt8(context->data, context->type);
	return 0;
}

TUInt8 TDataToUInt8(const TData *context) {
	if (context) return TConvertToUInt8(context->data, context->type);
	return 0;
}

TInt16 TDataToInt16(const TData *context) {
	if (context) return TConvertToInt16(context->data, context->type);
	return 0;
}

TUInt16 TDataToUInt16(const TData *context) {
	if (context) return TConvertToUInt16(context->data, context->type);
	return 0;
}

TInt32 TDataToInt32(const TData *context) {
	if (context) return TConvertToInt32(context->data, context->type);
	return 0;
}

TUInt32 TDataToUInt32(const TData *context) {
	if (context) return TConvertToUInt32(context->data, context->type);
	return 0;
}

TInt64 TDataToInt64(const TData *context) {
	if (context) return TConvertToInt64(context->data, context->type);
	return 0;
}

TUInt64 TDataToUInt64(const TData *context) {
	if (context) return TConvertToUInt64(context->data, context->type);
	return 0;
}

float TDataToFloat(const TData *context) {
	if (context) return TConvertToFloat(context->data, context->type);
	return 0;
}

double TDataToDouble(const TData *context) {
	if (context) return TConvertToDouble(context->data, context->type);
	return 0;
}

void TDataFree(TData *context) {
	if (context) {
		if (context->type != T_DATA_CONST_UNKNOWN)
			TFree((TPtr)context->data);

		TFree(context);
	}
}

#ifdef __cplusplus
}
#endif
