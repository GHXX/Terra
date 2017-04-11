#include "stdafx.h"
#include <math.h>
#include <time.h>
#include "debugging/tdebug.h"
//#include "structure/tarray.h"
//#include "utility/tinteger.h"
#include "trand.h"
#include "test.h"
#include "ttime.h"
#include "ttest.h"

unsigned int randtest_min_range, randtest_max_range;
signed int randtest_min_range_test, randtest_max_range_test;
WORD seed = 0xFFFF;

int getRandomNumberForTesting(int min, int max) {
	
	//printf("\n\nTime:%d\n\n", (long)clock() % 0x1F);
	//seed %= 2;// ((unsigned int)((long)clock()));//*(0.5+min)*(5.5+max))
	//printf("\nseed:%f",(double) seed);
	//unsigned int random = (int)seed;// % 0xFF;
	//printf("\nrandom:%f", ((double)random / (double)0xFF));
	//return (int) round( (max - min + 1)*((double)random/(double)0xFFFF) + min);
	//seed *= random;
	return (int) round(((double)rand()/ (double)RAND_MAX) * (max - min) + min);
}

/*
int getRandomNumberForTesting(int min, int max) {
	
	//printf("\n\nTime:%d\n\n", (long)clock() % 0x1F);
	seed %= 2;// ((unsigned int)((long)clock()));//*(0.5+min)*(5.5+max))
	//printf("\nseed:%f",(double) seed);
	unsigned int random = (int) seed % 0xFF;
	//printf("\nrandom:%f", ((double)random / (double)0xFF));
	//return (int) round( (max - min + 1)*((double)random/(double)0xFFFF) + min);
	seed *= random;
	return (int) (((double)random / (double)0xFF) * (max - min + 1) + min);
}*/

void rand_test_init(void) {
	SYSTEMTIME system_time;
	GetSystemTime(&system_time);
	srand(system_time.wMilliseconds);
	randtest_min_range = getRandomNumberForTesting(0, 50);
	randtest_max_range = getRandomNumberForTesting(51, 100);
	printf("\tRandom start value: %d\n\tRandom stop value:%d\n",randtest_min_range, randtest_max_range);

}

void rand_test_min_max_rand(void) {
	boolean success = FALSE;
	printf("\tTesting Min/Max boundaries.\n");

	unsigned int calculations = 0;
	unsigned int calculations_max = MAXUINT32>>14;
	randtest_min_range_test = randtest_max_range_test = (int)round((randtest_max_range + randtest_min_range) / 2);
	long double sum = 0;
	for (calculations = 0; calculations< calculations_max; calculations++)
	{
		
		int rand =(int) TRandInteger(randtest_min_range, randtest_max_range);
		if (rand > randtest_max_range_test)
		{
			randtest_max_range_test = rand;
		}
		else if(rand < randtest_min_range_test) 
		{ 
			randtest_min_range_test = rand;
		}
		printf("\r\t[%d/%d calculations done] (%f%%)..",calculations,calculations_max, (double)calculations / (double)calculations_max * (float)100);
		//printf("\r                                                                      \rSmallest random value:%d | Biggest random value:%d | Total generated values:%d", randtest_min_range_test, randtest_max_range_test, calculations);
		sum += rand;
	} 
	printf("\r\t\t\t\t\t\t\t\r\t100%% done.");
	printf("\n\n\tSmallest random value:%d | Biggest random value:%d | Total generated values:%d\n", randtest_min_range_test, randtest_max_range_test, calculations);
	if (randtest_max_range_test != randtest_max_range || randtest_min_range_test != randtest_min_range) {
		success = FALSE;
		printf("\n\tError: Boundaries werent matched properly after %d tries:",calculations);
		printf("\n\tLower Boundary | Upper Boundary | Required Lower Boundary | Required Upper Boundary");
		printf("\n\t     %d        |      %d        |         %d              |         %d        \n",randtest_min_range_test,randtest_max_range_test,randtest_min_range,randtest_max_range);
	}
	else
	{
		printf("\n\tBoundary test was successful. Average weight of the random number:");
		int avg = (int)(sum / (double)calculations);
		int middle = (randtest_max_range_test + randtest_min_range_test) / 2;
		int zerobased = avg - middle;
		double normalized = (double)zerobased /(double) (randtest_max_range_test - randtest_min_range_test);
		//printf("\n%d\n%d\n%d\n%f\n",avg,middle,zerobased,normalized);
		short drawQuality = 21;
		printf("\n\t|");
		for (short i = 0; i < drawQuality; i++)
		{
			if (((int)(normalized*drawQuality+0.5*drawQuality)) == i) {
				printf("O");
			}
			else if (((int)(drawQuality/2))==i) {
				printf("=");
			}
			else {
				printf("-");
			}
		}
		printf("|\n\tO..current average weight      =..ideal average weight\n");
		printf("\tResult: Weighting is ");
		if (normalized == 0) { printf("perfect!"); }
		else if (normalized == -1) { printf("far too low."); }
		else if (normalized <= -0.5) { printf("too low."); }
		else if (normalized <= -0.25) { printf("a bit too low."); }
		else if (normalized < 0) { printf("slightly too low."); }
		else if (normalized == 1) { printf("far too high."); }
		else if (normalized >= 0.5) { printf("too high."); }
		else if (normalized >= 0.25) { printf("a bit too high."); }
		else if (normalized > 0) { printf("slightly too high."); }
		printf("\n\tWeight distribution: %s%f\n", ((normalized == 0) ? "+/-" : (normalized < 0) ? "" : "+"), normalized);
		
	}
}

#if !DEBUG
	//#pragma optimize("",on)
#endif

#define rand_test_len 1E6
void rand_test_cost(void) {

	printf("\tRunning Speed test...(This may take a while)");
	double clockEnd;
	double clockStart;
	TTimeInitialise();
	
	clockStart = TTimeComputeTime();
	for (unsigned long i = 0; i<rand_test_len; i++)
	{
		int rand = TRandInteger(randtest_min_range, randtest_max_range);
	}
	clockEnd = TTimeComputeTime();
	double diff = clockEnd - clockStart;
	printf("\r\t\t\t\t\t\t\t\r\tAverage generation time of one random number: %Lf nanoseconds\n\n",( (long double) diff / (long double)rand_test_len)* (long double)1E9);
}

#if !DEBUG
	//#pragma optimize("",off)
#endif

void rand_test(void) {

	TLogWrite(testLog, "Testing randomness...\n");

	rand_test_init();

	rand_test_min_max_rand();

	rand_test_cost();

	TLogWrite(testLog, "Randomness tests completed.\n");
}
