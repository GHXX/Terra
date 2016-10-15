
#ifndef _included_terra_quaternion_h
#define _included_terra_quaternion_h

#include "ttuple.h"

typedef TTuple4f TQuat;

static inline TQuat TQuatFromEulerAngles(float ax, float ay, float az) {
	float c1, s1, c2, s2, c3, s3;
	TQuat q;

	ax *= 0.5f;
	ay *= 0.5f;
	az *= 0.5f;

	c1 = cosf(ax);
	s1 = sinf(ax);
	c2 = cosf(ay);
	s2 = sinf(ay);
	c3 = cosf(az);
	s3 = sinf(az);

	q.t = c1*c2*c3 - s1*s2*s3;
	q.x = c1*c2*s3 + s1*s2*c3;
	q.y = s1*c2*c3 + c1*s2*s3;
	q.z = c1*s2*c3 - s1*c2*s3;

	return q;
}

static inline TQuat TQuatFromAxisAngle(float a, float rx, float ry, float rz) {
	float sa;
	TQuat q;

	a *= 0.5f;

	sa = sinf(a);
	q.x = rx * sa;
	q.y = ry * sa;
	q.z = rz * sa;
	q.t = cosf(a);

	return q;
}

#endif
