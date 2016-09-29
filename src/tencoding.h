
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
	T_ENCODING_FLAG_VALID = 1,
	T_ENCODING_FLAG_NULL_TERMINATED = 2,
	T_ENCODING_FLAG_BOM_PRESENT = 4,
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
* Returns the data Encoding statistics
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
* Check if character is ascii
*
* @param data                The data to be verified.
*
* @return                    1 if ascii, 0 otherwise.
*
*/
TUInt8 TEncodingIsASCIIChr(const unsigned char *data);

/**
* Encodes the data to ASCII
*
* @param data                The data to be encoded.
* @param size                The data size.
* @param sizeOut             The output size.
* @param stats               The input statistics.
*
* @return                    The ASCII encoded data.
*
*/
unsigned char *TEncodingToASCII(const unsigned char *data, TSize size, TSize *sizeOut, TEncodingStats *stats);

/*
*Encodes null terminated data to ASCII
*
* @param data                The data to be encoded.
* @param sizeOut             The output size.
* @param stats               The input statistics.
*
* @return                    The ASCII encoded data.
*
*/
unsigned char *TEncodingToASCIINT(const unsigned char *data, TSize *sizeOut, TEncodingStats *stats);

/**
* Encodes the data to utf-8
*
* @param data                The data to be encoded.
* @param size                The data size.
* @param sizeOut             The output size.
* @param stats               The input statistics.
*
* @return                    The utf-8 encoded data.
*
*/
unsigned char *TEncodingToUTF8(const unsigned char *data, TSize size, TSize *sizeOut, TEncodingStats *stats);

/**
* Encodes null terminated data to utf-8
*
* @param data                The data to be encoded.
* @param sizeOut             The output size.
* @param stats               The input statistics.
*
* @return                    The utf-8 encoded data.
*
*/
unsigned char *TEncodingToUTF8NT(const unsigned char *data, TSize *sizeOut, TEncodingStats *stats);

/**
* Encodes character to utf-8
*
* @param data                The character to be encoded.
* @param sizeOut             The output size.
* @param stats               The input statistics.
*
* @return                    The utf-8 encoded character.
*
*/
unsigned char *TEncodingToUTF8Chr(const unsigned char *data, TSize *sizeOut, TEncodingStats *stats);

/**
* Encodes the data to utf-16 little endian
*
* @param data                The data to be encoded.
* @param size                The data size.
* @param sizeOut             The output size.
* @param stats               The input statistics.
*
* @return                    The utf-16 little endian encoded data.
*
*/
unsigned char *TEncodingToUTF16LE(const unsigned char *data, TSize size, TSize *sizeOut, TEncodingStats *stats);

/**
* Encodes null terminated data to utf-16 little endian
*
* @param data                The data to be encoded.
* @param sizeOut             The output size.
* @param stats               The input statistics.
*
* @return                    The utf-16 little endian encoded data.
*
*/
unsigned char *TEncodingToUTF16LENT(const unsigned char *data, TSize *sizeOut, TEncodingStats *stats);

/**
* Gets the number of characters of an utf-8 string
*
* @param data                The data to be processed.
* @param size                The data size.
*
* @return                    The number of characters.
*
*/
TSize TEncodingUTF8Strlen(const unsigned char *data, TSize size);

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
* Return a lower case string for utf-8
*
* @param data                The data to be processed.
* @param size                The data size.
* @param sizeOut             The output size.
*
* @return                    The lower cased string.
*
*/
unsigned char *TEncodingUTF8LowerCase(const unsigned char *data, TSize size, TSize *sizeOut);

/**
* Return a lower case string for utf-8
*
* @param data                The data to be processed.
* @param size                The data size.
* @param sizeOut             The output size.
*
* @return                    The lower cased string.
*
*/
unsigned char *TEncodingUTF8UpperCase(const unsigned char *data, TSize size, TSize *sizeOut);

/**
* shift the data pointer by the number of characters
*
* @param data                The data to be processed.
* @param size                The data size.
* @param encoding            The data encoding.
* @param numCharacters       The amount of character to skip.
*
* @return                    an error code. 0 for no error, 1 for failing
*/
int TEncodingIncrement(const unsigned char **data, TSize *size, TUInt8 encoding, TLInt numCharacters);

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
