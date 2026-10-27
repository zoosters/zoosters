// The "behavior" op family: OpBehavior (vtable 0x5ce520: the "behavior" type object, the "new behavior" prototype, a script's
// local behaviors) and the section headers derived from it, OpPreScript / OpStartup / OpShutdown (vtables 0x5ce5bc / 0x5ce604 /
// 0x5ce648; the class names are ours).
#ifndef ZOOSTERS_SCRIPT_OPS_BEHAVIOR_H
#define ZOOSTERS_SCRIPT_OPS_BEHAVIOR_H

#include "script/opcode.h"

namespace Script {

// 0x10 B: OpBlock {vptr, flags}, the name, the start node
class OpBehavior : public OpBlock {
public:
	OpBehavior(const char *name) : m_name(name) {}
	OpBehavior(const OpBehavior &src);          // 0x432540 (out of line)
	const char *GetName();                      // 0: 0x43ba80
	const char *GetAltName();                   // 1: 0x431840
	ScriptReference *GetType();                 // 4: 0x432150
	ScriptReference *GetBaseType();             // 5: 0x431870
	void vf10();                                // 10: 0x431e10
	OpCode *Clone();                            // 12: 0x4324d0
	int Parse();                                // 14: 0x431850
	__declspec(noinline) static OpBehavior *CloneToPool(const OpBehavior *src);   // 0x4324e0
	const char *m_name;  // (GetName 0x43ba80 returns it; the copy constructor 0x432540 copies GetName into it)
	int m_startPC;  // node index of the block start (OpContext::Load 0x4349e0 stores g_scriptPC)
};

// the section header objects (0x10 B in the script pool): the pre-script section enters a behavior block, the start-up section runs
// its block only while no behavior is active, the shut-down section only while the player actor is live
// GLOBAL: 0x5ce5bc Script::OpPreScript::vftable
// GLOBAL: 0x5ce604 Script::OpStartup::vftable
// GLOBAL: 0x5ce648 Script::OpShutdown::vftable
class OpPreScript : public OpBehavior {
public:
	OpPreScript(const char *n) : OpBehavior(n) {}
	int GetKind();
	int Execute();   // 13: 0x432db0
	int Parse();
};
class OpStartup : public OpBehavior {
public:
	OpStartup(const char *n) : OpBehavior(n) {}
	int GetKind();
	int Execute();   // 13: 0x432e60
	int Parse();
};
class OpShutdown : public OpBehavior {
public:
	OpShutdown(const char *n) : OpBehavior(n) {}
	int GetKind();
	int Execute();   // 13: 0x431940
	int Parse();
};

}  // namespace Script

#endif
