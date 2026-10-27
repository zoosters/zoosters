// 0x433140 (the type of the innermost enclosing block of a kind) and 0x432f70 (the type a reference names)
#include "script/runtime.h"
#include "script/symbolregistry.h"
#include "script/ops_object.h"
#include "script/ops_set.h"
using namespace Script;
ScriptReference *TFBType_Behavior();   // 0x4318f0

#include "script/ops_actor.h"

// 0x433140
ScriptReference *OpContext::FindBlockType(int kind)
{
	OpContext *self = this;
	int mode = kind;
	for (int i = g_scriptPC - 1; i >= 0; i--) {
		OpCode *node = m_nodes[i];
		if (node == 0)
			continue;
		if (node->GetType() == TFBType_Behavior())
			break;
		if (node->IsBlock() != mode && (mode != 0 || node->IsBlock() != 2))
			continue;
		if ((node->m_flags >> 11) + i < (unsigned)g_scriptPC)
			continue;
		if (mode == 0) {
			mode = 2;
			continue;
		}
		void *ref;
		switch (mode - 2) {
		case 0x63:   // "for each" (IsBlock 101)
			ref = &((SetForEach *)node)->m_set;
			break;
		case 0:      // ObjectControl (IsBlock 2)
			ref = &((ObjectControl *)node)->m_08;
			break;
		case 1:      // CheckFOV / SetFindSub (IsBlock 3): both keep the target reference first (+8); no common base declares it
			if (CheckFOV *fov = dynamic_cast<CheckFOV *>(node))
				ref = &fov->m_target;
			else
				ref = &static_cast<SetFindSub *>(node)->m_target;
			break;
		case 2:
			return TFBType_Actor();
		case 4:
			return ((VariableFind *)node)->m_type->GetType();
		default:
			ref = (void *)(intptr_t)kind;   // unreachable arm
			break;
		}
		int savedPC = g_scriptPC;
		g_scriptPC = i;
		ScriptReference *type = self->Lookup(ref, mode);
		g_scriptPC = savedPC;
		return type;
	}
	if (mode == 2)
		return TFBType_Actor();
	return 0;
}

ScriptReference *TFBType_User();   // 0x43c530
ScriptReference *TFBType_Set();    // 0x43d2d0
// GLOBAL: 0x61f878 g_symRegistryY

// 0x432f70 Script::OpContext::Lookup
ScriptReference *OpContext::Lookup(void *refp, int mode)
{
	unsigned *ref = (unsigned *)refp;
	int sel = (*ref >> 6) & 3;
	unsigned idx = *ref >> 14;
	ScriptReference *obj;
	switch (idx) {
	case 0x3ffff:
		obj = FindBlockType(2);
		break;
	case 0x3fffe:
		obj = FindBlockType(0);
		break;
	case 0x3fffd:
		obj = FindBlockType(0x65);
		break;
	case 0x3fffc:
		obj = FindBlockType(3);
		break;
	case 0x3fffb:
		obj = TFBType_Actor();
		break;
	default:
		if (idx == 0x3fffa) {
			obj = FindBlockType(6);
		} else if ((int)idx >= 0x3fc0d) {
			obj = g_curScript->m_locals[idx - 0x3fc0d];
		} else {
			SymbolSlot *slot = &(*g_symRegistryY)[idx];
			uintptr_t bits = slot->obj;
			obj = (ScriptReference *)(bits & ~(uintptr_t)1);
		}
		// a user object named without member but with a selector: the selector is dropped
		if (obj != 0 && obj->GetType() == TFBType_User() && (*ref & 0x3f00) == 0 && sel != 0)
			sel = 0;
		break;
	}
	if (obj != 0) {
	unsigned member = (*ref >> 8) & 0x3f;
	int count;
	if (member != 0)
		obj = &((ScriptReference *)obj->GetMembers(&count))[member - 1];
	if (mode == 0x65) {
		obj = obj->GetBaseType()->GetType();
	} else if (mode != 3) {
		if (sel != 0 || (*ref & 0x3f) != 0) {
			bool isSet = obj->GetType() == TFBType_Set();
			if (isSet)
				obj = obj->GetBaseType();
			if (obj != 0) {
				unsigned sub = *ref & 0x3f;
				if (sub != 0) {
					if (!isSet && sel == 0 && (*ref & 0x3f00) == 0)
						obj = &((ScriptReference *)obj->GetBaseType()->GetMembers(&count))[(*ref & 0x3f) - 1];
					else
						obj = &((ScriptReference *)obj->GetMembers(&count))[sub - 1];
				}
			}
		}
	}
	if (obj->GetType() == TFBType_User() && mode != 6)
		obj = obj->GetBaseType();
	}
	return obj;
}
