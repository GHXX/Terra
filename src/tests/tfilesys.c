#include "stdafx.h"

#include "utility/tfilesys.h"
#include "talloc.h"

#include "test.h"

#include "ttest.h"

int TFileSysTestConcatPath(void) {
	const char *test[] = {"/test/","..","..","bleh","..","blah",".","blih"};
	TSize size = 8;

	char *result = TFileSysConcat(*test, *(test + 1), *(test + 2), *(test + 3), *(test + 4), *(test + 5), *(test + 6), *(test + 7), 0);
	if (strcmp(result, "/blah/blih")) {
		TFree(result);
		TTestValidate(0);
	}

	TFree(result);

	return 0;
}

int TFileSysTestReadOnly(void) {
	FILE *f;
	
	f = TFileSysOpen("test_file", "w");
	TTestMustValidate(f);

	if (TFileSysIsReadOnly(f)) {
		TFileSysClose(f);
		TFileSysDelete("test_file");
		TTestValidate(0);
	}
	TFileSysClose(f);

	f = TFileSysOpen("test_file", "r");
	if (!TFileSysIsReadOnly(f)) {
		TFileSysClose(f);
		TFileSysDelete("test_file");
		TTestValidate(0);
	}
	TFileSysClose(f);

	f = TFileSysOpen("test_file", "rb");
	if (!TFileSysIsReadOnly(f)) {
		TFileSysClose(f);
		TFileSysDelete("test_file");
		TTestValidate(0);
	}
	TFileSysClose(f);
	TFileSysDelete("test_file");

	return 0;
}

void TFileSysTest(void) {
	TestFunc tests[] = {
		TFileSysTestConcatPath,
		TFileSysTestReadOnly
	};

	TTestRun("File System Utilities", tests, sizeof(tests) / sizeof(TestFunc));
}
