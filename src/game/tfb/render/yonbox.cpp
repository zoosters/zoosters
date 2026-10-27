// YonBox: filing the distant geometry into its octree and walking it against the camera frustum (declaration: yonbox.h).
#include <string.h>
#include "tfb/render/yonbox.h"
#include "modules/tfb/renderqueue.h"
#include "tfb/collision/collisionbox.h"
#include "rw/rpcollis.h"

extern const float g_floatZero;   // GLOBAL: 0x5ca5a8 g_floatZero
extern "C" const void *port_RwCameraFrustumPlanes(const RwCamera *camera);   // port/shim/rwcore.cpp: the six RwFrustumPlanes (0x14 B each)

// 0x4417e0
__declspec(noinline) YonBox::YonBox(const RwBBox *b)
{
	float hx, hy, hz;
	memset(m_cells, 0, sizeof(m_cells));
	m_nodeCount = 1;
	m_minCell.z = -1.0f;
	m_minCell.y = -1.0f;
	m_minCell.x = -1.0f;
	m_half.x = b->sup.x - b->inf.x;
	m_half.y = b->sup.y - b->inf.y;
	m_half.z = b->sup.z - b->inf.z;
	hx = m_half.x * 0.5f;
	m_half.x = hx;
	m_half.y *= 0.5f;
	hy = m_half.y;
	hz = m_half.z * 0.5f;
	m_half.z = hz;
	m_center.x = hx + b->inf.x;
	m_center.y = hy + b->inf.y;
	m_center.z = hz + b->inf.z;
	int n = 0;
	if (hx <= 480.0f) {
		n = 1;
		m_half.x = 480.0f;
		m_minCell.x = 480.0f;
	}
	if (hy <= 480.0f) {
		n++;
		m_half.y = 480.0f;
		m_minCell.y = 480.0f;
	} else {
		m_nodeCount = 2;
	}
	if (hz <= 480.0f) {
		n++;
		m_half.z = 480.0f;
		m_minCell.z = 480.0f;
	}
	if (n == 3)
		*(short *)m_cells = -1;
}

// 0x4417b0
void YonBox::QueueVisibleAtomics(RwCamera *cam)
{
	m_camera = cam;
	Walk(1, m_center.x, m_center.y, m_center.z, 0, &m_half);
}


// 0x441340
void YonBox::AddAtomic(RpAtomic *a)
{
	if (RpAtomicGetGeometry(a) && RpGeometryGetNumTriangles(RpAtomicGetGeometry(a))) {   // the game reads the geometry twice
		const RwSphere *s = RpAtomicGetWorldBoundingSphere(a);
		RwBBox bb;
		int one;
		bb.inf.x = s->center.x - s->radius;
		bb.inf.y = s->center.y - s->radius;
		one = 1;
		bb.inf.z = s->center.z - s->radius;
		bb.sup.x = s->center.x + s->radius;
		bb.sup.y = s->center.y + s->radius;
		bb.sup.z = s->center.z + s->radius;
		InsertNode((short *)&one, m_center.x, m_center.y, m_center.z, a, &bb, &m_half);
	}
}


// True when a triangle (world space) may touch the cell [lo, hi]: never when all three vertices lie beyond one face of the cell;
// always when every vertex is inside; else the box-triangle test decides.
static bool TouchesCell(const RwV3d tri[3], RwV3d *const verts[3], const RwV3d &lo, const RwV3d &hi, CollisionBox *cell,
                        const RwV3d *noMotion)
{
	int beyond, sum;
	beyond = (tri[0].x < lo.x) + (tri[1].x < lo.x) + (tri[2].x < lo.x);
	if (beyond == 3)
		return false;
	sum = beyond;
	beyond = (tri[0].y < lo.y) + (tri[1].y < lo.y) + (tri[2].y < lo.y);
	if (beyond == 3)
		return false;
	sum += beyond;
	beyond = (tri[0].z < lo.z) + (tri[1].z < lo.z) + (tri[2].z < lo.z);
	if (beyond == 3)
		return false;
	sum += beyond;
	beyond = (tri[0].x > hi.x) + (tri[1].x > hi.x) + (tri[2].x > hi.x);
	if (beyond == 3)
		return false;
	sum += beyond;
	beyond = (tri[0].y > hi.y) + (tri[1].y > hi.y) + (tri[2].y > hi.y);
	if (beyond == 3)
		return false;
	sum += beyond;
	beyond = (tri[0].z > hi.z) + (tri[1].z > hi.z) + (tri[2].z > hi.z);
	if (beyond == 3)
		return false;
	sum += beyond;
	if (sum == 0)
		return true;
	return cell->FindTriangleContact(1.0f, verts, noMotion, 0, 0, 0) == 0;
}

// Files the atomic `at` into the octree node `node` (cell centre a, b, c, half size ext) and recurses into the children; at the leaf size (below 480 per axis, also recorded as the smallest cell) the atomic is linked into the cell's list.
// 0x4409f0
void YonBox::InsertNode(short *node, float a, float b, float c, RpAtomic *at, const RwBBox *bb, const RwV3d *ext)
{
	RwV3d v = *ext;
	if (v.x < m_minCell.x)
		v.x = m_minCell.x;
	if (v.y < m_minCell.y)
		v.y = m_minCell.y;
	if (v.z < m_minCell.z)
		v.z = m_minCell.z;
	RwV3d lo, hi;
	lo.x = a - v.x;
	lo.y = b - v.y;
	lo.z = c - v.z;
	hi.x = v.x + a;
	hi.y = v.y + b;
	hi.z = v.z + c;
	if (lo.x > bb->sup.x || lo.y > bb->sup.y || lo.z > bb->sup.z)
		return;
	if (hi.x < bb->inf.x || hi.y < bb->inf.y || hi.z < bb->inf.z)
		return;
	CollisionBox box;   // the cell as an axis-aligned box
	box.m_extent[0] = v.x;
	box.m_extent[1] = v.y;
	box.m_extent[2] = v.z;
	box.m_frame.right.x = box.m_frame.up.y = box.m_frame.at.z = 1.0f;
	box.m_frame.right.y = box.m_frame.right.z = box.m_frame.up.x = 0.0f;
	box.m_frame.up.z = box.m_frame.at.x = box.m_frame.at.y = 0.0f;
	box.m_frame.pos.x = a;
	box.m_frame.pos.y = b;
	box.m_frame.pos.z = c;
	box.m_frame.flags = 0x20003;   // the game ORs this into the uninitialised word; nothing reads the flags
	RwV3d tri[3];
	RwV3d *verts[3];
	verts[0] = &tri[0];
	verts[1] = &tri[1];
	verts[2] = &tri[2];
	RwV3d noMotion;
	noMotion.x = noMotion.y = noMotion.z = 0.0f;
	RpGeometry *geom = RpAtomicGetGeometry(at);
	int n = RpGeometryGetNumTriangles(geom);
	const RwV3d *vtx = RpMorphTargetGetVertices(RpGeometryGetMorphTarget(geom, 0));
	const RpTriangle *tris = RpGeometryGetTriangles(geom);
	do {   // the first triangle that touches the cell (n stays > 0)
		tri[0] = vtx[tris->vertIndex[0]];
		tri[1] = vtx[tris->vertIndex[1]];
		tri[2] = vtx[tris->vertIndex[2]];
		RwV3dTransformPoints(tri, tri, 3, RwFrameGetLTM(RpAtomicGetFrame(at)));
		if (TouchesCell(tri, verts, lo, hi, &box, &noMotion))
			break;
		tris++;
	} while (--n);
	if (n == 0)
		return;
	bool created = *node == 0;
	if (created)
		*node = (short)++m_nodeCount;
	RwV3d half;
	half.x = ext->x * 0.5f;
	half.y = ext->y * 0.5f;
	half.z = ext->z * 0.5f;
	if (half.x < 480.0f) {
		half.x = 0.0f;
		if (m_minCell.x < g_floatZero)
			m_minCell.x = ext->x;
	}
	if (half.y < 480.0f) {
		half.y = 0.0f;
		if (m_minCell.y < g_floatZero)
			m_minCell.y = ext->y;
	} else if (created)
		m_nodeCount++;
	if (half.z < 480.0f) {
		half.z = 0.0f;
		if (m_minCell.z < g_floatZero)
			m_minCell.z = ext->z;
	}
	short *cell = (short *)Slot(*node);
	if (half.x == g_floatZero && half.y == g_floatZero && half.z == g_floatZero) {
		*cell = -1;
		void **link = (void **)Slot(m_nodeCount + 1);   // link entry: the atomic, then the previous head of the leaf's list
		link[0] = at;
		link[1] = *LeafList(cell);
		*LeafList(cell) = link;
		m_nodeCount++;
		return;
	}
	InsertNode(cell, a - half.x, b - half.y, c - half.z, at, bb, &half);
	if (half.x != g_floatZero)
		InsertNode(cell + 1, a + half.x, b - half.y, c - half.z, at, bb, &half);
	if (half.z != g_floatZero) {
		InsertNode(cell + 2, a - half.x, b - half.y, c + half.z, at, bb, &half);
		if (half.x != g_floatZero)
			InsertNode(cell + 3, a + half.x, b - half.y, c + half.z, at, bb, &half);
	}
	if (half.y != g_floatZero) {
		InsertNode(cell + 4, a - half.x, b + half.y, c - half.z, at, bb, &half);
		if (half.x != g_floatZero)
			InsertNode(cell + 5, a + half.x, b + half.y, c - half.z, at, bb, &half);
		if (half.z != g_floatZero) {
			InsertNode(cell + 6, a - half.x, b + half.y, c + half.z, at, bb, &half);
			if (half.x != g_floatZero)
				InsertNode(cell + 7, a + half.x, b + half.y, c + half.z, at, bb, &half);
		}
	}
}

// Grid walk: cull the cell against the camera frustum, then visit its atomics or recurse into the 8 sub-cells.
struct YonPlane { float n[3]; float d; unsigned char idx[4]; };   // 0x14 (frustum plane + the indices of the corner to test)
struct YonNode { unsigned short tag; unsigned short child[7]; };   // 0xffff tag = leaf list at +4 (overlaps child[1..2])
#include "tfb/render/yonbox_views.h"
struct YonLink { AtomicStampView *obj; YonLink *next; };   // the atomic (an RpAtomic) and the next entry

// dot sums are (t1 + t0) + t2
// 0x4413d0
void YonBox::Walk(short index, float cx, float cy, float cz, int d, const RwV3d *box)
{
	if (index == 0)
		return;
	if (d == 0) {
		RwV3d e = *box;
		if (e.x < m_minCell.x)
			e.x = m_minCell.x;
		if (e.y < m_minCell.y)
			e.y = m_minCell.y;
		if (e.z < m_minCell.z)
			e.z = m_minCell.z;
		float corner[2][3];
		corner[1][0] = cx - e.x;  corner[1][1] = cy - e.y;  corner[1][2] = cz - e.z;
		corner[0][0] = e.x + cx;  corner[0][1] = e.y + cy;  corner[0][2] = e.z + cz;
		const YonPlane *p = (const YonPlane *)port_RwCameraFrustumPlanes(m_camera);
		PORT_RQ(14);
		for (int i = 6; i; i--, p++) {
			float a = corner[p->idx[0]][0];
			float b = corner[p->idx[1]][1];
			float c = corner[p->idx[2]][2];
			float dot = (b * p->n[1] + a * p->n[0]) + c * p->n[2];
			if (dot - p->d > 0.0f) {
				PORT_RQ(15);
				return;
			}
		}
	}
	YonNode *node = (YonNode *)Slot((unsigned short)index);
	if (node->tag == 0xffff) {
		for (YonLink *it = *(YonLink **)LeafList((short *)node); it; it = it->next) {
			PORT_RQ(16);
			PORT_RQ_IF(it->obj->m_renderFrame == RwEngineGlobals()->renderFrame, 17);
			if (it->obj->m_renderFrame != RwEngineGlobals()->renderFrame) {
				it->obj->m_renderFrame = RwEngineGlobals()->renderFrame;
				RenderQueue_AddAtomic(m_camera, (RpAtomic *)it->obj);
			}
		}
		return;
	}
	RwV3d h;
	h.x = box->x * 0.5f;
	h.y = box->y * 0.5f;
	h.z = box->z * 0.5f;
	if (h.x < 480.0f)   // game: test ah,5; jp (h < 480; NaN keeps h)
		h.x = 0.0f;
	if (h.y < 480.0f)
		h.y = 0.0f;
	if (h.z < 480.0f)
		h.z = 0.0f;
	Walk(node->tag, cx - h.x, cy - h.y, cz - h.z, d, &h);
	Walk(node->child[0], cx + h.x, cy - h.y, cz - h.z, d, &h);
	Walk(node->child[1], cx - h.x, cy - h.y, cz + h.z, d, &h);
	Walk(node->child[2], cx + h.x, cy - h.y, cz + h.z, d, &h);
	if (h.y == 0.0f)
		return;
	Walk(node->child[3], cx - h.x, cy + h.y, cz - h.z, d, &h);
	Walk(node->child[4], cx + h.x, cy + h.y, cz - h.z, d, &h);
	Walk(node->child[5], cx - h.x, cy + h.y, cz + h.z, d, &h);
	Walk(node->child[6], cx + h.x, cy + h.y, cz + h.z, d, &h);
}
