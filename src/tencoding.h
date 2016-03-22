
#ifndef __included_terra_encoding_h
#define __included_terra_encoding_h

#include "io/tstream.h"

enum TEncodingTypes {
	T_ENCODING_UNKNOWN,
	T_ENCODING_ASCII,
	T_ENCODING_UTF8,
	T_ENCODING_UTF16_BE,
	T_ENCODING_UTF16_LE,
	T_ENCODING_UTF32_BE,
	T_ENCODING_UTF32_LE,
};

enum T_ENCODING_ERROR {
	T_ENCODING_NONE,
	T_ENCODING_TRUNCATED,
	T_ENCODING_INVALID,
};


/**
* Returns the string Encoding
*
* @param data                The data to be analyzed.
* @param size                The data size.
*
* @return                    The data encoding.
*
*/
TUInt8 TEncodingGetDataEncoding(const unsigned char *data, TSize size);

/**
* Returns the stream Encoding
*
* @param stream              The stream to be analyzed.
*
* @return                    The data encoding.
*
*/
TUInt8 TEncodingGetStreamEncoding(TStream *stream);

/**
* Encodes the data to utf-8
*
* @param data                The data to be encoded.
* @param size                The data size.
* @param hint_encoding       The input encoding. can be T_ENCODING_UNKNOWN
*
* @return                    The encoded data.
*
*/
unsigned char *TEncodingToUTF8(const unsigned char *data, TSize size, TUInt8 hint_encoding);

/**
* Encodes the data to utf-16 little endian
*
* @param data                The data to be encoded.
* @param size                The data size.
* @param hint_encoding       The input encoding. can be T_ENCODING_UNKNOWN
*
* @return                    The encoded data.
*
*/
unsigned char *TEncodingToUTF16LE(const unsigned char *data, TSize size, TUInt8 hint_encoding);

/**
* Gets the current character of an utf-8 string and updates the position
*
* @param data                The data to be processed.
* @param size                The data size.
*
* @return                    The character.
*
*/
TUInt32 TEncodingUTF8GetChr(const unsigned char *data, TSize *size);

/**
* Gets the previous character of an utf-8 string and updates the position
*
* @param data                The data to be processed.
* @param size                The data size.
*
* @return                    The character.
*
*/
TUInt32 TEncodingUTF8GetPreviousChr(const unsigned char *data, TSize *size);

/**
* Goes to the next character of an utf-8 sequence
*
* @param data                The data to be processed.
* @param size                The data size.
*
*/
void TEncodingUTF8Increment(const unsigned char *data, TSize *size);

/**
* Goes to the previous character of an utf-8 sequence
*
* @param data                The data to be processed.
* @param size                The data size.
*
*/
void TEncodingUTF8Decrement(const unsigned char *data, TSize *size);

#endif
