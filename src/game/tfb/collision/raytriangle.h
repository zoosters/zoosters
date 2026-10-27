// A ray against a triangle (raytriangle.cpp).
#ifndef ZOOSTERS_TFB_COLLISION_RAYTRIANGLE_H
#define ZOOSTERS_TFB_COLLISION_RAYTRIANGLE_H

#include "rw/rwcore.h"

// What RayTriangleIntersect found. The segment is start .. start + dir.
enum RayTriangleResult {
	RAYTRI_MISS = 0,   // the ray's line misses the triangle (or runs parallel to it)
	RAYTRI_BEHIND = 1, // the line meets the triangle behind the start
	RAYTRI_BEYOND = 2, // beyond the end of the segment
	RAYTRI_INSIDE = 3  // within the segment
};

// Where the line start + t * dir meets the triangle v0 v1 v2 (Moeller-Trumbore). Optional outputs (0 to skip): `hit` the point,
// `uv` its barycentric coordinates (along v1 - v0 and v2 - v0), `t` the line parameter.
int RayTriangleIntersect(const RwV3d *start, const RwV3d *dir, const RwV3d *v0, const RwV3d *v1, const RwV3d *v2, RwV3d *hit,
                         float *uv, float *t);   // 0x448d10

#endif
