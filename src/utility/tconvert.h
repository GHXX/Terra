
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
TPtr convertToPointer(TCPtr data, TUInt16 type);

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
TCPtr convertToConstPointer(TCPtr data, TUInt16 type);

/**
* Converts data to a constant char pointer.
*
*  Only Supports strings.
*
* @param data                The input data.
* @param type                The data type.
*
* @return                    A const pointer to the string
*
*/
const char *convertToConstString(TCPtr data, TUInt16 type);

/**
* Converts data to a char pointer.
*
* @param data                The input data.
* @param type                The data type.
*
* @return                    A pointer to the string (needs manual deallocation)
*
*/
char *convertToString(TCPtr data, TUInt16 type);

/**
* Converts data to a char representation.
*
* @param data                The input data.
* @param type                The data type.
*
* @return                    a character (in case of string, returns the first character)
*
*/
char convertToChar(TCPtr data, TUInt16 type);

/**
* Converts data to an 8-bit integer representation.
*
* @param data                The input data.
* @param type                The data type.
*
* @return                    an 8-bit integer.
*
*/
TInt8 convertToInt8(TCPtr data, TUInt16 type);

/**
* Converts data to an unsigned 8-bit integer representation.
*
* @param data                The input data.
* @param type                The data type.
*
* @return                    an unsigned 8-bit integer.
*
*/
TUInt8 convertToUInt8(TCPtr data, TUInt16 type);

/**
* Converts data to a 16-bit integer representation.
*
* @param data                The input data.
* @param type                The data type.
*
* @return                    a 16-bit integer.
*
*/
TInt16 convertToInt16(TCPtr data, TUInt16 type);

/**
* Converts data to an unsigned 16-bit integer representation.
*
* @param data                The input data.
* @param type                The data type.
*
* @return                    an unsigned 16-bit integer.
*
*/
TUInt16 convertToUInt16(TCPtr data, TUInt16 type);

/**
* Converts data to a 32-bit integer representation.
*
* @param data                The input data.
* @param type                The data type.
*
* @return                    a 32-bit integer.
*
*/
TInt32 convertToInt32(TCPtr data, TUInt16 type);

/**
* Converts data to an unsigned 32-bit integer representation.
*
* @param data                The input data.
* @param type                The data type.
*
* @return                    an unsigned 32-bit integer.
*
*/
TUInt32 convertToUInt32(TCPtr data, TUInt16 type);

#ifdef PLATFORM_X86_64
/**
* Converts data to a 64-bit integer representation.
*
* @param data                The input data.
* @param type                The data type.
*
* @return                    a 64-bit integer.
*
*/
TInt64 convertToInt64(TCPtr data, TUInt16 type);

/**
* Converts data to an unsigned 64-bit integer representation.
*
* @param data                The input data.
* @param type                The data type.
*
* @return                    an unsigned 64-bit integer.
*
*/
TUInt64 convertToUInt64(TCPtr data, TUInt16 type);
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
float convertToFloat(TCPtr data, TUInt16 type);

/**
* Converts data to a double representation.
*
* @param data                The input data.
* @param type                The data type.
*
* @return                    a double.
*
*/
double convertToDouble(TCPtr data, TUInt16 type);

/**
* Converts data to another type.
*
* @param data                The input data.
* @param type                The data type.
* @param targetType          The new data type.
*
* @return                    a pointer to the new data.
*
*/
TPtr convertTo(TCPtr data, TUInt16 type, TUInt16 targetType);

#endif
