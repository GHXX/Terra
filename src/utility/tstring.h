
#ifndef __included_terra_string_h
#define __included_terra_string_h

#include "tdefine.h"
#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TStringSize(s) (strlen(s) + 1) * sizeof(char)

static inline char TStringIsAlphabetCharacter(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static inline char TStringIsDigit(char c) {
	return (c >= '0' && c <= '9');
}

char *TStringLwr(const char *s);

char *TStringUpr(const char *s);

char *TStringCopy(const char *text);
char *TStringNCopy(const char *text, int num);
char *TStringSCopy(const char *text, TSize size);

int TStringAdjustSize(char **text, TSize oldsize, TSize newsize);

#ifdef _WINDOWS
char *TStringFromWideChar(wchar_t *string);
wchar_t *TStringToWideChar(const char *string);
#endif

TSize TStringLen(const char *string);

TUInt32 TStringGetCharacter(const char *string, int position);
char *TStringGetCharacterStr(const char *string, int position);

int TStringIsCharEscaped(const char *start, const char *pos);

int TStringCaseInsensitiveCompare(const char *str1, const char *str2);

TSize TStringRCSpn(const char *_str, const char *_control);

TSize TStringNumOccurences(const char *target, const char *match);

/**
 * Replace a substring of a string with another string
 *
 * @param source              The string to modify.
 * @param match               The substring to be matched.
 * @param replacement         The string replacing the match.
 * @param limit               The maximum of replacement allowed, 0 for none
 * @param hint_numoccurence   The number of occurence of the match in the string.
 *                            This value can be 0 and is useless if the match length
 *                            equals the replacement length.
 * @return                    A new string with the matching strings replaced.
 *
 * @sa TStringReplaceInplace
 */
char *TStringReplace(const char *source, const char *match, const char *replacement, TSize limit, TSize hint_numoccurence);

/**
 * Replace a substring of a string with another string in place.
 * The match and replacement must be of the same size.
 *
 * @param source              The string to modify.
 * @param match               The substring to be matched.
 * @param replacement         The string replacing the matches.
 * @param limit               The maximum of replacement allowed, 0 for none
 * @return                    0 on success,
 *                            1 if a parameter is invalid,
 *                            2 for a length mismatch.
 *
 * @sa TStringReplace
 */
unsigned char TStringReplaceInplace(char *source, const char *match, const char *replacement, TSize limit);

void TStringSafetyString(char *string);

char **TStringSplit(const char *string, const char *substr, TSize *size, TSize limit);
char **TStringRSplit(const char *string, const char *substr, TSize *size, TSize limit);

char *TStringAddCharacter(const char *string, char character, TSize start, TSize end);
char *TStringAppendCharacter(const char *string, char character);

char *TStringInsert(const char *src, const char *target, int position);

char *TStringConcat(const char *str, ...);
char *TStringConcatInplace(char *str, ...);
char *TStringConcatSeparator(const char *separator, const char *str, ...);

char *TStringStripTrailingWhitespace(const char *string);	// Removes trailing /n, /r, space
const char *TStringStripLeadingSpaces(const char *string);

char *TStringPasswordEncrypt(const char *src);

char *TStringDoubleChars(const char *string, const char escchar);
char *TStringRemoveDuplication(const char *string, const char escchar);


TInt8 stoi8(const char *str);
TUInt8 stoui8(const char *str);

TInt16 stoi16(const char *str);
TUInt16 stoui16(const char *str);

TUInt32 stoui32(const char *str);

TInt64 stoi64(const char *str);
TUInt64 stoui64(const char *str);

#ifdef __cplusplus
}
#endif

#endif
