#include "stdafx.h"

#include "debugging/tdebug.h"
#include "utility/tstring.h"

#include "string_test.h"

void string_test_split(void)
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
}


void string_test(void)
{
	TLogReport(T_LOG_PROGRESS,0,"Testing string utilities...\n");

	string_test_split();

	string_test_replace();

	TLogReport(T_LOG_PROGRESS,0,"string utilities tests completed.\n");
}
