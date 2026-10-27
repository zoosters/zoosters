#include <math.h>
#include "modules/tfb/gamecamera.h"
#include "framework/startup/win32/appstate.h"
#include "modules/tfb/camera_views.h"
#include "modules/tfb/gameactor.h"
#include "tfb/corpus/corpus.h"
#include "tfb/math/vector_ops.h"
#include "script/actor.h"
#include "script/types/scriptsystem.h"
#include "script/types/scriptsound.h"
#include "script/types/scriptfog.h"
#include "script/types/scriptcamera.h"
#include "rw/rphanim.h"
#include "rw/rtanim.h"

// GLOBAL: 0x5cd030 g_camAxisX
// GLOBAL: 0x5cd03c g_camAxisY
// GLOBAL: 0x5cd048 g_camAxisZ
// GLOBAL: 0x617bcc g_camDirectTurn
// GLOBAL: 0x61ff58 g_camLookAt
// GLOBAL: 0x618318 g_frameDt

extern const RwV3d g_camAxisX, g_camAxisY, g_camAxisZ; // the unit axes (the camera is rotated about them)
extern RwV3d g_camLookAt;                                // the eye of the last update (read by the script's camera location)
extern float g_frameDt;                                  // seconds of the last frame
#ifdef ZOOSTERS_WIDESCREEN
extern "C" int port_widescreen_view_window(void *camera, float zoom);   // port/fixes/widescreen.cpp (PORT_WIDESCREEN)
#endif

int *CutScene_StackSlot(int delta);                   // 0x436c00
void *ScriptCamera_DefaultLookAtSlot();   // 0x438910
float GetPlayerInput(int input, float threshold, int player);   // 0x402880 input axis
float Math_EaseInOut(float t, float easeIn, float easeOut);
float LerpFloat(float t, float a, float b);          // 0x464b10 (1 - t) * b + t * a
float AngleDelta(float a, float b);                  // 0x44aa00 b - a wrapped to [-180, 180)
namespace RWS { namespace CameraHelper { void SetFieldOfView(float zoom, RwCamera *camera); } }   // 0x44ea40 sets the camera's view window from the tangent of the half field of view

// 0x41f790
// Poses the camera matrix from the animation's node matrices. `frame` is the camera's frame (the hierarchy is attached to it while it updates).
void RWS::TFB::GameCamera::ApplyCameraAnimation(RwFrame *frame)
{
	int numNodes = RtAnimAnimationGetNumNodes(g_camAnim);
	RpHAnimFrameSetHierarchy(frame, g_camHAnim);
	RpHAnimHierarchyAttachFrameIndex(g_camHAnim, numNodes - 1);
	bool atEnd = !(g_frameDt + InterpolatorTime(HierarchyInterpolator(g_camHAnim)) < AnimationDuration(g_camAnim));
	if (!atEnd && *CutScene_StackSlot(0) == -1) { // outside cut-scenes the camera animation jumps to its end
		g_camModeAux = 0;
		g_camTimeLeft = 0.0f;
		atEnd = true;
	}
	if (atEnd)
		RtAnimInterpolatorSetCurrentTime(HierarchyInterpolator(g_camHAnim), AnimationDuration(g_camAnim));
	else
		RtAnimInterpolatorAddAnimTime(HierarchyInterpolator(g_camHAnim), g_frameDt);
	RpHAnimHierarchyUpdateMatrices(g_camHAnim);
	RpHAnimFrameSetHierarchy(frame, 0);
	RwMatrixTransform(&m_matrix, HierarchyMatrices(g_camHAnim), rwCOMBINEREPLACE);
	for (int i = 1; i < numNodes; i++)
		RwMatrixTransform(&m_matrix, HierarchyMatrices(g_camHAnim) + i, rwCOMBINEPOSTCONCAT);
}

// 0x41f8a0
void RWS::TFB::GameCamera::RebuildAnimHierarchy()
{
	if (g_camHAnim)
		RpHAnimHierarchyDestroy(g_camHAnim);
	int numNodes = RtAnimAnimationGetNumNodes(g_camAnim);
	g_camHAnim = RpHAnimHierarchyCreate(numNodes, g_camNodeFlags, 0,
	                                    (RpHAnimHierarchyFlag)(rpHANIMHIERARCHYUPDATELTMS | rpHANIMHIERARCHYUPDATEMODELLINGMATRICES), 0x24);
	RtAnimInterpolatorSetCurrentAnim(HierarchyInterpolator(g_camHAnim), g_camAnim);
	RtAnimInterpolatorSetCurrentTime(HierarchyInterpolator(g_camHAnim), 0.0f);
}

// 0x41e970
// Keeps P (the actor's head) inside the view frustum: when P is closer than -h to a side plane (through the near / far corner pairs), tilts
// the camera about its
// local X axis by asin of the signed distance ratio (clamped to +-90 deg). The game passes cam / m / P in eax / esi / edi and h on the stack.
static __declspec(noinline) void KeepPointInFrustum(RwCamera *camera, RwMatrix *m, const RwV3d *P, float h)
{
	const RwCameraView *cam = CameraView(camera);
	float ox = -cam->viewOffset.x;
	float oy = cam->viewOffset.y;
	RwV3d V;
	V.x = oy * m->up.x + ox * m->right.x;
	V.y = m->up.y * oy + ox * m->right.y;
	V.z = oy * m->up.z + ox * m->right.z;
	RwV3d a, b, c1, c2, c3, c4;
	float wx = cam->viewWindow.x, wy = cam->viewWindow.y;
	a.x = wx * m->right.x;
	a.y = wx * m->right.y;
	a.z = wx * m->right.z;
	b.x = wy * m->up.x;
	b.y = wy * m->up.y;
	b.z = wy * m->up.z;
	c1.x = b.x + (a.x + m->at.x);
	c1.y = b.y + (a.y + m->at.y);
	c1.z = b.z + (a.z + m->at.z);
	c2.x = c1.x - (a.x + a.x);
	c2.y = c1.y - (a.y + a.y);
	c2.z = c1.z - (a.z + a.z);
	c3.x = c2.x - (b.x + b.x);
	c3.y = c2.y - (b.y + b.y);
	c3.z = c2.z - (b.z + b.z);
	c4.x = c3.x + (a.x + a.x);
	c4.y = c3.y + (a.y + a.y);
	c4.z = c3.z + (a.z + a.z);
	RwV3d Q;
	Q.x = V.x + m->pos.x;
	Q.y = V.y + m->pos.y;
	Q.z = V.z + m->pos.z;
	float nearP = cam->nearPlane, farP = cam->farPlane;
	RwV3d d, F1, F2, F3, F4, N2, N4;
	d = V3SubInline(&c1, &V);
	V3ScaleInline(&F1, &d, farP); V3AddToInline(&F1, &Q);
	d = V3SubInline(&c2, &V);
	V3ScaleInline(&N2, &d, nearP); V3AddToInline(&N2, &Q);
	V3ScaleInline(&F2, &d, farP); V3AddToInline(&F2, &Q);
	d = V3SubInline(&c3, &V);
	V3ScaleInline(&F3, &d, farP); V3AddToInline(&F3, &Q);
	d = V3SubInline(&c4, &V);
	V3ScaleInline(&N4, &d, nearP); V3AddToInline(&N4, &Q);
	V3ScaleInline(&F4, &d, farP); V3AddToInline(&F4, &Q);
	// plane 1 through N2, F2, F1; plane 2 through N4, F4, F3 (unit normals n1, n2)
	RwV3d e, g, n1, n2;
	e = V3SubInline(&N2, &F2);
	g = V3SubInline(&F1, &F2);
	n1.x = g.y * e.z - g.z * e.y;
	n1.y = g.z * e.x - g.x * e.z;
	n1.z = g.x * e.y - g.y * e.x;
	float inv = _rwInvSqrt(n1.y * n1.y + n1.x * n1.x + n1.z * n1.z);
	n1.x = n1.x * inv; n1.y = n1.y * inv; n1.z = n1.z * inv;
	float d1 = n1.y * N2.y + n1.x * N2.x + n1.z * N2.z;
	e = V3SubInline(&N4, &F4);
	g = V3SubInline(&F3, &F4);
	n2.x = g.y * e.z - g.z * e.y;
	n2.y = g.z * e.x - g.x * e.z;
	n2.z = g.x * e.y - g.y * e.x;
	inv = _rwInvSqrt(n2.y * n2.y + n2.x * n2.x + n2.z * n2.z);
	n2.x = n2.x * inv; n2.y = n2.y * inv; n2.z = n2.z * inv;
	float dist = (n2.y * P->y + n2.x * P->x + n2.z * P->z) - (n2.y * N4.y + n2.x * N4.x + n2.z * N4.z);
	float nh = -h;
	RwV3d w;
	float r, ang;
	if (dist > nh) {
		w.x = P->x - m->pos.x;
		w.y = P->y - m->pos.y;
		w.z = P->z - m->pos.z;
		w.x = n2.x * h + w.x;
		w.y = n2.y * h + w.y;
		w.z = n2.z * h + w.z;
		dist = dist + h;
		r = dist / RwV3dLength(&w);
		if (r <= -1.0f)
			ang = -90.0f;
		else if (r >= 1.0f)
			ang = 90.0f;
		else
			ang = (float)asin(r) * 57.295776f;
		RwMatrixRotate(m, &g_camAxisX, ang, rwCOMBINEPRECONCAT);
	} else {
		float s1 = (n1.y * P->y + n1.x * P->x + n1.z * P->z) - d1;
		if (s1 > nh) {
			w.x = P->x - m->pos.x;
			w.y = P->y - m->pos.y;
			w.z = P->z - m->pos.z;
			w.x = n1.x * h + w.x;
			w.y = n1.y * h + w.y;
			w.z = n1.z * h + w.z;
			RwV3dLength(&w);
			s1 = s1 + h;
			r = s1 / RwV3dLength(&w);
			if (r <= -1.0f)
				ang = -90.0f;
			else if (r >= 1.0f)
				ang = 90.0f;
			else
				ang = (float)asin(r) * 57.295776f;
			RwMatrixRotate(m, &g_camAxisX, -ang, rwCOMBINEPRECONCAT);
		}
	}
}

// ---- UpdateGameCamera (one function in the game; split into the steps below)
using RWS::TFB::GameActor;

// The actor the lazy cut-scene camera looks at: its collider moved this tick
static bool ActorMoves(const GameActor *a)
{
	return !(a->m_velocity.x == g_floatZero && a->m_velocity.y == g_floatZero && a->m_velocity.z == g_floatZero);
}

// The holds after a collision turned the camera: for 60 frames (g_camPitchHold pitch, g_camYawHold yaw; the sign is the
// direction) input in the same direction is ignored. Counts the hold towards 0; true when it ends (input against it, or run out).
static bool HoldEnds(int &hold, float input)
{
	if (hold < 0) {
		if (input > g_floatZero)
			return true;
		return ++hold == 0;
	}
	if (hold > 0) {
		if (input < g_floatZero)
			return true;
		return --hold == 0;
	}
	return false;
}

// Moves the eye (m_matrix.pos) and the focus towards the script camera's targets by the follow rate k: during a timed transition
// (eased blend from the snap point, or a fade towards the target), a snap, the lazy cut-scene follow (keeps the distance while the
// looked-at actor moves), or a plain lerp. Returns the actor of the lazy follow (0: none).
static GameActor *FollowTargets(RWS::TFB::GameCamera *cam, const Script::ScriptCamera *sc, float k)
{
	RwV3d *eye = (RwV3d *)&cam->m_matrix.pos;
	RwV3d *focus = (RwV3d *)&cam->m_focus;
	GameActor *actor = 0;
	bool lerp = false;
	if (g_camTimeLeft > g_floatZero) {
		g_camModeAux = 0;
		g_camTimeLeft = g_camTimeLeft - g_frameDt;
		if (g_camTimeLeft < g_floatZero)
			g_camTimeLeft = 0;
		float el = g_camTimeTotal - g_camTimeLeft;
		if (el < g_camTimeStart) {
			g_camModeAux = g_camMode;
			g_camFade = el / g_camTimeStart;
			if (g_camMode == 0) {   // eased blend from the snap point to the targets
				float f = Math_EaseInOut(g_camFade, 0.15f, 0.15f);
				{
					float dx = g_camTgt.x - g_camSnapTgt.x;
					float dy = g_camTgt.y - g_camSnapTgt.y;
					float dz = g_camTgt.z - g_camSnapTgt.z;
					cam->m_matrix.pos.x = dx * f + g_camSnapTgt.x;
					cam->m_matrix.pos.y = dy * f + g_camSnapTgt.y;
					cam->m_matrix.pos.z = dz * f + g_camSnapTgt.z;
				}
				{
					RwV3d d;
					d.x = g_camPos.x - g_camSnapPos.x;
					d.y = g_camPos.y - g_camSnapPos.y;
					d.z = g_camPos.z - g_camSnapPos.z;
					cam->m_focus.x = d.x * f + g_camSnapPos.x;
					cam->m_focus.y = d.y * f + g_camSnapPos.y;
					cam->m_focus.z = d.z * f + g_camSnapPos.z;
				}
				if (g_camSnapPending)
					g_camSnapPending = 0;
			} else {   // a fade: follow, the fade runs backwards
				g_camFade = 1.0f - g_camFade;
				if (g_camSnapPending) {
					*eye = g_camTgt;
					*focus = g_camPos;
				}
				Vec3LerpInPlace(eye, &g_camTgt, k);
				Vec3LerpInPlace(focus, &g_camPos, k);
				g_camFade = Math_EaseInOut(g_camFade, 0.15f, 0.15f);
			}
		} else {   // the end of the transition: the added part fades out, the camera follows
			if (g_camTimeLeft < g_camAddAmount) {
				g_camModeAux = g_camAddKind;
				if (g_camAddKind == 0)
					g_camFade = 0;
				else {
					g_camFade = 1.0f - g_camTimeLeft / g_camAddAmount;
					g_camFade = Math_EaseInOut(g_camFade, 0.15f, 0.15f);
				}
			}
			lerp = true;
		}
	} else if (g_camSnapPending) {   // snap
		*focus = g_camPos;
		*eye = g_camTgt;
	} else if (*CutScene_StackSlot(0) == 2 && (sc->m_flags & 1)) {   // lazy: the distance changes slowly
		RwV3d d;
		d.x = g_camTgt.x - g_camPos.x;
		d.y = g_camTgt.y - g_camPos.y;
		d.z = g_camTgt.z - g_camPos.z;
		float len1 = RwV3dLength(&d);
		d.x = eye->x - cam->m_focus.x;
		d.y = eye->y - cam->m_focus.y;
		d.z = eye->z - cam->m_focus.z;
		float len2 = RwV3dLength(&d);
		float lenT = (len1 - len2) * k + len2;
		actor = (GameActor *)*(Script::ScriptReference **)ScriptCamera_DefaultLookAtSlot();
		if (actor && actor->m_corpus == 0)
			actor = 0;
		float c;
		if (actor && ActorMoves(actor)) {
			g_camPitchHold = 0;
			g_camYawHold = 0;
			c = 0.75f;
		} else if (lenT > len2)
			c = 0.95f;
		else
			c = 0.75f;
		lenT = (len2 - lenT) * c + lenT;
		if (g_camYawHold == 0 && g_camPitchHold == 0 && fabs(lenT - len2) > 0.01f) {
			float ratio = lenT / len2;
			d.x = d.x * ratio;
			d.y = d.y * ratio;
			d.z = d.z * ratio;
			eye->x = d.x + cam->m_focus.x;
			eye->y = d.y + cam->m_focus.y;
			eye->z = d.z + cam->m_focus.z;
		}
		cam->m_focus.x = g_camPos.x;
		cam->m_focus.y = (g_camPos.y - cam->m_focus.y) * k * k + cam->m_focus.y;
		cam->m_focus.z = g_camPos.z;
	} else {
		lerp = true;
	}
	if (lerp) {
		Vec3LerpInPlace(eye, &g_camTgt, k);
		Vec3LerpInPlace(focus, &g_camPos, k);
	}
	return actor;
}

// The collision resolution of UpdateGameCamera: the camera probe's hit fractions per side (f: 0 right, 1 left, 2 below, 3
// above), the sides hit now (mask) and in the round before (prevMask), the sides handled (state), the pitch correction of the
// round (adj) and the extra one when all four sides hit, the player's input (ax yaw, ay pitch), the pitch and heading at the start,
// the distance.
struct CamResolve {
	int mask, state, prevMask, iter;
	float f[4];
	float extra, adj;
	float pitch, pitchTarget, pitch0, heading0, dist;
	float ax, ay;
	unsigned char moved, b8;
};

// A side hit (0 right, 1 left): the camera turns away from it by up to half a degree per round (or, turning into it, the player's
// turn is held and the camera pulls in instead).
static void ResolveSide(RWS::TFB::GameCamera *cam, CamResolve &r, int side)
{
	const int bit = side == 0 ? 1 : 2;
	const int otherBit = side == 0 ? 2 : 1;
	float &f = r.f[side];
	// turning into the hit side
	bool turning = side == 0 ? cam->m_yawStep < g_floatZero : cam->m_yawStep > g_floatZero;
	if (!(r.state & bit) && turning) {
		r.state |= bit;
		r.mask &= ~otherBit;
		if (r.ay > g_floatZero) {
			cam->m_heading = r.heading0;
		} else if ((side == 0 ? r.ax < g_floatZero : r.ax > g_floatZero) && !(r.dist < 16.0f)) {
			if ((1.0f - f) * r.dist > 2.0f)
				f = (r.dist - 2.0f) / r.dist;
			r.mask = bit;
			r.moved = 1;
			g_camYawHold = side == 0 ? -60 : 60;
		} else {
			cam->m_heading = r.heading0;
			g_camYawHold = side == 0 ? -60 : 60;
		}
		return;
	}
	int other = r.mask & otherBit;
	if (side == 0) {
		if (other) {
			if (!(r.state & 2) && cam->m_yawStep > g_floatZero)
				return;
			if (r.f[1] <= r.f[0])
				return;
		}
	} else if (other && ((!(r.state & 1) && cam->m_yawStep < g_floatZero) || !(r.f[0] > r.f[1]))) {
		// both sides hit and the right one counts: the left hit becomes a hit from above (unless below/above already hit)
		if (r.mask & 0xc)
			return;
		r.mask |= 8;
		r.f[3] = (r.f[1] < r.f[0]) ? r.f[0] : r.f[1];
		return;
	}
	if (r.state != 0 || other != 0) {
		if (f > 0.98f) {   // barely hit: pull in
			r.mask &= ~otherBit;
			r.state |= bit;
			r.moved = 1;
			return;
		}
	}
	r.state |= bit;
	r.mask &= ~otherBit;
	float a = (float)(atan2(0.5, (double)(f * r.dist)) * 57.295776f);
	if (a > 0.5f)
		a = 0.5f;
	RwMatrixRotate(&cam->m_matrix, &cam->m_matrix.up, side == 0 ? a : -a, rwCOMBINEPOSTCONCAT);
	float r1 = AngleDelta(r.heading0, cam->m_heading);
	cam->m_heading = (float)(atan2(cam->m_matrix.at.x, cam->m_matrix.at.z) * 57.295776f) + 180.0f;
	if (side == 0 ? r1 < g_floatZero : r1 > g_floatZero) {   // do not turn past the start heading
		if (side == 0 ? AngleDelta(r.heading0, cam->m_heading) > g_floatZero : AngleDelta(r.heading0, cam->m_heading) < g_floatZero)
			cam->m_heading = r.heading0;
	}
}

// A hit from below (side 2): pitch up by 0.25..0.5 degrees per round (with the hold of the player's pitch input). True when the
// resolution ends (looking straight down and no side hit, or both).
static bool ResolveBelow(RWS::TFB::GameCamera *cam, CamResolve &r)
{
	float a1 = (float)(atan2(0.5, (double)(r.f[2] * r.dist)) * 57.295776f);
	if (r.f[2] > 0.9f) {
		if (!(r.state & 4) && r.ay > g_floatZero) {
			g_camPitchHold = 60;
			r.moved = 1;
			return false;
		}
		if (r.ay == g_floatZero && r.ax == g_floatZero && (r.state & 8)) {
			cam->m_pitchOffset = r.pitch0;
			r.moved = 1;
			return false;
		}
	}
	if (r.b8 && r.ay > g_floatZero) {
		if ((1.0f - r.f[2]) * r.dist > 4.0f)
			r.f[2] = 0.975f;
		r.moved = 1;
		g_camPitchHold = 60;
		r.b8 = 0;
		return false;
	}
	if (a1 < 0.25f)
		a1 = 0.25f;
	else if (a1 > 0.5f)
		a1 = 0.5f;
	r.adj = -a1;
	r.state |= 4;
	if (r.mask & 8) {
		r.adj = r.adj - 2.0f;
		r.extra = r.adj;
		return false;
	}
	return r.pitch <= -89.5f && ((r.mask & 3) == 0 || (r.mask & 3) == 3);
}

// A hit from above (side 3): pitch down by up to 0.5 degrees per round; mirror of ResolveBelow.
static bool ResolveAbove(RWS::TFB::GameCamera *cam, CamResolve &r)
{
	float a2 = (float)(atan2(0.5, (double)(r.f[3] * r.dist)) * 57.295776f);
	if (r.f[3] > 0.9f) {
		if (!(r.state & 8) && r.ay < g_floatZero) {
			g_camPitchHold = -60;
			r.moved = 1;
			return false;
		}
		if (r.ay == g_floatZero && r.ax == g_floatZero && (r.state & 4)) {
			cam->m_pitchOffset = r.pitch0;
			r.moved = 1;
			return false;
		}
	}
	if (r.b8 && r.ay < g_floatZero) {
		if ((1.0f - r.f[3]) * r.dist > 4.0f)
			r.f[3] = 0.975f;
		r.moved = 1;
		g_camPitchHold = -60;
		r.b8 = 0;
		return false;
	}
	r.state |= 8;
	if (a2 > 0.5f)
		a2 = 0.5f;
	r.adj = a2 + r.adj;
	if (r.mask == 0xf) {
		r.adj = r.adj + 2.0f;
		r.extra = r.adj;
		return false;
	}
	return r.pitch >= 89.5f && ((r.mask & 3) == 0 || (r.mask & 3) == 3);
}

// Side 3's dispatch: with all four sides hit, a hit from above that is not clearly farther than the one from below ends the
// round (or undoes the extra correction of the round before); a hit from above that is closer than the one from below is
// handled as a hit from below.
static bool ResolveVertical(RWS::TFB::GameCamera *cam, CamResolve &r)
{
	if (r.mask == 0xf) {
		if (!(r.f[2] - r.f[3] > -0.02f)) {
			r.moved = 1;
			return false;
		}
		if (r.prevMask == 0xf && r.extra < g_floatZero) {
			r.moved = 1;
			r.adj = -r.extra;
			return false;
		}
		return ResolveAbove(cam, r);
	}
	if ((r.mask & 4) && !(r.f[2] - r.f[3] > -0.02f)) {
		r.mask &= ~8;
		return ResolveBelow(cam, r);
	}
	return ResolveAbove(cam, r);
}

// The round's pitch correction: not past the start pitch; when it would push the pitch over (or the player pushes against it),
// it waits for the next round (g_camPitchDeferred), else it goes into the stored pitch at once.
static void FoldPitchCorrection(RWS::TFB::GameCamera *cam, CamResolve &r)
{
	if (r.adj > g_floatZero) {
		if (cam->m_pitchOffset < r.pitch0 && r.adj + cam->m_pitchOffset > r.pitch0)
			r.adj = r.pitch0 - cam->m_pitchOffset;
	} else if (r.adj < g_floatZero) {
		if (cam->m_pitchOffset > r.pitch0 && r.adj + cam->m_pitchOffset < r.pitch0)
			r.adj = r.pitch0 - cam->m_pitchOffset;
	}
	bool wait;
	if (r.adj > g_floatZero)
		wait = !(cam->m_pitchOffset < g_floatZero) || r.ay < g_floatZero;
	else if (r.adj < g_floatZero)
		wait = cam->m_pitchOffset <= g_floatZero || r.ay > g_floatZero;
	else
		wait = false;
	if (wait) {
		g_camPitchDeferred = 1;
	} else if (r.adj != g_floatZero) {
		float t = r.adj;
		r.adj = 0;
		cam->m_pitchOffset = t + cam->m_pitchOffset;
		cam->m_pitchInput = cam->m_pitchOffset;
	}
}

// Per frame: the camera follows its targets, orbits the anchor (the looked-at actor's chest, else the focus) by the player's
// stick (lazy cameras), resolves collisions with up to 100 rounds of the 4-sided probe (turn away from side hits, pitch away from
// hits below/above, pull in), then looks at the focus, keeps the actor in the frustum in cut-scenes and applies the script roll.
// Near-match of 0x4207a0: FPU scheduling of 3-component differences (the game spills the third / all three), actor kept in ebx (the game reloads it),
// frame slots off by 4, layout of case 3 / side 2 / side 3.
void RWS::TFB::GameCamera::UpdateGameCamera(RwCamera *camera)
{
	const Script::ScriptCamera *sc = static_cast<const Script::ScriptCamera *>(GetCameraObject());
	if (!sc)
		return;
	if (*CutScene_StackSlot(0) == 3 && g_gameCamValid)
		return;
	if (CameraView(camera)->frameBuffer) {
		float half = sc->m_fov * 0.5f;
		RWS::CameraHelper::SetFieldOfView((float)tan(half * 0.017453292f), camera);
#ifdef ZOOSTERS_WIDESCREEN   // widescreen port build only (port/fixes/widescreen.cpp): Hor+ view window from the raster's aspect
		port_widescreen_view_window(camera, (float)tan(half * 0.017453292f));
#endif
		g_gameCamValid = 1;
	}
	float k = (float)sc->m_horzFollow * 0.01f;
	GameActor *actor = FollowTargets(this, sc, k);
	RwV3d *eye = (RwV3d *)&m_matrix.pos;

	// ---- the anchor P (the actor's chest: its box centre, raised by a quarter of its height when it is taller than wide) and the
	// angles from it to the eye
	float ty = 0;
	RwV3d P;
	if (actor) {
		const RwReal *extent = actor->m_worldBox.m_extent;
		float v;
		ty = (extent[2] + extent[0]) * 0.5f;
		if (extent[1] > extent[0] && extent[1] > extent[2])
			v = extent[1] * 0.25f;
		else
			v = g_floatZero;
		RwV3d scaled;
		scaled.x = v * actor->m_worldBox.m_frame.up.x;
		scaled.y = v * actor->m_worldBox.m_frame.up.y;
		scaled.z = v * actor->m_worldBox.m_frame.up.z;
		P = V3Add((const RwV3d *)&actor->m_worldBox.m_frame.pos, &scaled);
	} else {
		P = *(RwV3d *)&m_focus;
	}
	RwV3d d;
	d.x = eye->x - P.x;
	d.y = m_matrix.pos.y - P.y;
	d.z = m_matrix.pos.z - P.z;
	float ang = (float)(atan2(d.x, d.z) * 57.295776f);
	if (fabs(AngleDelta(ang, m_heading)) > 0.01f)
		m_heading = ang;
	CamResolve r;
	r.heading0 = m_heading;
	r.pitch = (float)(atan2(d.y, _rwSqrt(d.z * d.z + d.x * d.x)) * 57.295776f);
	r.dist = RwV3dLength(&d);
	r.ax = 0;
	r.ay = 0;

	// ---- the player's stick (lazy cameras): pitch and yaw inputs, smoothed; the holds after collisions
	if (sc->m_flags & 1) {
		if (g_camTimeLeft == g_floatZero) {
			r.ax = GetPlayerInput(0x11, 0.4f, 0);
			if (r.ax < g_floatZero)
				r.ax = (r.ax + 0.4f) * -1.6666666f;
			else if (r.ax > g_floatZero)
				r.ax = (r.ax - 0.4f) * -1.6666666f;
			if (!((sc->m_flags >> 1) & 1)) {
				r.ay = GetPlayerInput(0x12, 0.4f, 0);
				if (r.ay < g_floatZero)
					r.ay = (r.ay + 0.4f) * 1.6666666f;
				else if (r.ay > g_floatZero)
					r.ay = (r.ay - 0.4f) * 1.6666666f;
			}
		}
		d.x = g_camTgt.x - P.x;
		d.z = g_camTgt.z - P.z;
		d.y = (m_focus.y - g_camPos.y) + (g_camTgt.y - P.y);
		r.pitchTarget = (float)(atan2(d.y, _rwSqrt(d.z * d.z + d.x * d.x)) * 57.295776f);
		if (!g_camSnapPending) {
			if (g_camTgtPrev.y - g_camPosPrev.y != g_camTgtCur.y - g_camPosCur.y) {   // the script moved the targets' height
				g_camPitchDeferred = 0;
				d.x = g_camTgtPrev.x - g_camPosPrev.x;
				d.y = g_camTgtPrev.y - g_camPosPrev.y;
				d.z = g_camTgtPrev.z - g_camPosPrev.z;
				float p1 = (float)(atan2(d.y, _rwSqrt(d.z * d.z + d.x * d.x)) * 57.295776f);
				m_pitchOffset += p1;
				m_pitchInput += p1;
				d.x = g_camTgtCur.x - g_camPosCur.x;
				d.y = g_camTgtCur.y - g_camPosCur.y;
				d.z = g_camTgtCur.z - g_camPosCur.z;
				float p2 = -(float)(atan2(d.y, _rwSqrt(d.z * d.z + d.x * d.x)) * 57.295776f);
				m_pitchOffset += p2;
				m_pitchInput += p2;
			}
		}
		if (g_camPitchDeferred
			&& (r.ay == g_floatZero || (r.ay < g_floatZero && m_pitchOffset >= g_floatZero)
				|| (r.ay > g_floatZero && m_pitchOffset <= g_floatZero))
			&& (actor == 0 || !ActorMoves(actor)))
			m_pitchInput = m_pitchOffset;
		else if (r.ay == g_floatZero && g_camPitchHold != 0)
			m_pitchInput = m_pitchOffset;
		else {
			float range;
			if (r.ay > g_floatZero)
				range = 75.0f - r.pitchTarget;
			else
				range = r.pitchTarget + 40.0f;
			r.ay = range * r.ay;
			if (!g_camDirectTurn || r.ay != g_floatZero)
				m_pitchInput = LerpFloat(0.97f, m_pitchInput, r.ay);
			m_pitchOffset = LerpFloat(0.2f, m_pitchInput, m_pitchOffset);
			if (fabs(m_pitchOffset) < 0.01f) {
				g_camPitchHold = 0;
				m_pitchOffset = 0;
			}
		}
		if (HoldEnds(g_camPitchHold, r.ay)) {
			g_camYawHold = 0;
			g_camPitchHold = 0;
		}
		r.ax = g_frameDt * 60.0f * r.ax * 1.25f;
		float lv = LerpFloat(0.921500027179718f, m_yawInput, r.ax);
		m_yawInput = lv;
		m_yawStep = LerpFloat(0.2f, lv, m_yawStep);
		if (fabs(m_yawStep) < 0.01f) {
			g_camYawHold = 0;
			m_yawStep = 0;
		} else if (HoldEnds(g_camYawHold, r.ax)) {
			g_camYawHold = 0;
			g_camPitchHold = 0;
		}
		float yawStep;
		if (g_camDirectTurn) {   // direct turning: the player's actor turns with the camera
			if (r.ax != g_floatZero && actor && actor->m_corpus) {
				GameActor *owner = actor->m_corpus->m_owner;
				if (fabs(owner->m_targetFacing - CameraFacing()) > 20.0f)
					owner->m_targetFacing = CameraFacing();
			}
			yawStep = r.ax;
		} else
			yawStep = m_yawStep;
		m_heading += yawStep;
	}

	// ---- collision resolution: up to 100 rounds of the 4-sided probe
	r.mask = 0;
	r.state = 0;
	r.prevMask = 0;
	r.iter = 0;
	r.extra = 0;
	r.adj = 0;
	r.pitch0 = m_pitchOffset;
	r.b8 = g_camPitchDeferred;
	r.moved = 0;
	g_camPitchDeferred = 0;
	for (;;) {
		r.iter++;
		if (sc->m_flags & 1) {
			if (r.iter > 100)
				break;
			m_pitchOffset += r.adj;
			r.adj = 0;
			if (r.mask != 0) {
				bool finished = false;
				for (int i = 0; i <= 3 && !finished; i++) {
					if (!(r.mask & (1 << i)))
						continue;
					if (i <= 1)
						ResolveSide(this, r, i);
					else if (i == 2)
						finished = !(r.mask & 8) && ResolveBelow(this, r);
					else
						finished = ResolveVertical(this, r);
				}
				if (finished)
					break;
				FoldPitchCorrection(this, r);
			}
			r.pitch = r.adj + m_pitchOffset + r.pitchTarget;
			if (r.pitch < -89.5f) {
				r.pitch = -89.5f;
				m_pitchOffset = -89.5f - r.pitchTarget - r.adj;
			} else if (r.pitch > 89.5f) {
				r.pitch = 89.5f;
				m_pitchOffset = 89.5f - r.pitchTarget - r.adj;
			}
			float best = -3.4028235e38f;
			r.prevMask = r.mask;
			if (r.moved) {   // pull in to the nearest hit
				r.moved = 0;
				g_camPitchDeferred = 0;
				if ((r.mask & 1) && r.f[0] > best)
					best = r.f[0];
				if ((r.mask & 2) && r.f[1] > best)
					best = r.f[1];
				if ((r.mask & 4) && r.f[2] > best)
					best = r.f[2];
				if ((r.mask & 8) && r.f[3] > best)
					best = r.f[3];
				if (best != -3.4028235e38f)
					r.dist = best * r.dist;
			} else if (r.iter > 10)
				r.dist = 0.995f * r.dist;
		}
		RwMatrixRotate(&m_matrix, &g_camAxisX, r.pitch, rwCOMBINEREPLACE);
		RwMatrixRotate(&m_matrix, &g_camAxisY, m_heading + 180.0f, rwCOMBINEPOSTCONCAT);
		d.x = -r.dist * m_matrix.at.x + P.x;
		d.y = -r.dist * m_matrix.at.y + P.y;
		d.z = -r.dist * m_matrix.at.z + P.z;
		RwMatrixTranslate(&m_matrix, &d, rwCOMBINEPOSTCONCAT);
		r.mask = CollisionProbe(actor, &P, ty, r.f, r.state);
		if (!r.mask)
			break;
	}

	// ---- look at the focus, the cut-scene frustum, the script roll; publish
	d.x = eye->x - m_focus.x;
	d.y = m_matrix.pos.y - m_focus.y;
	d.z = m_matrix.pos.z - m_focus.z;
	RwMatrixRotate(&m_matrix, &g_camAxisX, (float)(atan2(d.y, _rwSqrt(d.z * d.z + d.x * d.x)) * 57.295776f), rwCOMBINEREPLACE);
	RwMatrixRotate(&m_matrix, &g_camAxisY, (float)(atan2(d.x, d.z) * 57.295776f) + 180.0f, rwCOMBINEPOSTCONCAT);
	d.x = d.x + m_focus.x;
	d.y = d.y + m_focus.y;
	d.z = d.z + m_focus.z;
	RwMatrixTranslate(&m_matrix, &d, rwCOMBINEPOSTCONCAT);
	if ((sc->m_flags & 1) && *CutScene_StackSlot(0) == 2 && actor)
		KeepPointInFrustum(camera, &m_matrix, &P, (ty + actor->m_worldBox.m_extent[1]) * 0.5f);
	RwMatrixRotate(&m_matrix, &g_camAxisZ, sc->m_roll, rwCOMBINEPRECONCAT);
	g_camSnapPending = 0;
	if (g_camAnim == 0)
		g_camValueD0 = r.dist;
	else {
		g_camValueD0 = -3.4028235e38f;
		ApplyCameraAnimation(RwCameraGetFrame(camera));
	}
	if (m_atomic.ptr())
		RwFrameTransform(RpAtomicGetFrame(m_atomic.ptr()), &m_matrix, rwCOMBINEREPLACE);
	g_camLookAt = *eye;
}

// 0x422090
// The camera tick (called after the tick handlers move the follow targets): updates the game camera when both it and its RwCamera exist.
void Camera_Update()
{
	if (g_cameraObject && g_gameCamDst)
		g_cameraObject->UpdateGameCamera(g_gameCamDst);
}
