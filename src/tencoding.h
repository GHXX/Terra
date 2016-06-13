
#ifndef __included_terra_encoding_h
#define __included_terra_encoding_h

#include "terror.h"

#include "io/tstream.h"

enum T_ENCODING_TYPES {
	T_ENCODING_UNKNOWN,
	T_ENCODING_ASCII,
	T_ENCODING_UTF8,
	T_ENCODING_UTF16_BE,
	T_ENCODING_UTF16_LE,
	T_ENCODING_UTF32_BE,
	T_ENCODING_UTF32_LE,

	T_NUM_ENCODING,
};

enum T_ENCODING_ERROR {
	T_ENCODING_ERROR_TRUNCATED = T_ERROR_AMOUNT,
	T_ENCODING_ERROR_INVALID,
};

enum T_ENCODING_FLAGS {
	T_ENCODING_VALID = 1,
	T_ENCODING_NULL_TERMINATED = 2,
	T_ENCODING_BOM_PRESENT = 4,
};


typedef struct TEncodingStats {
	TUInt8 encoding;

	TSize numChars;
	TSize numOneByteChars;
	TSize numTwoByteChars;
	TSize numThreeByteChars;
	TSize numFourByteChars;

	TUInt8 flags;
} TEncodingStats;

/**
* Returns the string Encoding statistics
*
* @param data                The data to be analyzed.
* @param size                The data size.
*
* @return                    Statistics on the data.
*
*/
TEncodingStats *TEncodingGetStats(const unsigned char *data, TSize size);

/**
* Returns the string Encoding statistics
*
* @param data                The data to be analyzed.
* @param size                The data size.
*
* @return                    Statistics on the data.
*
*/
TEncodingStats *TEncodingGetStreamStats(TStream *stream);

/**
* Get the BOM of an encoding
*
* @param encoding            The encoding desired.
*
* @return                    the BOM for the provided encoding.
*
*/
const unsigned char *TEncodingGetBOM(TUInt8 encoding);

/**
* Get the BOM size of an encoding
*
* @param encoding            The encoding desired.
*
* @return                    the size of the BOM for the provided encoding.
*
*/
TUInt8 TEncodingGetBOMSize(TUInt8 encoding);

/**
* Encodes the data to ASCII
*
* @param data                The data to be encoded.
* @param size                The data size.
* @param sizeOut             The output size.
* @param hint_encoding       The input encoding. can be T_ENCODING_UNKNOWN
*
* @return                    The encoded data.
*
*/
unsigned char *TEncodingToASCII(const unsigned char *data, TSize size, TSize *sizeOut, TEncodingStats *stats);

/**
* Encodes the data to utf-8
*
* @param data                The data to be encoded.
* @param size                The data size.
* @param sizeOut             The output size.
* @param hint_encoding       The input encoding. can be T_ENCODING_UNKNOWN
*
* @return                    The encoded data.
*
*/
unsigned char *TEncodingToUTF8(const unsigned char *data, TSize size, TSize *sizeOut, TEncodingStats *stats);

/**
* Encodes the data to utf-16 little endian
*
* @param data                The data to be encoded.
* @param size                The data size.
* @param sizeOut             The output size.
* @param hint_encoding       The input encoding. can be T_ENCODING_UNKNOWN
*
* @return                    The encoded data.
*
*/
unsigned char *TEncodingToUTF16LE(const unsigned char *data, TSize size, TSize *sizeOut, TEncodingStats *stats);

/**
* Gets the current character of an utf-8 string and updates the position
*
* @param data                The data to be processed.
* @param size                The data size.
*
* @return                    The character.
*
*/
TUInt32 TEncodingUTF8GetChr(const unsigned char **data, TSize *size);

/**
* Gets the previous character of an utf-8 string and updates the position
*
* @param data                The data to be processed.
* @param size                The data size.
*
* @return                    The character.
*
*/
TUInt32 TEncodingUTF8GetPreviousChr(const unsigned char **data, TSize *size);


/**
* shift the ASCII data pointer by the number of characters
*
* @param data                The data to be processed.
* @param size                The data size.
* @param numCharacters       The amount of character to skip.
*
* @return                    an error code. 0 for no error, 1 for failing
*/
int TEncodingASCIIIncrement(const unsigned char **data, TSize *size, TLInt numCharacters);

/**
* shift the utf8 data pointer by the number of characters
*
* @param data                The data to be processed.
* @param size                The data size.
* @param numCharacters       The amount of character to skip.
*
* @return                    an error code. 0 for no error, 1 for failing
*/
int TEncodingUTF8Increment(const unsigned char **data, TSize *size, TLInt numCharacters);

#endif
