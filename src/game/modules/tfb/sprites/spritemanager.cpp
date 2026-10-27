#include <float.h>
#include <string.h>
#include "modules/tfb/sprites/spritemanager.h"
#include "rw/rttoolkits.h"
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
#include "rw/rwcore.h"
#include "rw/rpworld.h"
#include "rw/rwmacros.h"
#include "framework/toolkits/world/smartptrs/catomicptr.h"
#include "framework/toolkits/world/smartptrs/cclumpptr.h"
#include "framework/toolkits/math/maths.h"
#include "tfb/level/level.h"
#include "rw/rpcollis.h"
#include "framework/core/cresourcemanager.h"
#include "framework/toolkits/world/framehelper.h"
#include "framework/toolkits/world/atomichelper.h"
#include "framework/toolkits/world/camerahelper.h"
#include "framework/toolkits/world/lighthelper.h"
#include "framework/toolkits/world/clumphelper.h"
#include "framework/toolkits/world/factory.h"
#include "modules/tfb/camera_views.h"
#include "framework/mainloop/gameclock.h"

extern "C" int __cdecl _stricmp(const char *, const char *);
namespace RWS { namespace MainLoop { namespace Render { RwCamera *GetCamera(); } } }    // 0x462660

// GLOBAL: 0x5cbd48 RWS::TFB::SpriteManager::vftable_for_CAttributeHandler
// GLOBAL: 0x5cbd40 RWS::TFB::SpriteManager::vftable_for_CEventHandler
// GLOBAL: 0x550250 ??0CEventId@RWS@@QAE@XZ
// GLOBAL: 0x464ef0 ??1CEventId@RWS@@QAE@XZ
// GLOBAL: 0x550760 ?UnLinkMsg@CEventHandler@RWS@@QAEXAAVCEventId@2@@Z
// GLOBAL: 0x5506d0 ?UnRegisterMsg@CEventHandler@RWS@@SAXAAVCEventId@2@@Z
// 0x417670
// 0x414780
// GLOBAL: 0x617e80 g_spriteManager
// GLOBAL: 0x617e84 g_spriteGeneration

namespace RWS { namespace TFB {

// 0x413d50
SpriteManager::SpriteManager(const CAttributePacket &) : m_caption(0), m_shownCaption(0), m_renderList((GameSprite *)-1), m_camera(0), m_letterboxAlpha(0)
{
	g_spriteManager = this;
	g_spriteGeneration = 0;
	RegisterMsg(m_Render_In, "3rd_person_cam", "RwCamera*");
	LinkMsg(m_Render_In, "RwCamera*", (unsigned short)m_link_priority);
	RegisterMsg(m_Render_Out, "radar_render", "RwCamera*");
	m_camera = RwCameraCreate();
	if (m_camera) {
		_rwObjectHasFrameSetFrame(m_camera, RwFrameCreate_thunk());
		CameraView(m_camera)->frameBuffer = CameraView(RWS::MainLoop::Render::GetCamera())->frameBuffer;
		CameraView(m_camera)->zBuffer = CameraView(RWS::MainLoop::Render::GetCamera())->zBuffer;
		Rt2dDeviceSetCamera(m_camera);
		Rt2dDeviceSetMetric(0.0f, 0.0f, 640.0f, 480.0f);
	}
}

// 0x413e90
SpriteManager::~SpriteManager()
{
	UnLinkMsg(m_Render_In);
	UnLinkMsg(m_Render_Out);
	UnRegisterMsg(m_Render_In);
	UnRegisterMsg(m_Render_Out);
	if (m_camera) {
		Rt2dDeviceSetCamera(0);
		RwFrame *frame = RwCameraGetFrame(m_camera);
		_rwObjectHasFrameSetFrame(m_camera, 0);
		RwCameraDestroy(m_camera);
		FrameHelper::FrameDestroy(frame);
	}
	g_spriteManager = 0;
}

// 0x414070
// The game aligns the shared exit to 16 B with a lea pad.
void SpriteManager::RemoveSprite(const char *name)
{
	GameSprite *s = FindSpriteByName(name);
	if (s) {
		Script::TFB_GetRemovedSpriteSet()->Insert(s, FLT_MAX);
		Script::TFB_GetActiveSpriteSet()->Remove(s);
		s->Remove();
		if (g_spriteManager)
			g_spriteManager->MarkRenderListDirty();
	}
}

// Near-match of 0x416900: the game emits the constant stores early and pads the shared epilogue to 16 B with jmp + lea.
// 0x416900
// Returns the named sprite if it exists, else a new type 2 text sprite (font tsa16, tint aa 00 00 dd).
GameSprite *SpriteManager::CreateTextSprite(const char *name, const char *text, float x, float y, float width, float height)
{
	GameSprite *found = FindSpriteByName(name);
	if (found)
		return found;
	GameSprite *sp = new GameSprite(name, text);
	sp->m_font = Hud2D::GetFont(0, "tsa16");
	sp->m_visible = 1;
	sp->m_posX = x;
	sp->m_posZ = y;
	sp->m_posY = FLT_MAX;
	sp->m_boxWidth = width;
	sp->m_boxHeight = height;
	sp->m_type = 2;
	sp->m_tint.red = 0xaa;
	sp->m_tint.blue = 0;
	sp->m_tint.green = 0;
	sp->m_tint.alpha = 0xdd;
	strcpy(sp->m_textBuf, text);
	sp->m_format = sp->m_textBuf;
	sp->m_priorityKey = 1;
	sp->m_priority = 1;
	sp->m_drawPriority = 1;
	Script::TFB_GetActiveSpriteSet()->Insert(sp, 0.0f);
	if (g_spriteManager)
		g_spriteManager->MarkRenderListDirty();
	return sp;
}

// 0x416a40
// NOTE: the fifth and sixth argument are (height, width) here but (width, height) for CreateTextSprite: the game forwards them swapped.
void SpriteManager::SetTextSprite(const char *name, const char *text, float x, float y, float height, float width)
{
	GameSprite *s = FindSpriteByName(name);
	if (s) {
		strcpy(s->m_textBuf, text);
		s->m_format = s->m_textBuf;
		return;
	}
	CreateTextSprite(name, text, x, y, width, height);
}

}}

namespace Script {

// 0x413f90
// Re-resolved at the next Update, with every listed sprite whose parent chain contains it. Null: rebuild the render list.
void InvalidateSprite(RWS::TFB::GameSprite *sprite)
{
	RWS::TFB::SpriteManager *mgr = g_spriteManager;
	if (mgr) {
		if (!sprite) {
			mgr->MarkRenderListDirty();
			return;
		}
		if (!mgr->RenderListDirty()) {
			if (sprite->m_resolveGeneration == g_spriteGeneration) {
				int refs = sprite->m_childRefs;
				sprite->m_resolveGeneration = 0;
				if (refs) {
					for (RWS::TFB::GameSprite *other = mgr->m_renderList; other; other = other->m_renderNext) {
						for (RWS::TFB::GameSprite *link = other->m_parent; link; link = link->m_parent) {
							if (link == sprite) {
								InvalidateSprite(other);
								break;
							}
						}
					}
				}
			}
		}
	}
}

}

// 0x413f80
RWS::TFB::SpriteManager *SpriteManager_GetInstance()
{
	return g_spriteManager;
}

// 0x414020
RWS::TFB::GameSprite *FindSpriteByName(const char *name)
{
	if (!name)
		return 0;
	for (Script::SetNode *n = Script::TFB_GetActiveSpriteSet()->m_head; n; n = n->m_next) {
		RWS::TFB::GameSprite *s = static_cast<RWS::TFB::GameSprite *>(n->m_object);
		if (s->m_name && _stricmp(name, s->m_name) == 0)
			return s;
	}
	return 0;
}

// 0x4140d0
void RemoveSpriteByName(const char *name)
{
	if (g_spriteManager)
		g_spriteManager->RemoveSprite(name);
}

// 0x414720
void ShowCaption(const char *name)
{
	if (g_spriteManager && name && *name) {
		g_spriteManager->m_caption = name;
		int nowMs = TFBClock_GetRawTime();
		RWS::TFB::SpriteManager *mgr = g_spriteManager;
		mgr->m_fadeStart = nowMs;
		mgr->m_shownCaption = 0;
	}
}

// 0x414750
void HideCaption(const char *name)
{
	if (g_spriteManager) {
		if (g_spriteManager->m_caption == name) {
			g_spriteManager->m_caption = 0;
			g_spriteManager->m_fadeStart = TFBClock_GetRawTime();
		}
	}
}

// 0x416aa0
RWS::TFB::GameSprite *SpriteMgr_CreateTextSprite(const char *name, const char *text, float x, float y, float width, float height)
{
	if (g_spriteManager)
		return g_spriteManager->CreateTextSprite(name, text, x, y, width, height);
	return 0;
}

// 0x416ae0
void SpriteMgr_SetTextSprite(const char *name, const char *text, float x, float y, float height, float width)
{
	if (g_spriteManager)
		g_spriteManager->SetTextSprite(name, text, x, y, height, width);
}

// Forwarding thunks: ecx (this) passes through the jmp.
struct GnThis_rest {
	ZOOSTERS_NOINLINE void FUN_00402c00();
	void Placement400_Dtor_thunk();
};

void GnThis_rest::FUN_00402c00() { ((Script::Placement *)this)->~Placement(); }
// 0x414790
void GnThis_rest::Placement400_Dtor_thunk()
{
	FUN_00402c00();
}
