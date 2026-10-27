// Cut scenes: moving the actors of a set into the cut-scene slot, and the clock / input state a slot change leaves behind.
#include "modules/tfb/gameactor.h"
#include "tfb/corpus/corpus.h"
#include "script/referenceset.h"
#include "script/cutscenerecord.h"
#include "script/types/scriptsystem.h"
#include "tfb/audio/voice.h"
#include "framework/mainloop/gameclock.h"

int *CutScene_StackSlot(int delta);              // 0x436c00
bool TFBClock_Resume();                          // 0x461c30
void BlockInputsUntilReleased(int delay);        // 0x4028a0
// GLOBAL: 0x621f34 g_voiceDefaultRef
namespace Script { class ScriptReference; }
extern Script::ScriptReference *g_voiceDefaultRef;
Script::ScriptReference *TFB_GetMessageSenderSet();   // 0x43aa90

// 0x429a40
void CutScene_ApplyState()
{
	int state = *CutScene_StackSlot(0);
	if (state == 3) {
		TFBClock_Stop();
		return;
	}
	bool wasRunning = TFBClock_Resume();   // whether the clock was running
	BlockInputsUntilReleased(state == 2 || wasRunning ? 250 : 1000);
}

// Move every actor of a set into the cut-scene slot's set.
// 0x42bf80
void CutScene_ProcessActorSet(Script::ReferenceSet *set, bool *flag)
{
	Script::CutSceneRecord *slot = (Script::CutSceneRecord *)CutScene_StackSlot(0);
	Script::SetNode *node = set->m_head;
	while (node) {
		RWS::TFB::GameActor *actor = (RWS::TFB::GameActor *)node->m_object;
		Script::SetNode *next = node->m_next;
		if (actor == g_voiceDefaultRef) {
			*flag = false;
			StopVoicesForObject(actor, -1);
		} else if ((actor->m_params.m_flags & RWS::TFB::ACTOR_IGNORES_CUT_SCENES) && slot->m_state != 3) {
			StopVoicesForObject(actor, -1);
		} else {
			slot->m_actors.Insert(actor, 3.4028235e+38f);
			set->RemoveNode(node);
			if (*flag && actor->m_controlMode >= ((RWS::TFB::GameActor *)g_voiceDefaultRef)->m_controlMode)
				actor->m_scriptClockHold = 1;
			if (actor->m_script.m_nodes && !actor->m_script.m_0c)
				actor->m_script.m_0c = *Script::ScriptSystem::GetScriptTimePtr();
			if (actor->m_08.m_count)
				((Script::ReferenceSet *)TFB_GetMessageSenderSet())->Remove(actor);
			Corpus *body = actor->m_corpus;
			if (body)
				body->SetEnabled(slot->m_state == 3 ? 0 : -1);
			else
				actor->Collider::Reset();
		}
		node = next;
	}
}
