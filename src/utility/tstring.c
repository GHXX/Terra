
#include "stdafx.h"

#include "tstring.h"

#include "ttime.h"

#include "talloc.h"
#include "terror.h"

int TStringCaseInsensitiveCompare(const char *str1, const char *str2)
{
	char *lstr1 = TStringLowerCase(str1);
	char *lstr2 = TStringLowerCase(str2);

	int res = strcmp(lstr1, lstr2);

	TFree(lstr1);
	TFree(lstr2);

	return res;
}

char *TStringCopy(const char *text)
{
	char *cpy = 0;

	if(text) {
		int size = sizeof(char) * (strlen(text) + 1);
		cpy = (char *) TAlloc(size);
		memcpy(cpy,text,size);
	}

	return cpy;
}

char *TStringNCopy(const char *text, int num)
{
	char *cpy = 0;

	if (text) {
		int size = sizeof(char) * (num + 1);
		cpy = (char *)TAlloc(size);
		memcpy(cpy, text, size-1);
		cpy[num] = 0;
	}

	return cpy;
}

int TStringAdjustSize(char **text,size_t oldsize,size_t newsize)
{
	void *nptr = TRAlloc(text, newsize);
	if(!nptr) return 1;
	text = (char **) nptr;

	if(newsize > oldsize) memset(text+oldsize,'\0',newsize);

	return 0;
}

size_t TStringRCSpn(const char *_str,const char *_control)
{
	size_t len = 0;
	
	//sanity check
	if(!_str) return 0;

	len = strlen(_str);

	if(!_control) return len+1;

	{
		size_t i = len;
		const char *c = 0;

		for(c = _str+len; c != _str; c--) {
			if(strchr(_control,*c)) return i;
			i--;
		}

		if(strchr(_control,*c)) return 0;
	}

	return len+1;
}

inline void TStringReplaceOp(char *target,const char *match, const char *replacement,size_t repllen, size_t limit)
{
	char *curptr = target;
	size_t i = 0;

	do {
		curptr = strstr(curptr,match);
		if(curptr) {
			strncpy(curptr,replacement,repllen);
			curptr += repllen;
			i++;
		}
	} while(curptr && (!limit || i < limit));
}

inline void TStringCopyReplaceOp(char *target,const char *source,const char *match, const char *replacement, size_t limit)
{
	const char *srcptr = source;
	char *tarptr = target;
	size_t srclen = strlen(source);
	size_t matchlen = strlen(match);
	size_t repllen = strlen(replacement);
	size_t i = 0;

	while(srcptr && (!limit || i < limit)) {
		*tarptr = *srcptr;
		if(*srcptr == match[0] && i + matchlen <= srclen) {
			if(strncmp(srcptr,match,matchlen) == 0) {
				strncpy(tarptr,replacement,repllen);
				srcptr += matchlen-1;
				tarptr += repllen-1;
				i++;
			}
		}
		srcptr = *(srcptr+1) == '\0' ? 0 : srcptr+1;
		tarptr +=1;
	}

	if(srcptr) strcpy(tarptr,srcptr);
	else *tarptr = '\0';
}

size_t TStringNumOccurences(const char *target,const char *match)
{
	const char *curptr = target;
	size_t matchlen = strlen(match);
	size_t counter = 0;

	do {
		curptr = strstr(curptr,match);
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
char *TStringReplace(const char *source, const char *match, const char *replacement, size_t limit, size_t hint_numoccurence)
{
	char *result = 0;
	size_t srclen = 0;
	size_t matchlen = 0;
	size_t repllen = 0;
	size_t reslen = 0;

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
				hint_numoccurence = TStringNumOccurences(source,match);
			}

			reslen -= ((int)matchlen - (int)repllen)*((int)hint_numoccurence);
		}
	}

	result = (char *) TAlloc(reslen);
	if(result) TStringCopyReplaceOp(result,source,match,replacement,limit);
	return result;
}

TUInt8 TStringReplaceInplace(char *source, const char *match, const char *replacement, size_t limit) {
	size_t repllen = 0;

	if(source == 0 || match == 0 || replacement == 0)
		return 1;

	repllen = strlen(replacement);

	if(strlen(match) != repllen)
		return 2;

	TStringReplaceOp(source,match,replacement,repllen,limit);

	return 0;
}

void TStringSafetyString(char *string)
{
	int strlength = strlen(string);
	TStringReplaceOp(string,"%"," ",strlength,0);
}

char **TStringSplit(const char *string, const char *substr, size_t *size, size_t limit)
{
	char **sto;
	char *str;
	char *ptr;
	size_t len;
	size_t i;
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
		size_t i = 1;
		sto[0] = str;

		while (i < *size) {
			ptr = strchr(sto[i-1],0)+len;
			sto[i++] = ptr;
		}
	} else free(str);
	return sto;
}

char **TStringRSplit(const char *string, const char *substr, size_t *size, size_t limit)
{
	char **sto;
	char *str;
	char *ptr;
	size_t len;
	size_t i;
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
			ptr = strrchr(str,substr[0]);
		} else {
			char *tmp = ptr, v = *ptr;
			*tmp = 0;
			ptr = strrchr(str,substr[0]);
			*tmp = v;
		}
	}

	sto = (char **)TAlloc(sizeof(char *) * (*size));
	if(sto) {
		size_t i = 1;
		sto[0] = str;

		while (i < *size) {
			ptr = strchr(sto[i-1],0)+len;
			sto[i++] = ptr;
		}
	} else TFree(str);
	return sto;
}

char *TStringAddCharacter(const char *string, char character, size_t start, size_t end)
{
	size_t len = strlen(string) + (character ? 1 : 0) + 1;
	int nextindex = character ? start+1 : start;
	char *newstring = 0;

	if(start != end) len -= end-start;

	newstring = (char *) TAlloc(sizeof(char) * len);
	if(newstring) {
		if(start != 0) strncpy(newstring,string,start);
		newstring[start] = character;
		strncpy(newstring+nextindex,string+end,len-start);
	}
	
	return newstring;
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
		size_t llen = strlen(component);
		buffer = TRAlloc(buffer, bufferlen + llen + 1);
		snprintf(buffer + bufferlen,llen + 1,"%s",component);
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
		size_t llen = strlen(component) + seplen;
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

char *TStringLowerCase(const char *thestring)
{
	const char *srcptr = thestring;
	char *thecopy = 0, *ptr = 0;

	if(!thestring) return 0;

	thecopy = (char *) TAlloc(strlen(thestring)+ 1);

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
	size_t len = 0;
	char *result = 0;

	if(!src) return 0;

	len = strlen(src);
	if(!len) return 0;

	result = (char *) TAlloc(sizeof(char) *(len+1));
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
		size_t size = strlen(string)*2;
		char *newstring = (char *) TAlloc(sizeof(char)*size), *cur = 0;
		if(!newstring) return 0;
		cur = newstring;

		do {
			esc = strchr(string, escchar);
			if(esc) {
				size_t escsize = esc-string +1;
				strncpy(cur,string,escsize); cur += escsize;
				snprintf(cur,1,"%c",escchar); cur += 1;

				string = esc+1;
			} else {
				strncpy(cur,string,size);
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
		size_t size = strlen(string)+1;
		char doubled[] = {escchar,escchar, 0};
		char *newstring = (char *) TAlloc(sizeof(char)*size), *cur = 0;
		if(!newstring) return 0;
		cur = newstring;

		do {
			esc = strstr(string, doubled);
			if(esc) {
				size_t escsize = esc-string +1;
				strncpy(cur,string,escsize); cur += escsize;

				string = esc+2;
			} else {
				strncpy(cur,string,size);
			}
		} while(esc);

		escstring = TStringCopy(newstring);
		TFree(newstring);
	}

	return escstring;
}

TInt8 stoi8(const char *str) {
	size_t len = strlen(str);
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
	size_t len = strlen(str);
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
	size_t len = strlen(str);
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
	size_t len = strlen(str);
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
	size_t len = strlen(str);
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

#ifdef _X86_64
TInt64 stoi64(const char *str) {
	size_t len = strlen(str);
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
	size_t len = strlen(str);
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

