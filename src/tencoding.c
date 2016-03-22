
#include "stdafx.h"

#include "tencoding.h"

#include "terror.h"
#include "talloc.h"

static const char *TENCODING_UTF_16_BE_BOM = "\xFE\xFF";
static const char *TENCODING_UTF_16_LE_BOM = "\xFF\xFE";
static const char *TENCODING_UTF_8_BOM     = "\xEF\xBB\xBF";
static const char *TENCODING_UTF_32_BE_BOM = "\x00\x00\xFE\xFF";
static const char *TENCODING_UTF_32_LE_BOM = "\xFF\xFE\x00\x00";

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

TUInt32 TEncodingUTF8GetChrInternal(const char **data, TSize *size) {
	unsigned char byte;
	TUInt8 codeLength, i;
	TUInt32 ch;

	byte = **data;
	codeLength = TEncodingGetUTF8CodeLength(byte);
	if (!codeLength) return 0;

	ch = TEncodingGetUTF8Firstbyte(byte, codeLength);

	if (codeLength * sizeof(unsigned char) > *size) {
		// truncated string or invalid byte sequence
		return 0;
	}

	for (i = 1; i < codeLength; i++, byte++) {
		ch = (ch << 6) | (byte & 0x3F);
	}

	(*data) += codeLength;
	*size -= sizeof(unsigned char) * codeLength;

	return ch;
}

static inline int TEncodingIsValidUTF8(const unsigned char **data, TSize *size) {
	unsigned char byte;
	TUInt8 codeLength, i;
	TUInt32 ch;

	byte = **data;
	codeLength = TEncodingGetUTF8CodeLength(byte);
	if (!codeLength) return 0;

	ch = TEncodingGetUTF8Firstbyte(byte, codeLength);

	if (codeLength * sizeof(unsigned char) > *size) {
		// truncated string or invalid byte sequence
		return 0;
	}

	for (i = 1; i < codeLength; i++, byte++) {
		// Check continuation bytes
		if ((byte & 0xC0) != 0x80)
			return 0;

		ch = (ch << 6) | (byte & 0x3F);
	}

	if ((ch > 0x10FFFF) ||
		((ch >= 0xD800) && (ch <= 0xDFFF)) ||
		((ch <= 0x007F) && (codeLength != 1)) ||
		((ch >= 0x0080) && (ch <= 0x07FF) && (codeLength != 2)) ||
		((ch >= 0x0800) && (ch <= 0xFFFF) && (codeLength != 3)) ||
		((ch >= 0x10000) && (ch <= 0x1FFFFF) && (codeLength != 4)))
		return 0;

	(*data) += codeLength;
	*size -= sizeof(unsigned char) * codeLength;

	return 1;
}

TUInt8 TEncodingGuess(const unsigned char *data, TSize size) {
	TUInt8 encoding = T_ENCODING_UNKNOWN;

	while (size) {
		if (*data < 0x80) {
			data += 1;
			if (encoding == T_ENCODING_UNKNOWN)
				encoding = T_ENCODING_ASCII;
			continue;
		}

		if (TEncodingIsValidUTF8(&data, &size))
			encoding = T_ENCODING_UTF8;
		else
			return T_ENCODING_UNKNOWN;
	}

	return encoding;
}

TUInt8 TEncodingStreamGuess(TStream *stream) {
	TUInt8 encoding = T_ENCODING_UNKNOWN;
	unsigned char buffer[4];
	unsigned char *ptr;
	TSize size;

	while (!TStreamEOF(stream)) {
		size = TStreamReadBlock(stream, buffer, 4 * sizeof(unsigned char));
		ptr = buffer;

		if (*buffer < 0x80) {
			if (encoding == T_ENCODING_UNKNOWN)
				encoding = T_ENCODING_ASCII;
			continue;
		}

		if (TEncodingIsValidUTF8(&ptr, &size))
			encoding = T_ENCODING_UTF8;
		else
			return T_ENCODING_UNKNOWN;
	}

	return encoding;
}

TUInt8 TEncodingCheckBOM(const unsigned char *data, TSize size) {
	if (size >= 3) {
		if (memcmp(data, TENCODING_UTF_8_BOM, 3) == 0)
			return T_ENCODING_UTF8;
	}
	if (size >= 4) {
		if (memcmp(data, TENCODING_UTF_32_LE_BOM, 4) == 0)
			return T_ENCODING_UTF32_LE;
		if (memcmp(data, TENCODING_UTF_32_BE_BOM, 4) == 0)
			return T_ENCODING_UTF32_BE;
	}
	if (size >= 2) {
		if (memcmp(data, TENCODING_UTF_16_LE_BOM, 2) == 0)
			return T_ENCODING_UTF16_LE;
		if (memcmp(data, TENCODING_UTF_16_BE_BOM, 2) == 0)
			return T_ENCODING_UTF16_BE;
	}
	return T_ENCODING_UNKNOWN;
}

TUInt8 TEncodingGetDataEncoding(const unsigned char *data, TSize size) {
	TUInt8 encoding;

	if (!data || !size) {
		TErrorReportDefault(T_ERROR_INVALID_INPUT);
		return T_ENCODING_UNKNOWN;
	}

	//first check for BOM
	encoding = TEncodingCheckBOM(data, size);

	if (encoding == T_ENCODING_UNKNOWN) {
		//guess the encoding
		encoding = TEncodingGuess(data, size);
	}

	return encoding;
}

TUInt8 TEncodingGetStreamEncoding(TStream *stream) {
	unsigned char buffer[4];
	TUInt8 encoding;
	TUInt8 size;
	TLInt pos;

	if (!stream) {
		TErrorReportDefault(T_ERROR_INVALID_INPUT);
		return T_ENCODING_UNKNOWN;
	}

	if (TStreamEOF(stream)) {
		return T_ENCODING_UNKNOWN;
	}

	pos = TStreamTell(stream);

	// read the BOM
	size = TStreamReadBlock(stream, buffer, 4 * sizeof(unsigned char));
	encoding = TEncodingCheckBOM(buffer, size);

	//reset to position
	TStreamSeek(stream, pos, SEEK_SET);

	if (encoding == T_ENCODING_UNKNOWN) {
		//guess the encoding
		encoding = TEncodingStreamGuess(stream);

		//reset to start
		TStreamSeek(stream, pos, SEEK_SET);
	}

	return encoding;
}

static inline unsigned char *TEncodingASCIIToUTF8(const unsigned char *data, TSize size) {
	char *cpy = TAlloc(size);
	if (cpy) memcpy(cpy, data, size);
	return cpy;
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


unsigned char *TEncodingToUTF8(const unsigned char *data, TSize size, TUInt8 hint_encoding) {
	if (!data || !size) {
		TErrorZero(T_ERROR_INVALID_INPUT);
	}

	if (hint_encoding == T_ENCODING_UNKNOWN)
		hint_encoding = TEncodingGetDataEncoding(data, size);
	
	if (hint_encoding == T_ENCODING_ASCII) {
		return TEncodingASCIIToUTF8(data, size);
	}

	TErrorReportDefault(T_ERROR_INVALID_INPUT);
	return 0;
}

static inline unsigned char *TEncodingASCIIToUTF16LE(const unsigned char *data, TSize size) {
	TStream *out;
	unsigned char byte;
	unsigned char *buffer;
	TUInt16 ch;

	if (!data) return 0;

	buffer = TAlloc(size * 2);
	out = TStreamFromMem(buffer, size * 2, 0);

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

	TStreamFree(out);
	return buffer;
}

static inline unsigned char *TEncodingUTF8ToUTF16LE(const unsigned char *data, TSize size) {
	TStream *out;
	unsigned char byte;
	int codeLength, i;
	unsigned char *buffer;
	TUInt32 ch;

	if (!data) return 0;

	buffer = TAlloc(size * 2);
	out = TStreamFromMem(buffer, size * 2, 0);

	while (size) {
		// get the char and ensure it can be converted
		byte = *data;
		
		codeLength = TEncodingGetUTF8CodeLength(byte);
		if (!codeLength || (codeLength == 2 && byte < 0xC0))
			goto error;
		if (codeLength * sizeof(unsigned char) > size) {
			// truncated string or invalid byte sequence
			goto error;
		}

		ch = TEncodingGetUTF8Firstbyte(byte, codeLength);

		for (i = 1; codeLength; i++, byte++) {
			ch = (ch << 6) | (byte & 0x3F);
		}

		data += codeLength;
		size -= sizeof(unsigned char) * codeLength;

		// write the char in utf-16
		// assertion: ch is a single UTF-4 value
		if (ch < 0x10000) {
			TStreamWrite16(out, ch << 16);
		} else if (ch < 0x110000) {
			ch -= 0x10000;
			TStreamWrite16(out, 0xD800 | (ch >> 10));
			TStreamWrite16(out, 0xDC00 | (ch & 0x03FF));
		} else
			break;
	}

	TStreamFree(out);
	return buffer;

error:
	TStreamFree(out);
	TFree(buffer);
	return 0;
}

unsigned char *TEncodingToUTF16LE(const unsigned char *data, TSize size, TUInt8 hint_encoding) {
	if (!data || !size) {
		TErrorZero(T_ERROR_INVALID_INPUT);
	}

	if (hint_encoding == T_ENCODING_UNKNOWN)
		hint_encoding = TEncodingGetDataEncoding(data, size);

	if (hint_encoding == T_ENCODING_ASCII) {
		return TEncodingASCIIToUTF16LE(data, size);
	} else if (hint_encoding == T_ENCODING_UTF8) {
		return TEncodingUTF8ToUTF16LE(data, size);
	}

	TErrorReportDefault(T_ERROR_INVALID_INPUT);
	return 0;
}

TUInt32 TEncodingUTF8GetChr(const unsigned char **data, TSize *size) {
	if (!data || !size) {
		TErrorZero(T_ERROR_INVALID_INPUT);
	}

	return TEncodingUTF8GetChrInternal(data, size);
}

TUInt32 TEncodingUTF8GetPreviousChr(const unsigned char **data, TSize *size) {
	unsigned char byte;
	TUInt32 ch;
	TSize i;

	if (!data || !size) {
		TErrorZero(T_ERROR_INVALID_INPUT);
	}

	i = 0;
	do {
		(*data)--;
		(*size) += sizeof(unsigned char);
		byte = **data;
		TUInt32 temp;

		if ((byte & 0xC0) != 0x80) {
			int codeLength = TEncodingGetUTF8CodeLength(byte);
			if (!codeLength) return 0;

			temp = TEncodingGetUTF8Firstbyte(byte, codeLength);
		} else {
			temp = (byte & 0x3F);
		}

		ch |= temp << i;
		i += 6;
	} while ((byte & 0xC0) == 0x80);

	return ch;
}

void TEncodingUTF8Increment(const unsigned char **data, TSize *size) {
	unsigned char byte;
	TUInt8 codeLength;

	if (!data || !size) {
		TError(T_ERROR_INVALID_INPUT);
	}

	byte = **data;

	codeLength = TEncodingGetUTF8CodeLength(byte);
	if (!codeLength) return;

	if (codeLength * sizeof(unsigned char) > *size) {
		// truncated string or invalid byte sequence
		return;
	}

	(*data) += codeLength;
	(*size) -= sizeof(unsigned char) * codeLength;
}

void TEncodingUTF8Decrement(const unsigned char **data, TSize *size) {
	unsigned char byte;

	if (!data || !size) {
		TError(T_ERROR_INVALID_INPUT);
	}

	do {
		(*data)--;
		(*size) += sizeof(unsigned char);
		byte = **data;
	} while ((byte & 0xC0) == 0x80);
}

/*void
encode_utf16_pair(uint32_t character, uint16_t *units) {
	unsigned int code;
	assert(0x10000 <= character && character <= 0x10FFF);
	code = (character - 0x10000);
	units[0] = 0xD800 | (code >> 10);
	units[1] = 0xDC00 | (code & 0x3FF);
}

uint32_t
decode_utf16_pair(uint16_t *units) {
	uint32_t code;
	assert(0xD800 <= units[0] && units[0] <= 0xDBFF);
	assert(0xDC00 <= units[1] && units[1] <= 0xDFFF);
	code = 0x10000;
	code += (units[0] & 0x03FF) << 10;
	code += (units[1] & 0x03FF);
	return code;
}*/
