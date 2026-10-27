// The game's out-of-line RwV3d helpers of the camera unit: vectors by value and the camera's follow lerp.
#ifndef ZOOSTERS_TFB_MATH_VECTOR_OPS_H
#define ZOOSTERS_TFB_MATH_VECTOR_OPS_H

#include "rw/rwcore.h"

float Atan2f(float y, float x);
RwV3d V3Add(const RwV3d *a, const RwV3d *b);
RwV3d V3Sub(const RwV3d *a, const RwV3d *b);
RwV3d V3Negate(const RwV3d *a);
void Vec3LerpInPlace(RwV3d *a, const RwV3d *b, float t);  // a += (b - a) * t


// The inlined forms (cl inlines them in the camera code).
static inline RwV3d V3SubInline(const RwV3d *a, const RwV3d *b)
{
	RwV3d r = *a;
	r.x -= b->x;
	r.y -= b->y;
	r.z -= b->z;
	return r;
}
static inline void V3ScaleInline(RwV3d *dst, const RwV3d *v, float k)
{
	dst->x = v->x * k;
	dst->y = v->y * k;
	dst->z = v->z * k;
}
static inline void V3AddToInline(RwV3d *dst, const RwV3d *v)
{
	dst->x = v->x + dst->x;
	dst->y = v->y + dst->y;
	dst->z = v->z + dst->z;
}

#endif
