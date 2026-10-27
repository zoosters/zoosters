// Prototype getters (static T s; return &s;).
#include "script/actor.h"
using namespace Script;

// GLOBAL: 0x5ca6b8 Script::VectorMeasurement::vftable
// 0x42fe70
ScriptReference *TFBType_Vector()
{
	static VectorMeasurement s;
	return &s;
}

// GLOBAL: 0x5ca760 Script::PositionMeasurement::vftable
// 0x430300
ScriptReference *TFBType_Position()
{
	static PositionMeasurement s;
	return &s;
}
