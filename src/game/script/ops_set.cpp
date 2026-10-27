// Script set ops (for each / change membership / check membership / find subset).
#include "script/ops_set.h"

// GLOBAL: 0x603044 g_forEachOrderNames
extern const char *g_forEachOrderNames[4];

// GLOBAL: 0x622000 g_forEachSet
extern Script::ScriptReference *g_forEachSet;
// GLOBAL: 0x621f1c g_forEachItem
extern Script::ScriptReference *g_forEachItem;

// GLOBAL: 0x622004 g_scriptSubset
extern Script::ScriptReference *g_scriptSubset;
// GLOBAL: 0x61f514 g_findSubOwner
extern Script::ScriptReference *g_findSubOwner;
// GLOBAL: 0x621f14 g_scriptSelf
extern Script::ScriptReference *g_scriptSelf;   // the running block's own object, left out of the subset

namespace Script {

// Defined here, in another translation unit than ValueSlide's copy constructor: cl 13.10 /GX looks at a callee's
// body in the same file and drops the EH frame that the game's ValueSlide constructor has when it can't throw.
ValueSlideEhTag::~ValueSlideEhTag() {}

// 0x430e50
ValueSlideCurve::ValueSlideCurve(const ValueSlideCurve &o) : ReferenceSet()
{
	m_sorted = o.m_sorted;
	m_count = o.m_count;
	m_stamp = o.m_stamp;
	m_head = o.m_head;
	m_tail = o.m_tail;
}

// 0x43cbc0
// (the set's membership test: the list node holding obj, or 0; the game's callers in this file rely on it
// leaving edx alone, which cl 13.10 only knows for a callee defined in the same translation unit)
__declspec(noinline) SetNode *ReferenceSet::FindNode(ScriptReference *obj)
{
	for (SetNode *n = m_head; n != 0; n = n->m_next) {
		if (n->m_object == obj) {
			return n;
		}
	}
	return 0;
}

// 0x43d590
int SetForEach::Execute()
{
	ScriptReference *savedSet = g_forEachSet;
	{
		ScriptReference *owner;
		ScriptReference *set;
		g_forEachSet = m_set.Resolve(&owner, &set, true);
	}
	if (g_forEachSet == 0) {
		SkipBlock();
		g_forEachSet = savedSet;
		return 1;
	}
	int dir = m_order;
	int reverse = 0;
	if (dir == 2) {
		dir = 0;
		reverse = 1;
	}
	int r;
	{
		ScriptSubset iter((ReferenceSet *)g_forEachSet, reverse);
		ScriptReference *savedItem = g_forEachItem;
		int pc = g_scriptPC;
		SetNode *cursor;
		g_forEachItem = iter.Next(dir + 1, &cursor);
		for (;;) {
			bool have = g_forEachItem != 0;
			if (!have) {
				SkipBlock();
				r = 1;
			} else {
				r = OpCode::Execute();
				if (r != 1) {
					break;
				}
			}
			if (!(m_flags & 0x80)) {
				break;
			}
			g_forEachItem = iter.Next(dir + 5, &cursor);
			if (g_forEachItem == 0) {
				break;
			}
			g_scriptPC = pc;
		}
		g_forEachItem = savedItem;
	}
	g_forEachSet = savedSet;
	return r;
}

// 0x430610
int SetFindSub::Execute()
{
	ScriptReference *owner;
	ReferenceSet *set;
	m_target.Resolve(&owner, (ScriptReference **)&set, true);
	if (set == 0) {
		SkipBlock();
		return 1;
	}
	ScriptSubset *sub = new ScriptSubset(set, set->GetBaseType());
	((ScriptReference *)sub)->AdjustRefCount(1);
	float rhs;
	if (m_value.EvalFloat(&rhs)) {
		Measurement *member;
		unsigned idx = m_target.bits & 0x3f;
		if (idx == 0) {
			member = (Measurement *)set->GetBaseType();
		} else {
			int count;
			char *members = (char *)set->GetBaseType()->GetMembers(&count);
			member = (Measurement *)(members - sizeof(ScriptReference) + idx * sizeof(ScriptReference));   // the member table is an array of ScriptReference-sized slots
		}
		ScriptReference *skip = g_scriptSelf;
		SetNode *cursor;
		ScriptReference *item = set->Next(1, &cursor);
		while (item != 0) {
			if (item != skip) {
				float v = member->GetFloat(item);
				if (TFB_CompareFloat(v, m_value.opArg, rhs)) {
					sub->Insert(item, v);
				}
			}
			item = set->Next(5, &cursor);
		}
	}
	ScriptReference *savedOwner = g_findSubOwner;
	ScriptReference *savedSub = g_scriptSubset;
	g_scriptSubset = (ScriptReference *)sub;
	if (set != (ReferenceSet *)savedSub) {
		g_findSubOwner = set;
	}
	int r;
	if (sub->m_count == 0) {
		SkipBlock();
		r = 1;
	} else {
		r = OpCode::Execute();
	}
	g_findSubOwner = savedOwner;
	g_scriptSubset = savedSub;
	((ScriptReference *)sub)->AdjustRefCount(-1);
	return r;
}

// 0x43d7a0
int SetForEach::Parse()
{
	GetName();
	ScriptRefReader refs(TFBType_Set());
	refs.Read(&m_set);
	ScriptEnumReader en(&m_order, g_forEachOrderNames, 0);
	return sizeof(SetForEach);
}

// 0x43d9f0
// Shared with SetMembershipCheck::Parse: the game's linker folded the two into one body (same fields at the same offsets), so the body is
// written once over the op and its fields and both members call it.
static int ParseMembershipOp(OpCode *op, ScriptRef *set, unsigned char *arg, ScriptRef *object, int size)
{
	op->GetName();
	ScriptReference *type = TFBType_Set();
	ScriptRefReader refs(type);
	refs.Read(set);
	TFBRead_u8(arg);
	ScriptRefReader refs2(type);
	refs2.Read(object);
	return size;
}
int SetMembershipChange::Parse() { return ParseMembershipOp(this, &m_set, &m_action, &m_object, sizeof(SetMembershipChange)); }
// FOLDED: 0x43d9f0 Script::SetMembershipCheck::Parse
int SetMembershipCheck::Parse() { return ParseMembershipOp(this, &m_set, &m_mode, &m_object, sizeof(SetMembershipCheck)); }

// 0x43d7f0
int SetMembershipChange::Execute()
{
	ScriptReference *owner;
	ScriptReference *set;
	ScriptReference *owner2;
	ScriptReference *set2;
	ReferenceSet *theSet = (ReferenceSet *)m_set.Resolve(&owner, &set, true);
	if (theSet != 0) {
		ScriptReference *object = m_object.Resolve(&owner2, &set2, true);
		if (object != 0) {
			switch (m_action) {
			case 0:
				theSet->Add(object);
				break;
			case 1:
				theSet->Subtract(object);
				break;
			case 2:
				theSet->Intersect(object);
				break;
			case 3:
				theSet->Assign(object, 0);
				break;
			}
		}
	}
	return 1;
}

// 0x43d8a0
int SetMembershipCheck::Execute()
{
	ScriptReference *owner;
	ScriptReference *set;
	ScriptReference *owner2;
	ScriptReference *set2;
	bool go = false;
	ReferenceSet *theSet = (ReferenceSet *)m_set.Resolve(&owner, &set, true);
	if (theSet != 0) {
		ScriptReference *object = m_object.Resolve(&owner2, &set2, true);
		if (object != 0) {
			if (object->GetType() != TFBType_Set()) {
				switch (m_mode) {
				case 0:
					go = theSet->FindNode(object) != 0;
					break;
				case 1:
					go = theSet->FindNode(object) == 0;
					break;
				case 2:
					go = theSet->FindNode(object) != 0;
					break;
				}
			} else {
				SetNode *n;
				switch (m_mode) {
				case 0:
					n = ((ReferenceSet *)object)->m_head;
					if (n == 0) {
						go = true;
					} else {
						do {
							go = theSet->FindNode(n->m_object) != 0;
							if (!go) {
								break;
							}
							n = n->m_next;
						} while (n != 0);
					}
					break;
				case 1:
					n = ((ReferenceSet *)object)->m_head;
					if (n == 0) {
						go = true;
					} else {
						do {
							go = theSet->FindNode(n->m_object) == 0;
							if (!go) {
								break;
							}
							n = n->m_next;
						} while (n != 0);
					}
					break;
				case 2:
					n = ((ReferenceSet *)object)->m_head;
					if (n == 0) {
						go = true;
					} else {
						do {
							go = theSet->FindNode(n->m_object) != 0;
							if (go) {
								break;
							}
							n = n->m_next;
						} while (n != 0);
					}
					break;
				default:
					go = true;
					break;
				}
			}
		}
	}
	if (!go) {
		SkipBlock();
		return 1;
	}
	return OpCode::Execute();
}

// 0x43cc60
const char *SetForEach::GetName()
{
	return "for each";
}

// 0x43cc70
int SetForEach::IsBlock()
{
	return 101;
}

// 0x43d140
OpCode *SetForEach::Clone()
{
	return CloneToPool(this);
}

// 0x43d150
SetForEach *SetForEach::CloneToPool(const SetForEach *src)
{
	void *mem = g_scriptPool;
	SetForEach *r = new (mem) SetForEach(*src);
	g_scriptPool += sizeof(SetForEach);
	return r;
}

// 0x43cc80
const char *SetMembershipChange::GetName()
{
	return "change membership";
}

// 0x43d080
OpCode *SetMembershipChange::Clone()
{
	return CloneToPool(this);
}

// 0x43d090
SetMembershipChange *SetMembershipChange::CloneToPool(const SetMembershipChange *src)
{
	void *mem = g_scriptPool;
	SetMembershipChange *r = new (mem) SetMembershipChange(*src);
	g_scriptPool += sizeof(SetMembershipChange);
	return r;
}

// 0x43cc90
const char *SetMembershipCheck::GetName()
{
	return "check membership";
}

// 0x43d210
OpCode *SetMembershipCheck::Clone()
{
	return CloneToPool(this);
}

// 0x43d220
SetMembershipCheck *SetMembershipCheck::CloneToPool(const SetMembershipCheck *src)
{
	void *mem = g_scriptPool;
	SetMembershipCheck *r = new (mem) SetMembershipCheck(*src);
	g_scriptPool += sizeof(SetMembershipCheck);
	return r;
}

// 0x43d060
const char *SetFindSub::GetName()
{
	return "find subset";
}

// 0x43d070: folded with Script::CheckFOV::IsBlock (identical code), annotated in ops_actor.cpp
int SetFindSub::IsBlock()
{
	return 3;
}

// 0x43d1a0
OpCode *SetFindSub::Clone()
{
	return CloneToPool(this);
}

// 0x43d1b0
SetFindSub *SetFindSub::CloneToPool(const SetFindSub *src)
{
	void *mem = g_scriptPool;
	SetFindSub *r = new (mem) SetFindSub(*src);
	g_scriptPool += sizeof(SetFindSub);
	return r;
}

}  // namespace Script

namespace Script {

// 0x429730
const char *ScriptSubset::GetName()
{
	return m_name;
}

}  // namespace Script
