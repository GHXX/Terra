
#include "stdafx.h"

#include "trand.h"

#include <float.h>
#include <limits.h>

static TSize tRandSeed = UINT_MAX;

void TRandSetSeed(TSize seed)
{
	srand(seed);
	tRandSeed = seed;
}

unsigned char TRandBool(void)
{
	return (TRandUInteger(0, UINT_MAX) & (1 << 15)) != 0;
}

int TRandInteger(int begin, int end)
{
	return (int)(((double)rand() / RAND_MAX) * (end - begin) + begin);
}

TSize TRandUInteger(TSize begin,TSize end)
{
	return (TSize)(((double)rand() / RAND_MAX) * (end - begin) + begin);
}

void TRandUniqueIntegersArray(int offset, TSize range, int *intarray, TSize size)
{
	if(!intarray) return;

	if(range >= size)
	{
		TSize *integers = (TSize *)malloc(sizeof(TSize)*range);
		TSize i = 0;
		for(; i < range; ++i) integers[i] = i;

		for (i = 0; i < size; ++i) {
			TSize r = TRandInteger(0,range);
			TSize value = integers[r];
			integers[r] = integers[--range];

			intarray[i] = value + offset;
		}

		free(integers);
	}
}

TSize TRandPickOne(TSize start, TSize end, TSize reject)
{
	if (reject == start)		start++;
	else if (reject == end-1)	end--;
	else if (TRandInteger(0,2)) start = reject+1;
	else end = reject;
	
	return TRandInteger(start,end);
}

double TRandDouble(double begin, double end)
{
	return (((double)rand() / RAND_MAX) * (end - begin) + begin);
}

int TRandNormal(TSize mean, TSize range, TSize clip)
{
	//TODO : fix this
	// Central limit theorm - Maybe upgrade to Box-Muller later
	int counter;
	float seed = FLT_MAX;

	return mean;
	
	if(clip > range)		return 0;		// Don't be an asshole.
	while(seed > mean + range - clip || seed < mean - range + clip) {
		seed = 0.0f;

		for (counter = 0; counter < 12; ++counter)
			seed += ((float)rand() / (float)RAND_MAX);

		seed = (seed - 6.0f) * (range / 3.0f) + mean;

		seed = TCLAMP(seed, (float)(mean - range), (float)(mean + range));
	}

	return (int)seed;
}
