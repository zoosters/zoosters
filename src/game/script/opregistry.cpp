#include "script/script.h"
#include "script/ops_object.h"
#include "script/ops_value.h"
#include "script/ops_message.h"
#include "script/ops_behavior.h"
#include "script/ops_set.h"
#include "script/symbolregistry.h"
#include "script/value.h"
using namespace Script;

// the game's pool allocator pair (see ops_camera.h): the matching empty placement delete gives the EH frame
inline void *operator new(size_t, char *p) { return p; }
inline void operator delete(void *, char *) {}

// GLOBAL: 0x5cddc8 Script::ValueCheck::vftable
// 0x430030
ScriptReference *OpProto_ValueCheck()
{
	static ValueCheck s;
	return &s;
}

// GLOBAL: 0x5cde10 Script::ValueLoop::vftable
// 0x430090
ScriptReference *OpProto_ValueLoop()
{
	static ValueLoop s;
	return &s;
}

namespace Script {
}  // namespace Script

ScriptReference *TFBMember_GenericValue();
ScriptReference *OpProto_ValueInc();
ScriptReference *OpProto_ValueDec();
ScriptReference *OpProto_ValueSet();
ScriptReference *OpProto_ValueSlide();

// 0x430c00
void RegisterValueOps()
{
	((ScriptReference *)TFBMember_GenericValue())->ResolveSymbol(TFBType_Value(), 0, 0);
	((ScriptReference *)OpProto_ValueInc())->ResolveSymbol(TFBType_Value(), 0, 0);
	((ScriptReference *)OpProto_ValueDec())->ResolveSymbol(TFBType_Value(), 0, 0);
	((ScriptReference *)OpProto_ValueSet())->ResolveSymbol(TFBType_Value(), 0, 0);
	((ScriptReference *)OpProto_ValueCheck())->ResolveSymbol(TFBType_Value(), 0, 0);
	((ScriptReference *)OpProto_ValueLoop())->ResolveSymbol(TFBType_Value(), 0, 0);
	((ScriptReference *)OpProto_ValueSlide())->ResolveSymbol(TFBType_Value(), 0, 0);
}

// GLOBAL: 0x5ce240 Script::VariableFind::vftable
// 0x4311a0
ScriptReference *OpProto_VariableFind()
{
	static VariableFind s;
	return &s;
}

// GLOBAL: 0x5ce170 Script::OpComment::vftable
// 0x431250
ScriptReference *OpProto_OpComment()
{
	static OpComment s;
	return &s;
}

// GLOBAL: 0x5ce1b8 Script::OpDebugPrint::vftable
// 0x4312a0
ScriptReference *OpProto_OpDebugPrint()
{
	static OpDebugPrint s;
	return &s;
}

// GLOBAL: 0x5ce3bc Script::ObjectSpawn::vftable
// 0x431f80
ScriptReference *OpProto_ObjectSpawn()
{
	static ObjectSpawn s;
	return &s;
}

// GLOBAL: 0x5ce494 Script::ObjectDisplace::vftable
// 0x432070
ScriptReference *OpProto_ObjectDisplace()
{
	static ObjectDisplace s;
	return &s;
}

namespace Script {
// 0x431060
int OpContext::ExecuteNext()
{
	int idx = g_scriptPC++;
	OpCode *op = m_nodes[idx];
	if (op == 0)
		return 0;
	return op->Execute() - 1;
}
}  // namespace Script

// subset names (0x602fc0, 25 entries) and the live subset objects (g_subsetTable, 0x61f660)
// GLOBAL: 0x602fc0 g_subsetNames
extern unsigned g_subsetNames[25];   // 25 x 32-bit words (data_run.cpp), not pointers: the 8-byte `const char *` elements read past the array on x64

// 0x430f60
void RegisterSubset(ScriptReference *obj)
{
	for (int i = 0; i < 25; i++) {
		if ((unsigned)(uintptr_t)obj->GetName() == g_subsetNames[i] && Script::g_subsetTable[i] == 0) {   // the game compares the 32-bit pointer with the word
			Script::g_subsetTable[i] = (ScriptSubset *)obj;
			return;
		}
	}
}

ScriptReference *TFBUser_Behavior();
ScriptReference *TFBType_Behavior();

namespace Script {
}  // namespace Script

// GLOBAL: 0x5ce520 Script::OpBehavior::vftable
// 0x4318f0
ScriptReference *TFBType_Behavior()
{
	static OpBehavior s("behavior");
	return &s;
}

// the "new behavior" op prototype (an unnamed OpBehavior; RegisterOps registers it under that name)
// 0x431e30
ScriptReference *OpProto_NewBehavior()
{
	static OpBehavior s(0);
	return &s;
}

namespace Script {
// 0x432540
OpBehavior::OpBehavior(const OpBehavior &src) : OpBlock(src), m_name(((OpBehavior &)src).GetName()), m_startPC(src.m_startPC) {}

// 0x4324e0
OpBehavior *OpBehavior::CloneToPool(const OpBehavior *src)
{
	OpBehavior *r = new (g_scriptPool) OpBehavior(*src);
	g_scriptPool += sizeof(OpBehavior);
	return r;
}

// 0x4324d0
OpCode *OpBehavior::Clone()
{
	return CloneToPool(this);
}

// 0x431e10
void OpBehavior::vf10()
{
	TFBUser_Behavior()->SetReferent(0, 0);
}

// 0x431850
int OpBehavior::Parse()
{
	GetKind();
	GetName();
	return 0x10;
}
}  // namespace Script

// 0x431da0
ScriptReference *TFBUser_Behavior()
{
	static UserReference s("behavior", TFBType_Behavior(), (intptr_t)TFBType_Behavior(), 0);
	return &s;
}

// STL-style helpers of the symbol registries: MSVC 7.1 std::fill, copy_backward, _Uninit_copy, _Uninit_fill_n instances (element types 8 B and 0x10c B).
// The registry's element (SymE8 in symbolregistry.cpp: tagged object pointer + key) is 16 bytes on x64; an 8-byte int pair made
// SymbolRegistry_GetSize double the count there.
struct E8 { uintptr_t a, b; };
struct E10C { int d[0x43]; };
struct TreeNode { TreeNode *left, *parent, *right; int value0, value1; unsigned char color, nil; };   // std::_Tree_nod layout
struct Vec10C { int alval; E10C *begin; E10C *end; int Size(); };   // MSVC 7.1 std::vector: allocator word, _Myfirst, _Mylast
// GLOBAL: 0x61f878 g_symRegistryY (script/symbolregistry.h)

// 0x432be0
int Vec10C::Size()
{
	if (begin == 0)
		return 0;
	return end - begin;
}

// std::_Tree::_Min shape (leftmost node below `node`)
// 0x432c00
TreeNode *TreeMin(TreeNode *node)
{
	while (!node->left->nil)
		node = node->left;
	return node;
}

// 0x432c20
void E8Fill(E8 *first, E8 *last, const E8 *val)
{
	for (; first != last; ++first)
		*first = *val;
}

// 0x432c50
void E10CFill(E10C *first, E10C *last, const E10C *val)
{
	for (; first != last; ++first)
		*first = *val;
}

// 0x432cb0
int SymbolRegistry_GetSize()
{
	return g_symRegistryY->size();
}

// 0x432cd0
E8 *E8CopyBackward(E8 *first, E8 *last, E8 *dest)
{
	E8 *l = last;
	while (first != l)
		*--dest = *--l;
	return dest;
}

// 0x432d00
E10C *E10CCopyBackward(E10C *first, E10C *last, E10C *dest)
{
	while (first != last)
		*--dest = *--last;
	return dest;
}

namespace Script {
// 0x432880
void OpContext::Reset()
{
	m_nodes = 0;
	m_behaviorNode = 0;
	m_0c = 0;
	m_startTime = 0;
	for (unsigned i = 0; i < (unsigned)m_localCount; i++)
		m_locals[i]->AdjustRefCount(-1);
	void *l = m_locals;
	m_localCount = 0;
	operator_delete(l);
}

// 0x432bc0
void OpContext::FreeNodes()
{
	OpCode **n = m_nodes;
	if (n != 0) {
		Reset();
		operator_delete(n);
	}
}

// 0x432e60
int OpStartup::Execute()
{
	if (g_curScript->m_behaviorNode != 0) {
		SkipBlock();
		return 1;
	}
	return OpCode::Execute();
}
}  // namespace Script

// 0x432d40
E10C *E10CUninitCopy(E10C *first, E10C *last, E10C *dest)
{
	for (; first != last; ++first, ++dest) {
		if (dest)
			new ((void *)dest) E10C(*first);
	}
	return dest;
}

// 0x432d80
E8 *E8UninitCopy(E8 *first, E8 *last, E8 *dest)
{
	for (; first != last; ++first, ++dest) {
		if (dest)
			new ((void *)dest) E8(*first);
	}
	return dest;
}

// 0x432ee0
void E8UninitFillN(E8 *dest, unsigned n, const E8 *val)
{
	for (; n > 0; --n, ++dest) {
		if (dest)
			new ((void *)dest) E8(*val);
	}
}

// 0x432f10
void E10CUninitFillN(E10C *dest, unsigned n, const E10C *val)
{
	for (; n > 0; --n, ++dest) {
		if (dest)
			new ((void *)dest) E10C(*val);
	}
}

// 0x433290/0x4332c0 (vector::_Ufill wrappers) are MATCH with the real std::vector in script/w2c_stl.cpp (the junk `push eax` is the empty _Ptr_cat tag)

ScriptReference *TFBType_OpCode();
ScriptReference *OpProto_VariableCreate();
ScriptReference *OpProto_MemoryCheck();
ScriptReference *OpProto_ObjectControl();
ScriptReference *OpProto_ObjectReference();
ScriptReference *OpProto_BehaviorSet();
ScriptReference *OpProto_ObjectRemove();
ScriptReference *OpProto_ObjectReset();
ScriptReference *OpProto_OpIfElse();

struct OpRegistrar {
	void RegisterOps();
};

// Near miss: TFBType_OpCode (0x4310c0) stores only the vptr and calls RegisterOps (0x432390) with a 1-byte local at esp+3;
// ours makes an unwind funclet for the static and puts the local at [esp].

// 0x432390
void OpRegistrar::RegisterOps()
{
	((ScriptReference *)OpProto_NewBehavior())->ResolveSymbol(TFBType_OpCode(), "new behavior", 0);
	((ScriptReference *)OpProto_VariableCreate())->ResolveSymbol(TFBType_OpCode(), 0, 0);
	((ScriptReference *)OpProto_VariableFind())->ResolveSymbol(TFBType_OpCode(), 0, 0);
	((ScriptReference *)OpProto_MemoryCheck())->ResolveSymbol(TFBType_OpCode(), 0, 0);
	((ScriptReference *)OpProto_ObjectControl())->ResolveSymbol(TFBType_OpCode(), 0, 0);
	((ScriptReference *)OpProto_ObjectReference())->ResolveSymbol(TFBType_OpCode(), 0, 0);
	((ScriptReference *)OpProto_BehaviorSet())->ResolveSymbol(TFBType_OpCode(), 0, 0);
	((ScriptReference *)OpProto_ObjectSpawn())->ResolveSymbol(TFBType_OpCode(), 0, 0);
	((ScriptReference *)OpProto_ObjectRemove())->ResolveSymbol(TFBType_OpCode(), 0, 0);
	((ScriptReference *)OpProto_ObjectReset())->ResolveSymbol(TFBType_OpCode(), 0, 0);
	((ScriptReference *)OpProto_ObjectDisplace())->ResolveSymbol(TFBType_OpCode(), 0, 0);
	TFBType_Behavior()->vf10();
	((ScriptReference *)OpProto_OpComment())->ResolveSymbol(TFBType_OpCode(), 0, 0);
	((ScriptReference *)OpProto_OpDebugPrint())->ResolveSymbol(TFBType_OpCode(), 0, 0);
	((ScriptReference *)OpProto_OpIfElse())->ResolveSymbol(0, 0, 0);
}

