// The constructor makes cl emit the vtables, the scalar deleting destructor and the thunk.
#include "modules/tfb/tfbshadowcamera.h"
#include "rw/rpworld.h"
#include "tfb/render/tfbatomic.h"
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
#include "framework/core/memoryhandler.h"
#include "rw/rwmacros.h"
#include "framework/toolkits/world/smartptrs/catomicptr.h"
#include "framework/toolkits/world/smartptrs/cclumpptr.h"
#include "framework/toolkits/math/maths.h"
#include "tfb/level/level.h"
#include "framework/toolkits/world/factory.h"
// GLOBAL: 0x5cd114 g_shadowCamAxisX
// GLOBAL: 0x5cd120 g_shadowCamAxisY
// GLOBAL: 0x602f4c g_tfbShadowCameraClassId
// GLOBAL: 0x6182f4 g_shadowCamera
// GLOBAL: 0x62a548 g_evtShadowCamA
// GLOBAL: 0x62a550 g_evtShadowCamB
// GLOBAL: 0x62a344 RWS::iMsgRunningPostTick
extern const RwV3d g_shadowCamAxisX;       // 0x5cd114 (1, 0, 0)
extern const RwV3d g_shadowCamAxisY;       // 0x5cd120 (0, 1, 0)
extern const char *g_tfbShadowCameraClassId;
struct AtomicFlags;
__declspec(noinline) void AtomicSetIsVisible(AtomicFlags *a, bool on);   // 0x44e2e0 (g/g_misc.cpp): sets / clears the atomic flag bit 4
extern RWS::CEventId g_evtShadowCamA, g_evtShadowCamB;   // shown / hidden around the shadow pass (GameCamera::HandleEvents reads them too)
namespace RWS { extern CEventId iMsgRunningPostTick; }
namespace RWS { namespace TFB {
// GLOBAL: 0x5cd12c RWS::TFB::TFBShadowCamera::vftable_for_CAttributeHandler
// GLOBAL: 0x5cd13c RWS::TFB::TFBShadowCamera::vftable_for_CEventHandler
// 0x4227c0
// 0x422280
// GLOBAL: 0x422150 ??1TFBShadowCamera@TFB@RWS@@UAE@XZ

// Near-match of 0x422290: ecx / edx register choice and one extra mov ecx,eax copy.
TFBShadowCamera::TFBShadowCamera(const CAttributePacket &attr) : m_enabled(true), m_areaRadius(400.0f), m_pitch(75.0f), m_heading(0.0f), m_verticalOffset(500.0f)
{
	::g_shadowCamera = this;
	m_atomic = CreateAtomicInWorldFromResource(attr, this);
	if (m_atomic.ptr())
		TFBPlugin_FD_SetFlagBits(m_atomic, 0x20);
	RegisterMsg(m_evtLookAt, "look_at_point", "RwFrame*");
	LinkMsg(m_evtLookAt, "RwFrame*", 0x8000);
	RegisterMsg(m_evtCamera, "3rd_person_cam", "RwCamera*");
	LinkMsg(m_evtCamera, "RwCamera*", m_cameraPriority);
	LinkMsg(iMsgRunningPostTick, 0, 0x8000);
	LinkMsg(::g_evtShadowCamA, 0, 0x8000);
	LinkMsg(::g_evtShadowCamB, 0, 0x8000);
	*(int *)m_68 = 0;
}

// 0x422150
TFBShadowCamera::~TFBShadowCamera()
{
	UnLinkMsg(m_evtLookAt);
	UnLinkMsg(m_evtEnable);
	UnLinkMsg(m_evtDisable);
	UnLinkMsg(iMsgRunningPostTick);
	UnLinkMsg(g_evtShadowCamA);
	UnLinkMsg(g_evtShadowCamB);
	UnLinkMsg(m_evtCamera);
	UnRegisterMsg(m_evtLookAt);
	UnRegisterMsg(m_evtEnable);
	UnRegisterMsg(m_evtDisable);
	UnRegisterMsg(m_evtCamera);
	g_shadowCamera = 0;
}

// 0x422430
void TFBShadowCamera::HandleEvents(CMsg &msg)
{
	if (msg.Id == m_evtLookAt.pEventId) {
		if (m_atomic.ptr()) {
			RwFrame *frame = RpAtomicGetFrame(m_atomic.ptr());
			RwFrameRotate(frame, &g_shadowCamAxisX, m_pitch, rwCOMBINEREPLACE);
			RwFrameRotate(frame, &g_shadowCamAxisY, m_heading, rwCOMBINEPOSTCONCAT);
		}
	}
	if (msg.Id == g_evtShadowCamA.pEventId) {
		if (m_atomic.ptr())
			AtomicSetIsVisible((AtomicFlags *)m_atomic.ptr(), true);
	}
	if (msg.Id == g_evtShadowCamB.pEventId) {
		if (m_atomic.ptr())
			AtomicSetIsVisible((AtomicFlags *)m_atomic.ptr(), false);
	}
	if (msg.Id == m_evtEnable.pEventId)
		m_enabled = true;
	if (msg.Id == m_evtDisable.pEventId)
		m_enabled = false;
}

// 0x4224d0
void TFBShadowCamera::HandleSystemCommands(const CAttributePacket &attr)
{
	if (m_atomic.ptr())
		AtomicHelper::HandleSystemCommands(*m_atomic.ptr(), attr);
	CAttributeCommandIterator it(attr, CSystemCommands::pCSystemCommandsClassID);
	while (!it.IsFinished()) {
		it->GetCommandId();
		++it;
	}
}

// DIFF (528 B, same size and instruction stream): the register naming of the two persistent values differs (the game keeps `this`, the
// CAttributeHandler sub-object, in esi and
// this-8, the CEventHandler, in edi; ours are swapped). Kept disabled (below the bar).
// 0x422540
void TFBShadowCamera::HandleAttributes(const CAttributePacket &packet)
{
	CAttributeHandler::HandleAttributes(packet);
	HandleSystemCommands(packet);
	for (CAttributeCommandIterator it(packet, g_tfbShadowCameraClassId); !it.IsFinished(); ++it) {
		switch (it->GetCommandId()) {
		case 6: SetBits(m_areaRadius, it->GetAs_RwInt32()); break;
		case 7: SetBits(m_pitch, it->GetAs_RwInt32()); break;
		case 8: SetBits(m_heading, it->GetAs_RwInt32()); break;
		case 9: SetBits(m_verticalOffset, it->GetAs_RwInt32()); break;
		case 0: {
			const CAttributeCommand *cmd = it.operator->();
			UnLinkMsg(m_evtLookAt);
			UnRegisterMsg(m_evtLookAt);
			RegisterMsg(m_evtLookAt, cmd->GetAs_RwChar_ptr(), "RwFrame*");
			LinkMsg(m_evtLookAt, "RwFrame*", 0x8000);
			break;
		}
		case 1: {
			const CAttributeCommand *cmd = it.operator->();
			UnLinkMsg(m_evtEnable);
			UnRegisterMsg(m_evtEnable);
			RegisterMsg(m_evtEnable, cmd->GetAs_RwChar_ptr(), 0);
			LinkMsg(m_evtEnable, 0, 0x8000);
			break;
		}
		case 2: {
			const CAttributeCommand *cmd = it.operator->();
			UnLinkMsg(m_evtDisable);
			UnRegisterMsg(m_evtDisable);
			RegisterMsg(m_evtDisable, cmd->GetAs_RwChar_ptr(), 0);
			LinkMsg(m_evtDisable, 0, 0x8000);
			break;
		}
		case 3:
			UnLinkMsg(m_evtCamera);
			UnRegisterMsg(m_evtCamera);
			RegisterMsg(m_evtCamera, it->GetAs_RwChar_ptr(), "RwCamera*");
			LinkMsg(m_evtCamera, "RwCamera*", m_cameraPriority);
			break;
		case 5:
			m_cameraPriority = it->GetAs_RwUInt16();
			UnLinkMsg(m_evtCamera);
			LinkMsg(m_evtCamera, "RwCamera*", m_cameraPriority);
			break;
		}
	}
}
}}
