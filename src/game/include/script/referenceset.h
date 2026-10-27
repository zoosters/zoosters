// Script::ReferenceSet (PS2 RTTI name; vtable 0x5ca608): the script engine's set of script objects, a doubly linked list of nodes
// kept in key order, every member reference counted (AdjustRefCount +1 on insert, -1 on removal). Its subclasses: the second set of
// every Placement (Placement::PlacementSet, actor.h), the temporary sets of the set ops (ScriptSubset), the clone list, the rumble
// set and the message queue (whose nodes carry a post time instead of a sort key).
// The methods are the game's 0x43ca30-0x43d4f0.
#ifndef ZOOSTERS_SCRIPT_REFERENCESET_H
#define ZOOSTERS_SCRIPT_REFERENCESET_H

#include "zoosters/base.h"
#include "script/script.h"
#include "script/value.h"

namespace Script {

// One element of a set (0x10 bytes).
struct SetNode {
	SetNode *m_prev;
	SetNode *m_next;
	union {
		float m_key;  // sort key (Insert's key; FLT_MAX = append)
		unsigned m_postTime;  // the message queue's nodes: the time the message was posted (an integer in the same slot)
	};
	ScriptReference *m_object;  // the member
};
ZOOSTERS_ASSERT_SIZE(SetNode, 0x10);
ZOOSTERS_ASSERT_OFFSET(SetNode, m_next, 0x04);
ZOOSTERS_ASSERT_OFFSET(SetNode, m_key, 0x08);
ZOOSTERS_ASSERT_OFFSET(SetNode, m_object, 0x0c);   // the mirrors' node layouts (SlideNodeCb, RelNodeW, ...) of the game build

// The modes of ReferenceSet::Next.
enum SetCursorMode {
	SET_FIRST = 1,
	SET_LAST = 2,
	SET_RANDOM = 3,
	SET_FOLLOWING = 5,
	SET_PREVIOUS = 6,
	SET_RANDOM_WRAP = 7
};

// GLOBAL: 0x5ca608 Script::ReferenceSet::vftable
// normal build: the game's vtable 0x5ca608 has 18 slots: ScriptReference's 0..10, then the ValueInterface / Measurement slots
// 11..16 (GetMax, GetValueType, GetInt, GetFloat, SetValue x2: the sets are used as the owner objects of values) and
// AssignOrdered as 17; the compiler's vtable has them when the class derives from Measurement (no data members there, the
// layout is the same)
class ReferenceSet : public Measurement {
public:
	inline ReferenceSet() : m_sorted(1), m_count(0), m_stamp(0), m_head(0), m_tail(0) {}
	~ReferenceSet();                                          // 0x402b20: Clear
	// normal build: the compiler's vtable of every set class starts with the game's slots 0..6 of vtable 0x5ca608 (bodies bound in
	// script/normal_bringup.cpp); the game gets them from the address-bound vtable block
	const char *GetName();                                    // 0: 0x402ba0 "set"
	ScriptReference *GetType();                               // 4: 0x402bb0 jmp TFBType_Set
	ScriptReference *GetBaseType();                           // 5: 0x402b70 jmp TFBType_Value
	int GetKind();                                            // 6: 0x43e6f0 returns 0
	using Measurement::SetValue;
	void SetValue(int value, ScriptReference *owner);         // 16: 0x4025a0 (empty; bound in script/normal_bringup.cpp)

	SetNode *Insert(ScriptReference *obj, float key);         // 0x43ca30: new node in key order (FLT_MAX: at the end)
	SetNode *FindNode(ScriptReference *obj);                  // 0x43cbc0: the node holding obj, or 0
	void EnsureSorted();                                      // 0x43cbf0: insertion sort by key when m_sorted is clear
	void Clear();                                             // 0x43cca0: every node released
	ScriptReference *Next(int mode, SetNode **cursor);        // 0x43cd00: the element at a cursor (SetCursorMode), 0 at the end
	bool RemoveNode(SetNode *node);                           // 0x43ce50
	bool Remove(ScriptReference *obj);                        // 0x43cec0: one object
	void Assign(ScriptReference *src, char randomKeys);       // 0x43d340: Clear, then obj or every element of the set obj
	void Add(ScriptReference *obj);                           // 0x43d3d0: union with obj (an object or a set)
	void Subtract(ScriptReference *obj);                      // 0x43d460: difference (removes obj, or every element of the set obj)
	void Intersect(ScriptReference *obj);                     // 0x43d4f0: keeps obj only / the elements also in the set obj
	int GetInt(ScriptReference *owner);  // 13: 0x402bd0 (slot of the base vtable 0x5ca608): the element count of the owner set (ignores this)
	virtual void AssignOrdered(ScriptReference *obj, int unused);  // 17: 0x402bc0 (virtual in a normal build only): Assign without random keys
	void DeactivateAllActors();  // 0x435170: every actor in the set is reset to idle, the ones it spawned are released, then the set is cleared (script/gameactor.cpp)
	void DetachAll();  // 0x43b060: every member is a clone of a spawner: drop it from the spawner's set, then Clear

	unsigned char m_sorted;  // the list is in key order (Insert with a key out of order clears it)
	int m_count;
	int m_stamp;  // changes with every insert / removal / clear (cursors notice a change)
	SetNode *m_head;
	SetNode *m_tail;
};
ZOOSTERS_ASSERT_SIZE(ReferenceSet, 0x18);
ZOOSTERS_ASSERT_OFFSET(ReferenceSet, m_sorted, 0x04);
ZOOSTERS_ASSERT_OFFSET(ReferenceSet, m_count, 0x08);
ZOOSTERS_ASSERT_OFFSET(ReferenceSet, m_stamp, 0x0c);
ZOOSTERS_ASSERT_OFFSET(ReferenceSet, m_head, 0x10);
ZOOSTERS_ASSERT_OFFSET(ReferenceSet, m_tail, 0x14);

ReferenceSet *TFB_GetActiveSpriteSet();    // 0x43e520 the sprites that exist (sorted by priority key)
ReferenceSet *TFB_GetRemovedSpriteSet();   // 0x43e580 sprites that were removed (kept for the scripts that still name them)

}  // namespace Script

// The actor sets and the particle sets: function-local statics of the Expose*Set classes, global getters.
Script::ReferenceSet *TFB_GetPlayerSet();            // 0x437a20 the player actors
Script::ReferenceSet *TFB_GetNPCSet();               // 0x437b00 the NPC actors
Script::ReferenceSet *TFB_GetInactiveSet();          // 0x437be0 the parked actors
Script::ReferenceSet *TFB_GetDeadSet();              // 0x437ce0 the dead actors
Script::ReferenceSet *TFB_GetActiveParticleSet();    // 0x43b730
Script::ReferenceSet *TFB_GetInactiveParticleSet();  // 0x43b790
Script::ReferenceSet *TFB_GetRemovedParticleSet();   // 0x43b7f0

#endif
