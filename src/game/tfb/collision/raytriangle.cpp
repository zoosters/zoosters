// A ray against a triangle: the Moeller-Trumbore test (Fast, Minimum Storage Ray/Triangle Intersection, 1997) with a small tolerance
// on the triangle's edges. Used by the corpus's ground rays, the nearest-triangle queries and the camera's line-of-sight probe.
#include "tfb/collision/raytriangle.h"

static const float kParallelLimit = 1e-6f;  // |det| below this: the ray runs parallel to the triangle
static const float kEdgeTolerance = 1e-4f;  // barycentric coordinates may leave the triangle by this much

// NOTE: the sums below keep the game's order (y, x, z, and the hit point's terms as the game adds them). This test decides ground
// contact and the camera's line of sight; with the natural order a reference comparison found 327 of 300000
// cases off by more than 256 ulps (hit points near 0 after cancellation) and 56 outcome flips; with it, all are bit-identical.
static inline double DotYXZ(const RwV3d &a, double x, double y, double z)
{
	return (a.y * y + a.x * x) + a.z * z;
}
static inline float DotYXZ(const RwV3d &a, const RwV3d &b)
{
	return (a.y * b.y + a.x * b.x) + a.z * b.z;
}

// FUNCTION: 0x448d10 RayTriangleIntersect
int RayTriangleIntersect(const RwV3d *start, const RwV3d *dir, const RwV3d *v0, const RwV3d *v1, const RwV3d *v2, RwV3d *hit,
                         float *uv, float *t)
{
	RwV3d e1, e2;
	RwV3dSub(&e1, v1, v0);
	RwV3dSub(&e2, v2, v0);
	// NOTE: p = dir x e2 and the determinant stay at full precision until u is formed, as on the game's x87 stack: rounded to float
	// they change t by up to 0.2 % where det is small.
	double px = (double)dir->y * e2.z - (double)dir->z * e2.y;
	double py = (double)dir->z * e2.x - (double)dir->x * e2.z;
	double pz = (double)dir->x * e2.y - (double)dir->y * e2.x;
	double det = DotYXZ(e1, px, py, pz);
	if (det > -kParallelLimit && det < kParallelLimit)
		return RAYTRI_MISS;
	RwV3d toStart;
	RwV3dSub(&toStart, start, v0);
	float invDet = (float)(1.0 / det);
	float u = (float)(DotYXZ(toStart, px, py, pz) * invDet);
	if (u < -kEdgeTolerance || u > 1.0f + kEdgeTolerance)
		return RAYTRI_MISS;
	RwV3d q;
	RwV3dCrossProduct(&q, &toStart, &e1);
	float v = DotYXZ(q, *dir) * invDet;
	if (v < -kEdgeTolerance || v + u > 1.0f + kEdgeTolerance)
		return RAYTRI_MISS;
	if (uv) {
		uv[0] = u;
		uv[1] = v;
	}
	if (hit) {
		hit->x = e2.x * v + e1.x * u + v0->x;
		hit->y = e2.y * v + e1.y * u + v0->y;
		hit->z = e1.z * u + e2.z * v + v0->z;
	}
	float tLine = DotYXZ(q, e2) * invDet;
	if (t)
		*t = tLine;
	if (tLine < 0.0f)
		return RAYTRI_BEHIND;
	return tLine >= 1.0f ? RAYTRI_BEYOND : RAYTRI_INSIDE;
}
