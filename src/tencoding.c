
#include "stdafx.h"

#include "tencoding.h"

#include "talloc.h"

#include <ctype.h>


typedef int(*TEncodeIncrement)(const unsigned char **, TSize *, TLInt);

static TEncodeIncrement TEncodeIncrements[] = {
	0,
	TEncodingASCIIIncrement,
	TEncodingUTF8Increment,
	0,
	0,
	0,
	0,
};

static const char *TENCODING_UTF_16_BE_BOM = "\xFE\xFF";
static const char *TENCODING_UTF_16_LE_BOM = "\xFF\xFE";
static const char *TENCODING_UTF_8_BOM     = "\xEF\xBB\xBF";
static const char *TENCODING_UTF_32_BE_BOM = "\x00\x00\xFE\xFF";
static const char *TENCODING_UTF_32_LE_BOM = "\xFF\xFE\x00\x00";

#define BOM_SIZE_UTF8 3
#define BOM_SIZE_UTF16 2
#define BOM_SIZE_UTF32 4

static inline int TEncodingGetUTF8CodeLength(unsigned char byte) {
	if (byte < 0x80) {
		// 1 byte sequence: U+0000..U+007F
		return 1;
	} else if ((byte & 0xE0) == 0xC0) {
		// 2 bytes sequence: U+0080..U+07FF
		return 2;
	} else if ((byte & 0xF0) == 0xE0) {
		// 3 bytes sequence: U+0800..U+FFFF
		return 3;
	} else if ((byte & 0xF8) == 0xF0) {
		// 4 bytes sequence: U+10000..U+10FFFF
		return 4;
	}

	// invalid
	return 0;
}

static inline int TEncodingGetUTF8CodeLengthCharacter(TUInt32 ch) {
	if (ch < 0x80) {
		// 1 byte sequence: U+0000..U+007F
		return 1;
	} else if (ch < 0x800) {
		// 2 bytes sequence: U+0080..U+07FF
		return 2;
	} else if (ch < 0x10000) {
		// 3 bytes sequence: U+0800..U+FFFF
		return 3;
	} else if (ch < 0x110000) {
		// 4 bytes sequence: U+10000..U+10FFFF
		return 4;
	}

	// invalid
	return 0;
}

static inline TUInt32 TEncodingGetUTF8Firstbyte(unsigned char byte, int codeLength) {
	if (codeLength == 1)
		return (TUInt32)(byte & 0x7F);
	else if (codeLength == 2)
		return (TUInt32)(byte & 0x1F);
	else if (codeLength == 3)
		return (TUInt32)(byte & 0x0F);
	else if (codeLength == 4)
		return (TUInt32)(byte & 0x07);

	return 0;
}

TUInt32 TEncodingUTF8GetChrInternal(const unsigned char **data, TSize *size) {
	unsigned char byte;
	TUInt8 codeLength, i;
	TUInt32 ch;
	const unsigned char *ptr;

	ptr = *data;
	byte = *ptr;
	codeLength = TEncodingGetUTF8CodeLength(byte);
	if (!codeLength) {
		TErrorSet(T_ENCODING_ERROR_INVALID);
		return 0;
	}

	ch = TEncodingGetUTF8Firstbyte(byte, codeLength);

	if (codeLength * sizeof(unsigned char) > *size) {
		// truncated string or invalid byte sequence
		TErrorSet(T_ENCODING_ERROR_TRUNCATED);
		return 0;
	}

	ptr++;
	for (i = 1; i < codeLength; i++, ptr++) {
		ch = (ch << 6) | (*ptr & 0x3F);
	}

	*data += codeLength;
	*size -= sizeof(unsigned char) * codeLength;

	return ch;
}

TUInt32 TEncodingUTF16LEGetChrInternal(const unsigned char **data, TSize *size) {
	TUInt16 *ptr = (TUInt16 *)*data;
	TUInt32 code;

	//decode utf-16
	if (*ptr < 0x10000) {
		code = *ptr;
		*data += 2;
		*size -= sizeof(unsigned char) * 2;

	} else if (*ptr & 0xD800) {
		//decode utf-16 pair
		code = 0x10000;
		code += (*(ptr++) & 0x03FF) << 10;
		code += (*ptr & 0x03FF);

		*data += 4;
		*size -= sizeof(unsigned char) * 4;
	} else {
		TErrorSet(T_ENCODING_ERROR_INVALID);
		code = 0;
	}

	return code;
}

TUInt8 TEncodingCheckBOM(const unsigned char *data, TSize size) {
	if (size >= 3) {
		if (memcmp(data, TENCODING_UTF_8_BOM, BOM_SIZE_UTF8) == 0)
			return T_ENCODING_UTF8;
	}
	if (size >= 4) {
		if (memcmp(data, TENCODING_UTF_32_LE_BOM, BOM_SIZE_UTF32) == 0)
			return T_ENCODING_UTF32_LE;
		if (memcmp(data, TENCODING_UTF_32_BE_BOM, BOM_SIZE_UTF32) == 0)
			return T_ENCODING_UTF32_BE;
	}
	if (size >= 2) {
		if (memcmp(data, TENCODING_UTF_16_LE_BOM, BOM_SIZE_UTF16) == 0)
			return T_ENCODING_UTF16_LE;
		if (memcmp(data, TENCODING_UTF_16_BE_BOM, BOM_SIZE_UTF16) == 0)
			return T_ENCODING_UTF16_BE;
	}
	return T_ENCODING_UNKNOWN;
}

TEncodingStats *TEncodingGetStats(const unsigned char *data, TSize size) {
	TEncodingStats *stats = TAllocData(TEncodingStats);
	if (stats) {
		memset(stats, 0, sizeof(TEncodingStats));

		stats->encoding = TEncodingCheckBOM(data, size);
		if (stats->encoding) {
			TUInt8 BOMSize = TEncodingGetBOMSize(stats->encoding);
			stats->flags |= T_ENCODING_FLAG_BOM_PRESENT;

			data += BOMSize;
			size -= BOMSize;
		}

		if (!stats->encoding || stats->encoding == T_ENCODING_UTF8) {
			stats->encoding = T_ENCODING_UTF8;
			while (size) {
				if (*data < 0x80) {
					if (!*data && size == sizeof(unsigned char)) {
						stats->flags |= T_ENCODING_FLAG_NULL_TERMINATED | T_ENCODING_FLAG_VALID;
						break;
					}

					data += 1;
					stats->numChars++;
					stats->numOneByteChars++;
					size -= sizeof(unsigned char);
				} else {
					unsigned char byte;
					TUInt8 codeLength, i;
					TUInt32 ch;

					byte = *data;
					codeLength = TEncodingGetUTF8CodeLength(byte);
					if (!codeLength) {
						TErrorSet(T_ENCODING_ERROR_INVALID);
						return stats;
					}

					ch = TEncodingGetUTF8Firstbyte(byte, codeLength);

					if (codeLength * sizeof(unsigned char) > size) {
						// truncated string or invalid byte sequence
						TErrorSet(T_ENCODING_ERROR_TRUNCATED);
						return stats;
					}

					data++;
					for (i = 1; i < codeLength; i++, data++) {
						byte = *data;
						// Check continuation bytes
						if ((byte & 0xC0) != 0x80) {
							TErrorSet(T_ENCODING_ERROR_INVALID);
							return stats;
						}

						ch = (ch << 6) | (byte & 0x3F);
					}

					if ((ch > 0x10FFFF) ||
						((ch >= 0xD800) && (ch <= 0xDFFF)) ||
						((ch <= 0x007F) && (codeLength != 1)) ||
						((ch >= 0x0080) && (ch <= 0x07FF) && (codeLength != 2)) ||
						((ch >= 0x0800) && (ch <= 0xFFFF) && (codeLength != 3)) ||
						((ch >= 0x10000) && (ch <= 0x1FFFFF) && (codeLength != 4))) {
						TErrorSet(T_ENCODING_ERROR_INVALID);
						return stats;
					}

					size -= sizeof(unsigned char) * codeLength;

					stats->numChars++;
					(*((&stats->numChars) + codeLength))++;
				}
			}
			if (stats->numOneByteChars == stats->numChars)
				stats->encoding = T_ENCODING_ASCII;
		}
	}

	stats->flags |= T_ENCODING_FLAG_VALID;
	return stats;
}

TEncodingStats *TEncodingGetStreamStats(TStream *stream) {
	TEncodingStats *stats = TAllocData(TEncodingStats);
	if (stats) {
		unsigned char buffer[4], *ptr;
		TUInt8 size;
		TLInt pos;

		memset(stats, 0, sizeof(TEncodingStats));

		pos = TStreamTell(stream);

		size = TStreamReadBlock(stream, buffer, 4 * sizeof(unsigned char));
		stats->encoding = TEncodingCheckBOM(buffer, size);
		if (stats->encoding) {
			if (stats->encoding == T_ENCODING_UTF8) {
				TStreamSeek(stream, pos + BOM_SIZE_UTF8, SEEK_SET);
			} else if (stats->encoding == T_ENCODING_UTF16_LE || stats->encoding == T_ENCODING_UTF16_BE) {
				TStreamSeek(stream, pos + BOM_SIZE_UTF16, SEEK_SET);
			}

			stats->flags |= T_ENCODING_FLAG_BOM_PRESENT;
		} else {
			TStreamSeek(stream, pos, SEEK_SET);
		}

		if (!stats->encoding || stats->encoding == T_ENCODING_UTF8) {
			stats->encoding = T_ENCODING_UTF8;
			while (!TStreamEOF(stream)) {
				TStreamReadBlock(stream, buffer, sizeof(unsigned char));
				if (*buffer < 0x80) {
					if (!*buffer && TStreamEOF(stream)) {
						stats->flags |= T_ENCODING_FLAG_NULL_TERMINATED | T_ENCODING_FLAG_VALID;
						break;
					}

					stats->numChars++;
					stats->numOneByteChars++;
				} else {
					unsigned char byte;
					TUInt8 codeLength, i;
					TUInt32 ch;

					byte = *buffer;
					codeLength = TEncodingGetUTF8CodeLength(byte);
					if (!codeLength) {
						TErrorSet(T_ENCODING_ERROR_INVALID);
						return stats;
					}

					ch = TEncodingGetUTF8Firstbyte(byte, codeLength);

					size = TStreamReadBlock(stream, buffer + 1, (codeLength - 1) * sizeof(unsigned char)) + 1;
					if (codeLength * sizeof(unsigned char) > size) {
						// truncated string or invalid byte sequence
						TErrorSet(T_ENCODING_ERROR_TRUNCATED);
						TStreamSeek(stream, pos, SEEK_SET);
						return stats;
					}

					ptr = buffer + 1;
					for (i = 1; i < codeLength; i++, ptr++) {
						byte = *ptr;
						// Check continuation bytes
						if ((byte & 0xC0) != 0x80) {
							TErrorSet(T_ENCODING_ERROR_INVALID);
							TStreamSeek(stream, pos, SEEK_SET);
							return stats;
						}

						ch = (ch << 6) | (byte & 0x3F);
					}

					if ((ch > 0x10FFFF) ||
						((ch >= 0xD800) && (ch <= 0xDFFF)) ||
						((ch <= 0x007F) && (codeLength != 1)) ||
						((ch >= 0x0080) && (ch <= 0x07FF) && (codeLength != 2)) ||
						((ch >= 0x0800) && (ch <= 0xFFFF) && (codeLength != 3)) ||
						((ch >= 0x10000) && (ch <= 0x1FFFFF) && (codeLength != 4))) {
						TErrorSet(T_ENCODING_ERROR_INVALID);
						TStreamSeek(stream, pos, SEEK_SET);
						return stats;
					}

					stats->numChars++;
					(*((&stats->numChars) + codeLength))++;
				}
			}
			if (stats->numOneByteChars == stats->numChars)
				stats->encoding = T_ENCODING_ASCII;
		}

		TStreamSeek(stream, pos, SEEK_SET);
	}

	stats->flags |= T_ENCODING_FLAG_VALID;
	return stats;
}

const unsigned char *TEncodingGetBOM(TUInt8 encoding) {
	if (encoding == T_ENCODING_UTF8) {
		return TENCODING_UTF_8_BOM;
	} else if (encoding == T_ENCODING_UTF32_LE) {
		return TENCODING_UTF_32_LE_BOM;
	} else if (encoding == T_ENCODING_UTF32_BE) {
		return TENCODING_UTF_32_BE_BOM;
	} else if (encoding == T_ENCODING_UTF16_LE) {
		return TENCODING_UTF_16_LE_BOM;
	} else if (encoding == T_ENCODING_UTF16_BE) {
		return TENCODING_UTF_16_BE_BOM;
	}

	return 0;
}

TUInt8 TEncodingGetBOMSize(TUInt8 encoding) {
	if (encoding == T_ENCODING_UTF8) {
		return BOM_SIZE_UTF8;
	} else if (encoding == T_ENCODING_UTF32_LE || encoding == T_ENCODING_UTF32_BE) {
		return BOM_SIZE_UTF32;
	} else if (encoding == T_ENCODING_UTF16_LE || encoding == T_ENCODING_UTF16_BE) {
		return BOM_SIZE_UTF16;
	}

	return 0;
}

TUInt8 TEncodingIsASCIIChr(const unsigned char *data) {
	return TEncodingGetUTF8CodeLength(*data) == 1;
}

/*static inline TUInt16 TEncodingUTF16GetCh(const unsigned char **data, TSize *size) {
	TUInt16 ch;

	ch = *((TUInt16 *)*data);
	(*data)++;
	*size -= sizeof(TUInt16);

	// check for high surrogate
	if (ch >= 0xD800 && ch <= 0xDBFF) {
		if (size) {
			TUInt16 ch2 = *((TUInt16 *)data);
			// get low surrogate
			if (ch2 >= 0xDC00 && ch2 <= 0xDFFF) {
				ch = ((ch - UNI_SUR_HIGH_START) << halfShift)
					+ (ch2 - UNI_SUR_LOW_START) + halfBase;
				++source;
			} else {
				// we have the high surrogate but no low, this is not valid
				TFree(result);
				return 0;
			}
		} else {
			// we have the high surrogate but no low, this is not valid
			TFree(result);
			return 0;
		}
	} else if (flags == strictConversion) {
		// UTF-16 surrogate values are illegal in UTF-32
		if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END) {
			--source; // return to the illegal value itself
			result = sourceIllegal;
			break;
		}
	}

	return ch;
}

static inline char *TEncodingUTF16ToUTF8(const unsigned char *data, TSize size) {
	char *result;

	result = TAlloc(size);
	if (!result) return 0;

	while (size) {
		TUInt32 ch;
		unsigned short bytesToWrite;
		
		ch = *((TUInt16 *)data++);
		size -= sizeof(TUInt16);

		// check for high surrogate
		if (ch >= 0xD800 && ch <= 0xDBFF) {
			if (size) {
				TUInt16 ch2 = *((TUInt16 *)data);
				// get low surrogate
				if (ch2 >= 0xDC00 && ch2 <= 0xDFFF) {
					ch = ((ch - UNI_SUR_HIGH_START) << halfShift)
						+ (ch2 - UNI_SUR_LOW_START) + halfBase;
					++source;
				} else {
					// we have the high surrogate but no low, this is not valid
					TFree(result);
					return 0;
				}
			} else {
				// we have the high surrogate but no low, this is not valid
				TFree(result);
				return 0;
			}
		} else if (flags == strictConversion) {
			// UTF-16 surrogate values are illegal in UTF-32
			if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END) {
				--source; // return to the illegal value itself
				result = sourceIllegal;
				break;
			}
		}
		// Figure out how many bytes the result will require
		if (ch < (UTF32)0x80) {
			bytesToWrite = 1;
		} else if (ch < (UTF32)0x800) {
			bytesToWrite = 2;
		} else if (ch < (UTF32)0x10000) {
			bytesToWrite = 3;
		} else if (ch < (UTF32)0x110000) {
			bytesToWrite = 4;
		} else {
			bytesToWrite = 3;
			ch = UNI_REPLACEMENT_CHAR;
		}

		target += bytesToWrite;
		if (target > targetEnd) {
			source = oldSource; // Back up source pointer!
			target -= bytesToWrite; result = targetExhausted; break;
		}
		switch (bytesToWrite) { // note: everything falls through.
			case 4: *--target = (UTF8)((ch | 0x80) & 0xBF); ch >>= 6;
			case 3: *--target = (UTF8)((ch | 0x80) & 0xBF); ch >>= 6;
			case 2: *--target = (UTF8)((ch | 0x80) & 0xBF); ch >>= 6;
			case 1: *--target = (UTF8)(ch | firstByteMark[bytesToWrite]);
		}
		target += bytesToWrite;
	}
	*sourceStart = source;
	*targetStart = target;
	return result;
}

static inline char *TEncodingUTF32LEToUTF8(const unsigned char *data, TSize size) {
	char *cpy = TAlloc(size);
	if (cpy) memcpy(cpy, data, size);
	return cpy;
}

static inline char *TEncodingUTF32BEToUTF8(const unsigned char *data, TSize size) {
	char *cpy = TAlloc(size);
	if (cpy) memcpy(cpy, data, size);
	return cpy;
}*/

unsigned char *TEncodingToASCII(const unsigned char *data, TSize size, TSize *sizeOut, TEncodingStats *stats) {
	if (!data || !sizeOut || !stats) {
		TErrorZero(T_ERROR_INVALID_INPUT);
	}

	if (!size) {
		if (!(stats->flags & T_ENCODING_FLAG_NULL_TERMINATED)) {
			TErrorZero(T_ERROR_INVALID_INPUT);
		}
		return TEncodingToASCIINT(data, sizeOut, stats);
	}

	if (stats->flags & T_ENCODING_FLAG_BOM_PRESENT) {
		TUInt8 BOMSize = TEncodingGetBOMSize(stats->encoding);
		data += BOMSize;
		size -= BOMSize;
	}

	if (stats->encoding == T_ENCODING_ASCII || stats->encoding == T_ENCODING_UTF8) {
		return 0;
	}

	TErrorZero(T_ERROR_INVALID_INPUT);
}

static inline unsigned char *TEncodingToASCIINT(const unsigned char *data, TSize *sizeOut, TEncodingStats *stats) {
	if (stats->flags & T_ENCODING_FLAG_BOM_PRESENT) {
		data += TEncodingGetBOMSize(stats->encoding);
	}

	if (stats->encoding == T_ENCODING_ASCII || stats->encoding == T_ENCODING_UTF8) {
		return 0;
	}

	TErrorSet(T_ERROR_INVALID_INPUT);
	return 0;
}

static inline void TEncodingCodePointToBuffer(unsigned char *buffer, TSize size, TInt32 codePoint) {
	if (size == 1) {
		memcpy(buffer, (char *)&codePoint, sizeof(char));
	} else if (size == 2) {
		char c;

		c = 0xc0 | ((codePoint >> 6) & 0x1f); memcpy(buffer, (char *)&c, sizeof(char));
		c = 0x80 | (codePoint & 0x3f); memcpy(buffer + 1, (char *)&c, sizeof(char));
	} else if (size == 3) {
		char c;

		c = 0xe0 | ((codePoint >> 12) & 0x0f); memcpy(buffer, (char *)&c, sizeof(char));
		c = 0x80 | ((codePoint >> 6) & 0x3f); memcpy(buffer + 1, (char *)&c, sizeof(char));
		c = 0x80 | (codePoint & 0x3f); memcpy(buffer + 2, (char *)&c, sizeof(char));
	} else {
		char c;

		c = 0xf0 | ((codePoint >> 18) & 0x07); memcpy(buffer, (char *)&c, sizeof(char));
		c = 0x80 | ((codePoint >> 12) & 0x3f); memcpy(buffer + 1, (char *)&c, sizeof(char));
		c = 0x80 | ((codePoint >> 6) & 0x3f); memcpy(buffer + 2, (char *)&c, sizeof(char));
		c = 0x80 | (codePoint & 0x3f); memcpy(buffer + 3, (char *)&c, sizeof(char));
	}
}

static inline unsigned char *TEncodingUTF16LEToUTF8(const unsigned char *data, TSize size, TSize *sizeOut, TEncodingStats *stats) {
	TStream *out;
	unsigned char *buffer;
	TUInt8 codeLength;
	TUInt32 code;

	*sizeOut = ((stats->numTwoByteChars * 2) + (stats->numFourByteChars * 4)) * sizeof(unsigned char);
	if (stats->flags & T_ENCODING_FLAG_NULL_TERMINATED) {
		*sizeOut += sizeof(unsigned char);
	}

	buffer = TAlloc(*sizeOut);
	out = TStreamFromMem(&buffer, *sizeOut, 0);

	//reset value for proper calculation
	*sizeOut = 0;
	stats->numTwoByteChars = stats->numFourByteChars = 0;
	while (size) {
		//fetch utf16 character
		code = TEncodingUTF16LEGetChrInternal(&data, &size);
		if (TErrorGet()) goto error;

		// get code length
		codeLength = TEncodingGetUTF8CodeLengthCharacter(code);
		if (!codeLength) {
			TErrorSet(T_ENCODING_ERROR_INVALID);
			goto error;
		}

		//copy data
		code <<= (4 - codeLength) * 8;
		TStreamWriteBlock(out, (const unsigned char *)&code, sizeof(unsigned char) * codeLength);

		//update calculation
		*sizeOut += codeLength * sizeof(unsigned char);
		(*((&stats->numChars) + codeLength))++;
	}

	stats->encoding = T_ENCODING_UTF8;

	TStreamFree(out);
	buffer = TRAlloc(buffer, *sizeOut);
	if (!buffer) *sizeOut = 0;
	return buffer;

error:
	TStreamFree(out);
	TFree(buffer);
	*sizeOut = 0;
	return 0;
}

unsigned char *TEncodingToUTF8(const unsigned char *data, TSize size, TSize *sizeOut, TEncodingStats *stats) {
	if (!data || !sizeOut || !stats) {
		TErrorZero(T_ERROR_INVALID_INPUT);
	}

	if (!size) {
		if (!(stats->flags & T_ENCODING_FLAG_NULL_TERMINATED)) {
			TErrorZero(T_ERROR_INVALID_INPUT);
		}
		return TEncodingToUTF8NT(data, sizeOut, stats);
	}

	if (stats->flags & T_ENCODING_FLAG_BOM_PRESENT) {
		TUInt8 BOMSize = TEncodingGetBOMSize(stats->encoding);
		data += BOMSize;
		size -= BOMSize;
	}

	if (stats->encoding == T_ENCODING_ASCII || stats->encoding == T_ENCODING_UTF8) {
		return 0;
	} else if (stats->encoding = T_ENCODING_UTF16_LE) {
		return TEncodingUTF16LEToUTF8(data, size, sizeOut, stats);
	}

	TErrorZero(T_ERROR_INVALID_INPUT);
}

static inline unsigned char *TEncodingUTF16LEToUTF8NT(const unsigned char *data, TSize *sizeOut, TEncodingStats *stats) {
	TStream *out;
	unsigned char *buffer;
	TUInt8 codeLength;
	TUInt32 code;
	TSize size;

	*sizeOut = (stats->numChars * 2) * sizeof(unsigned char);
	*sizeOut += 1;

	buffer = TAlloc(*sizeOut);
	out = TStreamFromMem(&buffer, *sizeOut, 0);

	//reset value for proper calculation
	*sizeOut = 0;
	stats->numTwoByteChars = stats->numFourByteChars = 0;
	do {
		//fetch utf16 character
		size = 4;
		code = TEncodingUTF16LEGetChrInternal(&data, &size);
		if (TErrorGet()) goto error;

		// get code length
		codeLength = TEncodingGetUTF8CodeLengthCharacter(code);
		if (!codeLength) {
			TErrorSet(T_ENCODING_ERROR_INVALID);
			goto error;
		}

		//copy data
		code <<= (4 - codeLength) * 8;
		TStreamWriteBlock(out, (const unsigned char *)&code, sizeof(unsigned char) * codeLength);

		//update calculation
		*sizeOut += codeLength * sizeof(unsigned char);
		(*((&stats->numChars) + codeLength))++;
	} while (code);

	stats->encoding = T_ENCODING_UTF8;

	TStreamFree(out);
	buffer = TRAlloc(buffer, *sizeOut);
	if (!buffer) *sizeOut = 0;
	return buffer;

error:
	TStreamFree(out);
	TFree(buffer);
	*sizeOut = 0;
	return 0;
}

unsigned char *TEncodingToUTF8NT(const unsigned char *data, TSize *sizeOut, TEncodingStats *stats) {
	if (!data || !sizeOut || !stats) {
		TErrorZero(T_ERROR_INVALID_INPUT);
	}

	if (stats->flags & T_ENCODING_FLAG_BOM_PRESENT) {
		data += TEncodingGetBOMSize(stats->encoding);
	}

	if (stats->encoding == T_ENCODING_ASCII || stats->encoding == T_ENCODING_UTF8) {
		return 0;
	} else if (stats->encoding = T_ENCODING_UTF16_LE) {
		return TEncodingUTF16LEToUTF8NT(data, sizeOut, stats);
	}

	TErrorZero(T_ERROR_INVALID_INPUT);
}

static inline unsigned char *TEncodingUTF16LEToUTF8Chr(const unsigned char *data, TSize *sizeOut, TEncodingStats *stats) {
	unsigned char *buffer;
	TUInt8 codeLength;
	TUInt32 code;
	TSize size;

	//fetch utf-16 character
	size = 4;
	code = TEncodingUTF16LEGetChrInternal(&data, &size);
	if (TErrorGet()) goto error;

	// get code length
	codeLength = TEncodingGetUTF8CodeLengthCharacter(code);
	if (!codeLength) {
		TErrorSet(T_ENCODING_ERROR_INVALID);
		goto error;
	}

	//setup memory block
	*sizeOut = codeLength * sizeof(unsigned char);
	buffer = TAlloc(*sizeOut);
	
	//copy data
	TEncodingCodePointToBuffer(buffer, *sizeOut, code);

	//update stats
	stats->numTwoByteChars = stats->numFourByteChars = 0;
	(*((&stats->numChars) + codeLength))++;
	stats->encoding = T_ENCODING_UTF8;

	return buffer;

error:
	*sizeOut = 0;
	return 0;
}

unsigned char *TEncodingToUTF8Chr(const unsigned char *data, TSize *sizeOut, TEncodingStats *stats) {
	if (!data || !sizeOut || !stats) {
		TErrorZero(T_ERROR_INVALID_INPUT);
	}

	if (stats->encoding == T_ENCODING_ASCII || stats->encoding == T_ENCODING_UTF8) {
		return 0;
	} else if (stats->encoding = T_ENCODING_UTF16_LE) {
		return TEncodingUTF16LEToUTF8Chr(data, sizeOut, stats);
	}

	TErrorZero(T_ERROR_INVALID_INPUT);
}

static inline unsigned char *TEncodingASCIIToUTF16LE(const unsigned char *data, TSize size, TSize *sizeOut, TEncodingStats *stats) {
	TStream *out;
	unsigned char byte;
	unsigned char *buffer, *ptr;
	TUInt16 ch;

	(*sizeOut) = size * 2;

	ptr = buffer = TAlloc(*sizeOut);
	out = TStreamFromMem(&buffer, *sizeOut, 0);

	while (size) {
		// get the char and ensure it can be converted
		byte = *(data++);

		ch = (TUInt16)byte;

		size -= sizeof(unsigned char) * 1;

		// write the char in utf-16
		if (!TStreamWrite16(out, ch)) {
			TStreamFree(out);
			TFree(buffer);
			return 0;
		}
	}

	stats->numTwoByteChars = stats->numChars;
	stats->numOneByteChars = 0;
	stats->encoding = T_ENCODING_UTF16_LE;

	TStreamFree(out);
	return buffer;
}

static inline unsigned char *TEncodingUTF8ToUTF16LE(const unsigned char *data, TSize size, TSize *sizeOut, TEncodingStats *stats) {
	TStream *out;
	unsigned char *buffer;
	TUInt32 ch;

	*sizeOut = (((stats->numTwoByteChars + stats->numOneByteChars + stats->numThreeByteChars) * 2) + stats->numFourByteChars * 4) * sizeof(unsigned char);
	if (stats->flags & T_ENCODING_FLAG_NULL_TERMINATED) {
		*sizeOut += 2 * sizeof(unsigned char);
	}

	buffer = TAlloc(*sizeOut);
	out = TStreamFromMem(&buffer, *sizeOut, 0);

	while (size) {
		ch = TEncodingUTF8GetChrInternal(&data, &size);
		if (TErrorGet()) goto error;

		// write the char in utf-16
		if (ch < 0x10000) {
			TStreamWrite16(out, ch);
		} else if (ch < 0x110000) {
			ch -= 0x10000;
			TStreamWrite16(out, 0xD800 | (ch >> 10));
			TStreamWrite16(out, 0xDC00 | (ch & 0x03FF));
		} else {
			TErrorSet(T_ENCODING_ERROR_INVALID);
			goto error;
		}
	}

	stats->numTwoByteChars += (stats->numOneByteChars + stats->numThreeByteChars);
	stats->numOneByteChars = stats->numThreeByteChars = 0;
	stats->encoding = T_ENCODING_UTF16_LE;


	TStreamFree(out);
	return buffer;

error:
	TStreamFree(out);
	TFree(buffer);
	*sizeOut = 0;
	return 0;
}

unsigned char *TEncodingToUTF16LE(const unsigned char *data, TSize size, TSize *sizeOut, TEncodingStats *stats) {
	if (!data || !sizeOut || !size || !stats) {
		TErrorZero(T_ERROR_INVALID_INPUT);
	}

	if (stats->flags & T_ENCODING_FLAG_BOM_PRESENT) {
		TUInt8 BOMSize = TEncodingGetBOMSize(stats->encoding);
		data += BOMSize;
		size -= BOMSize;
	}

	if (stats->encoding == T_ENCODING_ASCII) {
		return TEncodingASCIIToUTF16LE(data, size, sizeOut, stats);
	} else if (stats->encoding == T_ENCODING_UTF8) {
		return TEncodingUTF8ToUTF16LE(data, size, sizeOut, stats);
	}

	TErrorZero(T_ERROR_INVALID_INPUT);
}

static inline unsigned char *TEncodingUTF8ToUTF16LENT(const unsigned char *data, TSize *sizeOut, TEncodingStats *stats) {
	TStream *out;
	unsigned char *buffer;
	TUInt32 ch;

	*sizeOut = (((stats->numTwoByteChars + stats->numOneByteChars + stats->numThreeByteChars) * 2) + stats->numFourByteChars * 4) * sizeof(unsigned char);
	if (stats->flags & T_ENCODING_FLAG_NULL_TERMINATED) {
		*sizeOut += 2 * sizeof(unsigned char);
	}

	buffer = TAlloc(*sizeOut);
	out = TStreamFromMem(&buffer, *sizeOut, 0);

	stats->numTwoByteChars = stats->numOneByteChars = stats->numThreeByteChars = stats->numFourByteChars = 0;
	do {
		TSize size = 4;
		ch = TEncodingUTF8GetChrInternal(&data, &size);
		if (TErrorGet()) goto error;

		// write the char in utf-16
		if (ch < 0x10000) {
			TStreamWrite16(out, ch);
			stats->numTwoByteChars++;
		} else if (ch < 0x110000) {
			ch -= 0x10000;
			TStreamWrite16(out, 0xD800 | (ch >> 10));
			TStreamWrite16(out, 0xDC00 | (ch & 0x03FF));
			stats->numFourByteChars++;
		} else {
			TErrorSet(T_ENCODING_ERROR_INVALID);
			goto error;
		}
	} while (ch);

	stats->encoding = T_ENCODING_UTF16_LE;

	TStreamFree(out);
	return buffer;

error:
	TStreamFree(out);
	TFree(buffer);
	*sizeOut = 0;
	return 0;
}

unsigned char *TEncodingToUTF16LENT(const unsigned char *data, TSize *sizeOut, TEncodingStats *stats) {
	if (!data || !sizeOut || !stats) {
		TErrorZero(T_ERROR_INVALID_INPUT);
	}

	if (stats->flags & T_ENCODING_FLAG_BOM_PRESENT) {
		data += TEncodingGetBOMSize(stats->encoding);
	}

	if (stats->encoding == T_ENCODING_ASCII || stats->encoding == T_ENCODING_UTF8) {
		return TEncodingUTF8ToUTF16LENT(data, sizeOut, stats);
	} else if (stats->encoding = T_ENCODING_UTF16_LE) {
		return 0;
	}

	TErrorZero(T_ERROR_INVALID_INPUT);
}

TSize TEncodingUTF8Strlen(const unsigned char *data, TSize size) {
	unsigned char byte;
	TUInt8 codeLength, len;
	TSize count = 0;

	if (!data || !size) return 0;

	while (size) {
		byte = *data;
		codeLength = TEncodingGetUTF8CodeLength(byte);
		if (!codeLength) {
			TErrorSet(T_ENCODING_ERROR_INVALID);
			return 0;
		}

		len = sizeof(unsigned char) * codeLength;
		if (len > size) {
			// truncated string or invalid byte sequence
			TErrorSet(T_ENCODING_ERROR_TRUNCATED);
			return 0;
		}

		data += len;
		size -= len;
		count++;
	}

	return count;
}

TUInt32 TEncodingUTF8GetChr(const unsigned char **data, TSize *size) {
	if (!data || !size) {
		TErrorZero(T_ERROR_INVALID_INPUT);
	}

	if (!*size) return 0;

	return TEncodingUTF8GetChrInternal(data, size);
}

TUInt32 TEncodingUTF8GetPreviousChr(const unsigned char **data, TSize *size) {
	unsigned char byte;
	TUInt32 ch, i;

	if (!data || !size) {
		TErrorZero(T_ERROR_INVALID_INPUT);
	}

	ch = 0;
	i = 0;
	do {
		TUInt32 temp;

		(*data)--;
		(*size) += sizeof(unsigned char);
		byte = **data;

		if ((byte & 0xC0) != 0x80) {
			int codeLength = TEncodingGetUTF8CodeLength(byte);
			if (!codeLength) return 0;

			temp = TEncodingGetUTF8Firstbyte(byte, codeLength);
			ch |= temp << i;
			break;
		} else {
			temp = (byte & 0x3F);
		}

		ch |= temp << i;
		i += 6;
	} while (i <= 18);

	if (i > 18) {
		*data += 5;
		*size -= 5;
		return 0;
	}

	return ch;
}

unsigned char *TEncodingUTF8LowerCase(const unsigned char *data, TSize size, TSize *sizeOut) {
	unsigned char *lcdata, *ptr;
	TUInt8 len, codeLength;
	TUInt32 ch;
	TSize remaining = size;

	if (!data || !size || !sizeOut) {
		TErrorSet(T_ERROR_INVALID_INPUT);
		return 0;
	}

	lcdata = TAllocNData(unsigned char, size);
	if (lcdata) {
		ptr = lcdata;
		while (size) {
			ch = TEncodingUTF8GetChrInternal(&data, &size);
			ch = tolower(ch);
			codeLength = TEncodingGetUTF8CodeLengthCharacter(ch);
			if (!codeLength) {
				TFree(lcdata);
				return 0;
			} else if (codeLength > remaining) {
				lcdata = TRAlloc(lcdata, sizeof(unsigned char) * size * 2);
				if (!lcdata) return 0;

				len = 4 - codeLength;
				ch <<= len * 8;
				memcpy(ptr, &ch, sizeof(unsigned char) * codeLength);
				ptr += len;
				break;
			}

			len = 4 - codeLength;
			ch <<= len * 8;
			memcpy(ptr, &ch, sizeof(unsigned char) * codeLength);
			ptr += len;
			remaining -= len;
		}

		while (size) {
			ch = TEncodingUTF8GetChrInternal(&data, &size);
			ch = tolower(ch);
			codeLength = TEncodingGetUTF8CodeLength((unsigned char)ch);
			
			len = 4 - codeLength;
			ch <<= len * 8;
			memcpy(ptr, &ch, sizeof(unsigned char) * codeLength);
			ptr += len;
		}

		*sizeOut = ptr - lcdata;
		lcdata = TRAlloc(lcdata, *sizeOut);
	}

	return lcdata;
}

unsigned char *TEncodingUTF8UpperCase(const unsigned char *data, TSize size, TSize *sizeOut) {
	unsigned char *lcdata, *ptr;
	TUInt8 len, codeLength;
	TUInt32 ch;
	TSize remaining = size;

	if (!data || !size || !sizeOut) {
		TErrorSet(T_ERROR_INVALID_INPUT);
		return 0;
	}

	lcdata = TAllocNData(unsigned char, size);
	if (lcdata) {
		ptr = lcdata;
		while (size) {
			ch = TEncodingUTF8GetChrInternal(&data, &size);
			ch = toupper(ch);
			codeLength = TEncodingGetUTF8CodeLengthCharacter(ch);
			if (!codeLength) {
				TFree(lcdata);
				return 0;
			} else if (codeLength > remaining) {
				lcdata = TRAlloc(lcdata, sizeof(unsigned char) * size * 2);
				if (!lcdata) return 0;

				len = 4 - codeLength;
				ch <<= len * 8;
				memcpy(ptr, &ch, sizeof(unsigned char) * codeLength);
				ptr += len;
				break;
			}

			len = 4 - codeLength;
			ch <<= len * 8;
			memcpy(ptr, &ch, sizeof(unsigned char) * codeLength);
			ptr += len;
			remaining -= len;
		}

		while (size) {
			ch = TEncodingUTF8GetChrInternal(&data, &size);
			ch = toupper(ch);
			codeLength = TEncodingGetUTF8CodeLength((unsigned char)ch);

			len = 4 - codeLength;
			ch <<= len * 8;
			memcpy(ptr, &ch, sizeof(unsigned char) * codeLength);
			ptr += len;
		}

		*sizeOut = ptr - lcdata;
		lcdata = TRAlloc(lcdata, *sizeOut);
	}

	return lcdata;
}

int TEncodingIncrement(const unsigned char **data, TSize *size, TUInt8 encoding, TLInt numCharacters) {
	TEncodeIncrement inc;

	if (!data || !size) {
		TErrorSet(T_ERROR_INVALID_INPUT);
		return 1;
	}

	inc = TEncodeIncrements[encoding];
	if (inc) return inc(data, size, numCharacters);

	TErrorSet(T_ERROR_INVALID_INPUT);
	return 1;
}

int TEncodingASCIIIncrement(const unsigned char **data, TSize *size, TLInt numCharacters) {
	if (!data || !size) {
		TErrorSet(T_ERROR_INVALID_INPUT);
		return 1;
	}

	if (!numCharacters) return 0;
	
	*data += numCharacters;
	(*size) -= sizeof(unsigned char) * numCharacters;

	return 0;
}

int TEncodingUTF8Increment(const unsigned char **data, TSize *size, TLInt numCharacters) {
	unsigned char byte;
	TUInt8 codeLength;

	if (!data || !size) {
		TErrorSet(T_ERROR_INVALID_INPUT);
		return 1;
	}

	if (!numCharacters) return 0;
	else if (numCharacters < 0) {
		while (numCharacters++) {
			do {
				(*data)--;
				(*size) += sizeof(unsigned char);
				byte = **data;
			} while ((byte & 0xC0) == 0x80);
		}
	} else {
		while (numCharacters-- && *size) {
			byte = **data;

			codeLength = TEncodingGetUTF8CodeLength(byte);
			if (!codeLength) return 1;

			if (codeLength * sizeof(unsigned char) > *size) {
				// truncated string or invalid byte sequence
				return 1;
			}

			*data += codeLength;
			(*size) -= sizeof(unsigned char) * codeLength;
		}
	}

	return 0;
}
