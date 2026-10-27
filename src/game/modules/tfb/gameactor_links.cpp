// ScriptActor message-link toggles 0x42c900/0x42c9a0/0x42ca50. Stand-in class with raw offsets.
#include "script/script.h"
#include "framework/core/rwsguid.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/attributemacros.h"
#include "framework/core/cclassfactory.h"
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/linearallocationpolicy.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/ceventhandler.h"
#include "tfb/render/atomicflags.h"
#include "modules/tfb/gameactor.h"
#include "tfb/corpus/corpus.h"
using namespace RWS;

extern CEventId g_msgCorpusCamera;  // GLOBAL: 0x618338 g_msgCorpusCamera
extern CEventId g_msgCorpusShadow;  // GLOBAL: 0x618330 g_msgCorpusShadow

// The members of the real GameActor (Collider::m_clump, ActorParams::m_flags, m_corpus); the layout above is the 32-bit one.
typedef RWS::TFB::GameActor ActorLinkToggles;   // the game's stand-in name of these three members (the annotations keep it)
#define LT_CLUMP m_clump
#define LT_FLAGS m_params.m_flags
#define LT_CORPUS m_corpus

// 0x42c900
void ActorLinkToggles::SetSolidCollider(bool on)
{
	if (on) {
		LT_FLAGS |= 1;
		if (IsMeshCollidee() && LT_CLUMP) {
			ClumpSetAtomicFlags(LT_CLUMP, 0x100);
			LT_CORPUS->UnLinkMsg(g_msgCorpusCamera);
			LT_CORPUS->LinkMsg(g_msgCorpusCamera, 0, 0x8000);
			return;
		}
	} else
		LT_FLAGS &= ~1u;
	if (LT_CLUMP) {
		ClumpClearAtomicFlags(LT_CLUMP, 0x100);
		LT_CORPUS->UnLinkMsg(g_msgCorpusCamera);
	}
}

// 0x42c9a0
void ActorLinkToggles::SetMeshCollidee(bool on)
{
	if (on) {
		LT_FLAGS |= 0x200;
		if (IsSolidCollider() && LT_CLUMP) {
			ClumpSetAtomicFlags(LT_CLUMP, 0x100);
			LT_CORPUS->UnLinkMsg(g_msgCorpusCamera);
			LT_CORPUS->LinkMsg(g_msgCorpusCamera, 0, 0x8000);
			return;
		}
	} else
		LT_FLAGS &= ~0x200u;
	if (LT_CLUMP) {
		ClumpClearAtomicFlags(LT_CLUMP, 0x100);
		LT_CORPUS->UnLinkMsg(g_msgCorpusCamera);
	}
}

// 0x42ca50
void ActorLinkToggles::SetNoShadow(bool on)
{
	if (on) {
		LT_FLAGS |= 8;
		if (LT_CLUMP) {
			ClumpSetAtomicFlags(LT_CLUMP, 0x20);
			LT_CORPUS->UnLinkMsg(g_msgCorpusShadow);
		}
	} else {
		LT_FLAGS &= ~8u;
		if (LT_CLUMP) {
			ClumpClearAtomicFlags(LT_CLUMP, 0x20);
			LT_CORPUS->UnLinkMsg(g_msgCorpusShadow);
			LT_CORPUS->LinkMsg(g_msgCorpusShadow, 0, 0x8000);
		}
	}
}
