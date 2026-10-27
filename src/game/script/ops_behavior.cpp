// Section header ops of the "behavior" family: OpPreScript (vtable 0x5ce5bc) enters a behavior block (0x432db0), OpShutdown (0x5ce648) runs its block while the player actor is live (0x431940).
#include "script/behaviorlayout.h"
#include "script/runtime.h"
#include "script/types/scriptactor.h"
#include "script/ops_behavior.h"
using namespace Script;

ScriptReference *TFBUser_Behavior();   // 0x431da0
ScriptReference *TFBType_Actor();      // 0x436e30
// GLOBAL: 0x621f34 g_voiceDefaultRef
extern ScriptReference *g_voiceDefaultRef;

namespace Script {

// 0x432db0
int OpPreScript::Execute()
{
	OpContext *cs = g_curScript;
	int sel = cs->m_behaviorNode == -1 ? 0 : BEHAVIOR_START(cs->m_nodes[cs->m_behaviorNode]);
	OpCode *target = cs->m_nodes[sel];
	TFBUser_Behavior()->SetReferent(target, 0);
	if (OpCode::Execute() == 1) {
		g_scriptPC = BEHAVIOR_START(TFBUser_Behavior()->Deref(0, 0));
		if (g_scriptPC != 0) {
			g_curScript->ExecuteNext();
			m_flags &= ~0x80;
		}
	}
	int v = BEHAVIOR_START(TFBUser_Behavior()->Deref(0, 0));
	if (v == 0)
		v = -1;
	g_curScript->m_behaviorNode = v;
	TFBUser_Behavior()->SetReferent(0, 0);
	g_scriptPC = 0;
	return 0;
}


// 0x431940
int OpShutdown::Execute()
{
	bool go = false;
	if (g_voiceDefaultRef->IsKindOf(TFBType_Actor())) {
		if (static_cast<ScriptActor *>(g_voiceDefaultRef)->m_controlMode >= 2)
			go = true;
	}
	if (!go) {
		SkipBlock();
		return 1;
	}
	int r = OpCode::Execute();
	if (r == 1 && static_cast<ScriptActor *>(g_voiceDefaultRef)->m_controlMode >= 2)
		r = 0;
	return r;
}

}  // namespace Script
