// Prototype getters of ops_object.h classes; see proto_op_getters.cpp.
#include "script/ops_object.h"
using namespace Script;

// GLOBAL: 0x5ce28c Script::VariableCreate::vftable
// 0x431e80
ScriptReference *OpProto_VariableCreate()
{
	static VariableCreate s;
	return &s;
}

// GLOBAL: 0x5ce2d8 Script::MemoryCheck::vftable
// 0x431ec0
ScriptReference *OpProto_MemoryCheck()
{
	static MemoryCheck s;
	return &s;
}

// GLOBAL: 0x5ce324 Script::ObjectReference::vftable
// 0x431f00
ScriptReference *OpProto_ObjectReference()
{
	static ObjectReference s;
	return &s;
}

// GLOBAL: 0x5ce370 Script::BehaviorSet::vftable
// 0x431f40
ScriptReference *OpProto_BehaviorSet()
{
	static BehaviorSet s;
	return &s;
}

// GLOBAL: 0x5ce40c Script::ObjectRemove::vftable
// 0x431ff0
ScriptReference *OpProto_ObjectRemove()
{
	static ObjectRemove s;
	return &s;
}

// GLOBAL: 0x5ce450 Script::ObjectReset::vftable
// 0x432030
ScriptReference *OpProto_ObjectReset()
{
	static ObjectReset s;
	return &s;
}

// GLOBAL: 0x5ce4dc Script::OpIfElse::vftable
// 0x432110
ScriptReference *OpProto_OpIfElse()
{
	static OpIfElse s;
	return &s;
}
