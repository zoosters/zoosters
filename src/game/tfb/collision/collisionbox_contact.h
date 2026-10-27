// Helpers shared by the two swept tests of CollisionBox (collisionbox_find_box.cpp, collisionbox_find_triangle.cpp): projected
// radii and the contact point (Eberly's GET_COEFF / GET_POINT as TFB ported them). Internal to tfb/collision.
#ifndef ZOOSTERS_TFB_COLLISION_COLLISIONBOX_CONTACT_H
#define ZOOSTERS_TFB_COLLISION_COLLISIONBOX_CONTACT_H

#include "tfb/collision/collisionbox.h"

// Projections below this are treated as zero (TFB's dead zone; Eberly tests exact signs).
static const RwReal kAxisEpsilon = 0.0001f;

// The radius of a box projected onto an axis: ext[0] f0 + ext[1] f1 + ext[2] f2 (f = |cosine| of the box axes with it).
static inline RwReal ProjectedExtent(const RwReal ext[3], RwReal f0, RwReal f1, RwReal f2)
{
	return ext[0] * f0 + ext[1] * f1 + ext[2] * f2;
}

// Coordinate of the contact point along a box axis: +-extent by the sign of the rotation entry `c` and the side of approach
// (`sign` = +-side), 0 when |c| is within kAxisEpsilon.
static inline RwReal ContactCoordinate(RwReal c, RwReal extent, int sign)
{
	if (c > kAxisEpsilon)
		return (RwReal)sign * extent;
	if (c < -kAxisEpsilon)
		return (RwReal)(-sign) * extent;
	return 0.0f;
}

// The contact point on `box` given its coordinates along the box axes, moved by `vel` over `time`.
// NOTE: the game takes component i of axis k, i.e. it applies the transposed axis matrix, where Eberly's GET_POINT sums
// coeff[i] * A_i; the point is off for rotated boxes. Kept as the original.
static inline void ContactPoint(const CollisionBox *box, const RwV3d *vel, const RwReal coeff[3], RwReal time, RwV3d *point)
{
	const RwReal *centre = &box->GetCentre()->x;
	RwReal *p = &point->x;
	for (int k = 0; k < 3; ++k) {
		const RwV3d *axis = box->GetAxis(k);
		p[k] = centre[k] + coeff[0] * axis->x + coeff[1] * axis->y + coeff[2] * axis->z;
	}
	RwV3dIncrementScaled(point, vel, time);
}

#endif
