// Script set ops. Address comments: the game function in that slot. Members past the base are sized from CloneToPool (m_XX = offset).
#ifndef ZOOSTERS_SCRIPT_OPS_SET_H
#define ZOOSTERS_SCRIPT_OPS_SET_H

#include "script/opcode.h"
#include "script/ops_value.h"

namespace Script {

class OpContext;

// Temporary named set of the ops (0x24 B, vtable 0x5cd2e0): ReferenceSet plus refcount, name and a copy of the source's base type.
// On the stack in "for each" (ctor 0x43d710 copies a source set via Assign) and CheckFOV (0x42cb40, out of line); on the heap in
// "find subset" and for the locals of a cloned script (OpContext::Instantiate).
// The destructor 0x43cfe0 unlinks it from the live-set table 0x61f660.
// GLOBAL: 0x5cd2e0 Script::ScriptSubset::vftable
// GLOBAL: 0x42cb40 ??0ScriptSubset@Script@@QAE@PBDPAVScriptReference@1@PAVOpContext@1@@Z
class ScriptSubset : public ReferenceSet {
public:
	ScriptSubset(ReferenceSet *src, int randomKeys);                              // 0x43d710 (ret 8)
	ScriptSubset(const char *name, ScriptReference *type, OpContext *script);        // 0x42cb40 (ret 0xc): CheckFOV makes its "subset" with it
	// the "find subset" op's constructor (inline there): an empty subset named "subset" that shares the source's type
	__forceinline ScriptSubset(ReferenceSet *src, ScriptReference *type);
	// --- the same with a given name (OpContext::Instantiate)
	__forceinline ScriptSubset(const char *name, ScriptReference *type) : m_refCount(0), m_name(name), m_type()
	{
		*(void **)&m_type = *(void **)type;
	}
	static void *operator new(size_t size) { return MemAlloc(size); }
	static void operator delete(void *p) { operator_delete_unwind(p); }
	~ScriptSubset();                                    // 0x43cfe0
	const char *GetName();                              // 0: 0x429730
	int AdjustRefCount(int delta);                      // 2: 0x42cbc0
	ScriptReference *GetBaseType();                     // 5: 0x42cbf0
	int GetKind();                                      // 6: 0x42cbe0 (returns 2; bound in script/normal_bringup.cpp)
	int m_refCount;
	const char *m_name;
	ScriptReference m_type;  // ScriptReference-shaped member (copy of the source's base type)
};
ZOOSTERS_ASSERT_SIZE(ScriptSubset, 0x24);
// The subsets that exist, by slot (filled when a subset is created, 0x430c00; cleared by ~ScriptSubset)
extern int g_subsetTableSize;               // 0x602fb8
extern ScriptSubset *g_subsetTable[];       // 0x61f660

// vtable 0x5d31f8, "for each", 16 B
class SetForEach : public OpBlock {
public:
	const char *GetName();                      // 0: 0x43cc60
	int IsBlock();                              // 11: 0x43cc70
	OpCode *Clone();                            // 12: 0x43d140
	int Execute();                              // 13: 0x43d590
	int Parse();                                // 14: 0x43d7a0
	__declspec(noinline) static SetForEach *CloneToPool(const SetForEach *src);
	SetForEach() : m_order(0) {}
	ScriptRef m_set;
	unsigned char m_order;  // enum (forward, ...: names at 0x603044)
};
ZOOSTERS_ASSERT_SIZE(SetForEach, 0x10);

// vtable 0x5d3240, "change membership", 20 B
class SetMembershipChange : public OpCode {
public:
	const char *GetName();                      // 0: 0x43cc80
	OpCode *Clone();                            // 12: 0x43d080
	int Execute();                              // 13: 0x43d7f0
	int Parse();                                // 14: 0x43d9f0
	__declspec(noinline) static SetMembershipChange *CloneToPool(const SetMembershipChange *src);
	ScriptRef m_set;
	SetMembershipChange() : m_action(0) {}
	ScriptRef m_object;
	unsigned char m_action;  // 0..3
};
ZOOSTERS_ASSERT_SIZE(SetMembershipChange, 0x14);

// vtable 0x5d3290, "check membership", 20 B
class SetMembershipCheck : public OpBlock {
public:
	const char *GetName();                      // 0: 0x43cc90
	OpCode *Clone();                            // 12: 0x43d210
	int Execute();                              // 13: 0x43d8a0
	int Parse();                                // 14: 0x43d9f0
	__declspec(noinline) static SetMembershipCheck *CloneToPool(const SetMembershipCheck *src);
	ScriptRef m_set;
	SetMembershipCheck() : m_mode(0) {}
	ScriptRef m_object;
	unsigned char m_mode;  // 0..2
};
ZOOSTERS_ASSERT_SIZE(SetMembershipCheck, 0x14);

// vtable 0x5d32e4, "find subset", 24 B
class SetFindSub : public ValueCheck {
public:
	const char *GetName();                      // 0: 0x43d060
	int IsBlock();                              // 11: 0x43d070
	OpCode *Clone();                            // 12: 0x43d1a0
	int Execute();                              // 13: 0x430610
	__declspec(noinline) static SetFindSub *CloneToPool(const SetFindSub *src);
};
ZOOSTERS_ASSERT_SIZE(SetFindSub, 0x18);

__forceinline ScriptSubset::ScriptSubset(ReferenceSet *src, ScriptReference *type)
	: m_refCount(0), m_name("subset"), m_type()
{
	*(void **)&m_type = *(void **)type;
}

}  // namespace Script

#endif
