#define ZOOSTERS_RWS_SENDMSG 1
#include "tfb/corpus/corpus.h"
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptactor.h"
#include "modules/tfb/gameactor.h"
#include "rw/rpworld.h"
#include "modules/tfb/ctfbmodel.h"
#include "tfb/render/shadow.h"
#include "rw/rwcore.h"
#include "tfb/render/atomicflags.h"
#include "tfb/render/tfbclump.h"
#include "framework/toolkits/world/framehelper.h"

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


// 0x424860
bool CorpusLink::Release()
{
	if (m_carrier) {
		if (m_index == -1 && m_carrier->m_clump && m_id == RWS::TFB::ActorOf(*m_carrier).m_corpus->m_id)
			m_attachPoint->m_riders.Remove((Script::ScriptReference *)m_owner);
		m_carrier->m_owner->AdjustRefCount(-1);
		m_carrier = 0;
		return true;
	}
	return false;
}

// 0x424500
void Corpus::SetPlayerControl(bool pinned, int v)
{
	if (pinned) {
		m_flags |= CORPUS_PLAYER_CONTROLLED;
		m_controller = v;
	} else {
		m_flags &= ~CORPUS_PLAYER_CONTROLLED;
	}
}

// 0x4248c0
void Corpus::DetachFromCarrier()
{
	if (m_link.Release()) {
		m_linkPriority = 0x8000;
		RWS::CEventHandler::SetLinkPriority(*this, g_msgCorpusPhysics, 0, 0x8000);
		RWS::TFB::GameActor *owner = m_owner;
		float heading = owner->m_heading + owner->m_facing;   // the carrier's yaw becomes the body's own facing
		if (heading < 0.0f)
			heading += 360.0f;
		else if (heading >= 360.0f)
			heading -= 360.0f;
		owner->m_facing = heading;
		owner->m_heading = 0.0f;
		m_stateTime = 0.0f;
	}
}

// 0x424950
void Corpus::LeaveGround(const RwV3d *pos, bool b)
{
	if (pos) {
		m_flags &= ~CORPUS_STEP_DONE;
		m_lastFrame.pos = *pos;
	}
	DetachFromCarrier();
	if (!(m_flags & CORPUS_AIRBORNE)) {
		m_flags |= (CORPUS_LEFT_GROUND | CORPUS_AIRBORNE);
		m_riseTime = 0.0f;
		if (b && m_owner->m_params.m_jumpDelay == 0) {
			m_lateJumpTime = 10.0f / (float)RWS::MainLoop::Logic::Rate;
			m_jumpSpeed = m_owner->m_params.m_initialJumpVelocity;
		} else {
			m_lateJumpTime = 0.0f;
		}
	}
}

// 0x426460
void Corpus::AttachToCarrier(RWS::TFB::GameActor *carrier)
{
	RWS::TFB::Collider *collider = carrier;
	if (m_link.m_carrier != collider) {
		((Script::ScriptReference *)carrier)->AdjustRefCount(1);
		if (m_link.m_index >= 0) {
			DetachFromCarrier();
		} else {
			DetachFromCarrier();
			if (!(m_flags & CORPUS_AIRBORNE)) {
				m_flags |= (CORPUS_LEFT_GROUND | CORPUS_AIRBORNE);
				m_riseTime = 0.0f;
				m_lateJumpTime = 0.0f;
			}
			m_stateTime = 0.0f;
			m_link.m_attachPoint->m_riders.Insert((Script::ScriptReference *)m_owner, 3.4028235e+38f);
		}
		m_link.m_carrier = carrier;
		m_link.m_id = carrier->m_corpus->m_id;
	}
	unsigned short carrierPrio = carrier->m_corpus->m_linkPriority;
	if (m_linkPriority != carrierPrio - 1) {
		m_linkPriority = carrierPrio - 1;
		RWS::CEventHandler::SetLinkPriority(*this, g_msgCorpusPhysics, 0, carrierPrio - 1);
	}
}

struct ActorOps { void SetNoShadow(int a); };
// GLOBAL: 0x42ca50 ActorOps::SetNoShadow

// 0x424bd0
void RWS::TFB::GameActor::PlaceAt(const Vec3 *v)
{
	Corpus *c = m_corpus;
	if (c) {
		c->LeaveGround((const RwV3d *)v, false);
		return;
	}
	if (m_controlMode < 2) {
		*(Vec3 *)&m_matrix.pos = *v;
		RwMatrixUpdate(&m_matrix);
		if (m_timedMoveMs == 0)
			UpdateBounds(&m_matrix, 0.0f);
	}
}

// Declared by the header's classes; no body here.
// GLOBAL: 0x4d9580 RWS::RwFrameSetIdentity
// GLOBAL: 0x43e6f0 Script::ScriptReference::GetType
// GLOBAL: 0x43e6f0 Script::ScriptReference::GetBaseType
// GLOBAL: 0x42cbe0 Script::ScriptReference::GetKind

struct RwFrame;
namespace RWS {
RwFrame *FindHAnimHierarchyCallback(RwFrame *pFrame, void *pData);   // 0x449270
namespace ClumpHelper {
extern CEventId iMsgPreRpClumpDestroy;                               // 0x629788
void SetEventHandler(RpClump &clump, CEventHandler *pEventHandler);  // 0x44ee20
}
}
namespace RWS { namespace MainLoop { namespace Render { unsigned GetFrameCount(); } } }  // 0x462710

// 0x429710
// 0x426860
Corpus::~Corpus()
{
	RpClump *&oc = m_owner->m_clump;
	if (oc) {
		RWS::ClumpHelper::Destroy(oc);
		oc = 0;
	}
	UnLinkMsg(RWS::ClumpHelper::iMsgPreRpClumpDestroy);
}

// >0 enable, 0 disable (flag), <0 tear down
// 0x424a00
void Corpus::SetEnabled(int enable)
{
	if (enable > 0) {
		m_owner->ClearLinks();
		if (m_flags & CORPUS_NO_PHYSICS)
			m_flags &= ~1;
		else
			LinkMsg(g_msgCorpusPhysics, 0, m_linkPriority);
		RpClump *clump = m_owner->m_clump;
		if (clump) {
			float tmp = (m_owner->m_params.m_activationRange - (float)(unsigned)m_owner->m_params.m_fadePercent * m_owner->m_params.m_activationRange * 0.01f) * 12.0f;
			ClumpSetIsVisible(clump, 1, tmp,
			             m_owner->m_params.m_activationRange * 12.0f);
			((ActorOps *)m_owner)->SetNoShadow((m_owner->m_params.m_flags >> 3) & 0xffffff01);
			if ((unsigned char)m_owner->IsSolidCollider() && (unsigned char)m_owner->IsMeshCollidee()) {
				ClumpSetAtomicFlags(m_owner->m_clump, 0x100);
				LinkMsg(g_msgCorpusCamera, 0, 0x8000);
			} else {
				ClumpClearAtomicFlags(m_owner->m_clump, 0x100);
			}
		}
	} else {
		m_owner->Collider::Reset();
		UnLinkMsg(g_msgCorpusCamera);
		RpClump *clump = m_owner->m_clump;
		if (clump && enable < 0) {
			ClumpSetIsVisible(clump, 0, -1.0f, -1.0f);
			ClumpSetAtomicFlags(m_owner->m_clump, 0x20);
			ClumpClearAtomicFlags(m_owner->m_clump, 0x100);
			UnLinkMsg(g_msgCorpusPhysics);
			UnLinkMsg(g_msgCorpusShadow);
		} else {
			m_flags |= 1;
		}
	}
}

// 0x4265b0
Corpus::Corpus(void *model, RWS::TFB::GameActor *owner)
	: m_owner(owner), m_modelPtr((CTFBModel *)model), m_flags(CORPUS_AIRBORNE), m_controller(-1), m_id(g_corpusCount++), m_linkPriority(0x8000), m_jumpPressed(0),
	  m_riseTime(0.0f), m_lateJumpTime(0.0f), m_jumpHoldTime(0.0f), m_standSpeed(0.05f), m_animOverride(-1), m_idleSet(0), m_prevIdleSet(0), m_surface(0), m_footstepSound(0)
{
	RpClump *clump = ((CTFBModel *)model)->CloneClump();
	if (clump) {
		TFBPlugin_B1_SetRenderFrame(clump, RWS::MainLoop::Render::GetFrameCount());
		bool hc = (m_owner->m_params.m_flags >> 11) & 1;
		if (hc)
			ClumpSetAtomicFlags(clump, 0x1000);
		if (clump->parent == 0) {
			RwFrame *f = RwFrameCreate_thunk();
			RwFrameSetIdentity(f);
			clump->parent = f;
		}
		m_frame = (RwFrame *)clump->parent;
		void *hier;
		if (clump->parent) {
			void *h = 0;
			RWS::FindHAnimHierarchyCallback((RwFrame *)clump->parent, &h);
			hier = h;
		} else {
			hier = 0;
		}
		m_model.InitFromModel((CTFBModel *)model, (RpHAnimHierarchy *)hier);
		m_model.SetTarget(clump);
		m_model.PlaySlot(0, 0.0f);
		m_prevIdleSet = 0;
		RpClump *&ownerClump = owner->m_clump;
		if (ownerClump != clump) {
			if (ownerClump)
				RWS::ClumpHelper::Destroy(ownerClump);
			ownerClump = clump;
		}
		m_velocity.x = 0.0f;
		m_velocity.y = 0.0f;
		m_velocity.z = 0.0f;
		m_groundColor[0] = 0x80;
		m_groundColor[1] = 0x80;
		m_groundColor[2] = 0x80;
		m_groundColor[3] = 0xff;
		LinkMsg(RWS::ClumpHelper::iMsgPreRpClumpDestroy, "RwFrame*", 0x8000);
		RWS::ClumpHelper::SetEventHandler(*owner->m_clump, this);
		ShadowClumpWalk(clump);
		// m_lastFrame = identity
		m_lastFrame.at.z = 1.0f;
		m_lastFrame.up.y = 1.0f;
		m_lastFrame.right.x = 1.0f;
		unsigned flags = m_lastFrame.flags;
		m_lastFrame.up.x = 0.0f;
		m_lastFrame.right.z = 0.0f;
		m_lastFrame.right.y = 0.0f;
		m_lastFrame.at.y = 0.0f;
		m_lastFrame.at.x = 0.0f;
		m_lastFrame.up.z = 0.0f;
		m_lastFrame.pos.z = 0.0f;
		m_lastFrame.pos.y = 0.0f;
		m_lastFrame.pos.x = 0.0f;
		m_lastFrame.flags = flags | 0x20003;
		m_link.m_owner = m_owner;
	}
}


inline void ActorOps::SetNoShadow(int a) { ((RWS::TFB::GameActor *)this)->SetNoShadow((unsigned char)a != 0); }   // (the callee reads the low byte only: the caller passes (flags >> 3) & 0xffffff01)
