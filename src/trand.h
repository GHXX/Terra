
#ifndef __included_terra_rand_h
#define __included_terra_rand_h

void TRandSetSeed(TSize seed);

unsigned char TRandBool(void);

int TRandInteger(int begin, int end);
TSize TRandUInteger(TSize begin, TSize end);
#define TRandRangeInteger(r) TRandInteger(0, r)

void TRandUniqueIntegersArray(int offset, TSize range, int *intarray, TSize size);

TSize TRandPickOne(TSize start, TSize end, TSize reject);

double TRandDouble(double begin, double end);

int TRandNormal(TSize mean, TSize range, TSize clip);

#endif
