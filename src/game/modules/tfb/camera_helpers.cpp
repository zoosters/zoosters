#include <math.h>
#include "script/actor.h"
#include "modules/tfb/camera_views.h"
#include "rw/rwcore.h"
#include "script/types/scriptcamera.h"
#include "modules/tfb/cameradata.h"

// GLOBAL: 0x5ca5a8 g_floatZero
extern const float g_floatZero;


// Signed distance of a point along the view axis: |p - pos| with the sign of (p - pos) . at. Works on the frame's LTM as floats.
#define LTM(f) reinterpret_cast<float *>(RwFrameGetLTM(static_cast<RwFrame *>(f)))
struct RwObj418 { int m_00; void *m_parent; };

// Codegen: the LTM translation pointer and `d` in pointer locals, parenthesised subtractions, the dot product accumulated through a copy
// (`tmp2 = tmp4; tmp2 = (x*..)+tmp4;`).
// 0x41ada0
float CameraSignedViewDistance(RwCamera *camera, const RwV3d *point)
{
	RwObj418 *obj = (RwObj418 *)camera;   // the camera as this code reaches it: its frame at +4
	const float *p = &point->x;
	void *frame = obj->m_parent;
	float *tmp = (LTM(frame) + 12);
	float d[3];
	float *tmp3 = d;
	d[0] = p[0] - tmp[0];
	tmp3[1] = (p[1] - tmp[1]);
	d[2] = (p[2] - tmp[2]);
	float len = RwV3dLength(reinterpret_cast<const RwV3d *>(tmp3));
	float tmp4 = (tmp3[1] * LTM(frame)[9]);
	float tmp2 = tmp4;
	tmp2 = ((d[0] * LTM(frame)[8]) + tmp4);
	tmp2 = tmp2 + (d[2] * LTM(frame)[10]);
	if (tmp2 < g_floatZero)
		return -len;
	return len;
}

// 'use camera' (0x41d4c0)
int *CutScene_StackSlot(int delta);           // 0x436c00
void CameraTransitionReset(float t);
void CameraSetTime1(float t);
void CameraSetTime2(float t);
void CameraSetAnimation(RtAnimAnimation *anim);
void CameraClearAdd(float t);                 // ignores its argument
void CameraAddTime1(float t);
void CameraAddTime2(float t);
struct CamHolder418 { char m_pad00[4]; struct CamAnimRef418 *m_04; };
struct CamAnimRef418 { char m_pad00[0xc]; RtAnimAnimation *m_0c; };

// 0x41d4c0
void CameraUse_Apply(int type, float param, int transition, float tparam)
{
	if (*CutScene_StackSlot(0) == -1) {
		tparam = 0;
		param = 0;
	}
	switch (type) {
	case 0: CameraTransitionReset(param); break;
	case 1: CameraSetTime1(param); break;
	case 2: CameraSetTime2(param); break;
	}
	switch (transition) {
	case 0: CameraClearAdd(tparam); break;
	case 1: CameraAddTime1(tparam); break;
	case 2: CameraAddTime2(tparam); break;
	}
	// The camera object's back pointer (+4) is the CameraData's m_cam slot; the animation is its m_animation (the raw offsets +4, +0xc are
	// the 32-bit layout).
	Script::ScriptCamera *cam = (Script::ScriptCamera *)GetCameraObject();
	if (cam) {
		RtAnimAnimation *anim = 0;
		if (cam->m_ownerSlot)
			anim = (RtAnimAnimation *)RWS::TFB::CameraData::FromCamSlot(cam->m_ownerSlot)->GetAnimation();
		CameraSetAnimation(anim);
	}
}

