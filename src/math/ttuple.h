
#ifndef _included_terra_tuple_h
#define _included_terra_tuple_h

// Common tuples

#define TTuple2(T) struct { T x, y; }

#define TTuple3(T) struct { T x, y, z; }

#define TTuple4(T) struct { T x, y, z, w; }

#define TTuple(T, s) struct { TUInt8 size = s; T d[s]; }

// common naming

typedef TTuple2(float) TTuple2f;
typedef TTuple2f TPoint2;
typedef TTuple2f TVector2;

typedef TTuple3(float) TTuple3f;
typedef TTuple3f TPoint;
typedef TTuple3f TVector;


typedef TTuple4(float) TTuple4f;

// 2D Point operation

#define TTuple2Set(t, a, b) { \
	(t).x = a; \
	(t).y = b; \
}

#define TTuple2Add(t1, t2) { \
	(t1).x += (t2).x; \
	(t1).y += (t2).y; \
}

#define TTuple2Sub(t1, t2) { \
	(t1).x -= (t2).x; \
	(t1).y -= (t2).y; \
}

#define TTuple2Div(t1, t2) { \
	(t1).x /= (t2).x; \
	(t1).y /= (t2).y; \
}

#define TTuple2DivV(t1, a) { \
	(t1).x /= a; \
	(t1).y /= a; \
}

#define TTuple2Eq(t1, t2) t1.x == t2.x && t1.y == t2.y

static inline float TPoint2EuclidianDistance(const TPoint2 *to, const TPoint2 *from) {
	float y = to->y - from->y, x = to->x - from->x;
	return sqrtf(y*y + x*x);
}

static inline float TPoint2EuclidianDistanceO(const TPoint2 *to) {
	float y = to->y, x = to->x;
	return sqrtf(y*y + x*x);
}

#define TTuple2Min(t1, t2) { \
	t1.x = TMIN(t1.x, t2.x); \
	t1.y = TMIN(t1.y, t2.y); \
}

#define TTuple2Max(t1, t2) { \
	t1.x = TMAX(t1.x, t2.x); \
	t1.y = TMAX(t1.y, t2.y); \
}

// 3-Tuple operations

#define TTuple3Set(t, a, b, c) { \
	(t).x = a; \
	(t).y = b; \
	(t).z = c; \
}

#define TTuple3Add(t1, t2) { \
	t1.x += t2.x; \
	t1.y += t2.y; \
	t1.z += t2.z; \
}

#define TTuple3Sub(t1, t2) { \
	t1.x -= t2.x; \
	t1.y -= t2.y; \
	t1.z -= t2.z; \
}

#define TTuple3Eq(t1, t2) t1.x == t2.x && t1.y == t2.y && t1.z == t2.z

#define TTuple3Min(t1, t2) { \
	t1.x = TMIN(t1.x, t2.x); \
	t1.y = TMIN(t1.y, t2.y); \
	t1.z = TMIN(t1.z, t2.z); \
}

#define TTuple3Max(t1, t2) { \
	t1.x = TMAX(t1.x, t2.x); \
	t1.y = TMAX(t1.y, t2.y); \
	t1.z = TMAX(t1.z, t2.z); \
}

static inline void TVectorRotate(TVector *vector, const TVector *cosangles, const TVector *sinangles) {
	float cx = cosangles->x, sx = sinangles->x;
	float cy = cosangles->y, sy = sinangles->y;
	float cz = cosangles->z, sz = sinangles->z;
	float x = vector->x;
	float y = vector->y;
	float z = vector->z;

	//rotate around x
	float t = y;

	y = cx * y - sx * z;
	z = sx * t + cx * z;

	//rotate around y
	t = x;

	x = cy * x + sy * z;
	vector->z = cy * z - sy * t;

	//rotate around z
	t = x;

	vector->x = cz * x - sz * y;
	vector->y = sz * t + cz * y;
}

// Vector operations

#define TVectorCross(v1, v2, r) { \
	(r).x = ((v1).y * (v2).z) - ((v1).z * (v2).y); \
	(r).y = ((v1).z * (v2).x) - ((v1).x * (v2).z); \
	(r).z = ((v1).x * (v2).y) - ((v1).y * (v2).x); \
}

inline static float TVectorDot(const TVector *v1, const TVector *v2) {
	return (v1->x * v2->x) + (v1->y * v2->y) + (v1->z * v2->z);
}

inline static float TVectorLengthSquared(const TVector *v) {
	return (v->x * v->x) + (v->y * v->y) + (v->z * v->z);
}

inline static float TVectorLength(const TVector *v) {
	return sqrtf(TVectorLengthSquared(v));
}

inline static void TVectorNormalize(TVector *v) {
	float length = TVectorLength(v);

	v->x /= length;
	v->y /= length;
	v->z /= length;
}

inline static TVector TVectorNormalfromTuples(const TVector *v1, const TVector *v2, const TVector *v3) {
	TVector vec1 = *v1, vec2 = *v2, out;

	TTuple3Sub(vec1, (*v2));
	TTuple3Sub(vec2, (*v3));

	TVectorCross(vec1, vec2, out);
	TVectorNormalize(&out);

	return out;
}

#endif
