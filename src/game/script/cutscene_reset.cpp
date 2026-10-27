#include "script/actor.h"
#include "script/runtime.h"
#include "script/referenceset.h"
#include "script/cutscenerecord.h"
#include "script/ops_value.h"
using namespace Script;

// vtable slot 10 of ScriptActor and GameActor: TypeRegistry_ResetAll calls it on every type
int *CutScene_StackSlot(int delta);    // 0x436c00
void CutScene_ApplyState();            // 0x429a40
void CutScene_RemoveActor(void *actor);        // 0x42cfc0
#include "script/types/animmap.h"

// 0x4386c0 ScriptActor_Reset
void ScriptActor_Reset()
{
	for (SetNode *n = GetSlideSet()->m_head; n != 0; n = n->m_next)
		((ValueSlide *)n->m_object)->m_slides.Clear();
	GetSlideSet()->Clear();
	// Unwind the whole cut-scene stack, as the game does, not just its top record: quitting from the pause menu during a cut scene leaves two records
	// (the cut scene under the pause), and the actor sets must be emptied before the level's scripts are freed, or the next level's
	// first ScriptActors_Update runs the old actors' freed scripts. The loop ends at the bottom record (CutScene_StackSlot clamps at 0).
	while (*CutScene_StackSlot(0) != 2) {
		CutSceneRecord *rec = (CutSceneRecord *)CutScene_StackSlot(0);
		rec->m_state = 2;
		while (rec->m_actors.m_head != 0)
			CutScene_RemoveActor(rec->m_actors.m_head->m_object);
		rec->m_actors.Clear();
		rec->m_counter = 0;
		CutScene_StackSlot(-1);
		CutScene_ApplyState();
	}
	TFBType_AnimMap();
	TFB_GetPlayerSet()->DeactivateAllActors();
	TFB_GetNPCSet()->DeactivateAllActors();
	TFB_GetInactiveSet()->DeactivateAllActors();
	TFB_GetDeadSet()->DeactivateAllActors();
}
