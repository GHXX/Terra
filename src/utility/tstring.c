
#include "stdafx.h"

#include "tstring.h"

#include "talloc.h"
#include "terror.h"

#include "tencoding.h"

struct TString {
	unsigned char *content;
	TSize size;
	TUInt8 encoding;
};

TString *TStringFromString(const char *string) {
	TString *s = TAllocData(TString);
	if (s) {
		s->encoding = T_ENCODING_ASCII;

		if (string) {
			s->size = (strlen(string) + 1) * sizeof(char);
			s->content = TAlloc(s->size);
			memcpy(s->content, string, s->size);
		} else {
			s->size = 0;
			s->content = 0;
		}
	}
	return s;
}

TString *TStringFromTString(TString *string) {
	TString *s;
	
	if (!string) TErrorZero(T_ERROR_INVALID_INPUT);

	s = TAllocData(TString);
	if (s) {
		s->encoding = string->encoding;
		s->size = string->size;
		s->content = TAlloc(s->size);
		memcpy(s->content, string->content, s->size);
	}
	return s;
}

TString *TStringNCopy(TString *string, TSize num) {
	TString *s;

	if (!string || !num) TErrorZero(T_ERROR_INVALID_INPUT);
	
	if (num + 1 >= string->size) return TStringFromTString(string);

	s = TAllocData(TString);
	if (s) {
		TSize size;

		s->encoding = string->encoding;

		if (string->encoding == T_ENCODING_UTF8) {
			unsigned char *ptr = string->content;
			size = string->size;
			while (num--) {
				TEncodingUTF8Increment(ptr, &size);
				if (size == 0) {
					s->encoding = string->encoding;
					s->size = string->size;
					s->content = TAlloc(s->size);
					memcpy(s->content, string->content, s->size);
					return s;
				}
			}
			size = ptr - string->content + 1;
		} else {
			size = num + 1;
		}

		s->size = size;
		s->content = TAlloc(s->size);
		memcpy(s->content, string->content, s->size);

		//add Termination
		s->content[s->size - 1] = 0;
	}
	return s;
}

void TStringFree(TString *string) {
	if (string) {
		TFree(string->content);
		TFree(string);
	}
}

#ifdef _WINDOWS
wchar_t *TStringToWideChar(TString *string) {
	if (!string) return 0;

	//wchar_t is utf-16 little endian in windows
	return (wchar_t *) TEncodingToUTF16LE(string->content, string->size, string->encoding);
}
#endif

int TStringCaseInsensitiveCompare(TString *str1, TString *str2) {
	TString *lstr1 = TStringLowerCase(str1);
	TString *lstr2 = TStringLowerCase(str2);
	TSize size = TMIN(lstr1->size, lstr2->size);

	int res = memcmp(lstr1, lstr2, size);

	TFree(lstr1);
	TFree(lstr2);

	return res;
}

TSize TStringRCSpn(TString *string, const char *control) {
	TSize i = 0;

	//sanity check
	if (!string) TErrorZero(T_ERROR_INVALID_INPUT);

	i = string->size;

	if (!control) return i + 1;

	if(string->encoding == T_ENCODING_ASCII) {
		const char *start = (const char *)string->content;
		const char *c;

		for (c = start + i; c != start; c--) {
			if (strchr(control, *c)) return i;
			i--;
		}

		if (strchr(control, *c)) return 0;
	} else if (string->encoding == T_ENCODING_UTF8) {
		unsigned char const *start = string->content;
		unsigned char const *c;
		TSize size = 0, previousSize = 0;
		TString *controlStr = TStringFromString(control);

		c = start + string->size;
		while (c != start) {
			TUInt32 ch = TEncodingUTF8GetPreviousChr(c, &size);
			if (TStringChr(controlStr, ch)) {
				TStringFree(controlStr);
				return i;
			}
			i -= previousSize - size;
			previousSize = size;
		}

		TStringFree(controlStr);
	}

	return i + 1;
}

const unsigned char *TStringChr(TString *string, TUInt32 character) {
	if (!string) {
		TErrorZero(T_ERROR_INVALID_INPUT);
	}

	if (string->encoding == T_ENCODING_ASCII) {
		//check for utf-8 character
		if (character <= 0x7F) {
			//there might be a match
			return (const unsigned char *)strchr((const char *)string->content, character);
		}
	} else if (string->encoding == T_ENCODING_UTF8) {
		TSize size = string->size;
		TUInt32 ch;
		unsigned char *ptr = string->content;
		unsigned char *previous = ptr;

		do {
			ch = TEncodingUTF8GetChr(ptr, &size);
			if (ch == character) return previous;
			previous = ptr;
		} while (ch);
	}

	return 0;
}

TString *TStringLowerCase(TString *string) {
	TString *s;

	if (!string) return 0;

	s = TAllocData(TString);
	if (s) {
		TSize size;
		unsigned char *ptr, *sptr;
		s->encoding = string->encoding;
		size = s->size = string->size;

		s->content = TAlloc(sizeof(unsigned char) * size);

		sptr = string->content;
		ptr = s->content;

		while (*sptr) {
			TUInt32 ch = TEncodingUTF8GetChr(sptr, &size);
			ch = tolower(ch);
			memcpy(ptr, &ch, sizeof(TUInt32));
			ptr += sizeof(TUInt32);
		}
		*ptr = 0;
	}

	return s;
}

char *TStringCopy(const char *text) {
	char *cpy = 0;

	if (text) {
		int size = sizeof(char) * (strlen(text) + 1);
		cpy = (char *)TAlloc(size);
		memcpy(cpy, text, size);
	}

	return cpy;
}

char *TStringNCopyO(const char *text, int num) {
	char *cpy = 0;

	if (text) {
		int size = sizeof(char) * (num + 1);
		cpy = (char *)TAlloc(size);
		memcpy(cpy, text, size - 1);
		cpy[num] = 0;
	}

	return cpy;
}

int TStringAdjustSize(char **text, TSize oldsize, TSize newsize) {
	TPtr nptr = TRAlloc(text, newsize);
	if (!nptr) return 1;
	text = (char **)nptr;

	if (newsize > oldsize) memset(text + oldsize, 0, newsize);

	return 0;
}

int TStringCaseInsensitiveCompareO(const char *str1, const char *str2) {
	char *lstr1 = TStringLowerCaseO(str1);
	char *lstr2 = TStringLowerCaseO(str2);

	int res = strcmp(lstr1, lstr2);

	TFree(lstr1);
	TFree(lstr2);

	return res;
}

TSize TStringRCSpnO(const char *_str, const char *_control) {
	TSize len = 0;

	//sanity check
	if (!_str) return 0;

	len = strlen(_str);

	if (!_control) return len + 1;

	{
		TSize i = len;
		const char *c = 0;

		for (c = _str + len; c != _str; c--) {
			if (strchr(_control, *c)) return i;
			i--;
		}

		if (strchr(_control, *c)) return 0;
	}

	return len + 1;
}

inline void TStringReplaceOp(char *target, const char *match, const char *replacement, TSize repllen, TSize limit)
{
	char *curptr = target;
	TSize i = 0;

	do {
		curptr = strstr(curptr, match);
		if (curptr) {
			strncpy(curptr, replacement, repllen);
			curptr += repllen;
			i++;
		}
	} while (curptr && (!limit || i < limit));
}

inline void TStringCopyReplaceOp(char *target, const char *source, const char *match, const char *replacement, TSize limit)
{
	const char *srcptr = source;
	char *tarptr = target;
	TSize srclen = strlen(source);
	TSize matchlen = strlen(match);
	TSize repllen = strlen(replacement);
	TSize i = 0;

	while (srcptr && (!limit || i < limit)) {
		*tarptr = *srcptr;
		if (*srcptr == match[0] && i + matchlen <= srclen) {
			if (strncmp(srcptr, match, matchlen) == 0) {
				strncpy(tarptr, replacement, repllen);
				srcptr += matchlen - 1;
				tarptr += repllen - 1;
				i++;
			}
		}
		srcptr = *(srcptr + 1) == '\0' ? 0 : srcptr + 1;
		tarptr += 1;
	}

	if (srcptr) strcpy(tarptr, srcptr);
	else *tarptr = '\0';
}

TSize TStringNumOccurences(const char *target, const char *match)
{
	const char *curptr = target;
	TSize matchlen = strlen(match);
	TSize counter = 0;

	do {
		curptr = strstr(curptr, match);
		if(curptr) {
			counter +=1;
			curptr += matchlen;
		}
	} while(curptr);

	return counter;
}

/*
 * Replace a substring of a string with another string
 */
char *TStringReplace(const char *source, const char *match, const char *replacement, TSize limit, TSize hint_numoccurence)
{
	char *result = 0;
	TSize srclen = 0;
	TSize matchlen = 0;
	TSize repllen = 0;
	TSize reslen = 0;

	if(source == 0 || match == 0 || replacement == 0 || source[0] == '\0' || match[0] == '\0' || replacement[0] == '\0')
		return 0;
	
	srclen = strlen(source);
	matchlen = strlen(match);
	repllen = strlen(replacement);
	reslen = srclen + 1;

	if(matchlen != repllen) {
		if (limit)
			reslen -= ((int)matchlen - (int)repllen)*((int)limit);
		else {
			if(!hint_numoccurence) {
				//find number of occurences
				hint_numoccurence = TStringNumOccurences(source, match);
			}

			reslen -= ((int)matchlen - (int)repllen)*((int)hint_numoccurence);
		}
	}

	result = (char *) TAlloc(reslen);
	if (result) TStringCopyReplaceOp(result, source, match, replacement, limit);
	return result;
}

TUInt8 TStringReplaceInplace(char *source, const char *match, const char *replacement, TSize limit) {
	TSize repllen = 0;

	if(source == 0 || match == 0 || replacement == 0)
		return 1;

	repllen = strlen(replacement);

	if(strlen(match) != repllen)
		return 2;

	TStringReplaceOp(source, match, replacement, repllen, limit);

	return 0;
}

void TStringSafetyString(char *string)
{
	int strlength = strlen(string);
	TStringReplaceOp(string, "%", " ", strlength, 0);
}

char **TStringSplit(const char *string, const char *substr, TSize *size, TSize limit)
{
	char **sto;
	char *str;
	char *ptr;
	TSize len;
	TSize i;
	if(!size || !string || !substr || !*substr) return 0;

	str = strdup(string);
	len = strlen(substr);
	ptr = str;
	*size = 1;
	i = 0;

	while ((ptr = strstr(ptr,substr)) && (!limit || (i++ < limit))) {
		*ptr = 0;

		ptr+=len;
		*size += 1;
	}

	sto = (char **)TAlloc(sizeof(char *) * (*size));
	if(sto) {
		TSize i = 1;
		sto[0] = str;

		while (i < *size) {
			ptr = strchr(sto[i-1],0)+len;
			sto[i++] = ptr;
		}
	} else free(str);
	return sto;
}

char **TStringRSplit(const char *string, const char *substr, TSize *size, TSize limit)
{
	char **sto;
	char *str;
	char *ptr;
	TSize len;
	TSize i;
	if(!size || !string || !substr || !*substr) return 0;

	str = TStringCopy(string);
	len = strlen(substr);
	*size = 1;
	i = 0;
	ptr = strrchr(str,substr[0]);

	while (ptr && (!limit || i < limit)) {
		if(!strncmp(ptr,substr,len)) {
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
	if(sto) {
		TSize i = 1;
		sto[0] = str;

		while (i < *size) {
			ptr = strchr(sto[i - 1], 0) + len;
			sto[i++] = ptr;
		}
	} else TFree(str);
	return sto;
}

char *TStringAddCharacter(const char *string, char character, TSize start, TSize end)
{
	TSize len;
	int nextindex;
	char *newstring = 0;

	if (start > end)
		TSWAP(start, end);

	len = strlen(string) + (character ? 1 : 0) + 1;
	nextindex = character ? start + 1 : start;

	if (start != end) len -= end - start;

	newstring = (char *)TAlloc(sizeof(char) * len);
	if(newstring) {
		if (start != 0) strncpy(newstring, string, start);
		newstring[start] = character;
		strncpy(newstring + nextindex, string + end, len - start);
	}
	
	return newstring;
}

char *TStringAppendCharacter(const char *string, char character) {
	TSize size;
	char *result;

	if (!string) { TErrorReportDefault(T_ERROR_NULL_POINTER); return 0; }

	size = strlen(string) + 2;
	result = (char *)TAlloc(sizeof(char) * size);
	memcpy(result, string, size - 2);
	result[size - 2] = character;
	result[size - 1] = 0;

	return result;
}

char *TStringConcat(const char *str, ...)
{
	va_list components;
	const char *component;
	char *buffer;
	unsigned int bufferlen = 0;
	
	buffer = TStringCopy(str);
	bufferlen = strlen(buffer);
	
	va_start(components, str);
	while((component = va_arg(components, const char *)))
	{
		TSize llen = strlen(component);
		buffer = TRAlloc(buffer, bufferlen + llen + 1);
		snprintf(buffer + bufferlen, llen + 1, "%s", component);
		bufferlen += llen;
	}
	va_end(components);
	
	return buffer;
}

char *TStringConcatSeparator(const char *separator, const char *str, ...)
{
	va_list components;
	const char *component;
	char *buffer;
	unsigned int bufferlen = 0;
	unsigned int seplen = strlen(separator);

	buffer = TStringCopy(str);
	bufferlen = strlen(buffer);

	va_start(components, str);
	while ((component = va_arg(components, const char *)))
	{
		TSize llen = strlen(component) + seplen;
		buffer = TRAlloc(buffer, bufferlen + llen + 1);
		snprintf(buffer + bufferlen, llen + 1, "%s%s", separator, component);
		bufferlen += llen;
	}
	va_end(components);

	return buffer;
}

void TStringStripTrailingWhitespace(char *string)
{
	int i = strlen(string)-1;
	for(; i >= 0; --i)
		if(string[i] == ' ' || string[i] == '\n' || string[i] == '\r')
			string[i] = '\x0';
		else
			break;
}

const char *TStringStripLeadingSpaces(const char *string)
{
	while(*string == ' ') string++;
	return string;
}

char *TStringLowerCaseO(const char *thestring)
{
	const char *srcptr = thestring;
	char *thecopy = 0, *ptr = 0;

	if(!thestring) return 0;

	thecopy = (char *)TAlloc(strlen(thestring) + 1);

	if(thecopy) {
		ptr = thecopy;

		while(*srcptr) {
			*ptr = tolower(*srcptr);
			srcptr += 1;
			ptr +=1;
		}
		*ptr = '\0';
	}

	return thecopy;
}

char *TStringPasswordEncrypt(const char *src)
{
	TSize len = 0;
	char *result = 0;

	if(!src) return 0;

	len = strlen(src);
	if(!len) return 0;

	result = (char *)TAlloc(sizeof(char) *(len + 1));
	if(result) {
		memset(result, '*', len);
		result[len] = 0;
	}

	return result;
}

char *TStringDoubleChars(const char *string, const char escchar)
{
	char *escstring = 0;

	if(string) {
		const char *esc = 0;
		TSize size = strlen(string) * 2;
		char *newstring = (char *)TAlloc(sizeof(char)*size), *cur = 0;
		if(!newstring) return 0;
		cur = newstring;

		do {
			esc = strchr(string, escchar);
			if(esc) {
				TSize escsize = esc - string + 1;
				strncpy(cur, string, escsize); cur += escsize;
				snprintf(cur, 1, "%c", escchar); cur += 1;

				string = esc + 1;
			} else {
				strncpy(cur, string, size);
			}
		} while(esc);

		escstring = TStringCopy(newstring);
		TFree(newstring);
	}

	return escstring;
}

char *TStringRemoveDuplication(const char *string, const char escchar)
{
	char *escstring = 0;

	if(string) {
		const char *esc = 0;
		TSize size = strlen(string) + 1;
		char doubled[] = { escchar, escchar, 0 };
		char *newstring = (char *)TAlloc(sizeof(char)*size), *cur = 0;
		if(!newstring) return 0;
		cur = newstring;

		do {
			esc = strstr(string, doubled);
			if(esc) {
				TSize escsize = esc - string + 1;
				strncpy(cur, string, escsize); cur += escsize;

				string = esc + 2;
			} else {
				strncpy(cur, string, size);
			}
		} while(esc);

		escstring = TStringCopy(newstring);
		TFree(newstring);
	}

	return escstring;
}

TInt8 stoi8(const char *str) {
	TSize len = strlen(str);
	if (len <= 4) {
		TInt8 convert = 0;
		TInt8 offset = 1;
		TUInt8 negative = str[0] == '-';
		TUInt8 i;

		if (negative) { str = str + 1; len--; }

		for (i = (TUInt8)len - 1; i >= 0; i--) {
			char c = str[i];
			if (c < '0' || c > '9') return 0;
			convert += (c - '0') * offset;
			offset *= 10;
		}

		if (negative) convert = -convert;
		return convert;
	}

	return 0;
};

TUInt8 stoui8(const char *str) {
	TSize len = strlen(str);
	if (len < 4) {
		TInt8 convert = 0;
		TInt8 offset = 1;
		TUInt8 i;

		for (i = (TUInt8)len - 1; i >= 0; i--) {
			if (str[i] < '0' || str[i] > '9') return 0;
			convert += (str[i] - '0') * offset;
			offset *= 10;
		}

		return convert;
	}

	return 0;
};

TInt16 stoi16(const char *str) {
	TSize len = strlen(str);
	if (len <= 6) {
		TInt16 convert = 0;
		TInt16 offset = 1;
		TUInt8 negative = str[0] == '-';
		TUInt8 i;

		if (negative) { str = str + 1; len--; }

		for (i = (TUInt8)len - 1; i >= 0; i--) {
			if (str[i] < '0' || str[i] > '9') return 0;
			convert += (str[i] - '0') * offset;
			offset *= 10;
		}

		if (negative) convert = -convert;
		return convert;
	}

	return 0;
};

TUInt16 stoui16(const char *str) {
	TSize len = strlen(str);
	if (len < 6) {
		TUInt16 convert = 0;
		TUInt16 offset = 1;
		TUInt8 i;

		for (i = (TUInt8)len - 1; i >= 0; i--) {
			if (str[i] < '0' || str[i] > '9') return 0;
			convert += (str[i] - '0') * offset;
			offset *= 10;
		}

		return convert;
	}

	return 0;
};

TUInt32 stoui32(const char *str) {
	TSize len = strlen(str);
	if (len < 11) {
		TUInt32 convert = 0;
		TUInt32 offset = 1;
		TUInt8 i;

		for (i = (TUInt8)len - 1; i >= 0; i--) {
			if (str[i] < '0' || str[i] > '9') return 0;
			convert += (str[i] - '0') * offset;
			offset *= 10;
		}

		return convert;
	}

	return 0;
};

#ifdef PLATFORM_X86_64
TInt64 stoi64(const char *str) {
	TSize len = strlen(str);
	if (len <= 20) {
		TInt64 convert = 0;
		TInt64 offset = 1;
		TUInt8 negative = str[0] == '-';
		TUInt8 i;

		if (negative) { str = str + 1; len--; }

		for (i = (TUInt8)len - 1; i >= 0; i--) {
			if (str[i] < '0' || str[i] > '9') return 0;
			convert += (str[i] - '0') * offset;
			offset *= 10;
		}

		if (negative) convert = -convert;
		return convert;
	}

	return 0;
};

TUInt64 stoui64(const char *str) {
	TSize len = strlen(str);
	if (len < 21) {
		TUInt64 convert = 0;
		TUInt64 offset = 1;
		TUInt8 i;

		for (i = (TUInt8)len - 1; i >= 0; i--) {
			if (str[i] < '0' || str[i] > '9') return 0;
			convert += (str[i] - '0') * offset;
			offset *= 10;
		}

		return convert;
	}

	return 0;
};
#endif

