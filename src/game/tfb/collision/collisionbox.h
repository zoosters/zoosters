// CollisionBox: the oriented box of the actor collision code and its intersection tests. The tests are TFB's port of David
// Eberly's Magic Software collision code (1999-2000: ln3bx3ti.cpp, bx3bx3fi.cpp, tr3bx3fi.cpp, accompanying his paper "Dynamic
// Collision Detection using Oriented Bounding Boxes"), adapted to RenderWare types.
#ifndef ZOOSTERS_TFB_COLLISION_COLLISIONBOX_H
#define ZOOSTERS_TFB_COLLISION_COLLISIONBOX_H

#include "zoosters/base.h"
#include "rw/rwcore.h"

// Results of CollisionBox::TestSegment. Values below COLLISIONBOX_INSIDE are misses.
enum CollisionBoxSegmentResult {
	COLLISIONBOX_MISS = 0,        // the point is outside / the segment's line passes the box
	COLLISIONBOX_MOVING_AWAY = 1, // the segment starts outside a pair of faces and does not move towards them
	COLLISIONBOX_FALLS_SHORT = 2, // it moves towards a pair of faces but ends before reaching them
	COLLISIONBOX_INSIDE = 3,      // it starts inside (entry time 0), or it hits and no entry time was asked for
	COLLISIONBOX_ENTERS = 4       // it hits: + the entry face, 0..2 = the positive side of axis 0..2, 3..5 = the negative side
};

// The separating axes FindBoxContact / FindTriangleContact report (OBB_AXIS_NONE: no axis separates, they touch). Eberly's
// numbering of bx3bx3fi.cpp, continued with the axes of the box-triangle test.
// clang-format off: the axes are grouped by kind, one kind per line
enum ObbAxis {
	OBB_AXIS_NONE = 0,
	OBB_AXIS_A0, OBB_AXIS_A1, OBB_AXIS_A2,                       // face normals of box a (box-triangle: of the box)
	OBB_AXIS_B0, OBB_AXIS_B1, OBB_AXIS_B2,                       // face normals of box b
	OBB_AXIS_A0xB0, OBB_AXIS_A0xB1, OBB_AXIS_A0xB2,              // edge pairs A_i x B_j
	OBB_AXIS_A1xB0, OBB_AXIS_A1xB1, OBB_AXIS_A1xB2,
	OBB_AXIS_A2xB0, OBB_AXIS_A2xB1, OBB_AXIS_A2xB2,
	OBB_AXIS_WxA0, OBB_AXIS_WxA1, OBB_AXIS_WxA2,                 // relative velocity W x A_k
	OBB_AXIS_WxB0, OBB_AXIS_WxB1, OBB_AXIS_WxB2,                 // W x B_k
	OBB_AXIS_N,                                                  // the triangle's normal
	OBB_AXIS_A0xE0, OBB_AXIS_A0xE1, OBB_AXIS_A0xE2,              // box axis x triangle edge A_i x E_j
	OBB_AXIS_A1xE0, OBB_AXIS_A1xE1, OBB_AXIS_A1xE2,
	OBB_AXIS_A2xE0, OBB_AXIS_A2xE1, OBB_AXIS_A2xE2,
	OBB_AXIS_WxE0, OBB_AXIS_WxE1, OBB_AXIS_WxE2                  // W x E_j
};
// clang-format on

// An oriented box: centre C, orthonormal axes A0, A1, A2 and the half extents a0, a1, a2 along them (Eberly's OBBox3). The axes
// and the centre are kept as an RwMatrix (right, up, at, pos), as the boxes are made from frame matrices.
struct CollisionBox {
	RwReal m_extent[3]; // +0x00 half sizes along the axes
	RwMatrix m_frame;   // +0x0c axes (right/up/at) and centre (pos)

	// Axis i (0..2) of the box. (The game has an out-of-line copy at 0x41e0e0.)
	const RwV3d *GetAxis(int i) const
	{
		switch (i) {
		case 0:
			return &m_frame.right;
		case 1:
			return &m_frame.up;
		}
		return &m_frame.at;
	}

	// The game's out-of-line copy of GetAxis (0x41e0e0, in the camera translation unit; tfb/math/vector_ops.cpp). Nothing calls it.
	const RwV3d *GetAxisOutOfLine(int i) const;

	const RwV3d *GetCentre() const { return &m_frame.pos; }

	int TestSegment(const RwV3d *start, const RwV3d *dir, RwReal *entryTime) const;
	static int FindBoxContact(RwReal t, CollisionBox *a, const RwV3d *va, CollisionBox *b, const RwV3d *vb, RwReal *tOut,
	                          RwV3d *point, RwV3d *normal);
	int FindTriangleContact(RwReal t, const RwV3d *const *tri, const RwV3d *vel, RwReal *tOut, RwV3d *point, RwV3d *normal);
};
ZOOSTERS_ASSERT_SIZE(CollisionBox, 0x4c);
ZOOSTERS_ASSERT_OFFSET(CollisionBox, m_frame, 0x0c);

#endif
