#include "stdafx.h"

#include "trand.h"
#include "ttest.h"

#include "test.h"

int TRandTest1(void) {
	TSize i = 0;

	for (; i < 10000; ++i) {
		int seed;
		TUInt8 randBool;
		int randInteger;
		double randDouble;

		TRandSetSeed(i);
		seed = TRandRangeInteger(UINT_MAX);

		// copy
		TRandSetSeed(seed);
		randBool = TRandBool();

		randInteger = TRandRangeInteger(80);

		randDouble = TRandDouble(0.1564, 0.9573);

		TRandSetSeed(seed);

		TTestValidate(TRandBool() == randBool);
		TTestValidate(TRandRangeInteger(80) == randInteger);
		TTestValidate(TRandDouble(0.1564, 0.9573) == randDouble);
	}

	return 0;
}

int TRandTest2(void) {
	TSize i = 0;
	int diffCount = 0;

	for (; i < 10000; ++i) {
		int seed;
		TUInt8 randBool;
		int randInteger;
		double randDouble;

		TRandSetSeed(i);
		seed = TRandRangeInteger(UINT_MAX);

		// copy
		TRandSetSeed(seed);
		randBool = TRandBool();

		randInteger = TRandRangeInteger(80);

		randDouble = TRandDouble(0.1564, 0.9573);

		TRandSetSeed(seed + 1);

		diffCount += TRandBool() != randBool ? 1 : 0;
		diffCount += TRandRangeInteger(80) != randInteger ? 1 : 0;
		diffCount += TRandDouble(0.1564, 0.9573) != randDouble ? 1 : 0;
	}

	TTestValidate(diffCount > 1);

	return 0;
}

void TRandTest(void) {
	TestFunc tests[] = {
		TRandTest1,
		TRandTest2,
	};

	TTestRun("Randomizer", tests, sizeof(tests) / sizeof(TestFunc));
}
