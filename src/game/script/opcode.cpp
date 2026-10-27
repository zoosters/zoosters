// Script::OpCode / OpBlock base behaviour and the flow ops that run child blocks.
#include "script/ops_object.h"
#include "script/runtime.h"

namespace Script {

// 0x431130
int OpCode::Execute()
{
	return RunBlock((m_flags >> 11) + g_scriptPC);
}

// 0x431790
int OpIfElse::Execute()
{
	int idx = g_scriptPC++;
	OpCode *cond = g_curScript->m_nodes[idx];
	unsigned end = (m_flags >> 11) + idx;
	unsigned r = cond->Execute();
	m_flags &= ~0x80;
	if (r <= 1 && (cond->m_flags & 0x80) == 0) {
		if ((m_flags & 0x100) != 0) {
			r = 1;
		} else {
			while ((unsigned)g_scriptPC < end) {
				int i = g_scriptPC++;
				OpCode *op = g_curScript->m_nodes[i];
				if (op != 0) {
					r = op->Execute() - 1;
					if (r != 0) {   // a child did not finish normally (its result is not 1): skip the rest of the branch
						g_scriptPC = end;
						return r;
					}
				}
			}
			m_flags |= 0x80;
			return m_flags & 7;
		}
	}
	g_scriptPC = end;
	return r;
}

}  // namespace Script

Script::ScriptReference *TFBType_OpCode();
Script::ScriptReference *TFBType_Behavior();

// 0x431870
Script::ScriptReference *Script::OpCode::GetType()
{
	return TFBType_OpCode();
}

// 0x432150
Script::ScriptReference *OpBehavior_GetType()
{
	return TFBType_Behavior();
}


namespace Script {

// 0x42f7a0
const char *OpCode::GetName()
{
	return "op-code";
}

}  // namespace Script

// 0x431840
const char *OpBehavior_GetAltName()
{
	return "behavior";
}
