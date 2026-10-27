// MainTickHandler (frame timing, clone list, physics step), the Corpus messages and the level's release of game objects.
#define ZOOSTERS_RWS_SENDMSG 1
#include "modules/tfb/renderqueue.h"
#include "tfb/corpus/corpus.h"
#include "modules/fx/gameparticle.h"
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptactor.h"
#include "modules/tfb/gameactor.h"
#include "tfb/corpus/corpus_private.h"
#include "rw/rwcore.h"
#include "modules/tfb/sprites/spritemanager.h"
#include "tfb/collision/broadphase.h"
#include "framework/mainloop/gameclock.h"
#include "framework/core/cresourcemanager.h"
#include "tfb/render/atomicflags.h"
#include "tfb/render/shadow.h"

// GLOBAL: 0x61831c RWS::ERROR_NOT_A_VALID_MESSAGE_ID
// GLOBAL: 0x618308 g_corpusCount
// GLOBAL: 0x618328 g_msgCorpusPhysics
// GLOBAL: 0x618330 g_msgCorpusShadow
// GLOBAL: 0x618338 g_msgCorpusCamera
extern int g_corpusCount;
extern RWS::CEventId g_msgCorpusPhysics;
extern RWS::CEventId g_msgCorpusShadow;
extern RWS::CEventId g_msgCorpusCamera;
namespace RWS { namespace MainLoop { namespace Logic { extern unsigned Rate; } } }


// GLOBAL: 0x5cd16c RWS::MainTickHandler::vftable
// 0x4228c0
// 0x4228b0
RWS::MainTickHandler::~MainTickHandler() {}
RWS::MainTickHandler *g_makeMainTick() { return new RWS::MainTickHandler; }

// 0x424540
RWS::CEventHandler *RWS::MainTickHandler::GetInstance()
{
	static RWS::MainTickHandler s;
	return &s;
}

namespace RWS { extern CEventId iMsgRunningPreTick; extern CEventId iMsgRunningPostTick; extern CEventId iMsgStartSystem; extern CEventId iMsgStopSystem; }

void LevelDestroyClump();
void ShadowSystemClose();
void LevelResetWorlds();
void FreeAllVoices();
void TypeRegistry_ResetAll(int hub);
void ResetOpContexts();
namespace Hud2D { void DestroyFontCache(); }
void StrPool_Destroy();
void LevelResetWorlds_thunk();
extern int g_globalSymbolCount;   // GLOBAL: 0x602ea0 g_globalSymbolCount

// 0x428270
void Level_ReleaseGameObjects()
{
	CloneList_Get()->Clear();
	LevelResetWorlds();
	FreeAllVoices();
	TypeRegistry_ResetAll(g_globalSymbolCount);
	ResetOpContexts();
	Hud2D::DestroyFontCache();
	char *names = RWS::SaveResourceNames();   // port: the resources that stay loaded keep valid names (cresourcemanager.cpp)
	StrPool_Destroy();
	RWS::RestoreResourceNames(names);
	BroadPhase_Reset();
	RwFreeListPurgeAllFreeLists();
}

// 0x4247c0
void Corpus_RegisterMessages()
{
	RWS::CEventHandler::RegisterMsg(g_msgCorpusPhysics, "CORPUS_PHYSICS_PROCESS", 0);
	RWS::CEventHandler::RegisterMsg(g_msgCorpusShadow, "CORPUS_SHADOW_PROCESS", 0);
	RWS::CEventHandler::RegisterMsg(g_msgCorpusCamera, "CORPUS_CAMERA_PROCESS", 0);
	RWS::MainTickHandler::GetInstance()->LinkMsg(RWS::iMsgRunningPreTick, 0, 0x8000);
	RWS::MainTickHandler::GetInstance()->LinkMsg(RWS::iMsgRunningPostTick, 0, 0x8000);
	RWS::MainTickHandler::GetInstance()->LinkMsg(RWS::iMsgStartSystem, 0, 0x8000);
	RWS::MainTickHandler::GetInstance()->LinkMsg(RWS::iMsgStopSystem, 0, 0x8000);
	ShadowSystemOpen();
	LevelResetWorlds_thunk();
}

// 0x4282b0
void Corpus_UnregisterMessages()
{
	LevelDestroyClump();
	ShadowSystemClose();
	RWS::MainTickHandler::GetInstance()->UnLinkMsg(RWS::iMsgRunningPreTick);
	RWS::MainTickHandler::GetInstance()->UnLinkMsg(RWS::iMsgRunningPostTick);
	RWS::MainTickHandler::GetInstance()->UnLinkMsg(RWS::iMsgStartSystem);
	RWS::MainTickHandler::GetInstance()->UnLinkMsg(RWS::iMsgStopSystem);
	Level_ReleaseGameObjects();
	RWS::CEventHandler::UnRegisterMsg(g_msgCorpusPhysics);
	RWS::CEventHandler::UnRegisterMsg(g_msgCorpusShadow);
	RWS::CEventHandler::UnRegisterMsg(g_msgCorpusCamera);
}

// 0x428360
void Corpus_SendPhysicsMessage()
{
	RWS::CMsg msg(g_msgCorpusPhysics, (void *)-1);
	RWS::SendMsg(msg);
	for (Script::SetNode *n = CloneList_Get()->m_head; n; n = n->m_next) {
		RpClump *clump = ((Script::ClumpCloneRef *)n->m_object)->m_clump.ptr();
		RwLLLink *cur = clump->atomicList.link.next;
		while (cur != &clump->atomicList.link) {
			RwLLLink *nx = cur->next;
			RpAtomic *atomic = RpAtomicFromClumpLink(cur);
			PORT_RQ(18);
			RenderQueue_AddAtomic(RwCameraGetCurrentCamera(), atomic);
			if (!atomic)
				break;
			cur = nx;
		}
	}
}

struct MsgPod { RWS::CRegisteredMsgs *id; int data; };
static inline void SendIdData(const RWS::CEventId &id, void *data)
{
	if (id != RWS::ERROR_NOT_A_VALID_MESSAGE_ID && id->Erm.p_next) {
		RWS::CMsg m(id, data);
		RWS::_SendMsg(m);
	}
}
// HandleEvents: PreTick is frame timing and the clone list, PostTick the physics step; Start/StopSystem reset.
// GLOBAL: 0x618310 g_corpusGravityDt
// GLOBAL: 0x61830c g_corpusGravityDt2
// GLOBAL: 0x618314 g_frameRate
// GLOBAL: 0x618318 g_frameDt
// GLOBAL: 0x618368 g_tickSum
// GLOBAL: 0x61836c g_tickBase
// the function-local `static s_lastTick` of HandleEvents and its init guard are game globals
// GLOBAL: 0x618370 ?s_lastTick@?1??HandleEvents@MainTickHandler@RWS@@UAEXAAVCMsg@3@@Z@4IA
// GLOBAL: 0x618374 ?$S4@?1??HandleEvents@MainTickHandler@RWS@@UAEXAAVCMsg@3@@Z@4IA
// GLOBAL: 0x6221b8 g_vsyncFlipCount
extern float g_corpusGravityDt, g_corpusGravityDt2, g_frameRate, g_frameDt;
extern unsigned g_tickSum, g_tickBase;
extern int g_vsyncFlipCount;
int *CutScene_StackSlot(int delta);              // 0x436c00
float GetPlayerInput(int input, float threshold, int player);
namespace RWS { namespace MainLoop { namespace Render { void SetEnabled(int e); } } }   // 0x4626f0
void CutScene_Begin(int a);
int TFBClock_GetTime();
void TFBClock_SetTime(int a);
void ScriptActors_Update(int a);
int MainLoop_LogicIsEnabled();
void CameraResetState();
void Corpus_RunPhysicsStep();
void Camera_PostTickFollowUpdate();              // 0x41dc60
int _ftol2(float f);

static __forceinline void TrimCloneList()
{
	int count = CloneList_Get()->m_count;
	if (count > 20) {
		Script::SetNode *node = CloneList_Get()->m_head;
		do {
			Script::SetNode *nextNode = node->m_next;
			Script::ClumpCloneRef *item = (Script::ClumpCloneRef *)node->m_object;
			unsigned expiry = item->m_08;
			unsigned cur = TFBClock_GetRawTime();
			if (expiry != 0) {
				if (cur > expiry) {
					CloneList_Get()->RemoveNode(node);
				} else {
					unsigned char rgba[4];
					rgba[2] = 0x80;
					rgba[1] = 0x80;
					rgba[0] = 0x80;
					rgba[3] = (unsigned char)((expiry - TFBClock_GetRawTime()) * 127 / 2000);
					ClumpSetAtomicColour(item->m_clump.ptr(), (RwRGBA *)rgba);
				}
			} else {
				item->m_08 = cur + 2000;
			}
			node = nextNode;
		} while (--count > 20);
	}
}
// 0x427f30
void RWS::MainTickHandler::HandleEvents(CMsg &msg)
{
	static unsigned s_lastTick = RWS::MainLoop::Render::GetFrameCount() - 10000;
	intptr_t data = 0;
	if (msg.Id == iMsgRunningPreTick.pEventId) {
		unsigned nextTick = g_tickBase + 0x10;
		unsigned now = RWS::MainLoop::Render::GetFrameCount();
		int cutScene = *CutScene_StackSlot(0);
		if (cutScene != -1 && (cutScene != 1 || !(GetPlayerInput(4, 0, 0) > 0.15f))) {
			MainLoop::Render::SetEnabled(1);
			bool measured = false;   // the rate of the last tick, when vsync paced it at 15 fps or more
			if (g_vsyncFlipCount <= 1 && now - s_lastTick <= 1) {
				g_frameRate = 1000.0f / (float)(nextTick - g_tickBase);
				measured = !(g_frameRate < 15.0f);
			}
			if (!measured) {   // else the logic rate
				g_frameRate = (float)MainLoop::Logic::Rate;
				g_tickBase = nextTick - (int)(1000.0f / g_frameRate + 0.5f);
			}
		} else {
			if (cutScene != -1)
				CutScene_Begin(-1);
			MainLoop::Render::SetEnabled(0);
			g_frameRate = 30.0f;
			g_tickBase = nextTick - 0x21;
			TFBClock_SetTime(TFBClock_GetTime() + (g_tickBase - nextTick));
		}
		s_lastTick = now;
		g_tickSum += nextTick - g_tickBase;
		float gravityDt = (g_frameDt = 1.0f / g_frameRate) * 12.0f;
		g_corpusGravityDt = gravityDt;
		g_corpusGravityDt2 = g_corpusGravityDt * g_frameDt;
		ScriptActors_Update(g_tickSum);
		g_tickBase = nextTick;
		TrimCloneList();
		SendIdData(g_msgCorpusPhysics, (void *)(intptr_t)data);
	} else if (msg.Id == iMsgRunningPostTick.pEventId) {
		Corpus_RunPhysicsStep();
		Camera_PostTickFollowUpdate();
		RWS::FX::UpdateGameParticles();
		SpriteManager_GetInstance()->Update(GetActiveGameCamera());
	} else if (msg.Id == iMsgStartSystem.pEventId) {
		if (MainLoop_LogicIsEnabled() == 0) {
			g_tickSum = 0;
			s_lastTick = RWS::MainLoop::Render::GetFrameCount() - 10000;
			CameraResetState();
		}
	} else if (msg.Id == iMsgStopSystem.pEventId) {
		g_corpusCount = 0;
	}
}


