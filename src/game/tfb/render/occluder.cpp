#include "script/value.h"
#include "script/types/scriptsystem.h"
#include "script/types/scriptsound.h"
#include "script/types/scriptmessage.h"
#include "script/types/scriptsets.h"
#include "script/types/scriptprop.h"
#include "modules/tfb/levelhub.h"
#include <stdlib.h>
#include "rw/rwcore.h"
#include "tfb/render/occluder.h"
#include <math.h>
#include "tfb/render/tfbrenderstate.h"
#include "tfb/render/camera_stub.h"

// GLOBAL: 0x5ca5a8 g_floatZero
// GLOBAL: 0x617e78 g_planeList413
extern const float g_floatZero;
struct SphereP413 { float x, y, z, r; };

// Box413: the game takes the object in esi (custom register convention), modelled as a thiscall here.
struct Box413 {
	char m_pad00[0x54];
	float *m_corners;
	bool IsOccluded();
};
extern unsigned char g_pixelFogColor[4];   // GLOBAL: 0x617d60 (annotated in gfx/small_e_expose.cpp)
extern int g_pixelCutoff;                  // GLOBAL: 0x617d98 (annotated elsewhere)
void SavePixelFogRenderStates();                                // 0x4113b0
void RestorePixelFogRenderStates();                                // 0x411470

struct CamPlane { float x, y, z, d, pad; };
extern "C" const void *port_RwCameraFrustumPlanes(const RwCamera *camera);   // port/shim/rwcore.cpp: the six RwFrustumPlanes (0x14 B each)
struct CamB {
	char m_pad00[4];
	void *m_frame;
	char m_pad08[0x8c];
	CamPlane m_planes[6];
};

// noinline helper so cl may take the plane entry in esi, as the game does
static __declspec(noinline) bool InsideList(OccluderNode *e)
{
	for (OccluderNode *n = g_planeList413; n; n = n->m_next) {
		float *p = e->m_box;
		if (((n->y * p[1] + p[0] * n->x) + n->z * p[2]) - n->d <= g_floatZero)
			continue;
		if (((n->y * p[4] + p[3] * n->x) + n->z * p[5]) - n->d <= g_floatZero)
			continue;
		if (((n->y * p[7] + p[6] * n->x) + n->z * p[8]) - n->d <= g_floatZero)
			continue;
		if (((n->y * p[10] + p[9] * n->x) + n->z * p[11]) - n->d <= g_floatZero)
			continue;
		int i = 0;
		for (; i < 4; i++) {
			OccluderPlane *s = &n->s[i];
			int k = 0;
			for (; k < 4; k++) {
				float *q = e->m_box + 3 * k;
				if (((s->x * q[0] + s->y * q[1]) + s->z * q[2]) - s->d <= g_floatZero)
					break;
			}
			if (k < 4)
				break;
		}
		if (i == 4)
			return true;
	}
	return false;
}

struct V3f { float x, y, z; };
struct BoxF { V3f min, max; };

// 0x4133e0
bool Box413::IsOccluded()
{
	for (OccluderNode *n = g_planeList413; n; n = n->m_next) {
		float t1, t0, t2;
		t1 = n->y * m_corners[1]; t0 = m_corners[0] * n->x; t2 = n->z * m_corners[2];
		if (!(((t1 + t0) + t2) - n->d > 0.0f)) continue;
		t1 = m_corners[4] * n->y; t0 = m_corners[3] * n->x; t2 = m_corners[5] * n->z;
		if (!(((t1 + t0) + t2) - n->d > 0.0f)) continue;
		t1 = m_corners[7] * n->y; t0 = m_corners[6] * n->x; t2 = m_corners[8] * n->z;
		if (!(((t1 + t0) + t2) - n->d > 0.0f)) continue;
		t1 = m_corners[10] * n->y; t0 = m_corners[9] * n->x; t2 = m_corners[11] * n->z;
		if (!(((t1 + t0) + t2) - n->d > 0.0f)) continue;
		int i = 0;
		OccluderPlane *s = n->s;
		for (; i < 4; i++, s++) {
			t0 = s->x * m_corners[0]; t1 = s->y * m_corners[1]; t2 = s->z * m_corners[2];
			if (!(((t0 + t1) + t2) - s->d > 0.0f)) break;
			t1 = m_corners[4] * s->y; t0 = m_corners[3] * s->x; t2 = m_corners[5] * s->z;
			if (!(((t1 + t0) + t2) - s->d > 0.0f)) break;
			t1 = m_corners[7] * s->y; t0 = m_corners[6] * s->x; t2 = m_corners[8] * s->z;
			if (!(((t1 + t0) + t2) - s->d > 0.0f)) break;
			t1 = m_corners[10] * s->y; t0 = m_corners[9] * s->x; t2 = m_corners[11] * s->z;
			if (!(((t1 + t0) + t2) - s->d > 0.0f)) break;
		}
		if (i == 4)
			return true;
	}
	return false;
}

// 0x413580
void BuildOccluderPlanes(CamB *cam, float *boxes, unsigned count4, OccluderNode *buf)
{
	g_planeList413 = 0;
	// the camera is a real RwCamera (its frame and frustum planes are not at the 32-bit offsets of CamB on x64)
	float *eye = &RwFrameGetLTM(RwCameraGetFrame((RwCamera *)cam))->pos.x;
	const CamPlane *camPlanes = (const CamPlane *)port_RwCameraFrustumPlanes((const RwCamera *)cam);
	OccluderNode *head = 0;
	OccluderNode *cur = buf;
	for (unsigned i = 0; i < count4; i += 4, boxes += 12) {
		int k = 6;
		const CamPlane *p = camPlanes;
		for (;;) {
			k--;
			if (((boxes[1] * p->y + boxes[0] * p->x) + p->z * boxes[2]) - p->d <= g_floatZero ||
				((boxes[3] * p->x + boxes[4] * p->y) + boxes[5] * p->z) - p->d <= g_floatZero ||
				((boxes[6] * p->x + boxes[7] * p->y) + boxes[8] * p->z) - p->d <= g_floatZero ||
				((boxes[10] * p->y + boxes[9] * p->x) + p->z * boxes[11]) - p->d <= g_floatZero) {
				p++;
				if (k == 0) {
					k = -1;
					break;
				}
				continue;
			}
			break;
		}
		cur->m_box = boxes;
		if (k >= 0)
			continue;
		if (InsideList(cur))
			continue;
		float e1x = boxes[3] - boxes[0], e1y = boxes[4] - boxes[1], e1z = boxes[5] - boxes[2];
		float e2x = boxes[6] - boxes[0], e2y = boxes[7] - boxes[1], e2z = boxes[8] - boxes[2];
		cur->x = e1y * e2z - e2y * e1z;
		cur->y = e2x * e1z - e2z * e1x;
		cur->z = e1x * e2y - e1y * e2x;
		RwV3dNormalize((RwV3d *)&cur->x, (RwV3d *)&cur->x);
		cur->d = (boxes[1] * cur->y + cur->x * boxes[0]) + boxes[2] * cur->z;
		float s = ((eye[0] * cur->x + eye[1] * cur->y) + cur->z * eye[2]) - cur->d;
		if (g_floatZero == s)
			continue;
		if (s > g_floatZero) {
			cur->x = -cur->x;
			cur->y = -cur->y;
			cur->z = -cur->z;
			cur->d = -cur->d;
		}
		OccluderPlane *sp = cur->s;
		float *pp = boxes;
		float *qq = boxes + 9;
		for (int j = 0; j < 4; j++, sp++) {
			float vx, vy, vz;
			if (s < g_floatZero) {
				vx = qq[0] - pp[0];
				vy = qq[1] - pp[1];
				vz = qq[2] - pp[2];
			} else {
				vx = pp[0] - qq[0];
				vy = pp[1] - qq[1];
				vz = pp[2] - qq[2];
			}
			float wx = pp[0] - eye[0], wy = pp[1] - eye[1], wz = pp[2] - eye[2];
			sp->x = vy * wz - wy * vz;
			sp->y = wx * vz - wz * vx;
			sp->z = vx * wy - vy * wx;
			RwV3dNormalize((RwV3d *)&sp->x, (RwV3d *)&sp->x);
			sp->d = (eye[1] * sp->y + sp->x * eye[0]) + sp->z * eye[2];
			qq = pp;
			pp += 3;
		}
		cur->m_next = head;
		head = cur;
		cur++;
	}
	OccluderNode *e = head;
	while (e) {
		OccluderNode *next = e->m_next;
		if (!InsideList(e)) {
			e->m_next = g_planeList413;
			g_planeList413 = e;
		}
		e = next;
	}
}

// 0x4138e0
int OccluderTestSphere(RwCamera *camera, SphereP413 *sphere, char enable)
{
	if (enable) {
		for (OccluderNode *n = g_planeList413; n; n = n->m_next) {
			if (n->z * sphere->z + (n->x * sphere->x + n->y * sphere->y) - n->d <= sphere->r)
				continue;
			int i = 0;
			OccluderPlane *p = n->s;
			for (; i < 4; i++, p++) {
				if (p->z * sphere->z + (p->x * sphere->x + p->y * sphere->y) - p->d <= sphere->r)
					break;
			}
			if (i == 4)
				return 0;
		}
	}
	return 2;
}

// 0x413970
int OccluderTestBox(RwCamera *camera, const RwBBox *rwBox, char enable)
{
	const BoxF *box = (const BoxF *)rwBox;   // min / max as in the game code; an RwBBox stores sup first
	if (enable) {
		OccluderNode *n = g_planeList413;
		if (n) {
			V3f c;
			c.x = box->max.x + box->min.x;
			c.y = box->max.y + box->min.y;
			c.z = box->max.z + box->min.z;
			c.x *= 0.5f;
			c.y *= 0.5f;
			c.z *= 0.5f;
			V3f e;
			e.x = box->min.x - c.x;
			e.y = box->min.y - c.y;
			e.z = box->min.z - c.z;
			for (; n; n = n->m_next) {
				if (!(c.z * n->z + (c.y * n->y + c.x * n->x) - n->d <= fabs(e.x * n->x) + (fabs(e.z * n->z) + fabs(e.y * n->y)))) {
					int i = 0;
					float *q = &n->s[0].z;
					for (; i < 4; i++, q += 4) {
						if (c.z * q[0] + (c.x * q[-2] + c.y * q[-1]) - q[1] <= fabs(e.z * q[0]) + fabs(e.y * q[-1]) + fabs(e.x * q[-2]))
							break;
					}
					if (i == 4)
						return 0;
				}
			}
		}
	}
	return 2;
}
