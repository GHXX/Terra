
#ifndef __included_terra_auadtree_h
#define __included_terra_TQuadTree_h

#include "tdefine.h"

#define TERRA_RECTANGLE
#include "math/tmath.h"

#include "tlist.h"

typedef struct {
	TRectanglei rect;
	void *data;
} TQuadObj;

typedef struct _TQuadTree TQuadTree;

TQuadTree *TQuadTreeNew(const TRectanglei *bounds);

void TQuadTreeSet(TQuadTree *qt, TSize obj_limit, TSize level_limit);

void TQuadTreeEmpty(TQuadTree *qt, TFreeFunc func);
void TQuadTreeFree(TQuadTree *qt, TFreeFunc func);

TQuadObj *TQuadTreeInsert(TQuadTree *qt, const TRectanglei *rect, TPtr data);

void TQuadTreeUpdate(const TQuadTree *qt, TQuadObj *obj, const TRectanglei *newposition);
int TQuadTreeBringToFront(const TQuadTree *qt, const TQuadObj *obj);
int TQuadTreePushToBack(const TQuadTree *qt, const TQuadObj *obj);

const TSList *TQuadTreeFetch(const TQuadTree *qt, const TPoint *p);
TSList *TQuadTreeFetchAll(const TQuadTree *qt, const TRectanglei *rect);
TSList *TQuadTreeFetchNear(const TQuadTree *qt, const TPoint *p, float dist);

void TQuadTreeRemove(TQuadTree *qt, TQuadObj *obj, TFreeFunc func);

#endif
