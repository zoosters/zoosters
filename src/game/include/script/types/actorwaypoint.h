// Script::ActorWayPoint (PS2 RTTI; vtable 0x5cd520): a waypoint object of an actor, a position with an index. The type prototype
// (TFBType_WayPoint 0x436a60) is an instance whose index is -1 (the constructor 0x42d480).
#ifndef ZOOSTERS_SCRIPT_TYPES_ACTORWAYPOINT_H
#define ZOOSTERS_SCRIPT_TYPES_ACTORWAYPOINT_H

#include "zoosters/base.h"
#include "script/actor.h"
#include "script/types/indexedposition.h"

void *MemAlloc(size_t n);               // 0x550170

namespace Script {

class ActorWayPoint : public IndexedPosition {
public:
	ActorWayPoint() { m_index = -1; }    // 0x42d480 (inline; the new[] in actorwaypoint.cpp emits the out-of-line copy)
	const char *GetName();               // 0: 0x42d490 "waypoint"
	ScriptReference *GetType();          // 4: 0x42d4a0
	ScriptReference *GetBaseType();      // 5: 0x4039f0
	int GetKind();                       // 6: 0x43e6f0 (returns 0)
	void *GetMembers(int *count);        // 7: 0x42d4c0
	Vec3 *GetVector(ScriptReference *owner);    // 17: 0x42d4b0
	static void *operator new[](size_t n) { return MemAlloc(n); }   // the waypoint array of an actor (CProtoActor::HandleAttributes)

	// GLOBAL: 0x5cd598 Script::ActorWayPoint::ExposeIndex::vftable
	class ExposeIndex : public Measurement {   // "index", vtable 0x5cd598
	public:
		const char *GetName();           // 0: 0x42d520
		ScriptReference *GetType();      // 4: 0x436a50
		ScriptReference *GetBaseType();  // 5: 0x402b70
		int GetKind();                   // 6: 0x43e6f0
		int GetInt(ScriptReference *owner);    // 13: 0x42d530
		void SetValue(int value, ScriptReference *owner);    // 16: 0x4025a0
	};
};
ZOOSTERS_ASSERT_SIZE(ActorWayPoint, 0x14);
ZOOSTERS_ASSERT_OFFSET(ActorWayPoint, m_index, 4);
ZOOSTERS_ASSERT_OFFSET(ActorWayPoint, m_pos, 8);

}  // namespace Script

#endif
