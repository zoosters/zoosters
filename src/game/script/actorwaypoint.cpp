// Script::ActorWayPoint (waypoint objects of an actor): constructor, compiler-made vector deleting destructor, names, index and position accessors.
#include "script/types/actorwaypoint.h"

ZOOSTERS_NOINLINE Script::ScriptReference *TFBType_WayPoint();

namespace Script {

// GLOBAL: 0x5cd520 Script::ActorWayPoint::vftable
// GLOBAL: 0x5cd598 Script::ActorWayPoint::ExposeIndex::vftable
// 0x42d480

// 0x42cae0
// (compiler-made vector deleting destructor, emitted because the class is array-new'ed in this TU)
ActorWayPoint *g_keepWayPointArray(int n)
{
	return new ActorWayPoint[n];
}

// 0x42d490
const char *ActorWayPoint::GetName()
{
	return "waypoint";
}

// 0x42d4b0
Vec3 *ActorWayPoint::GetVector(ScriptReference *owner)
{
	return &static_cast<ActorWayPoint *>(owner)->m_pos;
}

// 0x42d4c0
void *ActorWayPoint::GetMembers(int *count)
{
	static ExposeIndex s;
	*count = 1;
	return &s;
}

// 0x42d520
const char *ActorWayPoint::ExposeIndex::GetName()
{
	return "index";
}

// 0x42d530
int ActorWayPoint::ExposeIndex::GetInt(ScriptReference *owner)
{
	return static_cast<IndexedPosition *>(owner)->m_index;   // the one "index" member of waypoints, attachment points and turrets (all IndexedPosition)
}

}  // namespace Script

// 0x42d4a0
Script::ScriptReference *ActorWayPoint_GetType()
{
	return TFBType_WayPoint();
}
