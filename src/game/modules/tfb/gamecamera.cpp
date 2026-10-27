#define ZOOSTERS_RWS_SENDMSG   // the real inline SendMsg(CMsg &) (the game calls its out-of-line copy: see the ABI stand-in below)
#include "modules/tfb/gamecamera.h"
#include "framework/core/csystemcommands.h"
#include "modules/tfb/camera_views.h"
#include "rw/rphanim.h"
#include "tfb/render/tfbatomic.h"
#include "framework/toolkits/world/framehelper.h"

// GLOBAL: 0x550250 ??0CEventId@RWS@@QAE@XZ
// GLOBAL: 0x464ef0 ??1CEventId@RWS@@QAE@XZ
// GLOBAL: 0x550760 ?UnLinkMsg@CEventHandler@RWS@@QAEXAAVCEventId@2@@Z
// GLOBAL: 0x5506d0 ?UnRegisterMsg@CEventHandler@RWS@@SAXAAVCEventId@2@@Z
// GLOBAL: 0x550ac0 ?LinkMsg@CEventHandler@RWS@@QAEXAAVCEventId@2@PBDG@Z
// GLOBAL: 0x44e260 ?Destroy@AtomicHelper@RWS@@YAHPAURpAtomic@@@Z
// GLOBAL: 0x44e750 ?HandleSystemCommands@AtomicHelper@RWS@@YAXAAURpAtomic@@ABVCAttributePacket@2@@Z
// GLOBAL: 0x6182b5 g_camShadowPass
// GLOBAL: 0x62a344 RWS::iMsgRunningPostTick
// GLOBAL: 0x62a510 RWS::iMsgStartSystem
// GLOBAL: 0x62a548 g_evtShadowCamA
// GLOBAL: 0x62a550 g_evtShadowCamB
// GLOBAL: 0x602f40 g_gameCameraClassId
// GLOBAL: 0x611634 RWS::CSystemCommands::pCSystemCommandsClassID
// GLOBAL: 0x5cd084 RWS::TFB::GameCamera::vftable_for_CAttributeHandler
// GLOBAL: 0x5cd094 RWS::TFB::GameCamera::vftable_for_CEventHandler
// 0x422120
// 0x41fb70

extern bool g_camShadowPass;               // set between the two shadow-camera events (the camera's atomic is shown for the shadow pass)
extern const char *g_gameCameraClassId;
namespace RWS {
extern CEventId iMsgRunningPostTick;
extern CEventId iMsgStartSystem;
} // namespace RWS
extern RWS::CEventId g_evtShadowCamA;
extern RWS::CEventId g_evtShadowCamB;
namespace RWS {
RpAtomic *CreateAtomicInWorldFromResource(const CAttributePacket &attr, CEventHandler *owner); // 0x44e180
namespace FrameHelper {
int FrameDestroy(RwFrame *frame); // 0x44f000
}
inline int SendMsg(CMsg *msg) { return SendMsg(*msg); }
} // namespace RWS
void LevelHideClump();                                    // 0x44d9b0
void LevelShowClump();                                    // 0x44d9d0

namespace RWS {
namespace TFB {

// 0x41e710
GameCamera::~GameCamera()
{
	if (g_buttonTexture) {
		RwTextureDestroy(g_buttonTexture);
		g_buttonTexture = 0;
	}
	if (g_camHAnim) {
		RpHAnimHierarchyDestroy(g_camHAnim);
		g_camHAnim = 0;
	}
	RwFrame *frame = RwCameraGetFrame(g_gameCamDst);
	_rwObjectHasFrameSetFrame(g_gameCamDst, 0);
	FrameHelper::FrameDestroy(frame);
	RwCameraDestroy(g_gameCamDst);
	g_gameCamDst = 0;
	UnLinkMsg(iMsgRunningPostTick);
	UnLinkMsg(iMsgStartSystem);
	UnLinkMsg(g_evtShadowCamA);
	UnLinkMsg(g_evtShadowCamB);
	UnLinkMsg(m_evtCameraIn);
	UnLinkMsg(m_evtCameraOut);
	UnRegisterMsg(m_evtCameraIn);
	UnRegisterMsg(m_evtCameraOut);
	g_cameraObject = 0;
}

// Near-miss, not kept (565 B vs 569 B): the game copies the atomic pointer into edx for the TFBPlugin_FD_SetFlagBits call, which shifts the
// register choices of the
// g_camPos / g_camTgt copies and the five g_cameraObject stores.
// 0x41fc70
// The focus starts at the origin, the eye at (-100, 100, 0).
GameCamera::GameCamera(const CAttributePacket &attr)
    : m_linkPriority(0xffff), m_heading(0), m_pitchOffset(0), m_yawStep(0), m_yawInput(0), m_pitchInput(0), m_50(0)
{
	g_cameraObject = this;
	g_gameCamDst = RwCameraCreate();
	_rwObjectHasFrameSetFrame(g_gameCamDst, RwFrameCreate_thunk());
	g_gameCamValid = false;
	RpAtomic *atomic = CreateAtomicInWorldFromResource(attr, this);
	m_atomic = atomic;
	if (m_atomic)
		TFBPlugin_FD_SetFlagBits(m_atomic, 0x20);
	CameraResetState();
	g_camPos.x = 0;
	g_camPos.y = 0;
	g_camPos.z = 0;
	g_camTgt.x = -100.0f;
	g_camTgt.y = 100.0f;
	g_camTgt.z = 0;
	m_focus = g_camPos;
	m_matrix.pos = g_camTgt;
	g_camSnapPending = true;
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
	LinkMsg(iMsgRunningPostTick, 0, 0x8000);
	LinkMsg(iMsgStartSystem, 0, 0x8000);
	LinkMsg(g_evtShadowCamA, 0, 0x8000);
	LinkMsg(g_evtShadowCamB, 0, 0x8000);
	g_camHAnim = 0;
	g_buttonTexture = RwTextureRead("sprite_o_button", 0);
}

// 0x41feb0
// Copies the carried camera's render configuration into g_gameCamDst, places its frame at the camera matrix and re-sends the game camera as
// the out message.
// iMsgStartSystem (once) clears the state. The shadow cameras' messages show and hide the level clump around the shadow pass.
void GameCamera::HandleEvents(CMsg &msg)
{
	if (msg.Id == m_evtCameraIn.pEventId && msg.pData != 0 && m_evtCameraOut.m_registered_count != 0) {
		RwCamera *dst = g_gameCamDst;
		GameCameraCopyParams(dst, (const RwCamera *)msg.pData);
		if (!g_gameCamValid)
			return;
		RwFrameTransform(RwCameraGetFrame(dst), &m_matrix, rwCOMBINEREPLACE);
		RwFrameUpdateObjects(RwCameraGetFrame(dst));
		CMsg out;
		out.Id = m_evtCameraOut.pEventId;
		out.pData = dst;
		SendMsg(&out);
	}
	if (msg.Id == iMsgStartSystem.pEventId) {
		UnLinkMsg(iMsgStartSystem);
		g_camModeAux = 0;
		g_camSnapPending = true;
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
	if (msg.Id == g_evtShadowCamA.pEventId && m_atomic.ptr() != 0) {
		g_camShadowPass = true;
		LevelShowClump();
	}
	if (msg.Id == g_evtShadowCamB.pEventId && m_atomic.ptr() != 0) {
		g_camShadowPass = false;
		LevelHideClump();
	}
}

// 0x41ffd0
// The command ids of the system class are read and ignored.
void GameCamera::HandleSystemCommands(const CAttributePacket &attr)
{
	if (m_atomic.ptr())
		m_atomic.HandleSystemCommands(attr);
	for (CAttributeCommandIterator it(attr, RWS::CSystemCommands::pCSystemCommandsClassID); !it.IsFinished(); ++it)
		it->GetCommandId();
}

// 0x420040
// Commands: 0 registers the incoming "RwCamera*" message and links to it, 1 sets its link priority, 2 registers the outgoing message.
void GameCamera::HandleAttributes(const CAttributePacket &attr)
{
	CAttributeHandler::HandleAttributes(attr);
	HandleSystemCommands(attr);
	for (CAttributeCommandIterator it(attr, g_gameCameraClassId); !it.IsFinished(); ++it) {
		switch (it->GetCommandId()) {
		case 0:
			UnLinkMsg(m_evtCameraIn);
			UnRegisterMsg(m_evtCameraIn);
			{
				const CAttributeCommand *cmd = it.operator->();
				RegisterMsg(m_evtCameraIn, cmd->GetAs_RwChar_ptr(), "RwCamera*");
			}
			LinkMsg(m_evtCameraIn, "RwCamera*", m_linkPriority);
			break;
		case 1:
			m_linkPriority = it->GetAs_RwUInt16();
			UnLinkMsg(m_evtCameraIn);
			LinkMsg(m_evtCameraIn, "RwCamera*", m_linkPriority);
			break;
		case 2:
			UnRegisterMsg(m_evtCameraOut);
			{
				const CAttributeCommand *cmd = it.operator->();
				RegisterMsg(m_evtCameraOut, cmd->GetAs_RwChar_ptr(), "RwCamera*");
			}
			break;
		}
	}
}

// 0x4220b0
// Class factory entry; the pointer keeps the function referenced.
CAttributeHandler *(*g_keepMakeNew_GameCamera)(const CAttributePacket &) = &GameCamera::MakeNew;

} // namespace TFB
} // namespace RWS
