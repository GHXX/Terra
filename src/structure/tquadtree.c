
#include "stdafx.h"

#include "tquadtree.h"

#include "talloc.h"

#include "tarray.h"
#include "tstack.h"

#include "debugging/tlog.h"

#include "utility/tinteger.h"

TQuadObj *TQuadObjNew(const TRectangle *rect, TPtr data) {
	TQuadObj *pt = TAllocData(TQuadObj);
	if (!pt) return 0;

	pt->rect = *rect;
	pt->data = data;
	return pt;
}

typedef struct _TQuadNode {
	struct _TQuadNode *nodes[4];
	TRectangle bounds;
	TSList *objs;
} TQuadNode;

void TQuadNodeInit(TQuadNode *q, const TRectangle *bounds) {
	memset(q->nodes, 0, sizeof(TQuadNode *) * 4);
	q->bounds = *bounds;
	q->objs = TSListNew();
}


TQuadNode *TQuadNodeNew(const TRectangle *bounds) {
	TQuadNode *qn = TAllocData(TQuadNode);
	if (qn) TQuadNodeInit(qn, bounds);
	return qn;
}

void TQuadNodeFree(TQuadNode *q, TFreeFunc func) {
	if (q) {
		if (q->nodes[0]) {
			TQuadNodeFree(q->nodes[0], func);
			TQuadNodeFree(q->nodes[1], func);
			TQuadNodeFree(q->nodes[2], func);
			TQuadNodeFree(q->nodes[3], func);
		}
		
		if (func) {
			TQuadObj *obj = (TQuadObj *)TSListPop(q->objs);
			while (obj) {
				func(obj->data);
				TFree(obj);
				obj = (TQuadObj *)TSListPop(q->objs);
			}
		}
		TSListFree(q->objs, TFree);

		TFree(q);
	}
}

static inline int findObjPosition(TQuadNode **cur, TQuadNode *head, const TRectangle *r) {
	TSize i = 0;
	TQuadNode *c = 0;
	TSize level = 0;

	while (head) {
		if (!head->nodes[0]) {
			c = head;
			break;
		}

		for (; i < 4; ++i) {
			if (TRectangleContains(&head->nodes[i]->bounds, r)) {
				if (c) { c = head; break; }
				else c = head->nodes[i];
			}
		}

		if (c == head) break;
		head = c;
		level++;
	}

	*cur = c;

	return level;
}

void TQuadNodeSplit(TQuadNode *q) {
	TRectangle bounds = { q->bounds.x, q->bounds.y, q->bounds.w / 2, q->bounds.h / 2 };
	TQuadNode *s;
	TQuadObj *obj;
	TSize size = q->objs->len;

	q->nodes[0] = TQuadNodeNew(&bounds); bounds.x += bounds.w;
	q->nodes[1] = TQuadNodeNew(&bounds); bounds.x = q->bounds.x; bounds.y += bounds.h;
	q->nodes[2] = TQuadNodeNew(&bounds); bounds.x += bounds.w;
	q->nodes[3] = TQuadNodeNew(&bounds);

	obj = (TQuadObj *)TSListPop(q->objs);
	while (size--) {
		findObjPosition(&s, q, &obj->rect);

		TSListPrepend(s->objs, obj);

		obj = (TQuadObj *)TSListPop(q->objs);
	};
}

void TQuadNodeInsert(TQuadNode *q, TQuadObj *obj, TSize limit, TSize levellimit) {
	TSize level = 1;
	TQuadNode *s;

	if (!TRectangleContains(&q->bounds, &obj->rect)) return;

	level += findObjPosition(&s, q, &obj->rect);

	TSListAppend(q->objs, obj);
	if (q->objs->len >= limit && level < levellimit && !q->nodes[0]) TQuadNodeSplit(q);
}

void TQuadNodeRemove(TQuadNode *q, TQuadObj *obj, const TRectangle *r) {
	const TQuadObj *cur;

	if (!TRectangleContains(&q->bounds, r)) return;

	findObjPosition(&q, q, r);

	cur = (TQuadObj *)TSListFirst(q->objs);
	while (cur) {
		if (cur == obj) {
			TSListRemove(q->objs, cur);
			break;
		}
		obj = (TQuadObj *)TSListNext(q->objs);
	}
}

const TSList *TQuadNodeFetch(const TQuadNode *q, const TPoint *p) {
	const TSList *found = 0;

	if (!TRectangleContainsPointF(&q->bounds, p)) return 0;

	while (q) {
		if (q->nodes[0]) {
			if (TRectangleContainsPointF(&q->nodes[0]->bounds, p)) q = q->nodes[0];
			else if (TRectangleContainsPointF(&q->nodes[1]->bounds, p)) q = q->nodes[1];
			else if (TRectangleContainsPointF(&q->nodes[2]->bounds, p)) q = q->nodes[2];
			else q = q->nodes[3];
		} else {
			found = q->objs;
			q = 0;
		}
	}

	return found;
}

TSList *TQuadNodeFetchAll(const TQuadNode *q, const TRectangle *rect) {
	TSList *found, nodes;

	if (!TRectangleContains(&q->bounds, rect)) return 0;

	found = TSListNew();
	TSListInit(&nodes);

	do {
		if (q->nodes[0]) {
			if (TRectangleContains(&q->nodes[0]->bounds, rect)) TSListAppend(&nodes, q->nodes[0]);
			if (TRectangleContains(&q->nodes[1]->bounds, rect)) TSListAppend(&nodes, q->nodes[1]);
			if (TRectangleContains(&q->nodes[2]->bounds, rect)) TSListAppend(&nodes, q->nodes[2]);
			if (TRectangleContains(&q->nodes[3]->bounds, rect)) TSListAppend(&nodes, q->nodes[3]);
		} else {
			TQuadObj *p = (TQuadObj *)TSListFirst(q->objs);
			while (p) {
				if (TRectangleContains(rect, &p->rect)) TSListAppend(found, p);
				p = (TQuadObj *)TSListNext(q->objs);
			}
		}

		q = (TQuadNode *)TSListPopIndex(&nodes, 0);
	} while (q);

	return found;
}

struct _TQuadTree {
	TQuadNode *head;
	TSize obj_limit, level_limit;
};

TQuadTree *TQuadTreeNew(const TRectangle *bounds) {
	TQuadTree *qt = TAllocData(TQuadTree);
	if (!qt) return 0;

	qt->level_limit = 5;
	qt->obj_limit = 50;

	qt->head = TQuadNodeNew(bounds);

	return qt;
}

void TQuadTreeSet(TQuadTree *qt, TSize obj_limit, TSize level_limit) {
	if(qt) {
		qt->level_limit = level_limit;
		qt->obj_limit = obj_limit;
	}
}

void TQuadTreeEmpty(TQuadTree *qt, TFreeFunc func) {
	TRectangle bounds = qt->head->bounds;
	TQuadNodeFree(qt->head, func);

	qt->head = TQuadNodeNew(&bounds);
}

void TQuadTreeFree(TQuadTree *qt, TFreeFunc func) {
	if(qt) {
		TQuadNodeFree(qt->head, func);
		TFree(qt);
	}
}

TQuadObj *TQuadTreeInsert(TQuadTree *qt, const TRectangle *rect, TPtr data) {
	if (qt) {
		TQuadObj *obj = TQuadObjNew(rect, data);
		TQuadNodeInsert(qt->head, obj, qt->obj_limit, qt->level_limit);
		return obj;
	}

	return 0;
}

void TQuadTreeUpdate(const TQuadTree *qt, TQuadObj *obj, const TRectangle *newposition) {
	if (qt && obj && newposition) {
		TQuadNodeRemove(qt->head, obj, &obj->rect);
		obj->rect = *newposition;
		TQuadNodeInsert(qt->head, obj, qt->obj_limit, qt->level_limit);
	}
}

int TQuadTreeBringToFront(const TQuadTree *qt, const TQuadObj *obj) {
	TQuadNode *cur;
	findObjPosition(&cur, qt->head, &obj->rect);
	if (cur) {
		TSListRemove(cur->objs, obj);
		TSListPrepend(cur->objs, obj);

		return 0;
	}

	return 1;
}

int TQuadTreePushToBack(const TQuadTree *qt, const TQuadObj *obj) {
	TQuadNode *cur;
	findObjPosition(&cur, qt->head, &obj->rect);
	if (cur) {
		TSListRemove(cur->objs, obj);
		TSListAppend(cur->objs, obj);

		return 0;
	}

	return 1;
}

const TSList *TQuadTreeFetch(const TQuadTree *qt, const TPoint *p) {
	if (qt) return TQuadNodeFetch(qt->head, p);

	return 0;
}

TSList *TQuadTreeFetchAll(const TQuadTree *qt, const TRectangle *rect) {
	if (qt) return TQuadNodeFetchAll(qt->head, rect);

	return 0;
}

TSList *TQuadTreeFetchNear(const TQuadTree *qt, const TPoint *p, float dist) {
	if(qt) {
		TRectangle rect = { (int)(p->x - dist), (int)(p->y - dist), (int)(dist * 2), (int)(dist * 2) };
		return TQuadNodeFetchAll(qt->head, &rect);
	}

	return 0;
}

void TQuadTreeRemove(TQuadTree *qt, TQuadObj *obj, TFreeFunc func)
{
	if (qt && obj) {
		TQuadNodeRemove(qt->head, obj, &obj->rect);
		if (func) func(obj->data);
		TFree(obj);
	}
}
