#include <math.h>
#include <stddef.h>
#include <string.h>
#include "modules/tfb/gamecamera.h"
#include "modules/tfb/camera_views.h"
#include "script/actor.h"
#include "script/types/scriptsystem.h"
#include "script/types/scriptsound.h"
#include "script/types/scriptfog.h"
#include "script/types/scriptcamera.h"
#include "rw/rphanim.h"
#include "rw/rtanim.h"

static const float kDegToRad = 0.017453292f;
static const float kFloatMax = 3.4028235e38f; // "no value" marker of the filtered points

// 0x41e1d0
RwCamera *GetActiveGameCamera()
{
	if (g_cameraObject && g_gameCamDst)
		return g_gameCamDst;
	return 0;
}

// 0x41e1f0
// Snaps to the targets at the next update and drops pending pitch / yaw holds and smoothed input. The follow camera passes true at the cut time.
void CameraSetCut(bool snap)
{
	if (snap) {
		g_camSnapPending = snap;
		g_camPitchDeferred = false;
		g_camPitchHold = 0;
		g_camYawHold = 0;
		if (g_cameraObject) {
			g_cameraObject->m_50 = 0;
			g_cameraObject->m_pitchInput = 0;
			g_cameraObject->m_yawInput = 0;
			g_cameraObject->m_yawStep = 0;
			g_cameraObject->m_pitchOffset = 0;
		}
	}
}

// Integer word copy: the game copies the floats this way too.
static inline void CopyCameraWord(RwCamera *to, const RwCamera *from, size_t offset)
{
	((RwInt32 *)to)[offset / 4] = ((const RwInt32 *)from)[offset / 4];
}

// Copies one pointer member (callbacks, rasters): 8 bytes on x64, where a 4-byte word copy left the upper half of the destination stale.
static void CopyCameraPtr(RwCamera *to, const RwCamera *from, size_t offset)
{
	memcpy((char *)to + offset, (const char *)from + offset, sizeof(void *));
}

// 0x41e160
// Not the view window (UpdateGameCamera sets that from the script camera's field of view).
void GameCameraCopyParams(RwCamera *to, const RwCamera *from)
{
	CopyCameraPtr(to, from, offsetof(RwCameraView, beginUpdate));
	CopyCameraPtr(to, from, offsetof(RwCameraView, endUpdate));
	CopyCameraWord(to, from, offsetof(RwCameraView, viewOffset));
	CopyCameraWord(to, from, offsetof(RwCameraView, viewOffset) + 4);
	CopyCameraWord(to, from, offsetof(RwCameraView, nearPlane));
	CopyCameraWord(to, from, offsetof(RwCameraView, farPlane));
	CopyCameraWord(to, from, offsetof(RwCameraView, fogPlane));
	CopyCameraPtr(to, from, offsetof(RwCameraView, frameBuffer));
	CopyCameraPtr(to, from, offsetof(RwCameraView, zBuffer));
	CopyCameraWord(to, from, offsetof(RwCameraView, projection));
	CopyCameraWord(to, from, offsetof(RwCameraView, zScale));
	CopyCameraWord(to, from, offsetof(RwCameraView, zShift));
}

// 0x41e250
float CameraTimeRemaining()
{
	return g_camTimeLeft;
}

// 0x41e260
float CameraPercent()
{
	if (g_camTimeTotal > 0.0f)
		return 1.0f - g_camTimeLeft / g_camTimeTotal;
	return g_floatZero;
}

// Near-match of 0x41e290: null case first like the game; store scheduling differs.
// Starts the three timers at `duration`; the current focus and eye (or the targets, without a camera) are kept as the blend's start points.
// The game copies the float as an integer word.
void CameraTransitionReset(float duration)
{
	int bits = *(int *)&duration;
	*(int *)&g_camTimeLeft = bits;
	*(int *)&g_camTimeStart = bits;
	g_camMode = 0;
	g_camModeAux = 0;
	*(int *)&g_camTimeTotal = bits;
	if (g_cameraObject == 0) {
		g_camSnapPos = g_camPos;
		g_camSnapTgt = g_camTgt;
	} else {
		g_camSnapPos = g_cameraObject->m_focus;
		g_camSnapTgt = g_cameraObject->m_matrix.pos;
	}
}

// 0x41e350
// The argument is passed by the callers and ignored.
void CameraClearAdd(float)
{
	g_camAddKind = 0;
	g_camAddAmount = 0;
}

// 0x41e370
// Kind 1 (fade).
void CameraSetTime1(float seconds)
{
	g_camMode = 1;
	g_camModeAux = 0;
	g_camTimeLeft = seconds;
	g_camTimeTotal = seconds;
	g_camTimeStart = seconds;
}

// 0x41e3a0
// Kind 2 (letterbox).
void CameraSetTime2(float seconds)
{
	g_camMode = 2;
	g_camModeAux = 0;
	g_camTimeLeft = seconds;
	g_camTimeTotal = seconds;
	g_camTimeStart = seconds;
}

// 0x41e3d0
void CameraAddTime1(float seconds)
{
	g_camTimeLeft += seconds;
	g_camAddKind = 1;
	g_camAddAmount = seconds;
	g_camTimeTotal += seconds;
}

// 0x41e410
void CameraAddTime2(float seconds)
{
	g_camTimeLeft += seconds;
	g_camAddKind = 2;
	g_camAddAmount = seconds;
	g_camTimeTotal += seconds;
}

// 0x41e450
float CameraGetValueD0()
{
	return g_camValueD0;
}

// 0x41e460
void CameraGetPosition(RwV3d *out)
{
	*out = g_camPos;
}

// 0x41e480
// Remembers the last two raw points and filters the height: the focus follows only `vertFollowAt` percent of the height change of the looked-at
// actor (refY: its height without the change, curY: now).
void CameraSetLookAtPoint(const Script::ScriptCamera *cam, const RwV3d *pos, float refY, float curY)
{
	g_camPosPrev = g_camPosCur;
	g_camPosCur = *pos;
	RwV3d point = *pos;
	float follow = (float)cam->m_vertFollowAt * 0.01f;
	if (fabsf(1.0f - follow) > 0.0001f) {
		float change = curY - refY;
		if (fabsf(change) <= 0.0001f || g_camSnapPending)
			g_camPosBaseY = refY;
		else
			change = curY - g_camPosBaseY;
		if (change != g_floatZero && g_camPos.y != kFloatMax)
			point.y = point.y - (1.0f - follow) * change;
	}
	g_camPos = point;
}

// 0x41e590
// The look-from (eye) twin of CameraSetLookAtPoint, with `vertFollowFrom` percent.
// NOTE: the "no value" test is `==` here and `!=` in CameraSetLookAtPoint (as in the game).
void CameraSetLookFromPoint(const Script::ScriptCamera *cam, const RwV3d *pos, float refY, float curY)
{
	g_camTgtPrev = g_camTgtCur;
	g_camTgtCur = *pos;
	float change = curY - refY;
	if (fabsf(change) <= 0.0001f || g_camSnapPending)
		g_camTgtBaseY = refY;
	else
		change = curY - g_camTgtBaseY;
	RwV3d point = *pos;
	if (change != g_floatZero && g_camTgt.y == kFloatMax)
		point.y = g_camTgt.y;
	else
		point.y = point.y - (1.0f - (float)cam->m_vertFollowFrom * 0.01f) * change;
	g_camTgt = point;
}

// 0x41e690
void CameraSetHeading(float heading)
{
	if (g_cameraObject)
		g_cameraObject->m_heading = heading;
}

// 0x41e6b0
float CameraGetHeading()
{
	if (g_cameraObject)
		return g_cameraObject->m_heading;
	return g_floatZero;
}

// 0x41e6d0
// Degrees 0..360; the heading is the direction from the anchor to the camera.
float CameraFacing()
{
	if (g_cameraObject) {
		float facing = g_cameraObject->m_heading;
		if (facing < 0.0f)
			facing += 360.0f;
		facing += 180.0f;
		if (facing > 360.0f)
			facing -= 360.0f;
		return facing;
	}
	return g_floatZero;
}

// 0x41e870
// Sets the heading to the script's facing (+180: the heading points from the anchor to the camera). A lazy script camera also
// swings the eye around the focus, and the snap points around each other, to the new heading at their current distance.
void RWS::TFB::GameCamera::SetFacing(const Script::ScriptCamera *scriptCamera, float heading)
{
	m_heading = heading + 180.0f;
	if (scriptCamera->m_flags & 1) {
		float len;
		RwV3d d;
		d.y = 0;
		d.x = m_focus.x - m_matrix.pos.x;
		d.z = m_focus.z - m_matrix.pos.z;
		len = (RwV3dLength(&d));
		m_matrix.pos.x = (float)sin(m_heading * kDegToRad) * len + m_focus.x;
		m_matrix.pos.z = (float)cos(kDegToRad * m_heading) * len + m_focus.z;
		d.x = g_camSnapPos.x - g_camSnapTgt.x;
		d.z = g_camSnapPos.z - g_camSnapTgt.z;
		d.y = 0;
		len = (RwV3dLength(&d));
		g_camSnapTgt.x = (float)sin(m_heading * kDegToRad) * len + g_camSnapPos.x;
		g_camSnapTgt.z = (float)cos(m_heading * kDegToRad) * len + g_camSnapPos.z;
	}
}

// 0x41fc50
void SetCameraFacing(Script::ScriptReference *cam, float facing)
{
	if (g_cameraObject)
		g_cameraObject->SetFacing(static_cast<Script::ScriptCamera *>(cam), facing);
}

// 0x41fc00
// A longer animation lengthens the running transition; the node hierarchy is rebuilt.
void CameraSetAnimation(RtAnimAnimation *anim)
{
	if (anim != g_camAnim) {
		g_camAnim = anim;
		if (anim) {
			float duration = AnimationDuration(anim);
			if (duration > g_camTimeTotal)
				g_camTimeTotal = g_camTimeLeft = duration;
			if (g_cameraObject)
				g_cameraObject->RebuildAnimHierarchy();
		}
	}
}

// 0x41fb80
// NOTE: the game reloads g_cameraObject after the first store: the first member is written through a copy, the others through the global.
void CameraResetState()
{
	g_camModeAux = 0;
	g_camTimeLeft = 0;
	g_camTimeTotal = 0;
	g_camTimeStart = 0;
	g_camMode = 0;
	g_camAddAmount = 0;
	g_camAddKind = 0;
	g_camFade = 0;
	g_camSnapPending = true;
	g_camPitchDeferred = false;
	g_camPitchHold = 0;
	g_camYawHold = 0;
	RWS::TFB::GameCamera *camera = g_cameraObject;
	if (camera) {
		camera->m_50 = 0;
		g_cameraObject->m_pitchInput = 0;
		g_cameraObject->m_yawInput = 0;
		g_cameraObject->m_yawStep = 0;
		g_cameraObject->m_pitchOffset = 0;
	}
}

// 0x41f190
void CameraCopyFrustumPlanes(RwCamera *camera)
{
	g_camBlk[0] = CameraView(camera)->frustumPlanes[0];
	g_camBlk[1] = CameraView(camera)->frustumPlanes[1];
	g_camBlk[2] = CameraView(camera)->frustumPlanes[2];
	g_camBlk[3] = CameraView(camera)->frustumPlanes[3];
	g_camBlk[4] = CameraView(camera)->frustumPlanes[4];
	g_camBlk[5] = CameraView(camera)->frustumPlanes[5];
}
