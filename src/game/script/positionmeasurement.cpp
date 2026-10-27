#include <math.h>
#include "script/actor.h"

extern Script::ScriptReference *g_scriptSelf;     // 0x621f14
extern Script::ScriptReference *g_scriptOwner;    // 0x621f18
#ifdef ZOOSTERS_DEBUGUI   // debug overlay only (port/debugui/debugui_triggers.cpp)
namespace Script { class PositionMeasurement; }
void port_dbg_position_measure(Script::PositionMeasurement *m, Script::ScriptReference *owner, Script::ScriptReference *self,
                               Script::ScriptReference *selfOwner, float distance);
#endif
// 0x409700 is thiscall (mov esi,ecx; this->GetPosition(ownerA) vs b->GetPosition(ownerB)): game name PlayerObj::Dist
struct DistSelf { float FUN_00409700(Script::ScriptReference *ownerA, Script::ScriptReference *b, Script::ScriptReference *ownerB); };

namespace Script {

// 0x42f770
float PositionMeasurement::GetFloat(ScriptReference *owner)
{
	ScriptReference *selfOwner = g_scriptOwner;
	ScriptReference *self = g_scriptSelf;
#ifdef ZOOSTERS_DEBUGUI
	float distance = (float)sqrt((double)((DistSelf *)this)->FUN_00409700(owner, self, selfOwner)) * 0.083333336f;
	port_dbg_position_measure(this, owner, self, selfOwner, distance);
	return distance;
#else
	return (float)sqrt((double)((DistSelf *)this)->FUN_00409700(owner, self, selfOwner)) * 0.083333336f;
#endif
}

}  // namespace Script

inline float DistSelf::FUN_00409700(Script::ScriptReference *ownerA, Script::ScriptReference *b, Script::ScriptReference *ownerB)
{
	return ((Script::PositionMeasurement *)this)->Dist(ownerA, (Script::PositionMeasurement *)b, ownerB);
}
