// Script flow, variable and object ops: GetName / GetAltName / IsBlock / Clone / CloneToPool, Parse and Execute.
#include "script/behaviorlayout.h"
#include "script/ops_object.h"
#include "script/runtime.h"
#include "script/actor.h"
#include "script/types/scriptactor.h"
#include <math.h>
#include "modules/tfb/gameactor.h"
#include "script/ops_message.h"


Script::ScriptReference *TFBType_OpCode();
Script::ScriptReference *TFBType_Placement();
Script::ScriptReference *TFBType_Vector();
Script::ScriptReference *TFBType_Behavior();
Script::ScriptReference *TFBType_Angle();
Script::ScriptReference *TFBUser_Behavior();
namespace Script { void SetScriptContext(ScriptReference *self, void *owner); }  // 0x43bb70
uintptr_t TFB_ObjectFromIndex(int index, char userFlag);   // 0x416220 (script/objectindex.cpp): the RefTarget pointer

// Interpreter globals not named yet.
extern Script::ScriptReference *g_scriptVariable;  // the object a "find variable" block is running on
extern Script::ScriptReference *g_scriptSelf;   // 0x621f14
extern Script::ScriptReference *g_scriptOwner;  // 0x621f18
extern Script::ScriptReference *g_scriptOuterSelf;
extern Script::ScriptReference *g_scriptOuterOwner;
extern Script::ScriptReference *g_scriptSender;  // 0x621f10

namespace Script {

// The list GetVariableList returns is the actor's OpContext (w2c_f.cpp): count = OpContext::m_localCount, items = m_locals


}

namespace Script {

// 0x431040
const char *OpComment::GetName()
{
	return "comment:";
}

// 0x431050
const char *OpDebugPrint::GetName()
{
	return "print";
}

// 0x431770
const char *OpIfElse::GetName()
{
	return "if/else";
}

// 0x431780
int OpIfElse::IsBlock()
{
	return 5;
}

// 0x4325a0
OpCode *OpIfElse::Clone()
{
	return CloneToPool(this);
}

// 0x4325b0
OpIfElse *OpIfElse::CloneToPool(const OpIfElse *src)
{
	void *mem = g_scriptPool;
	OpIfElse *r = new (mem) OpIfElse(*src);
	g_scriptPool += sizeof(OpIfElse);
	return r;
}

// 0x4310a0
const char *VariableFind::GetName()
{
	return "find variable";
}

// 0x4310b0
int VariableFind::IsBlock()
{
	return 6;
}

// 0x432650
OpCode *VariableFind::Clone()
{
	return CloneToPool(this);
}

// 0x432660
VariableFind *VariableFind::CloneToPool(const VariableFind *src)
{
	void *mem = g_scriptPool;
	VariableFind *r = new (mem) VariableFind(*src);
	g_scriptPool += sizeof(VariableFind);
	return r;
}

// 0x4312f0
const char *VariableCreate::GetName()
{
	return "create variable";
}

// 0x431330
const char *MemoryCheck::GetName()
{
	return "check reference";
}

// 0x432820
OpCode *MemoryCheck::Clone()
{
	return CloneToPool(this);
}

// 0x432830
MemoryCheck *MemoryCheck::CloneToPool(const MemoryCheck *src)
{
	void *mem = g_scriptPool;
	MemoryCheck *r = new (mem) MemoryCheck(*src);
	g_scriptPool += sizeof(MemoryCheck);
	return r;
}

// 0x431090
const char *ObjectControl::GetName()
{
	return "control";
}

// Folded: 0x42cbe0 is also ScriptReference slot 6 (GetKind, returns 2); defined once, here.
// 0x42cbe0
int ObjectControl::IsBlock()
{
	return 2;
}

// 0x4325f0
OpCode *ObjectControl::Clone()
{
	return CloneToPool(this);
}

// 0x432600
ObjectControl *ObjectControl::CloneToPool(const ObjectControl *src)
{
	void *mem = g_scriptPool;
	ObjectControl *r = new (mem) ObjectControl(*src);
	g_scriptPool += sizeof(ObjectControl);
	return r;
}

// 0x4313f0
const char *ObjectSpawn::GetName()
{
	return "spawn";
}

// 0x431400
const char *ObjectSpawn::GetAltName()
{
	return "spawn actor";
}

// 0x4327a0
OpCode *ObjectSpawn::Clone()
{
	return CloneToPool(this);
}

// 0x4327b0
ObjectSpawn *ObjectSpawn::CloneToPool(const ObjectSpawn *src)
{
	void *mem = g_scriptPool;
	ObjectSpawn *r = new (mem) ObjectSpawn(*src);
	g_scriptPool += sizeof(ObjectSpawn);
	return r;
}

// 0x431390
const char *ObjectReference::GetName()
{
	return "set reference";
}

// 0x4321f0
OpCode *ObjectReference::Clone()
{
	return CloneToPool(this);
}

// 0x432200
ObjectReference *ObjectReference::CloneToPool(const ObjectReference *src)
{
	void *mem = g_scriptPool;
	ObjectReference *r = new (mem) ObjectReference(*src);
	g_scriptPool += sizeof(ObjectReference);
	return r;
}

// 0x4313e0
const char *BehaviorSet::GetName()
{
	return "set behavior";
}

// 0x432160
OpCode *BehaviorSet::Clone()
{
	return CloneToPool(this);
}

// 0x432170
BehaviorSet *BehaviorSet::CloneToPool(const BehaviorSet *src)
{
	void *mem = g_scriptPool;
	BehaviorSet *r = new (mem) BehaviorSet(*src);
	g_scriptPool += sizeof(BehaviorSet);
	return r;
}

// 0x4314c0
const char *ObjectRemove::GetName()
{
	return "remove";
}

// 0x432250
OpCode *ObjectRemove::Clone()
{
	return CloneToPool(this);
}

// 0x432260
ObjectRemove *ObjectRemove::CloneToPool(const ObjectRemove *src)
{
	void *mem = g_scriptPool;
	ObjectRemove *r = new (mem) ObjectRemove(*src);
	g_scriptPool += sizeof(ObjectRemove);
	return r;
}

// 0x431510
const char *ObjectReset::GetName()
{
	return "reset";
}

// 0x4322a0
OpCode *ObjectReset::Clone()
{
	return CloneToPool(this);
}

// 0x4322b0
ObjectReset *ObjectReset::CloneToPool(const ObjectReset *src)
{
	void *mem = g_scriptPool;
	ObjectReset *r = new (mem) ObjectReset(*src);
	g_scriptPool += sizeof(ObjectReset);
	return r;
}

// 0x431590
const char *ObjectDisplace::GetName()
{
	return "displace";
}

// 0x4322f0
OpCode *ObjectDisplace::Clone()
{
	return CloneToPool(this);
}

// 0x432300
ObjectDisplace *ObjectDisplace::CloneToPool(const ObjectDisplace *src)
{
	void *mem = g_scriptPool;
	ObjectDisplace *r = new (mem) ObjectDisplace(*src);
	g_scriptPool += sizeof(ObjectDisplace);
	return r;
}

// 0x431880
int OpComment::Parse()
{
	GetName();
	char *text = m_08;
	if (TFBType_OpCode() != 0) {
		TFBRead_pstr(text);
	}
	return 0;
}

// 0x4318a0
int OpDebugPrint::Parse()
{
	char buffer[0x100];
	ScriptRefReader refs(0);
	m_0c = buffer;
	GetName();
	refs.Read(&m_08);
	char *text = m_0c;
	if (TFBType_OpCode() != 0) {
		TFBRead_pstr(text);
	}
	return sizeof(OpDebugPrint);
}

// 0x431300
int VariableCreate::Parse()
{
	GetName();
	ScriptRefReader refs(0);
	refs.Read(&m_08);
	return 0;
}

// 0x431000
int VariableFind::Parse()
{
	GetName();
	ScriptRefReader refs(0);
	refs.Read(&m_08);
	ScriptRefReader refs2(TFBType_Actor());
	refs2.Read(&m_0c);
	return sizeof(VariableFind);
}

// 0x431340
int MemoryCheck::Parse()
{
	GetName();
	ScriptRefReader refs(TFBType_User());
	refs.Read(&m_08);
	ScriptRefReader refs2(g_scriptLastType->GetType());
	refs2.Read(&m_0c);
	return sizeof(MemoryCheck);
}

// 0x430fa0
int ObjectControl::Parse()
{
	GetName();
	ScriptRefReader refs(0);
	refs.Read(&m_08);
	if (g_scriptLastType->GetType() == TFBType_Actor())
		TFBRead_u8(&m_0c);
	else
		m_0c = 0;
	return sizeof(ObjectControl);
}

// 0x431410
int ObjectSpawn::Parse()
{
	GetName();
	ScriptRefReader refs(TFBType_Placement());
	refs.Read(&m_08);
	ScriptRefReader refs2(TFBType_Position());
	refs2.Read(&m_10);
	ScriptValueReader(&m_14, TFBType_Angle(), 0, 2);
	if (g_scriptLastType->GetType() == TFBType_Actor())
		TFBRead_u8(&m_0c);
	else
		m_0c = 0;
	return sizeof(ObjectSpawn);
}

// 0x4313a0
int ObjectReference::Parse()
{
	GetName();
	ScriptRefReader refs(TFBType_User());
	refs.Read(&m_08);
	ScriptRefReader refs2(0);
	refs2.Read(&m_0c);
	return sizeof(ObjectReference);
}

// 0x431aa0
int BehaviorSet::Parse()
{
	GetName();
	ScriptRefReader refs(TFBType_Behavior());
	refs.Read(&m_08);
	return sizeof(BehaviorSet);
}

// Shared with ObjectReset::Parse (same address).
// 0x431560
int ObjectRemove::Parse()
{
	GetName();
	ScriptRefReader refs(TFBType_Placement());
	refs.Read(&m_08);
	return sizeof(ObjectRemove);
}

// FOLDED: 0x431560 Script::ObjectReset::Parse
// (the game folds it into ObjectRemove::Parse; written out for ObjectReset instead of calling the sibling class's member)
int ObjectReset::Parse()
{
	GetName();
	ScriptRefReader refs(TFBType_Placement());
	refs.Read(&m_08);
	return sizeof(ObjectReset);
}

// 0x4316f0
int ObjectDisplace::Parse()
{
	GetName();
	ScriptRefReader refs(TFBType_Vector());
	refs.Read(&m_08);
	TFBRead_u8(&m_0c.opArg);
	ScriptValueReader(&m_0c, TFBType_Angle(), 0, 2), ScriptValueReader(&m_18, TFBType_Angle(), 0, 2), ScriptValueReader(&m_24, g_scriptLastType, 0, 4);
	return sizeof(ObjectDisplace);
}

// 0x4314d0
int ObjectRemove::Execute()
{
	ScriptReference *owner, *set;
	Placement *obj = (Placement *)m_08.Resolve(&owner, &set, true);
	if (obj != 0) {
		obj->m_20.m_1c = 0;
		obj->Remove();
	}
	return 1;
}

// 0x431520
int ObjectReset::Execute()
{
	ScriptReference *owner, *set;
	Placement *obj = (Placement *)m_08.Resolve(&owner, &set, true);
	if (obj != 0) {
		obj->m_20.m_1c = 0;
		obj->Reset();
	}
	return 1;
}

// 0x4321b0
int BehaviorSet::Execute()
{
	ScriptReference *owner, *set;
	ScriptReference *obj = m_08.Resolve(&owner, &set, true);
	if (obj != 0 && BEHAVIOR_START(obj) != 0) {
		TFBUser_Behavior()->SetReferent(obj, 0);
	}
	return 1;
}

// 0x431c30
int MemoryCheck::Execute()
{
	// two out-parameter slots: the game's frame has them in the other order than two named locals give; an array
	// reproduces it (owner = t[1], set = t[0] in the first call, swapped in the second)
	ScriptReference *t[2];
	int r;
	ScriptReference *a = m_08.Resolve(&t[1], &t[0], true);
	if (a != 0) {
		if (a != m_0c.Resolve(&t[0], &t[1], true))
			a = 0;
	}
	if (a == 0) {
		m_flags &= ~0x80;
		g_scriptPC += m_flags >> 11;
		r = 1;
	} else {
		r = OpCode::Execute();
	}
	return r;
}

// 0x431ca0
int VariableFind::Execute()
{
	ScriptReference *saved = g_scriptVariable;
	g_scriptVariable = 0;
	ScriptReference *owner, *set;
	ScriptReference *obj = m_0c.Resolve(&owner, &set, true);
	if (obj != 0) {
		if (obj->IsKindOf(TFBType_Actor()) != 0 && static_cast<ScriptActor *>(obj)->m_controlMode < 2) {
			OpContext *list = static_cast<RWS::TFB::GameActor *>(static_cast<ScriptActor *>(obj))->GetVariableList(1);
			if (list != 0) {
#define OL_COUNT m_localCount
#define OL_ITEMS m_locals
				int i = list->OL_COUNT;
				int want = m_08.bits;
				while (i != 0) {
					i--;
					if ((uintptr_t)list->OL_ITEMS[i]->GetName() == (uintptr_t)(unsigned)want) {
						g_scriptVariable = list->OL_ITEMS[i];
						g_scriptVariable->AdjustRefCount(1);
						break;
					}
				}
			}
		}
	}
	int r;
	if (g_scriptVariable == 0) {
		m_flags &= ~0x80;
		g_scriptPC += m_flags >> 11;
		r = 1;
	} else {
		r = OpCode::Execute();
	}
	if (g_scriptVariable != 0)
		g_scriptVariable->AdjustRefCount(-1);
	g_scriptVariable = saved;
	return r;
}

// 0x4319b0
int ObjectControl::Execute()
{
	ScriptReference *owner, *set;
	ScriptReference *obj = m_08.Resolve(&owner, &set, true);
	if (obj != 0 && obj != g_scriptSelf) {
		if (obj->GetType() != TFBType_User()) {
			if (obj->IsKindOf(TFBType_Placement()) != 0) {
				char c = ((Placement *)obj)->Control(m_0c == 0);
				if (c == 0)
					obj = 0;
			}
		}
	}
	ScriptReference *savedOuterSelf = g_scriptOuterSelf;
	ScriptReference *savedOuterOwner = g_scriptOuterOwner;
	g_scriptOuterOwner = g_scriptOwner;
	g_scriptOuterSelf = g_scriptSelf;
	SetScriptContext(obj, owner);
	int r;
	if (obj == 0) {
		m_flags &= ~0x80;
		g_scriptPC += m_flags >> 11;
		r = 1;
	} else {
		r = OpCode::Execute();
	}
	SetScriptContext(g_scriptOuterSelf, g_scriptOuterOwner);
	g_scriptOuterOwner = savedOuterOwner;
	g_scriptOuterSelf = savedOuterSelf;
	return r;
}

// 0x431ad0
int ObjectSpawn::Execute()
{
	float angle;                        // the heading (the game keeps it in the stack slot of the unused owner out of m_08: a pointer-sized write on x64)
	ScriptReference *unusedOwner, *posOwner, *set;
	Placement *obj = (Placement *)m_08.Resolve(&unusedOwner, &set, true);
	if (obj != 0) {
		VectorMeasurement *src = (VectorMeasurement *)m_10.Resolve(&posOwner, &set, true);
		if (src == 0) {
			posOwner = obj;
			src = obj;
		}
		if (m_14.EvalAngle(&angle, 0) == 0)
			angle = 0;
		// the heading is a float argument: x64 passes it in an xmm register
		obj = (Placement *)obj->FinishSpawn((intptr_t)src->GetVector(posOwner), angle, m_0c == 0);
	}
	void *result = obj;
	ScriptReference *savedOuterSelf = g_scriptOuterSelf;
	ScriptReference *savedOuterOwner = g_scriptOuterOwner;
	g_scriptOuterOwner = g_scriptOwner;
	g_scriptOuterSelf = g_scriptSelf;
	SetScriptContext((ScriptReference *)result, (ScriptReference *)result);
	int r;
	if (result == 0) {
		m_flags &= ~0x80;
		g_scriptPC += m_flags >> 11;
		r = 1;
	} else {
		r = OpCode::Execute();
	}
	SetScriptContext(g_scriptOuterSelf, g_scriptOuterOwner);
	g_scriptOuterOwner = savedOuterOwner;
	g_scriptOuterSelf = savedOuterSelf;
	return r;
}

// 0x4315a0
int ObjectDisplace::Execute()
{
	ScriptReference *owner, *set;
	VectorMeasurement *obj = (VectorMeasurement *)m_08.Resolve(&owner, &set, true);
	if (obj != 0) {
		float azimuth, elevation, distance;
		if (m_0c.EvalAngle(&azimuth, 0)) {
			if (m_18.EvalAngle(&elevation, 1)) {
				if (m_24.EvalFloat(&distance)) {
					distance = distance * 12.0f;  // constant at 0x5ca758
					Vec3 v;
					float z;
					if (m_0c.opArg == 1) {
						v.y = 0.0f;
						v.x = 0.0f;
						z = 0.0f;
					} else {
						v = *obj->GetVector(owner);
						z = v.z;
					}
					elevation = elevation * 0.017453292f;
					float horizontal = distance;
					float s = (float)sin(elevation);
					if (s != 0.0f) {
						horizontal = (float)cos(elevation) * horizontal;
						v.y = s * distance + v.y;
					}
					azimuth = azimuth * 0.017453292f;
					v.x = (float)sin(azimuth) * horizontal + v.x;
					v.z = (float)cos(azimuth) * horizontal + z;
					obj->SetVector(&v, owner);
				}
			}
		}
	}
	return 1;
}

typedef UserReference RefVar;   // the "user" variables (vtable 0x5d2f70)

// 0x4326c0
int ObjectReference::Execute()
{
	ScriptReference *owner, *set;
	ScriptReference *src = m_0c.Resolve(&owner, &set, true);
	if (src != 0) {
		unsigned dst = m_08.bits;
		if ((dst & 0x3f00) == 0 && (dst & 0xc0) == 0) {
			RefVar *target;
			if (dst >> 14 == 0x3fffa)
				target = (RefVar *)g_scriptVariable;
			else
				target = (RefVar *)TFB_ObjectFromIndex(dst >> 14, 1);
			if (target != 0) {
				target->SetReferent(src, owner);
				unsigned bits = m_0c.bits;
				if ((bits & 0xffffc000) == 0xfffec000 && (bits & 0x3f00) == 0 && (bits & 0x3f) == 0 && (bits & 0xc0) == 0) {
					target->SetVector(((RefVar *)g_scriptSender)->GetVector((RefVar *)g_scriptSender), target);
					return 1;
				}
			}
		}
		else {
			ScriptReference *target = m_08.Resolve(&owner, &set, false);
			if (target != 0 && owner != 0)
				target->SetReferent(src, owner);
		}
	}
	return 1;
}

}  // namespace Script
