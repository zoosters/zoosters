// 0x42cfc0, an actor leaves the current cut-scene record (stand-in structs, byte offsets).
#include "tfb/corpus/corpus.h"
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptactor.h"
#include "modules/tfb/gameactor.h"
#include "script/types/scriptsystem.h"
#include "script/cutscenerecord.h"
#include <float.h>

int *CutScene_StackSlot(int delta);              // 0x436c00
void StopVoicesForObject(Script::ScriptReference *obj, int a);   // 0x419220 (cdecl)
Script::ScriptReference *TFB_GetMessageSenderSet();         // 0x43aa90


// 0x42cfc0 CutScene_RemoveActor
void CutScene_RemoveActor(void *actor)
{
	Script::CutSceneRecord *slot = (Script::CutSceneRecord *)CutScene_StackSlot(0);
	if (!((Script::ReferenceSet *)&slot->m_actors)->Remove((Script::ScriptReference *)actor)   /* the parameter is the actor as a void * */)
		return;
	RWS::TFB::GameActor *a = (RWS::TFB::GameActor *)actor;
	int mode = a->m_controlMode;
	if (mode >= 0 && mode <= 1) {
		if (a->m_corpus)
			a->m_corpus->SetEnabled(1);
		else
			a->ClearLinks();
		const RwMatrix *matrix;
		if (a->m_corpus)
			matrix = &a->m_corpus->m_frame->modelling;
		else
			matrix = &a->m_matrix;
		a->UpdateBounds(matrix, 0);
		if (a->m_controlMode == 1)
			TFB_GetNPCSet()->Insert((Script::ScriptReference *)actor, FLT_MAX);
		else
			TFB_GetPlayerSet()->Insert((Script::ScriptReference *)actor, FLT_MAX);
	} else {
		TFB_GetInactiveSet()->Insert((Script::ScriptReference *)actor, FLT_MAX);
	}
	StopVoicesForObject((Script::ScriptReference *)actor, -1);
	if (a->m_08.m_count) {
		((Script::ReferenceSet *)TFB_GetMessageSenderSet())->Insert((Script::ScriptReference *)actor, FLT_MAX);
		int dt = *Script::ScriptSystem::GetScriptTimePtr() - slot->m_stamp;
		for (Script::SetNode *n = a->m_08.m_head; n; n = n->m_next)
			n->m_postTime += dt;
	}
	if (slot->m_state != 2) {
		Script::ReferenceSet attached;
		for (Script::SetNode *n = slot->m_actors.m_head; n; n = n->m_next) {
			RWS::TFB::GameActor *item = (RWS::TFB::GameActor *)n->m_object;
			Corpus *corpus = item->m_corpus;
			// compares with the leaving actor's bounds, not the item's
			if (corpus && (char *)corpus->m_link.m_carrier == (char *)&static_cast<RWS::TFB::Collider &>(*a))
				attached.Insert(item, FLT_MAX);
		}
		while (Script::SetNode *n = attached.m_head) {
			CutScene_RemoveActor(n->m_object);
			attached.RemoveNode(n);
		}
	}
}
