#include "stdafx.h"

#include "ttest.h"

TLog *testLog = 0;

void test_create_file(const char *filename, const char *data) {
	FILE *f = fopen(filename, "w");

	if (!f) TAbort("can't open test file for writing");

	fwrite(data, strlen(data), 1, f);

	fclose(f);
}

void TTestRun(const char *testCategory, TestFunc *tests, TSize numTests) {
	TSize i = 0;
	TSize numSuccess = 0;

	printf("Testing %s...\n", testCategory);


	for (; i < numTests; ++i) {
		int res = tests[i]();
		if (!res) {
			numSuccess++;
			printf("  Test %d: Passed.\n", i + 1);
		} else if (res == 1) {
			printf("  Test %d: Failed.\n", i + 1);
		} else {
			printf("  Test %d: Failed.\nAborting %s tests (%d/%d passed).\n", i + 1, testCategory, numSuccess, numTests);
			return;
		}
		TErrorSet(T_ERROR_NONE);
	}


	printf("%s tests completed (%d/%d passed).\n", testCategory, numSuccess, numTests);
}
