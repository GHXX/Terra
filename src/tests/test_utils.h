
#ifndef __terra_test_test_utils_h
#define __terra_test_test_utils_h

#include "terra.h"

typedef int(*TestFunc)(void);

void test_create_file(const char *filename, const char *data);

void TTestRun(const char *testCategory, TestFunc *tests, TSize numTests);

static inline void testReport(unsigned char v, const char *testmsg) {
	TLogWriteMain(testmsg);
	TLogWriteMain(v ? "Success\n" : "Failed\n");
}

static inline unsigned char testEqual(void *v1,void *v2, const char *testmsg) {
	testReport(v1 == v2, testmsg);

	return v1 == v2;
}

static inline unsigned char testNotEqual(void *v1,void *v2, const char *testmsg) {
	testReport(v1 != v2, testmsg);

	return v1 != v2;
}

static inline unsigned char testNull(void *v, const char *testmsg) {
	return testEqual(v, 0, testmsg);
}

static inline unsigned char testNotNull(void *v, const char *testmsg) {
	return testNotEqual(v, 0, testmsg);
}

#define TTestValidate(T) if(!(T)) return 1

#define TTestMustValidate(T) if(!(T)) return 2

#endif
