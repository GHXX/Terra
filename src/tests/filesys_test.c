#include "stdafx.h"

#include "utility/tfilesys.h"

#include "test.h"

#include "ttest.h"

void filesys_concat_path(void)
{
	const char *test[] = {"/test/","..","..","bleh","..","blah",".","blih"};
	size_t size = 8;

	char *result = TFileSysConcat(*test, *(test + 1), *(test + 2), *(test + 3), *(test + 4), *(test + 5), *(test + 6), *(test + 7), 0);

	TFree(result);
}

void filesys_test(void)
{
	TLogWrite(testLog, "Testing file system utilities...\n");

	filesys_concat_path();

	TLogWrite(testLog, "file system tests completed.\n");
}
