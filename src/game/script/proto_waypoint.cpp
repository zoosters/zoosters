#include "script/types/actorwaypoint.h"
using namespace Script;

// GLOBAL: 0x5cd520 Script::ActorWayPoint::vftable
// 0x436a60
ScriptReference *TFBType_WayPoint()
{
	static ActorWayPoint s;
	return &s;
}
