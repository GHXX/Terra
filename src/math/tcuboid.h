
#ifndef _included_terra_cuboid_h
#define _included_terra_cuboid_h

#include "tdefine.h"

typedef struct {
	float x, y, z;
	float w, h, d;
} TCuboid;

static inline void TCuboidCopy(TCuboid *c1, const TCuboid *c2)
{
	c1->x = c2->x;
	c1->y = c2->y;
	c1->z = c2->z;
	c1->w = c2->w;
	c1->h = c2->h;
	c1->d = c2->d;
}

#endif
