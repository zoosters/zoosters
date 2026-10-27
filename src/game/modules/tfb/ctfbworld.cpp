// The vtables, the scalar deleting destructor and the adjustor thunk are emitted from this file.
#include "modules/tfb/ctfbworld.h"
#include "modules/tfb/renderqueue.h"
#include "tfb/level/level.h"
#include "tfb/render/tfbclump.h"
#include "rw/rpplugins.h"
#include "tfb/render/atomicflags.h"
#include "framework/toolkits/world/framehelper.h"

// GLOBAL: 0x5ccb5c RWS::TFB::CTFBWorld::vftable_for_CEventHandler
// GLOBAL: 0x5ccb4c RWS::TFB::CTFBWorld::vftable_for_CAttributeHandler
// 0x41c3e0
// 0x41b140
// GLOBAL: 0x550250 ??0CEventId@RWS@@QAE@XZ
// GLOBAL: 0x464ef0 ??1CEventId@RWS@@QAE@XZ
// GLOBAL: 0x550760 ?UnLinkMsg@CEventHandler@RWS@@QAEXAAVCEventId@2@@Z
// GLOBAL: 0x5506d0 ?UnRegisterMsg@CEventHandler@RWS@@SAXAAVCEventId@2@@Z
// GLOBAL: 0x61811c g_renderQueueFreeList
// GLOBAL: 0x618124 g_renderQueueRefCount
// GLOBAL: 0x618114 g_renderQueueNodes
// GLOBAL: 0x618128 RWS::TFB::g_noMessageId
// GLOBAL: 0x629724 RWS::CLevel::sm_pOpaqueWorld
// GLOBAL: 0x62973c g_yonObject
// GLOBAL: 0x6296f0 g_fogFlag

namespace RWS { namespace TFB {

static const unsigned kRenderQueueNodes = 0x40;
static const float kInitialFogFarClip = 120000.0f;

// 0x41bcd0
CTFBWorld::CTFBWorld(const CAttributePacket &)
{
	m_unk4c = 1;
	m_yonBox = 0;
	m_renderSectors = &CTFBWorld::RenderSectors;
	RenderQueue_Open(kRenderQueueNodes);
	m_clearColor.red = 0;
	m_clearColor.green = 0;
	m_clearColor.blue = 0;
	SetFogFarClip(kInitialFogFarClip);
	SetFogPlane(0);
	g_fogFlag = 0;
	g_yonObject = this;
	m_yonIgnore = 1;
}

// 0x41aa30
CTFBWorld::~CTFBWorld()
{
	if (--g_renderQueueRefCount == 0) {
		if (g_renderQueueFreeList) {
			RwFreeListDestroy(g_renderQueueFreeList);
			g_renderQueueFreeList = 0;
		}
		if (g_renderQueueNodes) {
			RwFree(g_renderQueueNodes);
			g_renderQueueNodes = 0;
		}
	}
	if (CLevel::sm_pOpaqueWorld && RpPVSQuery(CLevel::sm_pOpaqueWorld))
		RpPVSUnhook(CLevel::sm_pOpaqueWorld);
	LevelDestroyWorlds(false);
	g_yonObject = 0;
	if (m_yonBox)
		::operator delete(m_yonBox);
	UnLinkMsg(m_renderEvent);
	UnRegisterMsg(m_renderEvent);
	UnRegisterMsg(m_afterOpaqueEvent);
	UnRegisterMsg(m_afterRenderEvent);
}

// The render event: flush the pending level objects into the worlds, render the frame for the camera in the message, then send the
// after-render event with the same camera.
// 0x41c310
void CTFBWorld::HandleEvents(CMsg &msg)
{
	if (msg.Id == m_renderEvent) {
		LevelWorldsSetReady();
		RenderFrame((RwCamera *)msg.pData);
		SendLinkedMessage(m_afterRenderEvent, msg.pData);
	}
}

// 0x41ab70
void CTFBWorld::AddClumpResource(RpClump *clump, const char *resourceType)
{
	if (clump && resourceType && rwstrcmp("rwID_CLUMP", resourceType) == 0) {
		ClumpSetAtomicFlags(clump, 0x30);
		TFBPlugin_B1_SetFlagBits(clump, LEVEL_CLUMP_YON);
		if (clump->parent == 0) {
			RwFrame *frame = RwFrameCreate_thunk();
			RwFrameSetIdentity(frame);
			clump->parent = frame;
		}
		RWS::CLevel::AddClump(clump);
	}
}

}}
