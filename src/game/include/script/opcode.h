// Script op base classes. Every op class: GetName (its script keyword), Clone (slot 12: a wrapper that calls the
// static CloneToPool, which copies the op into g_scriptPool), Execute (13) and Parse (14, reads the operands).
#ifndef ZOOSTERS_SCRIPT_OPCODE_H
#define ZOOSTERS_SCRIPT_OPCODE_H

#include "script/script.h"

namespace Script {

// vtable 0x5cdc80. Shared defaults: 0x43e6f0 returns 0 (slots 11, 12, 14), Execute 0x431130 runs the child ops.
class OpCode : public ScriptReference {
public:
	OpCode() : m_flags(1) {}   // prototype instances (TFBType getters): flags start at 1
	const char *GetName();      // 0: 0x42f7a0 "op-code"
	ScriptReference *GetType(); // 4: 0x431870 (jmp 0x4310c0: static OpCode prototype at 0x61f6e8); ops share it
	int GetKind();                 // 6: 0x43efe0 returns 1
	virtual int IsBlock();  // 11: 0 here, 1 in OpBlock (0x43efe0)
	virtual OpCode *Clone();    // 12: null pool = measure only
	virtual int Execute();      // 13: result in bits 0-2 of m_flags
	virtual int Parse();        // 14
	inline int RunBlock(unsigned end);  // defined in script/runtime.h
	inline void SkipBlock();            // defined in script/runtime.h
	// bits 0-2: last result, 0x80: done (OpCode::Execute), bits 11-31: number of child nodes that follow this one
	// in the script's node array.
	unsigned m_flags;
};

// No vtable in the game: cl 13.10 drops OpBlock's vptr stores in the inlined copy constructors and the vtable is
// referenced from nowhere else. Its only override is slot 11.
// NOVTABLE: Script::OpBlock
class OpBlock : public OpCode {
public:
	int IsBlock();  // 11: 0x43efe0 returns 1
};

}  // namespace Script

#endif
