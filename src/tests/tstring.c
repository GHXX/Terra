#include "stdafx.h"

#include "test_utils.h"

#include "utility/tstring.h"

#include "test.h"

/*void string_test_split(void)
{
	const char *path = "test/bleh/blah/";
	size_t size;
	char **split = TStringSplit(path,"/",&size,0);

	TAssert(split);
	TAssert(size == 4);
	TAssert(!strcmp(split[size-1], ""));

	free(split[0]);
	free(split);

	split = TStringRSplit(path,"/",&size,2);

	printf("%s\n",split[0]);
	printf("%s\n",split[1]);
	printf("%s\n",split[2]);

	free(split[0]);
	free(split);
}

void string_test_replace(void)
{
	const char *fstring = "This is such a useless string\n";

	char *result = TStringReplace(fstring,"i","01",2,3);
	printf("%s", result);
	free(result);

	result = TStringReplace(fstring,"i","01",0,3);
	printf("%s", result);
	free(result);

	result = strdup(fstring);
	TStringReplaceInplace(result,"i","1",2);
	printf("%s", result);
	free(result);

	result = strdup(fstring);
	TStringReplaceInplace(result,"i","1",0);
	printf("%s", result);
	free(result);
}*/

int TStringTestCreationFromASCIIString(void) {
	TString *testString = TStringFromString("myTest string");

	TTestMustValidate(testString);

	TStringFree(testString);

	return 0;
}

int TStringTestCreationFromBytes(void) {
	const unsigned char *data = "\xC5\xB6 \xE0\xB8\x9C \xF0\xA8\x9B\xB9";
	TString *testString = TStringFromBytes(data, strlen(data));

	TTestMustValidate(testString);

	TStringFree(testString);

	return 0;
}

int TStringTestNumCharacters(void) {
	const unsigned char *data = "\xC5\xB6 \xE0\xB8\x9C \xF0\xA8\x9B\xB9";
	TString *testString = TStringFromString(data), *copy;

	TTestMustValidate(testString);
	TTestValidate(TStringNumCharacters(testString) == 5);

	copy = TStringFromTString(testString);
	
	TTestMustValidate(copy);
	TTestValidate(TStringNumCharacters(testString) == 5);

	TStringFree(copy);
	TStringFree(testString);

	testString = TStringFromBytes(data, strlen(data));

	TTestMustValidate(testString);
	TTestValidate(TStringNumCharacters(testString) == 5);

	copy = TStringNCopy(testString, 3);

	TTestMustValidate(copy);
	TTestValidate(TStringNumCharacters(copy) == 3);

	TStringFree(copy);
	TStringFree(testString);

	return 0;
}

int TStringTestIndexAccess(void) {
	const unsigned char *data = "\xC5\xB6 \xE0\xB8\x9C \xF0\xA8\x9B\xB9";
	TString *testString = TStringFromString(data), *copy;
	char c;

	TTestMustValidate(testString);
	c = *TStringToIndex(testString, 2);
	TTestValidate(c == '\xE0');

	copy = TStringFromTString(testString);

	TTestMustValidate(copy);
	c = *TStringToIndex(copy, 4);
	TTestValidate(c == '\xF0');
	TTestValidate(TStringToIndex(testString, 6) == 0);
	c = *TStringToIndex(testString, 0);
	TTestValidate(c == '\xC5');

	TStringFree(copy);
	TStringFree(testString);

	return 0;
}

void TStringTest(void)
{
	TestFunc tests[] = {
		TStringTestCreationFromASCIIString,
		TStringTestCreationFromBytes,

		TStringTestNumCharacters,
		TStringTestIndexAccess,

		/*TString *TStringFromTString(const TString *string);
		TString *TStringNCopy(const TString *string, TSize num);
		void TStringFree(TString *string);

	#ifdef _WINDOWS
		const wchar_t *TStringToWideChar(TString *string);
	#endif
		const char *TStringToASCII(TString *string);
		const unsigned char *TStringToUTF8(TString *string);

		int TStringCaseInsensitiveCompare(TString *str1, TString *str2);

		TSize TStringRCSpn(TString *string, const char *control);

		const unsigned char *TStringChr(TString *string, TUInt32 character);
		TString *TStringLowerCase(TString *string);

		typedef struct TStringIterator TStringIterator;

		TStringIterator *TStringIteratorNew(TString *string);
		void TStringIteratorFree(TStringIterator *context);

		unsigned char *TStringIteratorData(TStringIterator *context);

		unsigned char *TStringIteratorNext(TStringIterator *context);
		unsigned char *TStringIteratorPrevious(TStringIterator *context);
		unsigned char *TStringIteratorIncrement(TStringIterator *context, TLInt numCharacters);*/

	};

	TTestRun("String", tests, sizeof(tests) / sizeof(TestFunc));
}
