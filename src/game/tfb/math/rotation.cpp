#include <stdexcept>
#include "tfb/render/yonbox.h"
#include <string.h>
#include "rw/rwcore.h"

// 0x449020
int RotationFromTo(const RwV3d *a, const RwV3d *b, RwMatrix *out)
{
	float d = a->x * b->x + b->y * a->y + a->z * b->z;
	if (d <= -0.99999f) {
		// (anti)parallel: a 180 degree turn about an axis v perpendicular to b (2 v v^T - I)
		RwV3d v;
		v.y = b->z;
		v.z = -b->y;
		v.x = 0.0f;
		float yy = v.y * v.y;
		if (yy + v.z * v.z < 1e-05f) {
			v.y = 0.0f;
			v.x = -b->z;
			v.z = b->x;
		}
		RwV3dNormalize(reinterpret_cast<RwV3d *>(&v), reinterpret_cast<const RwV3d *>(&v));
		out->flags = 3;
		out->right.x = 2.0f * (v.x * v.x) - 1.0f;
		float yx = 2.0f * (v.y * v.x);
		out->right.y = yx;
		float zx = 2.0f * (v.z * v.x);
		out->right.z = zx;
		out->up.x = yx;
		out->up.y = 2.0f * (v.y * v.y) - 1.0f;
		float zy = 2.0f * (v.z * v.y);
		out->up.z = zy;
		*(int *)&out->at.y = *(int *)&zy;           // the game copies the bits of a stored temp (fst tmp; mov ecx,tmp; mov at.y,ecx)
		out->at.x = zx;
		out->pos.z = 0.0f;
		out->pos.y = 0.0f;
		out->pos.x = 0.0f;
		out->at.z = 2.0f * (v.z * v.z) - 1.0f;
		return -1;
	}
	if (d < 0.99999f) {
		// Rodrigues with c = a x b: R = d I + [c]x + (1 - d) / |c|^2 c c^T
		RwV3d c;                                      // c.x is never stored by the game (x87 stack, unrounded)
		float cx = a->y * b->z - a->z * b->y;
		c.y = a->z * b->x - a->x * b->z;
		c.z = a->x * b->y - b->x * a->y;
		float cy2 = c.y * c.y;
		float cx2 = (float)(cx * cx);
		float cz2 = c.z * c.z;                       // unrounded
		float s = cx2 + cy2;
		float len = s + cz2;
		if (len > 1e-05f) {
			double inv = (1.0f - d) / len;
			out->pos.z = 0.0f;
			out->pos.y = 0.0f;
			out->pos.x = 0.0f;
			out->flags = 3;
			out->right.x = cx2 * inv + d;
			float cyx = cx * c.y * inv;
			out->right.y = c.z + cyx;
			float czx = cx * c.z * inv;
			out->right.z = czx - c.y;
			out->up.x = cyx - c.z;
			out->up.y = cy2 * inv + d;
			float czy = c.z * c.y * inv;
			out->up.z = czy + cx;
			out->at.x = czx + c.y;
			out->at.y = czy - cx;
			out->at.z = cz2 * inv + d;
			return 1;
		}
	}
	return 0;
}

// 0x44a1a0
void VectorXlen()
{
	throw std::length_error("vector<T> too long");
}

// 0x44aa00
float AngleDelta(float a, float b)
{
	float d = b - a;
	while (d < 0.0f)
		d += 360.0f;
	while (d >= 360.0f)
		d -= 360.0f;
	if (d >= 180.0f)
		d -= 360.0f;
	return d;
}
