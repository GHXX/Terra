#include "stdafx.h"

#include "debugging/tdebug.h"
//#include "structure/tarray.h"
//#include "utility/tinteger.h"
#include "trand.h"

#include "test.h"

#include "ttest.h"

unsigned int randtest_min;
unsigned int randtest_max;

void rand_test_init(void) {

	randtest_min = TRandInteger(0, 50);
	randtest_max = TRandInteger(51, 100);
	printf("Random start value: %d\nRandom stop value:%d", randtest_min,randtest_max);

}

void rand_test_min_max(void) {



}







void array_test(void) {
	TLogWrite(testLog, "Testing randomness...\n");

	rand_test_init();

	rand_test_min_max();

	rand_test_randomness();

	array_test_cost();

	TLogWrite(testLog, "Array tests completed.\n");
}
