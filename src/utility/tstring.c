
#include "stdafx.h"

#include "tstring.h"

#include "talloc.h"
#include "terror.h"

#include "tconvert.h"

#include "tencoding.h"

char *TStringLwr(const char *s) {
	TSize size = TStringSize(s);
	return TEncodingUTF8LowerCase(s, size, &size);
}

char *TStringUpr(const char *s) {
	TSize size = TStringSize(s);
	return TEncodingUTF8UpperCase(s, size, &size);
}

char *TStringCopy(const char *text) {
	char *cpy = 0;

	if (text) {
		TSize size = TStringSize(text);
		cpy = (char *)TAlloc(size);
		memcpy(cpy, text, size);
	}

	return cpy;
}

char *TStringNCopy(const char *text, int num) {
	char *cpy = 0;
	const char *ptr;
	TSize size;

	if (!num || !text) { return 0; }

	size = TStringSize(text);
	ptr = text;
	TEncodingUTF8Increment(&ptr, &size, num);
	if (!*ptr) return TStringCopy(text);
	size = (ptr - text) + sizeof(char);

	cpy = (char *)TAlloc(size);
	memcpy(cpy, text, size - 1);

	//add Termination
	*(cpy + (size - 1)) = 0;

	return cpy;
}

char *TStringSCopy(const char *text, TSize size) {
	char *cpy = 0;

	if (text) {
		cpy = (char *)TAlloc(size);
		memcpy(cpy, text, size - 1);
		*(cpy + (size - 1)) = 0;
	}

	return cpy;
}

int TStringAdjustSize(char **text, TSize oldsize, TSize newsize) {
	TPtr nptr = TRAlloc(*text, newsize);
	if (!nptr) return 1;
	*text = (char *)nptr;

	if (newsize > oldsize)
		memset((*text) + oldsize, 0, newsize);

	return 0;
}

#ifdef _WINDOWS
char *TStringFromWideChar(wchar_t *string) {
	TSize size;
	TEncodingStats stats = {
		T_ENCODING_UTF16_LE,
		0, 0, 0, 0, 0,
		T_ENCODING_FLAG_VALID | T_ENCODING_FLAG_NULL_TERMINATED,
	};

	return TEncodingToUTF8NT((unsigned char *)string, &size, &stats);
}

wchar_t *TStringToWideChar(const char *string) {
	TSize temp;
	TEncodingStats stats = {
		T_ENCODING_UTF8,
		0,0,0,0,0,
		T_ENCODING_FLAG_VALID | T_ENCODING_FLAG_NULL_TERMINATED,
	};

	return (wchar_t *)TEncodingToUTF16LENT(string, &temp, &stats);
}
#endif

TSize TStringLen(const char *string) {
	TSize size = TStringSize(string);
	return TEncodingUTF8Strlen(string, size);
}

TUInt32 TStringGetCharacter(const char *string, int position) {
	const char *pos = string + position;
	TSize size = TStringSize(string);
	return TEncodingUTF8GetChr(&pos, &size);
}

char *TStringGetCharacterStr(const char *string, int position) {
	const char *c;
	TUInt8 i;
	char sc[5] = { 0 };

	c = string + position;
	i = 0;

	while (!TEncodingIsASCIIChr(c)) sc[i++] = *(c++);
	sc[i] = *c;

	return TStringCopy(sc);
}

int TStringIsCharEscaped(const char *start, const char *pos) {
	const char *ptr;
	char odd = 0;

	if (pos == start) return 0;
	if (!pos || !start) return 0;

	ptr = pos;

	while (--ptr != start) {
		if (*ptr == '\\') odd = !odd;
		else return odd;
	}

	if (*ptr == '\\') odd = !odd;
	return odd;
}

int TStringCaseInsensitiveCompare(const char *str1, const char *str2) {
	char *lstr1 = TStringLwr(str1);
	char *lstr2 = TStringLwr(str2);

	int res = strcmp(lstr1, lstr2);

	TFree(lstr1);
	TFree(lstr2);

	return res;
}

TSize TStringRCSpn(const char *_str, const char *_control) {
	TSize len = 0;

	//sanity check
	if (!_str) return 0;

	len = TStringLen(_str);

	if (!_control) return len + 1;

	{
		TSize size = TStringSize(_str);
		TSize i = len;
		const char *c = _str + size - 1;

		while (c != _str) {
			TEncodingUTF8Increment(&c, &size, -1);
			if (strchr(_control, *c)) return i;
			i--;
		}

		if (strchr(_control, *c)) return 0;
	}

	return len + 1;
}

inline void TStringReplaceOp(char *target, const char *match, const char *replacement, TSize repllen, TSize limit) {
	char *curptr = target;
	TSize i = 0;

	do {
		curptr = strstr(curptr, match);
		if (curptr) {
			memcpy(curptr, replacement, repllen);
			curptr += repllen;
			i++;
		}
	} while (curptr && (!limit || i < limit));
}

inline void TStringCopyReplaceOp(char *target, const char *source, const char *match, const char *replacement, TSize limit) {
	const char *srcptr = source;
	const char *prevPtr;
	char *tarptr = target;
	TSize len;
	TSize repllen;
	TSize i = 0;

	repllen = strlen(replacement);
	prevPtr = srcptr;
	while ((srcptr = strstr(srcptr, match)) && i++ < limit) {
		len = srcptr - prevPtr;
		memcpy(tarptr, prevPtr, len); tarptr += len;
		memcpy(tarptr, replacement, repllen); tarptr += repllen;
		prevPtr = srcptr + 1;
	}

	if (srcptr) {
		srcptr = source + strlen(source);
		memcpy(tarptr, prevPtr, srcptr - prevPtr + 1);
	} else {
		memcpy(tarptr, prevPtr, srcptr - prevPtr + 1);
	}
}

TSize TStringNumOccurences(const char *target, const char *match) {
	const char *curptr = target;
	TSize matchlen = strlen(match);
	TSize counter = 0;

	do {
		curptr = strstr(curptr, match);
		if (curptr) {
			counter += 1;
			curptr += matchlen;
		}
	} while (curptr);

	return counter;
}

/*
 * Replace a substring of a string with another string
 */
char *TStringReplace(const char *source, const char *match, const char *replacement, TSize limit, TSize hint_numoccurence) {
	char *result = 0;
	TSize srclen = 0;
	TSize matchlen = 0;
	TSize repllen = 0;
	TSize reslen = 0;

	if (source == 0 || match == 0 || replacement == 0 || match[0] == '\0' || replacement[0] == '\0')
		return 0;
	else if (source[0] == '\0') {
		return TStringCopy(source);
	}

	srclen = strlen(source);
	matchlen = strlen(match);
	repllen = strlen(replacement);
	reslen = srclen + 1;

	if (matchlen != repllen) {
		if (!limit) {
			if (!hint_numoccurence) {
				//find number of occurences
				hint_numoccurence = TStringNumOccurences(source, match);
				if(!hint_numoccurence) return TStringCopy(source);
			}
			limit = hint_numoccurence;
		}

		reslen -= ((int)matchlen - (int)repllen) * (int)limit;
		result = (char *)TAlloc(reslen);
		if (result) TStringCopyReplaceOp(result, source, match, replacement, limit);
	} else {
		result = TStringCopy(source);
		if (result) TStringReplaceOp(result, match, replacement, repllen, limit);
	}

	return result;
}

TUInt8 TStringReplaceInplace(char *source, const char *match, const char *replacement, TSize limit) {
	TSize repllen = 0;

	if (source == 0 || match == 0 || replacement == 0)
		return 1;

	repllen = strlen(replacement);

	if (strlen(match) != repllen)
		return 2;

	TStringReplaceOp(source, match, replacement, repllen, limit);

	return 0;
}

void TStringSafetyString(char *string) {
	int strlength = strlen(string);
	TStringReplaceOp(string, "%", " ", strlength, 0);
}

char **TStringSplit(const char *string, const char *substr, TSize *size, TSize limit) {
	char **sto;
	char *str;
	char *ptr;
	TSize len;
	TSize i;
	if (!size || !string || !substr || !*substr) return 0;

	str = TStringCopy(string);
	len = strlen(substr);
	ptr = str;
	*size = 1;
	i = 0;

	while ((ptr = strstr(ptr, substr)) && (!limit || (i++ < limit))) {
		*ptr = 0;

		ptr += len;
		*size += 1;
	}

	sto = (char **)TAlloc(sizeof(char *) * (*size));
	if (sto) {
		TSize i = 1;
		sto[0] = str;

		while (i < *size) {
			ptr = strchr(sto[i - 1], 0) + len;
			sto[i++] = ptr;
		}
	} else free(str);
	return sto;
}

char **TStringRSplit(const char *string, const char *substr, TSize *size, TSize limit) {
	char **sto;
	char *str;
	char *ptr;
	TSize len;
	TSize i;
	if (!size || !string || !substr || !*substr) return 0;

	str = TStringCopy(string);
	len = strlen(substr);
	*size = 1;
	i = 0;
	ptr = strrchr(str, substr[0]);

	while (ptr && (!limit || i < limit)) {
		if (!strncmp(ptr, substr, len)) {
			i++;
			*size += 1;
			*ptr = 0;
			ptr = strrchr(str, substr[0]);
		} else {
			char *tmp = ptr, v = *ptr;
			*tmp = 0;
			ptr = strrchr(str, substr[0]);
			*tmp = v;
		}
	}

	sto = (char **)TAlloc(sizeof(char *) * (*size));
	if (sto) {
		TSize i = 1;
		sto[0] = str;

		while (i < *size) {
			ptr = strchr(sto[i - 1], 0) + len;
			sto[i++] = ptr;
		}
	} else TFree(str);
	return sto;
}

char *TStringAddCharacter(const char *string, const char *character, TSize start, TSize end) {
	TSize len;
	TSize cLen;
	char *newstring = 0;

	if (!string || !character) return 0;

	if (start > end) TSWAP(start, end);

	cLen = strlen(character);
	len = strlen(string);

	{
		const char *ptr = string;
		TSize remaining = strlen(string);
		TSize oStart;

		TEncodingUTF8Increment(&ptr, &remaining, start);

		oStart = start;
		start = len - remaining;

		if(end == oStart) end = start;
		else {
			TEncodingUTF8Increment(&ptr, &remaining, end - oStart);
			end = len - remaining;
		}
	}

	len += cLen + 1;

	if (start != end) len -= end - start;

	newstring = TAllocNData(char, len);
	if (newstring) {
		if (start) memcpy(newstring, string, start);
		memcpy(newstring + start, character, cLen);
		memcpy(newstring + start + cLen, string + end, len - cLen - end);
	}

	return newstring;
}

char *TStringAppendCharacter(const char *string, char character) {
	TSize size;
	char *result;

	if (!string) { TErrorZero(T_ERROR_NULL_POINTER); }

	size = strlen(string) + 2;
	result = TAllocNData(char, size);
	memcpy(result, string, size - 2);
	result[size - 2] = character;
	result[size - 1] = 0;

	return result;
}

char *TStringInsert(const char *src, const char *target, int position) {
	char *result;
	int len, slen;

	if (!src || !target) return 0;

	len = strlen(target);
	slen = strlen(src);
	result = TAllocNData(char, len + slen + 1);

	if (len < position) {
		memcpy(result, target, sizeof(char) * len);
		memcpy(result + len, src, sizeof(char) * (slen + 1));
	} else if (position <= 0) {
		memcpy(result, src, sizeof(char) * slen);
		memcpy(result + slen, target, sizeof(char) * (len + 1));
	} else {
		memcpy(result, target, sizeof(char) * position);
		memcpy(result + position, src, sizeof(char) * slen);
		memcpy(result + position + slen, target + position, sizeof(char) * (len - position + 1));
	}

	return result;
}

char *TStringConcat(const char *str, ...) {
	va_list components;
	const char *component;
	char *buffer;
	TSize bufferlen;

	if (!str) return 0;

	buffer = TStringCopy(str);
	bufferlen = strlen(buffer);

	va_start(components, str);
	while ((component = va_arg(components, const char *))) {
		TSize llen = strlen(component);
		buffer = TRAlloc(buffer, bufferlen + llen + 1);
		memcpy(buffer + bufferlen, component, (llen + 1) * sizeof(char));
		bufferlen += llen;
	}
	va_end(components);

	return buffer;
}

char *TStringConcatInplace(char *str, ...) {
	va_list components;
	const char *component;
	char *tmp;
	TSize bufferlen;

	if (!str) return 0;

	bufferlen = strlen(str);

	va_start(components, str);
	while ((component = va_arg(components, const char *))) {
		TSize llen = strlen(component);
		tmp = TRAlloc(str, bufferlen + llen + 1);
		if (!tmp) return 0;
		str = tmp;
		memcpy(str + bufferlen, component, (llen + 1) * sizeof(char));
		bufferlen += llen;
	}
	va_end(components);

	return str;
}

char *TStringConcatSeparator(const char *separator, const char *str, ...) {
	va_list components;
	const char *component;
	char *buffer;
	unsigned int bufferlen = 0;
	unsigned int seplen = strlen(separator);

	buffer = TStringCopy(str);
	bufferlen = strlen(buffer);

	va_start(components, str);
	while ((component = va_arg(components, const char *))) {
		TSize llen = strlen(component) + seplen;
		buffer = TRAlloc(buffer, bufferlen + llen + 1);
		snprintf(buffer + bufferlen, llen + 1, "%s%s", separator, component);
		bufferlen += llen;
	}
	va_end(components);

	return buffer;
}

char *TStringStripTrailingWhitespace(const char *string) {
	const char *ptr = string + (strlen(string) - 1);
	while (ptr != string && (*ptr == ' ' || *ptr == '\n' || *ptr == '\r')) ptr--;
	return TStringNCopy(string, ptr - string + 1);
}

const char *TStringStripLeadingSpaces(const char *string) {
	while (*string == ' ') string++;
	return string;
}

char *TStringPasswordEncrypt(const char *src) {
	TSize len = 0;
	char *result = 0;

	if (!src) return 0;

	len = strlen(src);
	if (!len) return 0;

	result = (char *)TAlloc(sizeof(char) *(len + 1));
	if (result) {
		memset(result, '*', len);
		result[len] = 0;
	}

	return result;
}

char *TStringDoubleChars(const char *string, const char escchar) {
	if (!string) return 0;

	char escStr[] = {escchar, 0};
	TSize numEsc, size;
	char *escString, *wPtr;
	const char *ptr;

	numEsc = TStringNumOccurences(string, escStr);
	if (!numEsc) return TStringCopy(string);

	size = strlen(string) + numEsc;
	escString = TAllocNData(char, size);
	if (!escString) return 0;
	ptr = string;
	wPtr = escString;

	do {
		ptr = strchr(string, escchar);
		if (ptr) {
			TSize escSize = ptr - string + 1;
			memcpy(wPtr, string, escSize);
			wPtr += escSize;
			*(wPtr++) = escchar;
			size -= escSize + 1;

			string = ptr + 1;
		} else {
			memcpy(wPtr, string, size);
		}
	} while (ptr);

	return escString;
}

char *TStringRemoveDuplication(const char *string, const char escchar) {
	if (!string) return 0;

	char escStr[] = {escchar, escchar, 0};
	TSize numEsc, size;
	char *escString, *wPtr;
	const char *ptr;

	numEsc = TStringNumOccurences(string, escStr);
	if (!numEsc) return TStringCopy(string);

	size = strlen(string) - numEsc;
	escString = TAllocNData(char, size);
	if (!escString) return 0;
	ptr = string;
	wPtr = escString;

	do {
		ptr = strstr(string, escStr);
		if (ptr) {
			TSize escSize = ptr - string + 1;
			memcpy(wPtr, string, escSize);
			wPtr += escSize;
			size -= escSize;

			string = ptr + 2;
		} else {
			memcpy(wPtr, string, size);
		}
	} while (ptr);

	return escString;
}

TLInt stoi(const char *str, TSize limit) {
	TSize len = strlen(str);
	if (len <= limit) {
		TLInt convert = 0;
		TLInt offset = 1;
		TUInt8 negative = str[0] == '-';
		TSize i;

		if (negative) { str = str + 1; len--; }

		for (i = len; i >= 0; i--) {
			char c = str[i];
			if (!TStringIsDigit(c)) return 0;
			convert += (c - '0') * offset;
			offset *= 10;
		}

		if (negative) convert = -convert;
		return convert;
	}

	return 0;
}

TLInt stoui(const char *str, TSize limit) {
	TSize len = strlen(str);
	if (len < limit) {
		TLInt convert = 0;
		TLInt offset = 1;
		int i;

		for (i = len - 1; i >= 0; i--) {
			char c = str[i];
			if (!TStringIsDigit(c)) return 0;
			convert += (c - '0') * offset;
			offset *= 10;
		}

		return convert;
	}

	return 0;
}

TInt8 stoi8(const char *str) {
	return stoi(str, 4);
};

TUInt8 stoui8(const char *str) {
	return stoui(str, 4);
};

TInt16 stoi16(const char *str) {
	return stoi(str, 6);
};

TUInt16 stoui16(const char *str) {
	return stoui(str, 6);
};

TUInt32 stoui32(const char *str) {
	return stoui(str, 11);
};

TInt64 stoi64(const char *str) {
	return stoi(str, 20);
};

TUInt64 stoui64(const char *str) {
	return stoui(str, 21);
};
