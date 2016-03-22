
#ifndef _included_terra_transform_h
#define _included_terra_transform_h

#include "ttuple.h"

// 2D Transformations

typedef struct _TTransform2D {
	TTuple2f translation;
	TTuple2f scaling;
	float rotation;
} TTransform2D;

inline static void TTransform2DInitialize(TTransform2D *context)
{
	if(context) {
		memset(context, 0, sizeof(TTransform2D));
		context->scaling.x = context->scaling.y = 1.0f;
	}
}

inline static void TTransform2DSetTranslation(TTransform2D *context, float x, float y)
{
	context->translation.x = x;
	context->translation.y = y;
}

inline static void TTransform2DSetTranslationI(TTransform2D *context, int x, int y)
{
	context->translation.x = (float) x;
	context->translation.y = (float) y;
}

inline static void TTransform2DSetScale(TTransform2D *context, float x, float y)
{
	context->scaling.x = x;
	context->scaling.y = y;
}

inline static void TTransform2DSetScaleT(TTransform2D *context, TTuple2f scale)
{
	context->scaling = scale;
}

inline static void TTransform2DSetRotation(TTransform2D *context, float angle)
{
	context->rotation = angle;
}

// 3D Transformations

typedef struct _TTransform3D {
	TTuple3f translation;
	TTuple3f scaling;
	TTuple3f rotation;
} TTransform3D;

inline static void TTransform3DInitialize(TTransform3D *context)
{
	if(context) {
		memset(context,0,sizeof(TTransform3D));
		context->scaling.x = context->scaling.y = context->scaling.z = 1.0f;
	}
}

inline static void TTransform3DSetTranslation(TTransform3D *context, float x, float y, float z)
{
	context->translation.x = x;
	context->translation.y = y;
	context->translation.z = z;
}

inline static void TTransform3DSetScale(TTransform3D *context, float x, float y, float z)
{
	context->scaling.x = x;
	context->scaling.y = y;
	context->scaling.z = z;
}

inline static void TTransform3DSetScaleT(TTransform3D *context, TTuple3f scale)
{
	context->scaling = scale;
}

inline static void TTransform3DSetRotation(TTransform3D *context, float ax, float ay, float az)
{
	context->rotation.x = ax;
	context->rotation.y = ay;
	context->rotation.z = az;
}

#endif
