
#ifndef __included_terra_string_h
#define __included_terra_string_h

#include "tdefine.h"
#include <ctype.h>

static inline char *TStringLwr(char *s) {
	char *p = s;
	for (; *p; p++)
		*p = tolower(*p);
	return s;
}

static inline char *TStringUpr(char *s) {
	char *p = s;
	for (; *p; p++)
		*p = toupper(*p);
	return s;
}

char *TStringCopy(const char *text);

int TStringAdjustSize(char **text,size_t oldsize,size_t newsize);

size_t TStringRCSpn(const char *_str,const char *_control);

size_t TStringNumOccurences(const char *target,const char *match);

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
char *TStringReplace(const char *source, const char *match, const char *replacement, size_t limit, size_t hint_numoccurence);

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
unsigned char TStringReplaceInplace(char *source, const char *match, const char *replacement, size_t limit);

void TStringSafetyString(char *string);

char **TStringSplit(const char *string, const char *substr, size_t *size, size_t limit);
char **TStringRSplit(const char *string, const char *substr, size_t *size, size_t limit);
char *TStringAddCharacter(const char *string, char character, size_t start, size_t end);

char *TStringConcat(const char *str, ...);

void TStringStripTrailingWhitespace(char *string);	// Removes trailing /n, /r, space
const char *TStringStripLeadingSpaces(const char *string);

char *TStringLowerCase(const char *thestring);

char *TStringPasswordEncrypt(const char *src);

char *TStringDoubleChars(const char *string, const char escchar);
char *TStringRemoveDuplication(const char *string, const char escchar);

#endif
