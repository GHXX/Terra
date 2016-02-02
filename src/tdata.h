
#ifndef __included_terra_data_h
#define __included_terra_data_h

typedef struct _TData TData;

TData *TDataFromPtr(TPtr data, TSize size);
TData *TDataFromConstPtr(TCPtr data);

TData *TDataFromInt8(TInt8 data);
TData *TDataFromUInt8(TUInt8 data);
TData *TDataFromInt16(TInt16 data);
TData *TDataFromUInt16(TUInt16 data);
TData *TDataFromInt32(TInt32 data);
TData *TDataFromUInt32(TUInt32 data);
#ifdef PLATFORM_X86_64
TData *TDataFromInt64(TInt64 data);
TData *TDataFromUInt64(TUInt64 data);
#define TDataFromSize(data) TDataFromUInt64(data)
#else
#define TDataFromSize(data) TDataFromUInt32(data)
#endif

TData *TDataFromConstString(const char *data);
TData *TDataFromString(char *data);
TData *TDataFromChar(char data);

TData *TDataFromFloat(float data);
TData *TDataFromDouble(double data);

TUInt16 TDataGetType(const TData *context);

TCPtr TDataToConstPointer(const TData *context, TUInt16 *type);
TPtr TDataToPointer(const TData *context, TUInt16 *type);

const char *TDataToConstString(const TData *context);
char *TDataToString(const TData *context);
char TDataToChar(const TData *context);

TInt8 TDataToInt8(const TData *context);
TUInt8 TDataToUInt8(const TData *context);
TInt16 TDataToInt16(const TData *context);
TUInt16 TDataToUInt16(const TData *context);
TInt32 TDataToInt32(const TData *context);
TUInt32 TDataToUInt32(const TData *context);
#ifdef PLATFORM_X86_64
TInt64 TDataToInt64(const TData *context);
TUInt64 TDataToUInt64(const TData *context);
#define TDataToSize(context) TDataToUInt64(context)
#else
#define TDataToSize(context) TDataToUInt32(context)
#endif

float TDataToFloat(const TData *context);
double TDataToDouble(const TData *context);

void TDataFree(TData *context);

#endif
