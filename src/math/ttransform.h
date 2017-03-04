
#ifndef _included_terra_transform_h
#define _included_terra_transform_h

#include "tquaternion.h"
#include "tmatrix.h"

// 2D Transformations

typedef struct {
	TVector2 translation;
	TVector2 scaling;
	float rotation;
} TTransform2;

inline static void TTransform2Initialize(TTransform2 *context) {
	if (context) {
		memset(context, 0, sizeof(TTransform2));
		context->scaling.x = context->scaling.y = 1.0f;
	}
}

static inline TMatrix4f TTransform2ToMatrix(const TTransform2 *context) {
	TMatrix4f res;

	TMatrixSetZero(&res);

	//scaling
	res.s.xx = context->scaling.x;
	res.s.yy = context->scaling.y;
	res.s.zz = res.s.tt = 1.0f;

	//rotation
	TMatrix4fSetRotation(&res, context->rotation, 0, 0, 1);

	//translation
	TMatrix4fSetTranslation(&res, context->translation.x, context->translation.y, 0.0f);

	return res;
}

inline static void TTransform2SetTranslation(TTransform2 *context, float x, float y) {
	context->translation.x = x;
	context->translation.y = y;
}

inline static void TTransform2SetTranslationI(TTransform2 *context, int x, int y) {
	context->translation.x = (float)x;
	context->translation.y = (float)y;
}

inline static void TTransform2SetScale(TTransform2 *context, float x, float y) {
	context->scaling.x = x;
	context->scaling.y = y;
}

inline static void TTransform2SetScaleT(TTransform2 *context, TVector2 scale) {
	context->scaling = scale;
}

inline static void TTransform2SetRotation(TTransform2 *context, float angle) {
	context->rotation = angle;
}

// 3D Transformations

typedef struct {
	TVector translation;
	TVector scaling;
	TQuat rotation;
} TTransform3;

inline static void TTransform3Initialize(TTransform3 *context) {
	if (context) {
		memset(context, 0, sizeof(TTransform3));
		context->scaling.x = context->scaling.y = context->scaling.z = 1.0f;
	}
}

static inline TMatrix4f TTransform3ToMatrix(const TTransform3 *context) {
	TMatrix4f res;

	TMatrixSetZero(&res);

	//scaling
	res.s.xx = context->scaling.x;
	res.s.yy = context->scaling.y;
	res.s.zz = context->scaling.z;
	res.s.tt = 1.0f;

	//rotation
	TMatrix4fSetRotationQ(&res, &context->rotation);

	//translation
	TMatrix4fSetTranslationV(&res, context->translation);

	return res;
}

inline static void TTransform3SetTranslation(TTransform3 *context, float x, float y, float z) {
	context->translation.x = x;
	context->translation.y = y;
	context->translation.z = z;
}

inline static void TTransform3SetScale(TTransform3 *context, float x, float y, float z) {
	context->scaling.x = x;
	context->scaling.y = y;
	context->scaling.z = z;
}

inline static void TTransform3SetScaleT(TTransform3 *context, TVector scale) {
	context->scaling = scale;
}

inline static void TTransform3SetRotation(TTransform3 *context, TQuat rotation) {
	context->rotation = rotation;
}

#endif
