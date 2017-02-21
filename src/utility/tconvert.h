
#ifndef __included_terra_convert_h
#define __included_terra_convert_h

/**
* Terra Data Type conversion
*
*   The purpose of this file is to convert from one type of
*   data to another through various functions.
*
*/

/**
* Converts data to a pointer.
*
*  Only Supports unknown data types.
*
* @param data                The input data.
* @param type                The data type.
*
* @return                    A pointer to the data
*
*/
TPtr TConvertToPointer(TCPtr data, TUInt8 type);

/**
* Converts data to a constant pointer.
*
*  Only Supports unknown data types.
*
* @param data                The input data.
* @param type                The data type.
*
* @return                    A constant pointer to the data
*
*/
TCPtr TConvertToConstPointer(TCPtr data, TUInt8 type);

/**
* Converts data to a char pointer.
*
* @param data                The input data.
* @param type                The data type.
*
* @return                    A pointer to the string (needs manual deallocation)
*
*/
char *TConvertToString(TCPtr data, TUInt8 type);

/**
* Converts data to a char representation.
*
* @param data                The input data.
* @param type                The data type.
*
* @return                    a character (in case of string, returns the first character)
*
*/
char TConvertToChar(TCPtr data, TUInt8 type);

/**
* Converts data to an 8-bit integer representation.
*
* @param data                The input data.
* @param type                The data type.
*
* @return                    an 8-bit integer.
*
*/
TInt8 TConvertToInt8(TCPtr data, TUInt8 type);

/**
* Converts data to an unsigned 8-bit integer representation.
*
* @param data                The input data.
* @param type                The data type.
*
* @return                    an unsigned 8-bit integer.
*
*/
TUInt8 TConvertToUInt8(TCPtr data, TUInt8 type);

/**
* Converts data to a 16-bit integer representation.
*
* @param data                The input data.
* @param type                The data type.
*
* @return                    a 16-bit integer.
*
*/
TInt16 TConvertToInt16(TCPtr data, TUInt8 type);

/**
* Converts data to an unsigned 16-bit integer representation.
*
* @param data                The input data.
* @param type                The data type.
*
* @return                    an unsigned 16-bit integer.
*
*/
TUInt16 TConvertToUInt16(TCPtr data, TUInt8 type);

/**
* Converts data to a 32-bit integer representation.
*
* @param data                The input data.
* @param type                The data type.
*
* @return                    a 32-bit integer.
*
*/
TInt32 TConvertToInt32(TCPtr data, TUInt8 type);

/**
* Converts data to an unsigned 32-bit integer representation.
*
* @param data                The input data.
* @param type                The data type.
*
* @return                    an unsigned 32-bit integer.
*
*/
TUInt32 TConvertToUInt32(TCPtr data, TUInt8 type);

/**
* Converts data to a 64-bit integer representation.
*
* @param data                The input data.
* @param type                The data type.
*
* @return                    a 64-bit integer.
*
*/
TInt64 TConvertToInt64(TCPtr data, TUInt8 type);

/**
* Converts data to an unsigned 64-bit integer representation.
*
* @param data                The input data.
* @param type                The data type.
*
* @return                    an unsigned 64-bit integer.
*
*/
TUInt64 TConvertToUInt64(TCPtr data, TUInt8 type);

#ifdef PLATFORM_X86_64
#define TConvertToTSize TConvertToUInt64
#else
#define TConvertToTSize TConvertToUInt32
#endif

/**
* Converts data to a float representation.
*
* @param data                The input data.
* @param type                The data type.
*
* @return                    a float.
*
*/
float TConvertToFloat(TCPtr data, TUInt8 type);

/**
* Converts data to a double representation.
*
* @param data                The input data.
* @param type                The data type.
*
* @return                    a double.
*
*/
double TConvertToDouble(TCPtr data, TUInt8 type);

/**
* Converts data to another type.
*
* @param data                The input data.
* @param type                The data type.
* @param targetType          The new data type.
* @param length              The number of objects to convert.
*
* @return                    a pointer to the new data.
*
*/
TPtr TConvertTo(TCPtr data, TUInt8 type, TUInt8 targetType, TSize length);

/**
* Converts data to another type and inserts it in target.
*
* @param data                The input data.
* @param type                The data type.
* @param target              The target buffer.
* @param targetType          The new data type.
* @param length              The number of objects to convert.
*
* @return                    a pointer to the new data.
*
*/
void TConvertToBuffer(TCPtr data, TUInt8 type, TPtr target, TUInt8 targetType, TSize length);

#endif
