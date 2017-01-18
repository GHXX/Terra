
#ifndef __included_terra_data_h
#define __included_terra_data_h

typedef struct _TData TData;

enum T_DATA_TYPE {
	T_DATA_UNKNOWN = 0,
	T_DATA_CONST_UNKNOWN,

	T_DATA_NULL,

	T_DATA_INT8,
	T_DATA_UINT8,
	T_DATA_INT16,
	T_DATA_UINT16,
	T_DATA_INT32,
	T_DATA_UINT32,
	T_DATA_INT64,
	T_DATA_UINT64,

	T_DATA_STRING,
	T_DATA_CHAR,

	T_DATA_FLOAT,
	T_DATA_DOUBLE,

	T_DATA_AMOUNT,
};

TData *TDataCopy(TData *data);

TData *TDataFromPtr(TPtr data, TSize size);
TData *TDataFromConstPtr(TCPtr data);

TData *TDataFromInt8(TInt8 data);
TData *TDataFromUInt8(TUInt8 data);
TData *TDataFromInt16(TInt16 data);
TData *TDataFromUInt16(TUInt16 data);
TData *TDataFromInt32(TInt32 data);
TData *TDataFromUInt32(TUInt32 data);
TData *TDataFromInt64(TInt64 data);
TData *TDataFromUInt64(TUInt64 data);
#ifdef PLATFORM_X86_64
#define TDataFromSize(data) TDataFromUInt64(data)
#define TDataFromLInt(data) TDataFromInt64(data)
#define T_DATA_SIZE T_DATA_UINT64
#else
#define TDataFromSize(data) TDataFromUInt32(data)
#define TDataFromLInt(data) TDataFromInt32(data)
#define T_DATA_SIZE T_DATA_UINT32
#endif

TData *TDataFromString(const char *data);
TData *TDataFromChar(char data);

TData *TDataFromFloat(float data);
TData *TDataFromDouble(double data);

TUInt8 TDataGetType(const TData *context);

TCPtr TDataToConstPointer(const TData *context, TUInt16 *type);
TPtr TDataToPointer(const TData *context, TUInt16 *type);

char *TDataToString(const TData *context);
char TDataToChar(const TData *context);

TInt8 TDataToInt8(const TData *context);
TUInt8 TDataToUInt8(const TData *context);
TInt16 TDataToInt16(const TData *context);
TUInt16 TDataToUInt16(const TData *context);
TInt32 TDataToInt32(const TData *context);
TUInt32 TDataToUInt32(const TData *context);
TInt64 TDataToInt64(const TData *context);
TUInt64 TDataToUInt64(const TData *context);
#ifdef PLATFORM_X86_64
#define TDataToTSize(context) TDataToUInt64(context)
#define TDataToTLInt(context) TDataToInt64(context)
#else
#define TDataToTSize(context) TDataToUInt32(context)
#define TDataToTLInt(context) TDataToInt32(context)
#endif

float TDataToFloat(const TData *context);
double TDataToDouble(const TData *context);

void TDataFree(TData *context);

#endif
