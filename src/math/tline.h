
#ifndef _included_terra_line_h
#define _included_terra_line_h

typedef struct {
	float x1, y1;
	float x2, y2;
} TLine;

static inline void TLineSet(TLine *l, float x1, float y1, float x2, float y2)
{
	if (l) {
		l->x1 = x1; l->y1 = y1;
		l->x2 = x2; l->y2 = y2;
	}
}

static inline unsigned char TLineIntersect(const TLine *l1, const TLine *l2, float *x, float *y)
{
	float a1, b1, c1;
	float a2, b2, c2;
	float det;

	if (!l1 || !l2) return 0;

	a1 = l1->y2 - l1->y1;
	b1 = l1->x1 - l1->x2;
	c1 = a1*l1->x1 + b1*l1->y1;

	a2 = l2->y2 - l2->y1;
	b2 = l2->x1 - l2->x2;
	c2 = a2*l2->x1 + b2*l2->y1;

	det = a1*b2 - a2*b1;

	
	if((int)det == 0) return 0;

	if (x) *x = (b2*c1 - b1*c2) / det;
	if (y) *y = (a1*c2 - a2*c1) / det;

	return 1;
}

#endif
