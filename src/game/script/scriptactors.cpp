// 0x437ec0, the per-frame update of the script actors: expires sender messages, stores the script
// Runs the scripts of the actors in the three sets (players / NPCs / inactive) and moves actors between them by control mode.
#include "script/ops_message.h"
#include "script/types/scriptsystem.h"
#include "modules/tfb/gameactor.h"
#include <float.h>
using namespace Script;

// GLOBAL: 0x61f65c g_curScript
// GLOBAL: 0x61f658 g_scriptPC
// GLOBAL: 0x621f34 g_voiceDefaultRef
// GLOBAL: 0x437a20 TFB_GetPlayerSet
// GLOBAL: 0x437b00 TFB_GetNPCSet
// GLOBAL: 0x437be0 TFB_GetInactiveSet
// GLOBAL: 0x43a460 TFBMember_ActivateMessage
// GLOBAL: 0x435ac0 ValueSlide_UpdateAll
extern ScriptReference *g_voiceDefaultRef;
// 0x409700 is thiscall (game name PlayerObj::Dist: this->GetPosition(ownerA) vs b->GetPosition(ownerB))
struct DistSelf437 { float FUN_00409700(ScriptReference *ownerA, ScriptReference *b, ScriptReference *ownerB); };
ScriptReference *TFBMember_ActivateMessage();   // 0x43a460
void ValueSlide_UpdateAll();        // 0x435ac0

// a node of a Script::ReferenceSet (0x10 B): +4 next, +8 key, +0xc object
struct BsNode {
	BsNode *prev;
	BsNode *next;
	float key;
	ScriptReference *item;
};
#include "script/referenceset.h"
typedef Script::ReferenceSet BsSet;
#define BS_HEAD(set) ((BsNode *)((Script::ReferenceSet *)(set))->m_head)   /* BsNode is SetNode's shape */
#define BS_REMOVE(set, node) (((Script::ReferenceSet *)(set))->RemoveNode((Script::SetNode *)(node)))
#define SA_3C(a) ((a)->m_20.m_1c)

namespace Script {

void ExpireSenderMessages();                                          // 0x43aaf0
void SetScriptContext(ScriptReference *self, ScriptReference *owner);   // 0x43bb70

typedef RWS::TFB::GameActor ScriptActorBs;

}  // namespace Script
// GLOBAL: 0x437ec0 ScriptActors_Update

#ifdef ZOOSTERS_DEBUGUI
int port_dbg_script_time(int time);   // debugui_vars.cpp: "freeze script time"
#endif
// 0x437ec0
void ScriptActors_Update(int time)
{
	BsNode *n;
	BsNode *next;
	ScriptActorBs *actor;
	int mode;
	int newMode;
	Script::ExpireSenderMessages();
#ifdef ZOOSTERS_DEBUGUI
	time = port_dbg_script_time(time);
#endif
	*Script::ScriptSystem::GetScriptTimePtr() = time;
	ValueSlide_UpdateAll();
	for (n = BS_HEAD(TFB_GetPlayerSet()); n != 0; n = next) {
		actor = (ScriptActorBs *)n->item;
		mode = actor->m_controlMode;
		bool run = true;
		if (((RWS::TFB::GameActor *)actor)->UpdateTimedMove()) {
			run = false;
			if (mode >= 2) {
				OpContext *vars = actor->GetVariableList(0);
				if (vars != 0 && vars->m_behaviorNode != 0)
					run = true;
			}
		}
		if (run) {
			if (mode == 0)
				actor->SetControlMode(1, -1);
			g_curScript = actor->GetVariableList(2);
			if (g_curScript != 0) {
				g_voiceDefaultRef = actor;
				Script::SetScriptContext(actor, actor);
				do {
					int pc = g_scriptPC++;
					OpCode *op = g_curScript->m_nodes[pc];
					if (op != 0)
						op->Execute();
					if (mode >= 2)
						break;
					mode = actor->m_controlMode;
				} while (mode >= 2);
			}
		}
		newMode = actor->m_controlMode;
		next = n->next;
		if (newMode == 0)
			continue;
		if (newMode != 1) {
			if (newMode <= 1 || newMode > 3) {
			} else {
				if (mode < 2)
					continue;
				actor->DeactivateActor(true);
			}
		} else {
			actor->SetScriptClockHold(1);
			((BsSet *)TFB_GetNPCSet())->Insert(actor, FLT_MAX);
		}
		BS_REMOVE(TFB_GetPlayerSet(), n);
	}
	for (n = BS_HEAD(TFB_GetNPCSet()); n != 0; n = next) {
		actor = (ScriptActorBs *)n->item;
		mode = actor->m_controlMode;
		bool run = true;
		if (((RWS::TFB::GameActor *)actor)->UpdateTimedMove()) {
			run = false;
			if (mode >= 2) {
				OpContext *vars = actor->GetVariableList(0);
				if (vars != 0 && vars->m_behaviorNode != 0)
					run = true;
			}
		}
		if (run) {
			if (mode == 1) {
				float range = actor->GetDeactivationRange();
				if (range != FLT_MAX) {
					float rangeSq = range * range;
					bool nearPlayer = false;
					for (BsNode *p = BS_HEAD(TFB_GetPlayerSet()); p != 0; p = p->next) {
						ScriptReference *player = p->item;
						if (((DistSelf437 *)player)->FUN_00409700(player, actor, actor) <= rangeSq) {
							nearPlayer = true;
							break;
						}
					}
					if (!nearPlayer) {
						actor->SetControlMode(mode = 2, -1);
					}
				}
			}
			g_curScript = actor->GetVariableList(2);
			if (g_curScript != 0) {
				g_voiceDefaultRef = actor;
				Script::SetScriptContext(actor, actor);
				do {
					int pc = g_scriptPC++;
					OpCode *op = g_curScript->m_nodes[pc];
					if (op != 0)
						op->Execute();
					if (mode >= 2)
						break;
					mode = actor->m_controlMode;
				} while (mode >= 2);
			}
		}
		newMode = actor->m_controlMode;
		next = n->next;
		if (newMode == 1)
			continue;
		if (newMode != 0) {
			if (newMode <= 1 || newMode > 3) {
			} else {
				if (mode < 2)
					continue;
				actor->DeactivateActor(true);
			}
		} else
			((BsSet *)TFB_GetPlayerSet())->Insert(actor, FLT_MAX);
		BS_REMOVE(TFB_GetNPCSet(), n);
	}
	for (n = BS_HEAD(TFB_GetInactiveSet()); n != 0; n = next) {
		actor = (ScriptActorBs *)n->item;
		next = n->next;
		if (actor->m_controlMode == 2) {
			float range = actor->GetActivationRange();
			if (range == 0.0f)
				continue;
			if (range != FLT_MAX) {
				float rangeSq = range * range;
				bool wake = false;
				for (BsNode *p = BS_HEAD(TFB_GetPlayerSet()); p != 0; p = p->next) {
					if (((DistSelf437 *)p->item)->FUN_00409700(p->item, actor, actor) < rangeSq) {
						wake = true;
						break;
					}
				}
				if (!wake)
					continue;
			}
			Vec3 *pos = actor->GetVector(actor);
			actor->PostMessage(TFBMember_ActivateMessage(), (ScriptReference *)TFB_GetPlayerSet(), pos, 0.0f);
			actor->PostMessage(TFBMember_ActivateMessage(), (ScriptReference *)TFB_GetNPCSet(), pos, 0.0f);
			actor->SetControlMode(1, -1);
			SA_3C(actor) = 0;
		}
		mode = actor->m_controlMode;
		if (!((RWS::TFB::GameActor *)actor)->UpdateTimedMove() && mode != 3) {
			g_curScript = actor->GetVariableList(2);
			if (g_curScript != 0) {
				g_voiceDefaultRef = actor;
				Script::SetScriptContext(actor, actor);
				do {
					int pc = g_scriptPC++;
					OpCode *op = g_curScript->m_nodes[pc];
					if (op != 0)
						op->Execute();
					if (mode >= 2)
						break;
					mode = actor->m_controlMode;
				} while (mode >= 2);
				next = n->next;
			}
		}
		switch (mode) {
		case 0:
			((BsSet *)TFB_GetPlayerSet())->Insert(actor, FLT_MAX);
			break;
		case 1:
			((BsSet *)TFB_GetNPCSet())->Insert(actor, FLT_MAX);
			break;
		case 2:
			actor->DeactivateActor(false);
			continue;
		case 3:
			actor->DeactivateActor(true);
			break;
		}
		BS_REMOVE(TFB_GetInactiveSet(), n);
	}
}

inline float DistSelf437::FUN_00409700(ScriptReference *ownerA, ScriptReference *b, ScriptReference *ownerB)
{
	return ((PositionMeasurement *)this)->Dist(ownerA, (PositionMeasurement *)b, ownerB);
}
