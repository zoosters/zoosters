#include <math.h>
#include "tfb/level/level.h"
#include "modules/tfb/gamecamera.h"
#include "modules/tfb/camera_views.h"
#include "modules/tfb/gameactor.h"
#include "tfb/corpus/corpus.h"
#include "tfb/collision/collider.h"
#include "tfb/collision/raytriangle.h"
#include "rw/rpcollis.h"
#include "tfb/math/vector_ops.h"
#include "tfb/render/tfbatomic.h"

// GLOBAL: 0x6182d8 g_msgInvalidCam
// GLOBAL: 0x618338 g_msgCorpusCamera
// GLOBAL: 0x62972c RWS::CLevel::sm_pActiveWorld
// GLOBAL: 0x5cd0c0 ?CameraProbe_vftable@@3PADA
// GLOBAL: 0x5cd030 g_camAxisX
// GLOBAL: 0x5cd03c g_camAxisY
extern RWS::CEventId g_msgCorpusCamera, g_msgInvalidCam; // g_msgInvalidCam: this module's copy of ERROR_NOT_A_VALID_MESSAGE_ID


// GLOBAL: 0x5cd0c0 CameraProbe::vftable
#include "modules/tfb/cameraprobe.h"

#include "modules/tfb/gamecamera_probe_views.h"
enum { kTriangleNotCamera = 0x10 }; // flag of a triangle's table entry: the camera ignores it

// SendMsg of the framework with this module's invalid-message constant (no static initialiser, see g_msgInvalidCam)
static inline void ProbeSendMsg(RWS::CMsg &m)
{
	if (m.Id != (RWS::CRegisteredMsgs *)g_msgInvalidCam && m.Id->Erm.p_next)
		RWS::_SendMsg(m);
}

static inline RwV3d Negated(const RwV3d &v)
{
	RwV3d r;
	r.x = -v.x;
	r.y = -v.y;
	r.z = -v.z;
	return r;
}

// Near-match of 0x41f980: the game keeps the loop counter in the dead stack slot of its last parameter (frame 0x18 instead of 0x1c) and
// loads the return value before the `jp`.
// RpAtomicForAllIntersections callback of the probe: a line query only clears m_noHit and keeps the nearest fraction; the box query pulls
// each of the four sight-line end points in to the triangle where its line from the anchor crosses it (at half the distance).
RpCollisionTriangle *CameraProbeAtomicTriangleCB(RpIntersection *intersection, RpCollisionTriangle *tri, float limit, void *data)
{
	CameraProbe *probe = (CameraProbe *)data;
	if (intersection->type == rpINTERSECTLINE) {
		probe->m_noHit = 0;
		if (limit < probe->m_best)
			probe->m_best = limit;
		return tri;
	}
	for (int i = 0; i < 4; i++) {
		RwV3d d = V3SubInline(&probe->m_pts[i], &probe->m_anchor);
		float t;
		if (RayTriangleIntersect(&probe->m_anchor, &d, tri->vertices[0], tri->vertices[1], tri->vertices[2], 0, 0, &t) >= RAYTRI_BEYOND) {
			t *= 0.5f;
			if (t <= 1.0f) {
				V3ScaleInline(&probe->m_pts[i], &d, t);
				V3AddToInline(&probe->m_pts[i], &probe->m_anchor);
			}
		}
	}
	return tri;
}

// 0x41fac0
// Skips triangles whose table entry has the flag 0x10, else clips with CameraProbeAtomicTriangleCB.
RpCollisionTriangle *CameraProbeTriangleCB(RpIntersection *intersection, RpWorldSector *sector, RpCollisionTriangle *tri, float limit,
                                           void *data)
{
	if (!(((unsigned char *)((RpWorldSectorView *)sector)->triangles)[tri->index * 8 + 6] & kTriangleNotCamera))
		return CameraProbeAtomicTriangleCB(intersection, tri, limit, data);
	return tri;
}

// 0x420160
// The hook of the Corpus handlers: probes the atomics of the collider's clump that carry flag 0x100, except those of the actor's own clump.
void CameraProbe::ForEachAtomic(const Collider *collider)
{
	RpClump *clump = collider->m_clump;
	RwLLLink *node = clump->atomicList.link.next;
	while (node != &clump->atomicList.link) {
		RwLLLink *next = node->next;
		RpAtomic *atomic = RpAtomicFromClumpLink(node);
		if (TFBPlugin_FD_TestFlags(atomic, 0x100) && ((RpAtomicView *)atomic)->clump != m_skipClump)
			RpAtomicForAllIntersections(atomic, &m_isect, CameraProbeAtomicTriangleCB, this);
		if (!atomic)
			break;
		node = next;
	}
}

// The game multiplies by a 0.0f literal (`fld d.x; fmul [__real@0]`) and merges the z-components of the two arms;
// the t-block store order (y, x, z) is a permuter result, the game computes x, y, z.
// Probes the four sides of the camera around `anchor`: first a box that contains the four sight-line end points (the anchor moved by
// +-scale along the camera's right and up axes; sides in `sideMask` or a still actor stay put, a moving one is pulled in) is swept
// through the world, then a sphere around it is offered to the Corpus bodies, then a line from each end point to the camera
// (8 units along the side's axis from the eye) is cast. Returns the sides whose line hit (bit i), hitFractions[i] = its fraction or -1.
int RWS::TFB::GameCamera::CollisionProbe(const GameActor *actor, const RwV3d *anchor, float scale, float *hitFractions, unsigned sideMask)
{
	if (!actor)
		return 0;
	CameraProbe probe;
	int hit;
	hit = 0;
	RwV3d d;
	probe.m_anchor = *anchor;
	int i;
	probe.m_skipClump = actor->m_clump;
	bool still = actor->m_velocity.x == g_floatZero && actor->m_velocity.y == g_floatZero && actor->m_velocity.z == g_floatZero;
	RwBBox &box = probe.m_isect.t.box;
	for (i = 0; i <= 3; i++) {
		switch (i) {
		case 2:
			d.x = scale * m_matrix.up.x;
			d.y = scale * m_matrix.up.y;
			d.z = scale * m_matrix.up.z;
			break;
		case 0:
			d.x = scale * m_matrix.right.x;
			d.y = scale * m_matrix.right.y;
			d.z = scale * m_matrix.right.z;
			break;
		case 1:
		case 3:
			d = Negated(d);
			break;
		}
		if (!((sideMask & (1 << i)) && (i < 2 || (actor->m_corpus->m_flags & CORPUS_AIRBORNE)))) {
			probe.m_pts[i].x = (d.x + probe.m_anchor.x);
			probe.m_pts[i].y = (d.y + probe.m_anchor.y);
			probe.m_pts[i].z = (d.z + probe.m_anchor.z);
		} else {
			RwV3d t;
			if (still) {
				t.x = d.x * g_floatZero;
				t.y = d.y * g_floatZero;
				t.z = d.z * g_floatZero;
			} else {
				float k;
				k = 2.0f / scale;
				t.x = k * d.x;
				t.y = k * d.y;
				t.z = k * d.z;
			}
			probe.m_pts[i].x = (t.x + probe.m_anchor.x);
			probe.m_pts[i].y = (t.y + probe.m_anchor.y);
			probe.m_pts[i].z = (t.z + probe.m_anchor.z);
		}
		if (!i) {
			RwV3d first = probe.m_pts[0];
			box.sup = first;
			box.inf = probe.m_pts[0];
		} else {
			if (probe.m_pts[i].x < box.inf.x)
				box.inf.x = probe.m_pts[i].x;
			else if (probe.m_pts[i].x > box.sup.x)
				box.sup.x = probe.m_pts[i].x;
			if (probe.m_pts[i].y < box.inf.y)
				box.inf.y = probe.m_pts[i].y;
			else if (probe.m_pts[i].y > box.sup.y)
				box.sup.y = probe.m_pts[i].y;
			if (probe.m_pts[i].z < box.inf.z)
				box.inf.z = probe.m_pts[i].z;
			else if (probe.m_pts[i].z > box.sup.z)
				box.sup.z = probe.m_pts[i].z;
		}
	}
	probe.m_isect.type = rpINTERSECTBOX;
	box.inf.x -= 6.0f;
	box.inf.y -= 6.0f;
	box.inf.z = box.inf.z - (6.0f);
	box.sup.x += 6.0f;
	box.sup.y += 6.0f;
	box.sup.z += 6.0f;
	if (RWS::CLevel::sm_pActiveWorld != 0)
		RpCollisionWorldForAllIntersections(RWS::CLevel::sm_pActiveWorld, &probe.m_isect, CameraProbeTriangleCB, &probe);
	// the box becomes the sphere around it: centre = middle, radius = distance to the corner
	probe.m_isect.type = rpINTERSECTSPHERE;
	RwV3dAdd(&box.sup, &box.sup, &box.inf);
	RwV3dScale(&box.sup, &box.sup, 0.5f);
	RwV3d *inf;
	inf = &box.inf;
	RwV3dSub(&box.inf, inf, &box.sup);
	probe.m_isect.t.sphere.radius = RwV3dLength(&box.inf);
	RWS::CMsg msg(g_msgCorpusCamera, &probe);
	ProbeSendMsg(msg);
	probe.m_isect.type = rpINTERSECTLINE;
	RwLine &line = probe.m_isect.t.line;
	for (i = 0; i <= 3; i++) {
		probe.m_best = 3.4028235e38f;
		switch (i) {
		case 2:
			d.x = m_matrix.up.x * 8.0f;
			d.y = m_matrix.up.y * 8.0f;
			d.z = m_matrix.up.z * 8.0f;
			break;
		case 0:
			d.x = m_matrix.right.x * 8.0f;
			d.y = m_matrix.right.y * 8.0f;
			d.z = m_matrix.right.z * 8.0f;
			break;
		case 1:
		case 3:
			d = Negated(d);
			break;
		}
		line.end.x = d.x + m_matrix.pos.x;
		line.end.y = d.y + m_matrix.pos.y;
		line.end.z = d.z + m_matrix.pos.z;
		line.start = probe.m_pts[i];
		RwV3dSub(&probe.m_anchor, &line.end, &line.start);
		probe.m_noHit = 1;
		if (RWS::CLevel::sm_pActiveWorld != 0)
			RpCollisionWorldForAllIntersections(RWS::CLevel::sm_pActiveWorld, &probe.m_isect, CameraProbeTriangleCB, &probe);
		ProbeSendMsg(msg);
		if (!probe.m_noHit && probe.m_best < 0.9999f) {
			hit = hit | (1 << i);
			hitFractions[i] = probe.m_best;
		} else
			hitFractions[i] = -1.0f;
	}
	return hit;
}
