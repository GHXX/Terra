#include "stdafx.h"

#include "tconvert.h"

#include "talloc.h"
#include "terror.h"
#include "tstring.h"

// https://msdn.microsoft.com/en-us/library/s3f49ktz.aspx

TPtr convertToPointer(TCPtr data, TUInt16 type) {
	if (type == T_DATA_CONST_UNKNOWN || type == T_DATA_UNKNOWN || type >= T_DATA_AMOUNT) {
		return (TPtr)data;
	}

	TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
	return 0;
}

TCPtr convertToConstPointer(TCPtr data, TUInt16 type) {
	if (type == T_DATA_CONST_UNKNOWN || type == T_DATA_UNKNOWN || type >= T_DATA_AMOUNT) {
		return data;
	}

	TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
	return 0;
}

const char *convertToConstString(TCPtr data, TUInt16 type) {
	if (type == T_DATA_CONST_STRING || type == T_DATA_STRING) {
		return (const char *)data;
	}

	TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
	return 0;
}

char *convertToString(TCPtr data, TUInt16 type) {
	if (type == T_DATA_CONST_STRING) {
		return TStringCopy((const char *)data);
	}
	else if (type == T_DATA_STRING) {
		return TStringCopy((char *)data);
	}
	else if (type == T_DATA_CHAR) {
		char *res = TAlloc(sizeof(char) * 2);
		*res = *(char *)data;
		*(res + 1) = 0;
		return res;
	}
	else if (type == T_DATA_NULL) {
		return TStringCopy("<null>");
	}
	else if (type == T_DATA_DOUBLE) {
		int size = 17 * sizeof(char);
		char *res = TAlloc(size);
		snprintf(res, size, "%lf", *(double *)data);
		return res;
	}
	else if (type == T_DATA_FLOAT) {
		int size = 9 * sizeof(char);
		char *res = TAlloc(size);
		snprintf(res, size, "%lf", *(double *)data);
		return res;
	}
	else if (type == T_DATA_INT8) {
		int size = 5 * sizeof(char);
		char *res = TAlloc(size);
		snprintf(res, size, "%lf", *(double *)data);
		return res;
	}
	else if (type == T_DATA_UINT8) {
		int size = 4 * sizeof(char);
		char *res = TAlloc(size);
		snprintf(res, size, "%lf", *(double *)data);
		return res;
	}
	else if (type == T_DATA_INT16) {
		int size = 7 * sizeof(char);
		char *res = TAlloc(size);
		snprintf(res, size, "%lf", *(double *)data);
		return res;
	}
	else if (type == T_DATA_UINT16) {
		int size = 6 * sizeof(char);
		char *res = TAlloc(size);
		snprintf(res, size, "%lf", *(double *)data);
		return res;
	}
	else if (type == T_DATA_INT32) {
		int size = 12 * sizeof(char);
		char *res = TAlloc(size);
		snprintf(res, size, "%lf", *(double *)data);
		return res;
	}
	else if (type == T_DATA_UINT32) {
		int size = 12 * sizeof(char);
		char *res = TAlloc(size);
		snprintf(res, size, "%lf", *(double *)data);
		return res;
	}
	else if (type == T_DATA_INT64) {
		int size = 21 * sizeof(char);
		char *res = TAlloc(size);
		snprintf(res, size, "%lf", *(double *)data);
		return res;
	}
	else if (type == T_DATA_UINT64) {
		int size = 21 * sizeof(char);
		char *res = TAlloc(size);
		snprintf(res, size, "%lf", *(double *)data);
		return res;
	}

	TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
	return 0;
}

char convertToChar(TCPtr data, TUInt16 type) {
	if (type == T_DATA_CHAR || type == T_DATA_STRING || type == T_DATA_INT8) {
		return *(char *)data;
	}
	else if (type == T_DATA_CONST_STRING) {
		return *(const char *)data;
	}
	else if (type == T_DATA_NULL) {
		return 0;
	}
	else if (type == T_DATA_UINT8) {
		return (char)*(TUInt8 *)data;
	}
	else if (type == T_DATA_INT16) {
		return (char)*(TInt16 *)data;
	}
	else if (type == T_DATA_UINT16) {
		return (char)*(TUInt16 *)data;
	}
	else if (type == T_DATA_INT32) {
		return (char)*(TInt32 *)data;
	}
	else if (type == T_DATA_UINT32) {
		return (char)*(TUInt32 *)data;
	}
	else if (type == T_DATA_INT64) {
		return (char)*(TInt64 *)data;
	}
	else if (type == T_DATA_UINT64) {
		return (char)*(TUInt64 *)data;
	}
	else if (type == T_DATA_FLOAT) {
		return (char)*(float *)data;
	}
	else if (type == T_DATA_DOUBLE) {
		return (char)*(double *)data;
	}

	TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
	return 0;
};

TInt8 convertToInt8(TCPtr data, TUInt16 type) {
	if (type == T_DATA_INT8 || type == T_DATA_CHAR) {
		return *(TInt8 *)data;
	}
	else if (type == T_DATA_UINT8) {
		return (TInt8)*(TUInt8 *)data;
	}
	else if (type == T_DATA_CONST_STRING || type == T_DATA_STRING) {
		return stoi8((const char *)data);
	}
	else if (type == T_DATA_NULL) {
		return 0;
	}
	else if (type == T_DATA_INT16) {
		return (TInt8)*(TInt16 *)data;
	}
	else if (type == T_DATA_UINT16) {
		return (TInt8)*(TUInt16 *)data;
	}
	else if (type == T_DATA_INT32) {
		return (TInt8)*(TInt32 *)data;
	}
	else if (type == T_DATA_UINT32) {
		return (TInt8)*(TUInt32 *)data;
	}
	else if (type == T_DATA_INT64) {
		return (TInt8)*(TInt64 *)data;
	}
	else if (type == T_DATA_UINT64) {
		return (TInt8)*(TUInt64 *)data;
	}
	else if (type == T_DATA_FLOAT) {
		return (TInt8)*(float *)data;
	}
	else if (type == T_DATA_DOUBLE) {
		return (TInt8)*(double *)data;
	}

	TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
	return 0;
}

TUInt8 convertToUInt8(TCPtr data, TUInt16 type) {
	if (type == T_DATA_UINT8) {
		return *(TUInt8 *)data;
	}
	else if (type == T_DATA_INT8) {
		return (TUInt8)(*(TInt8 *)data);
	}
	else if (type == T_DATA_CONST_STRING || type == T_DATA_STRING) {
		return stoui8((const char *)data);
	}
	else if (type == T_DATA_NULL) {
		return 0;
	}
	else if (type == T_DATA_CHAR) {
		return (TUInt8)(*(TInt8 *)data);
	}
	else if (type == T_DATA_INT16) {
		return (TUInt8)*(TInt16 *)data;
	}
	else if (type == T_DATA_UINT16) {
		return (TUInt8)*(TUInt16 *)data;
	}
	else if (type == T_DATA_INT32) {
		return (TUInt8)*(TInt32 *)data;
	}
	else if (type == T_DATA_UINT32) {
		return (TUInt8)*(TUInt32 *)data;
	}
	else if (type == T_DATA_INT64) {
		return (TUInt8)*(TInt64 *)data;
	}
	else if (type == T_DATA_UINT64) {
		return (TUInt8)*(TUInt64 *)data;
	}
	else if (type == T_DATA_FLOAT) {
		return (TUInt8)*(float *)data;
	}
	else if (type == T_DATA_DOUBLE) {
		return (TUInt8)*(double *)data;
	}

	TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
	return 0;
}

TInt16 convertToInt16(TCPtr data, TUInt16 type) {
	if (type == T_DATA_INT16) {
		return *(TInt16 *)data;
	}
	else if (type == T_DATA_UINT16) {
		return (TInt16)*(TUInt16 *)data;
	}
	else if (type == T_DATA_CONST_STRING || type == T_DATA_STRING) {
		return stoi16((const char *)data);
	}
	else if (type == T_DATA_NULL) {
		return 0;
	}
	else if (type == T_DATA_CHAR || type == T_DATA_UINT8) {
		return (TInt16)*(TUInt8 *)data;
	}
	else if (type == T_DATA_INT32) {
		return (TInt16)*(TInt32 *)data;
	}
	else if (type == T_DATA_UINT32) {
		return (TInt16)*(TUInt32 *)data;
	}
	else if (type == T_DATA_INT64) {
		return (TInt16)*(TInt64 *)data;
	}
	else if (type == T_DATA_UINT64) {
		return (TInt16)*(TUInt64 *)data;
	}
	else if (type == T_DATA_FLOAT) {
		return (TInt16)*(float *)data;
	}
	else if (type == T_DATA_DOUBLE) {
		return (TInt16)*(double *)data;
	}

	TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
	return 0;
}

TUInt16 convertToUInt16(TCPtr data, TUInt16 type) {
	if (type == T_DATA_UINT16) {
		return *(TUInt16 *)data;
	}
	else if (type == T_DATA_INT16) {
		return *(TUInt16 *)data;
	}
	else if (type == T_DATA_CONST_STRING || type == T_DATA_STRING) {
		return stoui16((const char *)data);
	}
	else if (type == T_DATA_NULL) {
		return 0;
	}
	else if (type == T_DATA_CHAR || type == T_DATA_INT8) {
		return (TUInt16)*(TInt8 *)data;
	}
	else if (type == T_DATA_UINT8) {
		return (TUInt16)*(TUInt8 *)data;
	}
	else if (type == T_DATA_INT32) {
		return (TUInt16)*(TInt32 *)data;
	}
	else if (type == T_DATA_UINT32) {
		return (TUInt16)*(TUInt32 *)data;
	}
	else if (type == T_DATA_INT64) {
		return (TUInt16)*(TInt64 *)data;
	}
	else if (type == T_DATA_UINT64) {
		return (TUInt16)*(TUInt64 *)data;
	}
	else if (type == T_DATA_FLOAT) {
		return (TUInt16)*(float *)data;
	}
	else if (type == T_DATA_DOUBLE) {
		return (TUInt16)*(double *)data;
	}

	TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
	return 0;
}

TInt32 convertToInt32(TCPtr data, TUInt16 type) {
	if (type == T_DATA_INT32) {
		return *(TInt32 *)data;
	}
	else if (type == T_DATA_UINT32) {
		return (TInt32)*(TUInt32 *)data;
	}
	else if (type == T_DATA_CONST_STRING || type == T_DATA_STRING) {
		return atoi((const char *)data);
	}
	else if (type == T_DATA_NULL) {
		return 0;
	}
	else if (type == T_DATA_CHAR || type == T_DATA_INT8) {
		return (TInt32)*(TInt8 *)data;
	}
	else if (type == T_DATA_UINT8) {
		return (TInt32)*(TUInt8 *)data;
	}
	else if (type == T_DATA_INT16) {
		return (TInt32)*(TInt16 *)data;
	}
	else if (type == T_DATA_UINT16) {
		return (TInt32)*(TUInt16 *)data;
	}
	else if (type == T_DATA_INT64) {
		return (TInt32)*(TInt64 *)data;
	}
	else if (type == T_DATA_UINT64) {
		return (TInt32)*(TUInt64 *)data;
	}
	else if (type == T_DATA_FLOAT) {
		return (TInt32)*(float *)data;
	}
	else if (type == T_DATA_DOUBLE) {
		return (TInt32)*(double *)data;
	}

	TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
	return 0;
}

TUInt32 convertToUInt32(TCPtr data, TUInt16 type) {
	if (type == T_DATA_UINT32) {
		return *(TUInt32 *)data;
	}
	else if (type == T_DATA_INT32) {
		return (TUInt32)*(TInt32 *)data;
	}
	else if (type == T_DATA_CONST_STRING || type == T_DATA_STRING) {
		return stoui32((const char *)data);
	}
	else if (type == T_DATA_NULL) {
		return 0;
	}
	else if (type == T_DATA_CHAR || type == T_DATA_INT8) {
		return (TUInt32)*(TInt8 *)data;
	}
	else if (type == T_DATA_UINT8) {
		return (TUInt32)*(TUInt8 *)data;
	}
	else if (type == T_DATA_INT16) {
		return (TUInt32)*(TInt16 *)data;
	}
	else if (type == T_DATA_UINT16) {
		return (TUInt32)*(TUInt16 *)data;
	}
	else if (type == T_DATA_INT64) {
		return (TUInt32)*(TInt64 *)data;
	}
	else if (type == T_DATA_UINT64) {
		return (TUInt32)*(TUInt64 *)data;
	}
	else if (type == T_DATA_FLOAT) {
		return (TUInt32)*(float *)data;
	}
	else if (type == T_DATA_DOUBLE) {
		return (TUInt32)*(double *)data;
	}

	TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
	return 0;
}

TInt64 convertToInt64(TCPtr data, TUInt16 type) {
	if (type == T_DATA_INT64) {
		return *(TInt64 *)data;
	}
	else if (type == T_DATA_UINT64) {
		return (TInt64)*(TUInt64 *)data;
	}
	else if (type == T_DATA_CONST_STRING || type == T_DATA_STRING) {
		return stoi64((const char *)data);
	}
	else if (type == T_DATA_NULL) {
		return 0;
	}
	else if (type == T_DATA_CHAR || type == T_DATA_INT8) {
		return (TInt64)*(TInt8 *)data;
	}
	else if (type == T_DATA_UINT8) {
		return (TInt64)*(TUInt8 *)data;
	}
	else if (type == T_DATA_INT16) {
		return (TInt64)*(TInt16 *)data;
	}
	else if (type == T_DATA_UINT16) {
		return (TInt64)*(TUInt16 *)data;
	}
	else if (type == T_DATA_INT32) {
		return (TInt64)*(TInt32 *)data;
	}
	else if (type == T_DATA_UINT32) {
		return (TInt64)*(TUInt32 *)data;
	}
	else if (type == T_DATA_FLOAT) {
		return (TInt64)*(float *)data;
	}
	else if (type == T_DATA_DOUBLE) {
		return (TInt64)*(double *)data;
	}

	TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
	return 0;
}

TUInt64 convertToUInt64(TCPtr data, TUInt16 type) {
	if (type == T_DATA_UINT64) {
		return *(TUInt64 *)data;
	}
	else if (type == T_DATA_INT64) {
		return (TUInt64)*(TInt64 *)data;
	}
	else if (type == T_DATA_CONST_STRING || type == T_DATA_STRING) {
		return stoui64((const char *)data);
	}
	else if (type == T_DATA_NULL) {
		return 0;
	}
	else if (type == T_DATA_CHAR || type == T_DATA_INT8) {
		return (TUInt64)*(TInt8 *)data;
	}
	else if (type == T_DATA_UINT8) {
		return (TUInt64)*(TUInt8 *)data;
	}
	else if (type == T_DATA_INT16) {
		return (TUInt64)*(TInt16 *)data;
	}
	else if (type == T_DATA_UINT16) {
		return (TUInt64)*(TUInt16 *)data;
	}
	else if (type == T_DATA_INT32) {
		return (TUInt64)*(TInt32 *)data;
	}
	else if (type == T_DATA_UINT32) {
		return (TUInt64)*(TUInt32 *)data;
	}
	else if (type == T_DATA_FLOAT) {
		return (TUInt64)*(float *)data;
	}
	else if (type == T_DATA_DOUBLE) {
		return (TUInt64)*(double *)data;
	}

	TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
	return 0;
}

float convertToFloat(TCPtr data, TUInt16 type) {
	if (type == T_DATA_FLOAT) {
		return *(float *)data;
	}
	else if (type == T_DATA_DOUBLE) {
		return (float)*(double *)data;
	}
	else if (type == T_DATA_CONST_STRING || type == T_DATA_STRING) {
		return (float)atof((const char *)data);
	}
	else if (type == T_DATA_NULL) {
		return 0;
	}
	else if (type == T_DATA_CHAR || type == T_DATA_INT8) {
		return (float)*(TInt8 *)data;
	}
	else if (type == T_DATA_UINT8) {
		return (float)*(TUInt8 *)data;
	}
	else if (type == T_DATA_INT16) {
		return (float)*(TInt16 *)data;
	}
	else if (type == T_DATA_UINT16) {
		return (float)*(TUInt16 *)data;
	}
	else if (type == T_DATA_INT32) {
		return (float)*(TInt32 *)data;
	}
	else if (type == T_DATA_UINT32) {
		return (float)*(TUInt32 *)data;
	}
	else if (type == T_DATA_INT64) {
		return (float)*(TInt64 *)data;
	}
	else if (type == T_DATA_UINT64) {
		return (float)*(TUInt64 *)data;
	}

	TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
	return 0;
}

double convertToDouble(TCPtr data, TUInt16 type) {
	if (type == T_DATA_DOUBLE) {
		return *(double *)data;
	}
	else if (type == T_DATA_FLOAT) {
		return (double)*(float *)data;
	}
	else if (type == T_DATA_CONST_STRING || type == T_DATA_STRING) {
		return atof((const char *)data);
	}
	else if (type == T_DATA_NULL) {
		return 0;
	}
	else if (type == T_DATA_CHAR) {
		return (double)*(TInt8 *)data;
	}
	else if (type == T_DATA_INT8) {
		return (double)*(TInt8 *)data;
	}
	else if (type == T_DATA_UINT8) {
		return (double)*(TUInt8 *)data;
	}
	else if (type == T_DATA_INT16) {
		return (double)*(TInt16 *)data;
	}
	else if (type == T_DATA_UINT16) {
		return (double)*(TUInt16 *)data;
	}
	else if (type == T_DATA_INT32) {
		return (double)*(TInt32 *)data;
	}
	else if (type == T_DATA_UINT32) {
		return (double)*(TUInt32 *)data;
	}
	else if (type == T_DATA_INT64) {
		return (double)*(TInt64 *)data;
	}
	else if (type == T_DATA_UINT64) {
		return (double)*(TUInt64 *)data;
	}

	TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
	return 0;
}

TPtr convertTo(TCPtr data, TUInt16 type, TUInt16 targetType)
{
	if (type == targetType) return (TPtr) data;

	TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
	return 0;
}
