// The camera behaviour and the state it shares with the script camera: the free Camera* functions the scripts call work on the globals declared here.
#ifndef ZOOSTERS_MODULES_TFB_GAMECAMERA_H
#define ZOOSTERS_MODULES_TFB_GAMECAMERA_H

#include "zoosters/base.h"
#include "rw/rwcore.h"
#include "script/script.h"
#include "framework/core/rwsguid.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/attributemacros.h"
#include "framework/core/cclassfactory.h"
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/linearallocationpolicy.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/ceventhandler.h"
#include "modules/fx/cfxbaselight.h"

namespace Script {
class ScriptReference;
class ScriptCamera;
} // namespace Script
struct RtAnimAnimation;
struct RpHAnimHierarchy;
struct RwTexture;

namespace RWS {
namespace TFB {

class GameActor;

// PS2 base order: EA
class GameCamera : public CSystemCommands, public CEventHandler, public CAttributeHandler, public LinearAllocationPolicy {
public:
	RWS_MAKENEWCLASS(GameCamera);
	GameCamera(const CAttributePacket &attr);
	~GameCamera();
	virtual void HandleEvents(CMsg &msg);
	virtual void HandleAttributes(const CAttributePacket &attr);
	void HandleSystemCommands(const CAttributePacket &attr);   // the atomic's system commands

	// Per frame: follows the targets, resolves collisions, publishes the pose.
	void UpdateGameCamera(RwCamera *camera);
	// Probes the four sides of the camera (0 right, 1 left, 2 below, 3 above) around `anchor` with a box of half size `scale` and
	// returns the sides that hit; hitFractions[i] is the fraction of the way to the camera, -1 when side i did not hit.
	int CollisionProbe(const GameActor *actor, const RwV3d *anchor, float scale, float *hitFractions, unsigned sideMask);
	void SetFacing(const Script::ScriptCamera *scriptCamera, float heading);
	void ApplyCameraAnimation(RwFrame *frame);
	// The game calls it on the camera object but does not use it.
	void RebuildAnimHierarchy();

	// (data public: the free camera functions on g_cameraObject read and clear them)
	// members start at +0x28 (the empty LinearAllocationPolicy base takes 4 bytes)
	CEventId m_evtCameraIn;    // +0x28 receives an RwCamera* to follow (attr cmd 0)
	unsigned short m_linkPriority; // +0x30 link priority (attr cmd 1)
	char m_pad32[2];
	CEventId m_evtCameraOut;   // +0x34 re-sent message: transmits the game camera (attr cmd 2); +0x3a = its registered count
	// +0x3c..+0x50: cleared by the constructor (+0x40..+0x50 also on iMsgStartSystem)
	float m_heading;           // +0x3c degrees, atan2(at.x, at.z) + 180
	float m_pitchOffset;       // +0x40 player pitch offset, lerps toward m_pitchInput
	float m_yawStep;           // +0x44 per-frame heading step, lerps toward m_yawInput, added to m_heading
	float m_yawInput;          // +0x48 smoothed X-axis input
	float m_pitchInput;        // +0x4c smoothed Y-axis input
	int m_50;                  // +0x50 cleared with the four above; never read
	RWS::CAtomicPtr m_atomic;   // +0x54 the camera atomic (shadow camera events test it)
	RwMatrix m_matrix;         // +0x58 the camera matrix; pos (+0x88) = the eye
	RwV3d m_focus;             // +0x98 the point the camera orbits / looks at
	CEventId m_evtA4;          // +0xa4
};
ZOOSTERS_ASSERT_SIZE(GameCamera, 0xac);
ZOOSTERS_ASSERT_OFFSET(GameCamera, m_heading, 0x3c);
ZOOSTERS_ASSERT_OFFSET(GameCamera, m_atomic, 0x54);
ZOOSTERS_ASSERT_OFFSET(GameCamera, m_matrix, 0x58);
ZOOSTERS_ASSERT_OFFSET(GameCamera, m_focus, 0x98);

} // namespace TFB
} // namespace RWS

// ---- camera state shared by the behaviour and the script-facing functions
// GLOBAL: 0x6182c8 g_cameraObject
// GLOBAL: 0x6182d0 g_gameCamDst
// GLOBAL: 0x6182d4 g_gameCamValid
// GLOBAL: 0x6182b4 g_camSnapPending
// GLOBAL: 0x6181b8 g_camPitchDeferred
// GLOBAL: 0x6181b0 g_camPitchHold
// GLOBAL: 0x6181b4 g_camYawHold
// GLOBAL: 0x6182b8 g_camTimeLeft
// GLOBAL: 0x6182bc g_camTimeTotal
// GLOBAL: 0x6182c0 g_camTimeStart
// GLOBAL: 0x6181cc g_camMode
// GLOBAL: 0x6181c0 g_camModeAux
// GLOBAL: 0x6181c4 g_camFade
// GLOBAL: 0x6181c8 g_camAddKind
// GLOBAL: 0x6182c4 g_camAddAmount
// GLOBAL: 0x6181d0 g_camValueD0
// GLOBAL: 0x6181d4 g_camSnapTgt
// GLOBAL: 0x6181e0 g_camSnapPos
// GLOBAL: 0x6181ec g_camTgt
// GLOBAL: 0x6181f8 g_camTgtCur
// GLOBAL: 0x618204 g_camTgtPrev
// GLOBAL: 0x618210 g_camPos
// GLOBAL: 0x61821c g_camPosCur
// GLOBAL: 0x618228 g_camPosPrev
// GLOBAL: 0x6182ec g_camPosBaseY
// GLOBAL: 0x6182f0 g_camTgtBaseY
// GLOBAL: 0x6182cc g_camAnim
// GLOBAL: 0x6181bc g_camHAnim
// GLOBAL: 0x602f44 g_camNodeFlags
// GLOBAL: 0x6182b0 g_buttonTexture
// GLOBAL: 0x5ca5a8 g_floatZero
extern RWS::TFB::GameCamera *g_cameraObject; // the one GameCamera (0 before the level creates it)
extern RwCamera *g_gameCamDst;               // the RwCamera it owns: the view the game renders through
extern bool g_gameCamValid;                  // g_gameCamDst has a view window (set by the first update that finds a frame buffer)
extern bool g_camSnapPending;                // snap the eye and the focus to the targets at the next update (a cut), then cleared
extern bool g_camPitchDeferred;              // a pitch correction of the collision resolution waits for the next update
extern int g_camPitchHold;                   // frames (+-60, the sign is the direction) in which pitch input in that direction is ignored
extern int g_camYawHold;                     // the same for yaw input
extern float g_camTimeLeft;                  // seconds left of the running camera transition (CameraSetTime1/2)
extern float g_camTimeTotal;                 // its length (grows by CameraAddTime1/2)
extern float g_camTimeStart;                 // the part of it spent blending
extern int g_camMode;                        // 0 blend, 1 / 2 = transition kinds (fade, letterbox)
extern int g_camModeAux;                     // overlay kind drawn this frame (0 none, 1 fade quad, 2 letterbox); CameraDrawOverlay
extern float g_camFade;                      // fade amount 0..1 of that overlay
extern int g_camAddKind;                     // kind of the transition time added at the end (0 none, 1, 2)
extern float g_camAddAmount;                 // its length
extern float g_camValueD0;                   // distance of the eye to the anchor (CameraGetValueD0); FLT_MAX-negative while an animation plays
extern RwV3d g_camSnapTgt, g_camSnapPos;     // eye and focus when the transition started
extern RwV3d g_camTgt, g_camTgtCur, g_camTgtPrev;   // eye the script asked for (filtered), and the last two raw eye points
extern RwV3d g_camPos, g_camPosCur, g_camPosPrev;   // focus the script asked for (filtered), and the last two raw focus points
extern float g_camPosBaseY, g_camTgtBaseY;   // reference heights of the vertical-follow filters
extern RtAnimAnimation *g_camAnim;           // the camera animation (a CameraData resource) the camera plays, 0 for none
extern RpHAnimHierarchy *g_camHAnim;         // its node hierarchy (RebuildAnimHierarchy)
extern RwUInt32 g_camNodeFlags[];            // the node flags the hierarchy is created with
extern RwTexture *g_buttonTexture;           // "sprite_o_button", the prompt of the letterbox (read by the GameCamera constructor)
extern const float g_floatZero;              // 0.0f as a data constant

void Camera_Update();
RwCamera *GetActiveGameCamera();
void CameraSetCut(bool snap);
float CameraTimeRemaining();
float CameraPercent();
void CameraTransitionReset(float duration);
void CameraClearAdd(float ignored);   // ignores its argument, like the game's
void CameraSetTime1(float seconds);
void CameraSetTime2(float seconds);
void CameraAddTime1(float seconds);
void CameraAddTime2(float seconds);
float CameraGetValueD0();
float CameraSignedViewDistance(RwCamera *camera, const RwV3d *point);
void CameraGetPosition(RwV3d *out);
float CameraGetHeading();
void CameraSetHeading(float heading);
float CameraFacing();
void GameCameraCopyParams(RwCamera *to, const RwCamera *from);
void CameraSetLookAtPoint(const Script::ScriptCamera *cam, const RwV3d *pos, float refY, float curY);
void CameraSetLookFromPoint(const Script::ScriptCamera *cam, const RwV3d *pos, float refY, float curY);
void SetCameraFacing(Script::ScriptReference *cam, float facing);
void CameraSetAnimation(RtAnimAnimation *anim);
void CameraResetState();
void CameraCopyFrustumPlanes(RwCamera *camera);
void CameraDrawOverlay(RwCamera *camera);

#endif
