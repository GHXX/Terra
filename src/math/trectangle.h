
#ifndef _included_terra_rectangle_h
#define _included_terra_rectangle_h

#define TRectangle(T,U) struct { T x, y; U w, h; }

typedef TRectangle(TInt32, TSize) TRectanglei;
typedef TRectangle(float, float) TRectanglef;

#define TRectangleSet(r, a, b, c, d) { \
	(r).x = a;\
	(r).y = b;\
	(r).w = c;\
	(r).h = d;\
}

static inline void TRectangleCopy(TRectanglei *r1, const TRectanglei *r2) {
	r1->x = r2->x;
	r1->y = r2->y;
	r1->w = r2->w;
	r1->h = r2->h;
}

static inline unsigned char TRectangleIntersect(const TRectanglei *r1, const TRectanglei *r2) {
	return (r1->x + (int)r1->w >= r2->x || r1->x <= r2->x + (int)r2->w) &&
		(r1->y + (int)r1->h >= r2->y || r1->y <= r2->y + (int)r2->h);
}

static inline unsigned char TRectangleContains(const TRectanglei *r1, const TRectanglei *r2) {
	return (r1->x + (int)r1->w >= r2->x + (int)r2->w && r1->x <= r2->x) &&
		(r1->y + (int)r1->h >= r2->y + (int)r2->h && r1->y <= r2->y);
}

static inline unsigned char TRectangleContainsPoint(const TRectanglei *r, int x, int y) {
	return (r->x + (int)r->w >= x && r->x <= x) &&
		(r->y + (int)r->h >= y && r->y <= y);
}

static inline unsigned char TRectangleContainsPointF(const TRectanglei *r, const TPoint *pt) {
	float x1 = (float)r->x, y1 = (float)r->y;
	float x2 = (float)(r->x + (int)r->w), y2 = (float)(r->y + (int)r->h);
	return (x2 >= pt->x && x1 <= pt->x) &&
		(y2 >= pt->y && y1 <= pt->y);
}

static inline unsigned char TRectangleXContainsPoint(const TRectanglei *r, TInt32 x, TInt32 y) {
	return (r->x + (int)r->w >= x && r->x <= x);
}

static inline unsigned char TRectangleYContainsPoint(const TRectanglei *r, TInt32 x, TInt32 y) {
	return (r->y + (int)r->h >= y && r->y <= y);
}

static inline unsigned char TRectangleEqual(const TRectanglei *r1, const TRectanglei *r2) {
	return r2->x == r1->x && r2->y == r1->y && r2->w == r1->w && r2->h == r1->h;
}

static inline TPoint TRectangleGetCenter(const TRectanglei *r) {
	TPoint center = { (float)r->x + ((float)r->w / 2.0f), (float)r->y + ((float)r->h / 2.0f) };
	return center;
}

static inline void TRectangleMove(TRectanglei *r, TInt32 x, TInt32 y) {
	r->x = x;
	r->y = y;
}

static inline void TRectangleResize(TRectanglei *r, TSize w, TSize h) {
	r->w = w;
	r->h = h;
}

static inline void TRectangleMerge(TRectanglei *r1, const TRectanglei *r2) {
	if (r2->x == -1) return;

	if (r1->x == -1) {
		r1->x = r2->x;
		r1->w = r2->w;
	} else {
		int x2 = r1->x + r1->w;
		r1->x = TMIN(r1->x, r2->x);
		r1->w = TMAX(x2, r2->x + (TInt32)r2->w) - r1->x;
	}

	if (r1->y == -1) {
		r1->y = r2->y;
		r1->h = r2->h;
	} else {
		int y2 = r1->y + r1->h;
		r1->y = TMIN(r1->y, r2->y);
		r1->h = TMAX(y2, r2->y + (TInt32)r2->h) - r1->y;
	}
}

#endif
