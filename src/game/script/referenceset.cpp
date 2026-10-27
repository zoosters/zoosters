// The script object set (Script::ReferenceSet) and the destructors of its set-member stand-ins.
#include "script/script.h"
#include "script/referenceset.h"
#include "script/actor.h"
#include "script/runtime.h"
#include <float.h>
using namespace Script;

void operator_delete(void *p);          // 0x5500f0

namespace Script {
// FindNode (0x43cbc0) is inlined by the callers in this unit
static __forceinline SetNode *FindNodeInl(ReferenceSet *set, ScriptReference *obj)
{
	SetNode *n = set->m_head;
	while (n != 0) {
		if (n->m_object == obj)
			return n;
		n = n->m_next;
	}
	return 0;
}
}

// 0x43cca0
void ReferenceSet::Clear()
{
	while ((m_tail = m_head) != 0) {
		SetNode *n = m_tail;
		m_head = m_head->m_next;
		if (n) {
			n->m_object->AdjustRefCount(-1);
			operator_delete(n);
		}
	}
	m_count = 0;
	m_sorted = 1;
	m_stamp++;
}

// Insertion sort of the list by key (descending sentinel logic as in the game: starts at the third node and never
// moves the tail). Its ecx stays `this` for the caller, so RemoveNode below does not reload it.
// 0x43cbf0
__declspec(noinline) void ReferenceSet::EnsureSorted()
{
	if (m_sorted)
		return;
	m_sorted = 1;
	SetNode *x = m_head->m_next->m_next;
	if (x == m_tail)
		return;
	do {
		SetNode *q = x->m_prev;
		SetNode *p = x->m_prev;
		while (x->m_key < q->m_key)
			q = q->m_prev;
		SetNode *next = x->m_next;
		if (q != p) {
			p->m_next = next;
			next->m_prev = x->m_prev;
			x->m_next = q->m_next;
			x->m_prev = q;
			q->m_next->m_prev = x;
			q->m_next = x;
		}
		x = next;
	} while (x != m_tail);
}

// 0x43ce50
bool ReferenceSet::RemoveNode(SetNode *node)
{
	if (node != 0) {
		EnsureSorted();
		if (node->m_prev == 0)
			m_head = node->m_next;
		else
			node->m_prev->m_next = node->m_next;
		if (node->m_next == 0)
			m_tail = node->m_prev;
		else
			node->m_next->m_prev = node->m_prev;
		m_count--;
		m_stamp++;
		node->m_object->AdjustRefCount(-1);
		operator_delete(node);
		return true;
	}
	return false;
}

// 0x43cec0
__declspec(noinline) bool ReferenceSet::Remove(ScriptReference *obj)
{
	return RemoveNode(FindNodeInl(this, obj));
}

// Each member of this set is a clone owned by a spawner (slot 31 of the member) whose clone set (+0x20) holds the member
// too: drop it from there (releasing that reference), then clear this set.
// 0x43b060
void ReferenceSet::DetachAll()
{
	for (SetNode *n = m_head; n != 0; n = n->m_next) {
		ScriptReference *obj = n->m_object;
		ScriptReference *owner = ((Placement *)obj)->GetSpawnSource();
		if (owner != 0 && static_cast<Placement *>(owner)->m_20.Remove(obj))
			owner->AdjustRefCount(-1);
	}
	Clear();
}

// The prototype set singletons (function-local statics) whose members get detached at level end
#include "modules/fx/partspray.h"

namespace Script { void DetachParticleSetClones(); }

// 0x43b880
void Script::DetachParticleSetClones()
{
	TFB_GetActiveParticleSet()->DetachAll();
	TFB_GetInactiveParticleSet()->DetachAll();
	TFB_GetRemovedParticleSet()->DetachAll();
}

namespace Script {
void DetachSpriteSetClones();
}

// 0x43e5e0
void Script::DetachSpriteSetClones()
{
	Script::TFB_GetActiveSpriteSet()->DetachAll();
	Script::TFB_GetRemovedSpriteSet()->DetachAll();
}

#include <stdlib.h>
static const float kSetNoKey = FLT_MAX;

// A set copies a set (members keep their order; with randomKey every member gets a random key); a single object
// becomes the only member.
// 0x43d340
void ReferenceSet::Assign(ScriptReference *src, char randomKey)
{
	if (src != this) {
		Clear();
		if (src->GetType() != TFBType_Set()) {
			Insert(src, kSetNoKey);
			return;
		}
		if (!randomKey)
			((ReferenceSet *)src)->EnsureSorted();
		for (SetNode *n = ((ReferenceSet *)src)->m_head; n != 0; n = n->m_next) {
			float key;
			if (randomKey)
				key = (float)rand();
			else
				key = kSetNoKey;
			Insert(n->m_object, key);
		}
	}
}

// 0x43d3d0
void ReferenceSet::Add(ScriptReference *obj)
{
	if (obj->GetType() != TFBType_Set()) {
		if (FindNodeInl(this, obj) == 0)
			Insert(obj, kSetNoKey);
		return;
	}
	if (obj != this) {
		for (SetNode *n = ((ReferenceSet *)obj)->m_head; n != 0; n = n->m_next) {
			ScriptReference *o = n->m_object;
			if (FindNodeInl(this, o) == 0)
				Insert(o, kSetNoKey);
		}
	}
}

// 0x43d460
void ReferenceSet::Subtract(ScriptReference *obj)
{
	if (obj->GetType() != TFBType_Set()) {
		RemoveNode(FindNodeInl(this, obj));
		return;
	}
	if (obj == this) {
		Clear();
		return;
	}
	for (SetNode *n = ((ReferenceSet *)obj)->m_head; n != 0; n = n->m_next) {
		ScriptReference *o = n->m_object;
		RemoveNode(FindNodeInl(this, o));
	}
}

// 0x43d4f0
void ReferenceSet::Intersect(ScriptReference *obj)
{
	if (obj->GetType() != TFBType_Set()) {
		SetNode *found = FindNodeInl(this, obj);
		Clear();
		if (found != 0)
			Insert(obj, kSetNoKey);
		return;
	}
	if (obj != this) {
		EnsureSorted();
		SetNode *n = ((ReferenceSet *)obj)->m_head;
		while (n != 0) {
			SetNode *next = n->m_next;
			ScriptReference *o = n->m_object;
			if (FindNodeInl((ReferenceSet *)obj, o) == 0)
				RemoveNode(n);
			n = next;
		}
	}
}

// Step through the set: mode 1 first, 2 last, 3 random member, 5 next, 6 previous, 7 a random number of steps from the
// current member (with wrap-around); *cur is the cursor node; returns the object at the cursor, or 0.
// 0x43cd00
ScriptReference *ReferenceSet::Next(int mode, SetNode **cur)
{
	if (m_count != 0) {
	switch (mode) {
	case 1:
		*cur = m_head;
		break;
	case 2:
		*cur = m_tail;
		break;
	case 3: {
		EnsureSorted();
		int idx = (int)((float)rand() * m_count * 3.0517578e-05f);
		*cur = m_head;
		if (idx > 0) {
			do {
				*cur = (*cur)->m_next;
			} while (--idx != 0);
		}
		break;
	}
	default:
		EnsureSorted();
		switch (mode) {
		case 5:
			*cur = (*cur)->m_next;
			if (*cur == 0)
				return 0;
			break;
		case 6:
			*cur = (*cur)->m_prev;
			if (*cur == 0)
				return 0;
			break;
		case 7: {
			int r = (int)((float)rand() * (m_count - 1) * 3.0517578e-05f);
			if (r * 2 <= m_count) {
				if (r >= 0) {
					int steps = r + 1;
					do {
						*cur = (*cur)->m_next;
						if (*cur == 0)
							*cur = m_head;
					} while (--steps != 0);
				}
			} else {
				int steps = m_count - r - 1;
				if (steps > 0) {
					do {
						*cur = (*cur)->m_prev;
						if (*cur == 0)
							*cur = m_tail;
					} while (--steps != 0);
				}
			}
			break;
		}
		}
	}
	return (*cur)->m_object;
	}
	return 0;
}

namespace Script {

// ~X { Clear; } with the inlined ~ReferenceSet (vptr 0x5ca608) is the same code as 0x402b20; each set member class
// gets its own copy (not folded by the linker).
// NOVTABLE: Script::ExposeSendersSetImpl
// GLOBAL: 0x43cca0 Script::ExposeSendersSetImpl::Clear
class ExposeSendersSetImpl : public ScriptReference {
public:
	~ExposeSendersSetImpl();
	void Clear();   // 0x43cca0
};
// 0x43a640
ExposeSendersSetImpl::~ExposeSendersSetImpl()
{
	Clear();
}

// NOVTABLE: Script::ExposeClonesImpl
// GLOBAL: 0x43cca0 Script::ExposeClonesImpl::Clear
class ExposeClonesImpl : public ScriptReference {
public:
	~ExposeClonesImpl();
	void Clear();   // 0x43cca0
	ScriptReference *Deref(ScriptReference *owner, int flags);   // 9: 0x43b150 (folded with the Sprite/Actor ExposeClones)
};
// 0x43b0b0
ExposeClonesImpl::~ExposeClonesImpl()
{
	Clear();
}

// 0x43b150
ScriptReference *ExposeClonesImpl::Deref(ScriptReference *owner, int flags)
{
	return &static_cast<Placement *>(owner)->m_20;
}

}  // namespace Script

namespace Script {
inline void ExposeSendersSetImpl::Clear() { ((ReferenceSet *)this)->Clear(); }
inline void ExposeClonesImpl::Clear() { ((ReferenceSet *)this)->Clear(); }
}
