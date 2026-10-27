// Out-of-line RwV3d helpers of the camera unit (cl 13.10 did not inline them everywhere).
#include "tfb/math/vector_ops.h"
#include "tfb/collision/collisionbox.h"

struct V3Obj : public RwV3d {
	void Scale(float s, const RwV3d *v);
	V3Obj &AddTo(const RwV3d *v);
};

// 0x41df80
void V3Obj::Scale(float s, const RwV3d *v)
{
	x = s * v->x;
	y = s * v->y;
	z = s * v->z;
}

// 0x41dfb0
V3Obj &V3Obj::AddTo(const RwV3d *v)
{
	x = v->x + x;
	y = v->y + y;
	z = v->z + z;
	return *this;
}

// a + b; the parentheses around the sum decide cl's x87 scheduling, as in RW's RwV3dAdd macro
// 0x41dfe0
RwV3d V3Add(const RwV3d *a, const RwV3d *b)
{
	RwV3d sum = *a;
	sum.x = (sum.x + b->x);
	sum.y = (sum.y + b->y);
	sum.z = (sum.z + b->z);
	return sum;
}

// 0x41e040
RwV3d V3Sub(const RwV3d *a, const RwV3d *b)
{
	RwV3d difference = *a;
	difference.x = (difference.x - b->x);
	difference.y = (difference.y - b->y);
	difference.z = (difference.z - b->z);
	return difference;
}

// 0x41e0a0
RwV3d V3Negate(const RwV3d *a)
{
	RwV3d negated;
	negated.x = -a->x;
	negated.y = -a->y;
	negated.z = -a->z;
	return negated;
}

// The game's out-of-line copy of CollisionBox::GetAxis; nothing in the tree calls it.
// 0x41e0e0
const RwV3d *CollisionBox::GetAxisOutOfLine(int i) const
{
	switch (i) {
	case 0:
		return &m_frame.right;
	case 1:
		return &m_frame.up;
	}
	return &m_frame.at;
}

// The camera's follow step towards a target. The y component is scaled by t twice, as the game does: the camera follows vertically more slowly than sideways.
// 0x41e100
void Vec3LerpInPlace(RwV3d *a, const RwV3d *b, float t)
{
	RwV3d d;
	d.x = b->x - a->x;
	d.y = b->y - a->y;
	d.z = b->z - a->z;
	d.x = d.x * t;
	d.y = d.y * t * t;
	d.z = d.z * t;
	a->x = a->x + d.x;
	a->y = a->y + d.y;
	a->z = a->z + d.z;
}
