
#ifndef __terra_test_test_utils_h
#define __terra_test_test_utils_h

#include "debugging/tlog.h"

typedef int(*TestFunc)(void);

extern TLog *testLog;

void TTestCreateFile(const char *filename, const char *data);

void TTestRun(const char *testCategory, TestFunc *tests, TSize numTests);

static inline void TTestReport(unsigned char v, const char *testmsg) {
	TLogWrite(testLog, testmsg);
	TLogWrite(testLog, v ? "Success\n" : "Failed\n");
}

static inline unsigned char TTestEqual(void *v1, void *v2, const char *testmsg) {
	TTestReport(v1 == v2, testmsg);

	return v1 == v2;
}

static inline unsigned char TTestNotEqual(void *v1, void *v2, const char *testmsg) {
	TTestReport(v1 != v2, testmsg);

	return v1 != v2;
}

static inline unsigned char TTestNull(void *v, const char *testmsg) {
	return TTestEqual(v, 0, testmsg);
}

static inline unsigned char TTestNotNull(void *v, const char *testmsg) {
	return TTestNotEqual(v, 0, testmsg);
}

#define TTestSuccess 0
#define TTestFail 1
#define TTestFailAndStop 2

#define TTestValidate(T) if(!(T)) return TTestFail

#define TTestMustValidate(T) if(!(T)) return TTestFailAndStop

#endif
