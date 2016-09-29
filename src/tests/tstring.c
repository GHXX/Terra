#include "stdafx.h"

#include "ttest.h"

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

int TStringTestInsert(void) {
	char *res;
	const char *testStr = "test my string";
	const char *insStr = "super ";
	int position = 8;


	res = TStringInsert(insStr, testStr, position);

	TTestMustValidate(res);
	TTestMustValidate(!strcmp(res,"test my super string"));

	return 0;
}


void TStringTest(void)
{
	TestFunc tests[] = {
		TStringTestInsert

	};

	TTestRun("String", tests, sizeof(tests) / sizeof(TestFunc));
}
