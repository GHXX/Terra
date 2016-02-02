#include "stdafx.h"

#include "utility/tfilesys.h"

#include "test.h"

#include "test_utils.h"

void filesys_concat_path(void)
{
	const char *test[] = {"/test/","..","..","bleh","..","blah",".","blih"};
	size_t size = 8;

	char *result = TFileSysConcatPathsArr(test,size);

	free(result);
}

void filesys_test(void)
{
	TLogReport(T_LOG_PROGRESS,0,"Testing file system utilities...\n");

	filesys_concat_path();

	TLogReport(T_LOG_PROGRESS,0,"file system tests completed.\n");
}
