
#ifndef __included_terra_integer_h
#define __included_terra_integer_h

static inline TSize TIntegerUpperPowerOfTwo(TSize v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
#ifdef _X86_64
	v |= v >> 32;
#endif
    v++;

    return v;
}


static inline unsigned char TIntegerIsPowerOfTwo(unsigned int v)
{
	return v && !(v & (v - 1));
}

static inline int TIntegerCompare(const int *a,const int *b)
{
	if(*a > *b) return 1;
	else if (*a < *b) return -1;

	return 0;
}

int *TIntegerToPtr(int data);
TSize *TIntegerToPtrU(TSize data);

#endif
