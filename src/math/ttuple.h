
#ifndef _included_terra_tuple_h
#define _included_terra_tuple_h

// Common tuples

typedef struct { short x, y; } TTuple2s;
typedef struct { int   x, y; } TTuple2i;
typedef struct { float x, y; } TTuple2f;

typedef struct { int   x, y, z; } TTuple3i;
typedef struct { float x, y, z; } TTuple3f;

typedef struct { int   x, y, z, t; } TTuple4i;
typedef struct { float x, y, z, t; } TTuple4f;

// common naming

typedef TTuple2f TPoint;
typedef TTuple2f TVector2f;

typedef TTuple3f TTPoint3;
typedef TTuple3f TVector;

typedef TTuple4f TQuat;

// 2D Point operation

inline static void TPointAdd(TPoint *to, const TPoint *from)
{
	to->x += from->x;
	to->y += from->y;
}

inline static void TPointSub(TPoint *to, const TPoint *from)
{
	to->x -= from->x;
	to->y -= from->y;
}

inline static unsigned char TPointEqual(const TPoint *p1, const TPoint *p2)
{
	return p1->x == p2->x && p1->y == p2->y;
}

inline static float TPointEuclidianDistance(const TPoint *to, const TPoint *from)
{
	float y = to->y - from->y, x = to->x - from->x;
	return sqrtf(y*y + x*x);
}

// 3-Tuple operations

inline static void TTuple3fSet(TTuple3f *t, float x, float y, float z)
{
	t->x = x; t->y = y; t->z = z;
}

inline static void TTuple3fCopy(TTuple3f *t1, const TTuple3f *t2)
{
	t1->x = t2->x; t1->y = t2->y; t1->z = t2->z;
}

inline static unsigned char TTuple3fEqual(const TTuple3f *t1, const TTuple3f *t2)
{
	return t1->x == t2->x && t1->y == t2->y && t1->z == t2->z;
}

inline static void TTuple3fMin(TTuple3f *t1, const TTuple3f *t2)
{
	t1->x = TMIN(t1->x, t2->x);
	t1->y = TMIN(t1->y, t2->y);
	t1->z = TMIN(t1->z, t2->z);
}

inline static void TTuple3fMax(TTuple3f *t1, const TTuple3f *t2)
{
	t1->x = TMAX(t1->x, t2->x);
	t1->y = TMAX(t1->y, t2->y);
	t1->z = TMAX(t1->z, t2->z);
}

inline static void TTuple3fAdd(TTuple3f *t1, const TTuple3f *t2)
{
	t1->x += t2->x;
	t1->y += t2->y;
	t1->z += t2->z;
}

inline static void TTuple3fSub(TTuple3f *t1, const TTuple3f *t2)
{
	t1->x -= t2->x;
	t1->y -= t2->y;
	t1->z -= t2->z;
}

inline static void TTuple3fRotate(TVector *vector, const TTuple3f *cosangles, const TTuple3f *sinangles)
{
	float cx = cosangles->x, sx = sinangles->x;
	float cy = cosangles->y, sy = sinangles->y;
	float cz = cosangles->z, sz = sinangles->z;
	float x = vector->x;
	float y = vector->y;
	float z = vector->z;

	//rotate around x
	float t = y;

	y = cx*y - sx*z;
	z = sx*t + cx*z;

	//rotate around y
	t = x;

	x = cy*x + sy*z;
	vector->z = cy*z - sy*t;

	//rotate around z
	t = x;

	vector->x = cz*x - sz*y;
	vector->y = sz*t + cz*y;
}

// Vector operations

#define TVectorSet TTuple3fSet
#define TVectorRotate TTuple3fRotate

inline static TVector TVectorCross(const TVector *v1, const TVector *v2)
{
	TVector result = {
		(v1->y * v2->z) - (v1->z * v2->y),
		(v1->z * v2->x) - (v1->x * v2->z),
		(v1->x * v2->y) - (v1->y * v2->x)
	};

	return result;
}

inline static float TVectorDot(const TVector *v1, const TVector *v2)
{
	return (v1->x * v2->x) + (v1->y * v2->y) + (v1->z * v2->z);
}

inline static float TVectorLengthSquared(const TVector *v)
{
	return (v->x * v->x) + (v->y * v->y) + (v->z * v->z);
}

inline static float TVectorLength(const TVector *v)
{
	return sqrtf(TVectorLengthSquared(v));
}

inline static void TVectorNormalize(TVector *v)
{
	float length = TVectorLength(v);

	v->x /= length;
	v->y /= length;
	v->z /= length;
}

inline static TVector TVectorNormalfromTuples(const TTuple3f *v1,const TTuple3f *v2,const TTuple3f *v3)
{
	TVector vec1 = *v1, vec2 = *v2, out;

	TTuple3fSub(&vec1,v2);
	TTuple3fSub(&vec2,v3);

	out = TVectorCross(&vec1, &vec2);
	TVectorNormalize(&out);

	return out;
}

#endif
