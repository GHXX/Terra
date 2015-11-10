
#ifndef _included_terra_rectangle_h
#define _included_terra_rectangle_h

#include "tdefine.h"

#include "ttuple.h"

typedef struct {
	int x,y;
	TSize w,h;
} TRectangle;

static inline void TRectangleSet(TRectangle *r,int x, int y, TSize w, TSize h)
{
	r->x = x;
	r->y = y;
	r->w = w;
	r->h = h;
}

static inline void TRectangleCopy(TRectangle *r1,const TRectangle *r2)
{
	r1->x = r2->x;
	r1->y = r2->y;
	r1->w = r2->w;
	r1->h = r2->h;
}

static inline unsigned char TRectangleIntersect(const TRectangle *r1, const TRectangle *r2)
{
	return (r1->x + (int)r1->w >= r2->x || r1->x <= r2->x + (int)r2->w) &&
		   (r1->y + (int)r1->h >= r2->y || r1->y <= r2->y + (int)r2->h);
}

static inline unsigned char TRectangleContains(const TRectangle *r1, const TRectangle *r2)
{
	return (r1->x + (int)r1->w >= r2->x + (int)r2->w && r1->x <= r2->x) &&
		   (r1->y + (int)r1->h >= r2->y + (int)r2->h && r1->y <= r2->y);
}

static inline unsigned char TRectangleContainsPoint(const TRectangle *r, int x, int y)
{
	return (r->x + (int)r->w >= x && r->x <= x) &&
		   (r->y + (int)r->h >= y && r->y <= y);
}

static inline unsigned char TRectangleContainsPointF(const TRectangle *r, const Point *pt)
{
	float x1 = (float) r->x, y1 = (float) r->y;
	float x2 = (float) (r->x + (int)r->w), y2 = (float)(r->y + (int)r->h);
	return (x2 >= pt->x && x1 <= pt->x) &&
		   (y2 >= pt->y && y1 <= pt->y);
}

static inline unsigned char TRectangleXContainsPoint(const TRectangle *r, int x, int y)
{
	return (r->x + (int)r->w >= x && r->x <= x);
}

static inline unsigned char TRectangleYContainsPoint(const TRectangle *r, int x, int y)
{
	return (r->y + (int)r->h >= y && r->y <= y);
}

static inline unsigned char TRectangleEqual(const TRectangle *r1, const TRectangle *r2)
{
	return r2->x == r1->x && r2->y == r1->y && r2->w == r1->w && r2->h == r1->h;
}

static inline void TRectangleMove(TRectangle *r, int x, int y)
{
	r->x = x;
	r->y = y;
}

static inline void TRectangleResize(TRectangle *r, TSize w, TSize h)
{
	r->w = w;
	r->h = h;
}

static inline void TRectangleMerge(TRectangle *r1, const TRectangle *r2)
{
	if(r2->x == -1) return;

	if (r1->x == -1) {
		r1->x = r2->x;
		r1->w = r2->w;
	} else {
		int x2 = r1->x + r1->w;
		r1->x = TMIN(r1->x,r2->x);
		r1->w = TMAX(x2,r2->x + (int)r2->w) - r1->x;
	}

	if (r1->y == -1) {
		r1->y = r2->y;
		r1->h = r2->h;
	} else {
		int y2 = r1->y + r1->h;
		r1->y = TMIN(r1->y,r2->y);
		r1->h = TMAX(y2,r2->y + (int)r2->h) - r1->y;
	}
}

#endif
