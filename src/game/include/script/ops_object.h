// Script object ops. Address comments: the game function in that slot. Members past the base are sized from CloneToPool (m_XX = offset).
#ifndef ZOOSTERS_SCRIPT_OPS_OBJECT_H
#define ZOOSTERS_SCRIPT_OPS_OBJECT_H

#include "script/opcode.h"
#include "script/runtime.h"

namespace Script {

// vtable 0x5ce170, "comment:", size unknown (no Clone)
class OpComment : public OpCode {
public:
	const char *GetName();                      // 0: 0x431040
	int Parse();                                // 14: 0x431880
	char *m_08;  // read before the (unknown) 0x4310c0 test; the op has no Clone
	OpComment() : m_08(0) {}
};

// vtable 0x5ce1b8, "print", size unknown (no Clone)
class OpDebugPrint : public OpCode {
public:
	const char *GetName();                      // 0: 0x431050
	int Parse();                                // 14: 0x4318a0
	ScriptRef m_08;
	char *m_0c;  // points at a 256-byte buffer on Parse's stack
	OpDebugPrint() : m_0c(0) {}
};

// vtable 0x5ce4dc, "if/else", 8 B
class OpIfElse : public OpBlock {
public:
	const char *GetName();                      // 0: 0x431770
	int IsBlock();                              // 11: 0x431780
	OpCode *Clone();                            // 12: 0x4325a0
	int Execute();                              // 13: 0x431790
	int Parse();                                // 14: 0x45d080
	__declspec(noinline) static OpIfElse *CloneToPool(const OpIfElse *src);
};
ZOOSTERS_ASSERT_SIZE(OpIfElse, 0x8);

// vtable 0x5ce240, "find variable", 24 B
class VariableFind : public OpBlock {
public:
	const char *GetName();                      // 0: 0x4310a0
	int IsBlock();                              // 11: 0x4310b0
	OpCode *Clone();                            // 12: 0x432650
	int Execute();                              // 13: 0x431ca0
	int Parse();                                // 14: 0x431000
	__declspec(noinline) static VariableFind *CloneToPool(const VariableFind *src);
	ScriptRef m_08;  // the variable
	ScriptRef m_0c;  // the actor to search
	ScriptReference *m_type;  // the declared type (script/reader.cpp ScriptRefReader::Read stores it while parsing)
	int m_notUser;  // -1 until the declaration is read, then 1 for a set, 0 for a user object
	VariableFind() : m_type(0), m_notUser(-1) {}
};
ZOOSTERS_ASSERT_SIZE(VariableFind, 0x18);

// vtable 0x5ce28c, "create variable", size unknown (no Clone)
class VariableCreate : public OpCode {
public:
	const char *GetName();                      // 0: 0x4312f0
	int Parse();                                // 14: 0x431300
	ScriptRef m_08;
};

// vtable 0x5ce2d8, "check reference", 16 B
class MemoryCheck : public OpBlock {
public:
	const char *GetName();                      // 0: 0x431330
	OpCode *Clone();                            // 12: 0x432820
	int Execute();                              // 13: 0x431c30
	int Parse();                                // 14: 0x431340
	__declspec(noinline) static MemoryCheck *CloneToPool(const MemoryCheck *src);
	ScriptRef m_08;
	ScriptRef m_0c;
};
ZOOSTERS_ASSERT_SIZE(MemoryCheck, 0x10);

// vtable 0x5ce1fc, "control", 16 B
class ObjectControl : public OpBlock {
public:
	const char *GetName();                      // 0: 0x431090
	int IsBlock();                              // 11: 0x42cbe0
	OpCode *Clone();                            // 12: 0x4325f0
	int Execute();                              // 13: 0x4319b0
	int Parse();                                // 14: 0x430fa0
	__declspec(noinline) static ObjectControl *CloneToPool(const ObjectControl *src);
	ObjectControl() : m_0c(0) {}    // the prototype instance (OpProto_ObjectControl)
	ScriptRef m_08;
	unsigned char m_0c;
};
ZOOSTERS_ASSERT_SIZE(ObjectControl, 0x10);

// vtable 0x5ce3bc, "spawn", 32 B
class ObjectSpawn : public ObjectControl {
public:
	const char *GetName();                      // 0: 0x4313f0
	const char *GetAltName();                   // 1: 0x431400
	OpCode *Clone();                            // 12: 0x4327a0
	int Execute();                              // 13: 0x431ad0
	int Parse();                                // 14: 0x431410
	__declspec(noinline) static ObjectSpawn *CloneToPool(const ObjectSpawn *src);
	ScriptRef m_10;
	ScriptValue m_14;
	ObjectSpawn() { ZeroScriptValue(m_14); }
};
ZOOSTERS_ASSERT_SIZE(ObjectSpawn, 0x20);

// vtable 0x5ce324, "set reference", 16 B
class ObjectReference : public OpCode {
public:
	const char *GetName();                      // 0: 0x431390
	OpCode *Clone();                            // 12: 0x4321f0
	int Execute();                              // 13: 0x4326c0
	int Parse();                                // 14: 0x4313a0
	__declspec(noinline) static ObjectReference *CloneToPool(const ObjectReference *src);
	ScriptRef m_08;
	ScriptRef m_0c;
};
ZOOSTERS_ASSERT_SIZE(ObjectReference, 0x10);

// vtable 0x5ce370, "set behavior", 12 B
class BehaviorSet : public OpCode {
public:
	const char *GetName();                      // 0: 0x4313e0
	OpCode *Clone();                            // 12: 0x432160
	int Execute();                              // 13: 0x4321b0
	int Parse();                                // 14: 0x431aa0
	__declspec(noinline) static BehaviorSet *CloneToPool(const BehaviorSet *src);
	ScriptRef m_08;
};
ZOOSTERS_ASSERT_SIZE(BehaviorSet, 0xc);

// vtable 0x5ce40c, "remove", 12 B
class ObjectRemove : public OpCode {
public:
	const char *GetName();                      // 0: 0x4314c0
	OpCode *Clone();                            // 12: 0x432250
	int Execute();                              // 13: 0x4314d0
	int Parse();                                // 14: 0x431560
	__declspec(noinline) static ObjectRemove *CloneToPool(const ObjectRemove *src);
	ScriptRef m_08;
};
ZOOSTERS_ASSERT_SIZE(ObjectRemove, 0xc);

// vtable 0x5ce450, "reset", 12 B
class ObjectReset : public OpCode {
public:
	const char *GetName();                      // 0: 0x431510
	OpCode *Clone();                            // 12: 0x4322a0
	int Execute();                              // 13: 0x431520
	int Parse();                                // 14: 0x431560
	__declspec(noinline) static ObjectReset *CloneToPool(const ObjectReset *src);
	ScriptRef m_08;
};
ZOOSTERS_ASSERT_SIZE(ObjectReset, 0xc);

// vtable 0x5ce494, "displace", 48 B
class ObjectDisplace : public OpCode {
public:
	const char *GetName();                      // 0: 0x431590
	OpCode *Clone();                            // 12: 0x4322f0
	int Execute();                              // 13: 0x4315a0
	int Parse();                                // 14: 0x4316f0
	__declspec(noinline) static ObjectDisplace *CloneToPool(const ObjectDisplace *src);
	__forceinline ObjectDisplace(const ObjectDisplace &o) throw() : OpCode(o), m_08(o.m_08), m_0c(o.m_0c), m_18(o.m_18), m_24(o.m_24) {}
	ScriptRef m_08;
	ScriptValue m_0c;
	ScriptValue m_18;
	ScriptValue m_24;
	ObjectDisplace() { ZeroScriptValue(m_0c); ZeroScriptValue(m_18); ZeroScriptValue(m_24); }
};
ZOOSTERS_ASSERT_SIZE(ObjectDisplace, 0x30);

}  // namespace Script

#endif
