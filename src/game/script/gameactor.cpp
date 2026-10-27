#include <float.h>
// GameActor / Placement: control mode, reset, spawn, leave, body and part creation.
#define ZOOSTERS_RWS_SENDMSG 1
#include "tfb/corpus/corpus.h"
#include "script/actor.h"
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptactor.h"
#include "modules/tfb/gameactor.h"
#include "modules/tfb/cprotoactor.h"
#include "modules/tfb/ctfbmodel.h"
#include "script/ops_message.h"
#include "script/types/partentry.h"
#include "script/types/actorturret.h"
#include "script/types/scriptsystem.h"
#include "rw/rwcore.h"
#ifdef ZOOSTERS_DEBUGUI
void port_debugui_actor_life(const void *actor, int alive);   // port/debugui/debugui_panels.cpp
#endif

// the three live-actor sets and the dead set (their getters are function-local statics)

// scaffold (the real constructor is matched as WaActor in actorprotos.cpp): defined here only so that cl infers GameActor's
// constructor nothrow, as in the game where both live in one translation unit
namespace Script { void p4b_ScriptActor_init_sets(void *actor); }                  // script/actorprotos.cpp
__declspec(noinline) Script::ScriptActor::ScriptActor() { Script::p4b_ScriptActor_init_sets(this); }

namespace Script {

// 0x42e530
void Placement::SetControlMode(int mode, int pin)
{
	RWS::TFB::GameActor *actor = static_cast<RWS::TFB::GameActor *>(this);
	int old = actor->m_controlMode;
	actor->m_controlMode = mode;
	if (old >= 2) {
		if (mode < 2)
			actor->CreateBody();
	} else if (mode >= 2) {
		if (actor->m_spawnSource != 0) {
			actor->m_controlMode = 3;
			((Script::ReferenceSet *)static_cast<ScriptActor *>(this)->m_wayPoints)->Clear();
		}
		actor->ReleaseBody();
		actor->m_embedded = 0;
		actor->m_velocity.z = 0.0f;
		actor->m_velocity.y = 0.0f;
		actor->m_velocity.x = 0.0f;
	}
	Corpus *c = actor->m_corpus;
	if (c) {
		c->m_controller = pin;
		actor->m_corpus->SetPlayerControl(mode == 0, pin);
	}
}

}  // namespace Script

namespace RWS { namespace TFB {

// 0x42e700
void GameActor::Reset()
{
	const ActorParams *attributes = (const ActorParams *)m_proto->m_attributes;
	m_params.m_activationRange = attributes->m_activationRange;
	m_params.m_deactivationRange = attributes->m_deactivationRange;
	m_matrix = m_proto->m_frame;
	if (m_controlMode == 3) {
		if (TFB_GetDeadSet()->Remove(this) ||
		    (((PlacementIdW *)this)->IsPlacementRemoved() && !TFB_GetInactiveSet()->FindNode(this) &&
		     !TFB_GetNPCSet()->FindNode(this) && !TFB_GetPlayerSet()->FindNode(this)))
			TFB_GetInactiveSet()->Insert(this, FLT_MAX);
	}
	SetControlMode(2, -1);
}

} }

// Mirror kept: GameActor::Spawn calls GameActor::ActivateActor (same file) as a separate function; cl 13.10 inlines it (201 -> 282 B)
struct ActorCalls2 {
	void ActivateActor(int a);        // 0x437d40 (ret 4)
};
// GLOBAL: 0x437d40 ?ActivateActor@ActorCalls2@@QAEXH@Z

namespace RWS { namespace TFB {

// GLOBAL: 0x5cd728 RWS::TFB::GameActor::vftable
// GLOBAL: 0x5cd2bc Script::OpContext::vftable

// Near-miss: 0x42d7b0 RWS::TFB::GameActor::GameActor (cl sinks the constant stores (-FLT_MAX, 1.0f) behind the zero stores and puts
// the vptr store last; the game keeps the source order. Kept as the scaffold for Spawn.)
// 0x42d7b0 RWS::TFB::GameActor::GameActor: full store set identical (62-row diff: 8 stores only-ours vs 8 only-game are the same stores reordered: -FLT_MAX/1.0f constants, both vptr stores, m_proto); no reads between stores, so no behaviour difference
GameActor::GameActor(CProtoActor *script)
{
	m_min.m_collider = 0;
	m_min.m_point.x = -FLT_MAX;
	m_max.m_collider = 0;
	m_max.m_point.x = -FLT_MAX;
	m_embedded = 0;
	AssignSerial();
	m_inverseMass = 1.0f;
	m_clump = 0;
	m_velocity.z = 0.0f;
	m_velocity.y = 0.0f;
	m_velocity.x = 0.0f;
	m_refCount = 0;
	m_corpus = 0;
	m_spawnSource = 0;
	m_scriptClockHold = 0;
	m_timedMoveMs = 0;
	m_proto = script;
	m_script.m_behaviorNode = 0;
	m_script.m_startTime = 0;
	m_script.m_0c = 0;
	m_script.m_nodes = 0;
	m_script.m_localCount = 0;
	m_heading = 0.0f;
	m_tilt = 0.0f;
	m_roll = 0.0f;
	m_matrix.up.x = 0.0f;
	m_matrix.right.z = 0.0f;
	m_matrix.right.y = 0.0f;
	m_matrix.at.y = 0.0f;
	m_matrix.at.x = 0.0f;
	m_matrix.up.z = 0.0f;
	m_matrix.pos.z = 0.0f;
	m_matrix.pos.y = 0.0f;
	m_matrix.pos.x = 0.0f;
	m_matrix.flags = (m_matrix.flags & 0xfffdffff) | 3;
	m_targetFacing = FLT_MAX;
	m_owner = this;
	m_matrix.at.z = 1.0f;
	m_matrix.up.y = 1.0f;
	m_matrix.right.x = 1.0f;
#ifdef ZOOSTERS_DEBUGUI
	port_debugui_actor_life(this, 1);   // the overlay reads only live actors (g_cameraFollowActor outlives its actor: port/debugui/debugui_panels.cpp)
#endif
}

// 0x42da70
intptr_t GameActor::Spawn(intptr_t pos, int flag)
{
	GameActor *copy = new GameActor(m_proto);
	copy->m_params = m_params;
	copy->m_initialFacing = m_initialFacing;
	copy->m_tilt = m_tilt;
	copy->m_roll = m_roll;
	copy->ReleaseBody();
	copy->m_spawnSource = this;
	Vec3 *posVec = (Vec3 *)pos;
	copy->m_origin.x = posVec->x;
	copy->m_origin.y = posVec->y;
	copy->m_origin.z = posVec->z;
	GetName();
	char *src = (char *)copy->m_spawnSource->m_wayPoints;
	((Script::ReferenceSet *)copy->m_wayPoints)->Assign((Script::ScriptReference *)src, 0);   // src is the spawn source held as a char * here
	((ActorCalls2 *)copy)->ActivateActor(1);
	if ((unsigned char)flag)
		copy->m_scriptClockHold = 1;
	return (intptr_t)copy;
}

} }

static __forceinline Script::OpContext *ActorVariableList(RWS::TFB::GameActor *a) { return a->GetVariableList(0); }

// Mirror kept: DeactivateAllActors and ActivateActor call Placement::SetControlMode as a separate function in the game, but cl 13.10 inlines
// a same-file callee (103 -> 209 B, 105 -> 151 B); `noinline` on the definition then inlines ActivateActor into Spawn instead. The mirror struct is the call-site view.
struct CtlModeCalls {
	void SetControlMode(int mode, int pin);   // 0x42e530
};
// GLOBAL: 0x42e530 ?SetControlMode@CtlModeCalls@@QAEXHH@Z

// 0x435170
void Script::ReferenceSet::DeactivateAllActors()
{
	for (Script::SetNode *n = m_head; n != 0; n = n->m_next) {
		RWS::TFB::GameActor *a = (RWS::TFB::GameActor *)n->m_object;
		Script::OpContext *vars = ActorVariableList(a);
		if (vars)
			vars->Reset();
		((CtlModeCalls *)a)->SetControlMode(2, -1);
		Script::ScriptReference *src = a->GetSpawnSource();
		if (src && ((Script::ReferenceSet *)&static_cast<Script::Placement *>(src)->m_20)->Remove(a))
			src->AdjustRefCount(-1);
	}
	Clear();
}

// ---- 0x437db0: an actor leaves play: reset its script and variables, tell the player/NPC sets, release its spawner or park it
void StopVoicesForObject(Script::ScriptReference *obj, int a);   // 0x419220 (cdecl)
Script::ScriptReference *TFBMember_DeactivateMessage();                  // 0x43a520
Script::ScriptReference *TFBMember_DeathMessage();                  // 0x43a5e0
Script::ScriptReference *TFBMember_ActivateMessage();                  // 0x43a460


// 0x437db0
void RWS::TFB::GameActor::DeactivateActor(bool park)
{
	StopVoicesForObject(this, 1);
	((Script::ScriptActor *)this)->SetScriptClockHold(0);
	Script::OpContext *vars = ActorVariableList((RWS::TFB::GameActor *)this);
	if (vars)
		vars->Reset();
	Vec3 *pos = GetVector(this);
	PostMessage(TFBMember_DeactivateMessage(), TFB_GetPlayerSet(), pos, 0.0f);
	PostMessage(TFBMember_DeactivateMessage(), TFB_GetNPCSet(), pos, 0.0f);
	if (m_controlMode == 3) {
		PostMessage(TFBMember_DeathMessage(), TFB_GetPlayerSet(), pos, 0.0f);
		PostMessage(TFBMember_DeathMessage(), TFB_GetNPCSet(), pos, 0.0f);
	}
	Script::ScriptReference *src = GetSpawnSource();
	if (src) {
		if (((Script::ReferenceSet *)&static_cast<Script::Placement *>(src)->m_20)->Remove(this))
			src->AdjustRefCount(-1);
	} else if (park) {
		if (m_controlMode == 2)
			TFB_GetInactiveSet()->Insert(this, FLT_MAX);
		else if (!((PlacementIdW *)this)->IsPlacementRemoved())
			TFB_GetDeadSet()->Insert(this, FLT_MAX);
	}
}

// 0x437d40
void RWS::TFB::GameActor::ActivateActor(bool park)
{
	Vec3 *pos = GetVector(this);
	PostMessage(TFBMember_ActivateMessage(), TFB_GetPlayerSet(), pos, 0.0f);
	PostMessage(TFBMember_ActivateMessage(), TFB_GetNPCSet(), pos, 0.0f);
	((CtlModeCalls *)this)->SetControlMode(1, -1);
	m_20.m_1c = 0;
	if (park)
		TFB_GetNPCSet()->Insert(this, FLT_MAX);
}

using namespace Script;
ScriptReference *TFBUser_GlobalPlacementRemoved();   // 0x42f370 "Global Placement Removed"
ScriptReference *TFBUser_LevelPlacementRemoved();   // 0x42f3f0 "Level Placement Removed"

// 0x42f470
bool PlacementIdW::IsPlacementRemoved()
{
	UserReference *globalRemoved = (UserReference *)TFBUser_GlobalPlacementRemoved();
	int savedIndex = globalRemoved->m_index;
	globalRemoved->m_index = m_placementId;
	bool removedGlobal = globalRemoved->GetInt(globalRemoved) != 0;
	globalRemoved->m_index = savedIndex;
	UserReference *levelRemoved = (UserReference *)TFBUser_LevelPlacementRemoved();
	int savedLevelIndex = levelRemoved->m_index;
	levelRemoved->m_index = m_placementId % 800;
	bool removedLevel = levelRemoved->GetInt(levelRemoved) != 0;
	levelRemoved->m_index = savedLevelIndex;
	return removedGlobal || removedLevel;
}


namespace RWS { namespace TFB {

// 0x42e390
void GameActor::CreateBody()
{
	if ((void *)m_proto->m_linkedHandler != 0) {
		Corpus *c = new Corpus((void *)m_proto->m_linkedHandler, this);
		m_corpus = c;
		RwFrameTransform(static_cast<RwFrame *>(c->m_frame), &m_proto->m_frame, rwCOMBINEREPLACE);
		m_corpus->m_model.PlaySlot(0, 0.0f);
		m_corpus->SetEnabled(1);
		*(Vec3 *)&m_corpus->m_contactPoint = m_origin;
		CreateParts();
	}
	ClearLinks();
	Corpus *c = m_corpus;
	UpdateBounds(c ? &c->m_frame->modelling : &m_matrix, 0);
	m_warpTime = *Script::ScriptSystem::GetScriptTimePtr();
	m_destination = m_origin;
	PlaceAt(&m_origin);
	m_facing = m_initialFacing;
	m_heading = 0.0f;
	m_roll = 0.0f;
	m_tilt = 0.0f;
	m_velocity.z = 0.0f;
	m_velocity.y = 0.0f;
	m_velocity.x = 0.0f;
	m_targetFacing = FLT_MAX;
	m_tint[2] = 0x80;
	m_tint[1] = 0x80;
	m_tint[0] = 0x80;
	m_tint[3] = 0xff;
}

} }

// ---- 0x42e240: build the child objects (attachment points, turrets, corpus parts) of an actor from its script descriptor

// the descriptor is the CTFBModel the proto linked (CProtoActor::m_linkedHandler, +0x188): m_numEntries / m_entries are +0x1c / +0x20

namespace RWS { namespace TFB {

// 0x42e240
void GameActor::CreateParts()
{
	CTFBModel *model = (CTFBModel *)m_proto->m_linkedHandler;
	if (model != 0 && (unsigned)model->m_numEntries != 0 && m_clump != 0) {
		for (unsigned i = 0; i < (unsigned)model->m_numEntries; i++) {
			PartEntry *e = &((PartEntry *)model->m_entries)[i];
			if (e->m_kind <= 2) {
				Script::ActorTurret *part = new Script::ActorTurret(this, e);
				if (part != 0) {
					part->m_index = (int)e->m_sortKey;
					part->m_pos.x = part->m_pos.y = part->m_pos.z = 0.0f;
					switch (e->m_kind) {
					case 0:
						reinterpret_cast<Script::ReferenceSet *>(m_attachmentPoints)->Insert(part, (float)e->m_sortKey);
						break;
					case 1:
						reinterpret_cast<Script::ReferenceSet *>(m_turrets)->Insert(part, (float)e->m_sortKey);
						break;
					case 2:
						m_corpus->m_feet.Insert(part, (float)e->m_sortKey);
						break;
					}
				}
			}
		}
	}
}

} }

using namespace Script;
namespace Script {


// 0x431be0
ScriptReference *Placement::FinishSpawn(intptr_t pos, float facing, int flag)
{
	ScriptReference *obj = (ScriptReference *)Spawn(pos, flag);
	if (obj) {
		AdjustRefCount(1);
		((ReferenceSet *)&m_20)->Insert(obj, FLT_MAX);
		((PositionMeasurement *)obj)->SetFacing(facing, 1);
	}
	return obj;
}

}  // namespace Script

inline void CtlModeCalls::SetControlMode(int mode, int pin) { ((Script::Placement *)this)->Script::Placement::SetControlMode(mode, pin); }
inline void ActorCalls2::ActivateActor(int a) { ((RWS::TFB::GameActor *)this)->ActivateActor((unsigned char)a != 0); }   // (the callee reads the low byte only: callers pass wider ints)
