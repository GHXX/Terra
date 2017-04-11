
#ifndef _included_terra_math_h
#define _included_terra_math_h

#include <math.h>

// Angle conversion

static inline float TMathDegToRad(float degrees)
{ 
	return (float)((double)degrees * (M_PI / 180.0));
}

static inline float TMathRadToDeg(float radian)
{ 
	return (float)((double)radian * (180.0 / M_PI));
}

// Additional math related utilities

static inline float TLOGbOFv(float base, float value)
{
	return (float)(log10f(value) / log10f(base));
	//log_a(x) = log_10(x)/log_10(a)
}

#include "ttuple.h"
#include "tquaternion.h"
#include "tmatrix.h"

#include "ttransform.h"
#include "tline.h"
#include "trectangle.h"
#include "tcuboid.h"

#endif
