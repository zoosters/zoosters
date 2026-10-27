#include "script/runtime.h"
#include "script/symbolregistry.h"
#include "script/referenceset.h"
using namespace Script;

// GLOBAL: 0x621f10 g_scriptSender
// GLOBAL: 0x621f14 g_scriptSelf
// GLOBAL: 0x621f18 g_scriptOwner
// GLOBAL: 0x621f1c g_forEachItem
// GLOBAL: 0x621f20 g_scriptVariable
// GLOBAL: 0x621f2c g_scriptOuterSelf
// GLOBAL: 0x621f30 g_scriptOuterOwner
// GLOBAL: 0x622004 g_scriptSubset
// GLOBAL: 0x61f878 g_symRegistryY
extern ScriptReference *g_scriptSender;
extern ScriptReference *g_scriptSelf;
extern ScriptReference *g_scriptOwner;
extern ScriptReference *g_forEachItem;
extern ScriptReference *g_scriptVariable;
extern ScriptReference *g_scriptOuterSelf;
extern ScriptReference *g_scriptOuterOwner;
extern ScriptReference *g_scriptSubset;


namespace Script {

// Root object of an operand. Sets *owner. A user object is replaced by what it refers to; `sel` is cleared when the user object itself is wanted.
static ScriptReference *ResolveRoot(int idx, unsigned bits, unsigned &sel, ScriptReference **owner)
{
	ScriptReference *cur;
	switch (idx) {
	case 0x3ffff:
		*owner = g_scriptOwner;
		return g_scriptSelf;
	case 0x3fffe:
		*owner = g_scriptOuterOwner;
		return g_scriptOuterSelf;
	case 0x3fffd:
		*owner = g_forEachItem;
		return g_forEachItem;
	case 0x3fffc:
		*owner = g_scriptSubset;
		return g_scriptSubset;
	case 0x3fffb:
		cur = g_scriptSender;
		break;
	case 0x3fffa:
		cur = g_scriptVariable;
		break;
	default:
		if (idx >= 0x3fc0d) {
			cur = g_curScript->m_locals[idx - 0x3fc0d];
		} else {
			SymbolSlot *slot = &(*g_symRegistryY)[idx];
			if (slot->obj & 1)
				cur = 0;
			else
				cur = (ScriptReference *)(slot->obj & ~(uintptr_t)1);
		}
		break;
	}
	if (cur != 0 && *(void **)cur == *(void **)TFBType_User()) {
		bool wantsUserObject = (bits & 0x3f00) == 0 && sel != 0;
		if (wantsUserObject)
			sel = 0;
		cur = cur->Deref(0, wantsUserObject);   // UserReference::Deref looks at the low byte only
	}
	*owner = cur;
	return cur;
}

// 0x43c5a0 Script::ScriptRef::Resolve: only the placement of the `tmp.b = false` block differs: the game puts it out of line after the first ret; if/else, ternary, sunk init, two calls all stay inline
ScriptReference *ScriptRef::Resolve(ScriptReference **owner, ScriptReference **set, bool resolveMember)
{
	unsigned sel = (bits >> 6) & 3;
	int idx = bits >> 14;
	SetNode *cursor;   // written by ReferenceSet::Next for selectors 1..3, never read
	ScriptReference *cur = ResolveRoot(idx, bits, sel, owner);
	*set = 0;
	if (cur != 0) {
		int cnt;
		unsigned member = (bits >> 8) & 0x3f;
		if (member != 0) {
			ScriptReference *memberObj = &((ScriptReference *)cur->GetMembers(&cnt))[member - 1];
			ScriptReference *resolved = memberObj->Deref(*owner, 0);
			cur = memberObj;
			if (resolved != memberObj && (resolveMember || sel != 0 || (bits & 0x3f) != 0)) {
				*owner = resolved;
				cur = resolved;
			}
		}
		if (cur != 0) {
			if (cur->GetType() == TFBType_Set()) {
				*set = cur;
				if (sel == 0) {
					*owner = cur;
					return cur;
				}
				cur = ((ReferenceSet *)cur)->Next(sel, &cursor);
				*owner = cur;
			}
			unsigned sub = bits & 0x3f;
			if (sub != 0 && cur != 0) {
					if (sel == 0 && cur == *owner) {
					void *vptr2 = *(void **)cur;
					if (vptr2 != *(void **)TFBType_User()) {
						ScriptReference *base = cur->GetBaseType();
						unsigned k = bits & 0x3f;
						cur = &((ScriptReference *)base->GetMembers(&cnt))[k - 1];
					} else {
						ScriptReference *base = TFBType_Position();
						unsigned k = bits & 0x3f;
						cur = &((ScriptReference *)base->GetMembers(&cnt))[k - 1];
					}
				} else
					cur = &((ScriptReference *)cur->GetMembers(&cnt))[sub - 1];
				ScriptReference *resolved = cur->Deref(*owner, 0);
				if (resolved != cur && resolveMember) {
					*owner = resolved;
					cur = resolved;
				}
			}
		}
	}
	return cur;
}


}  // namespace Script
