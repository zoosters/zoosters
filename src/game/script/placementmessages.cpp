// Placement-removed message prototypes 0x42f370 / 0x42f3f0: function-local static UserReference.
#include "script/ops_message.h"
#include "script/types/scriptsystem.h"
using namespace Script;

namespace Script {
// vtable 0x5cc468 (see expose_ctors_1.h); the constructor is inline here, like in the game
class BoolMeasurementInl : public Measurement {
public:
	BoolMeasurementInl() {}
	BoolMeasurementInl *Self() { return this; }
	ScriptReference *GetType();
	ScriptReference *GetBaseType();
	int GetMax();
	int GetInt(ScriptReference *owner);
	float GetFloat(ScriptReference *owner);
	void SetValue(float value, ScriptReference *owner);
	void SetValue(int value, ScriptReference *owner);
};
}

// GLOBAL: 0x5cc468 Script::BoolMeasurementInl::vftable
// 0x42f370
ScriptReference *TFBUser_GlobalPlacementRemoved()
{
	static UserReference s("Global Placement Removed", BoolMeasurementInl().Self(), (intptr_t)TFBType_System(), 0x3200);
	return &s;
}

// 0x42f3f0
ScriptReference *TFBUser_LevelPlacementRemoved()
{
	static UserReference s("Level Placement Removed", BoolMeasurementInl().Self(), (intptr_t)TFBType_System(), 0x320);
	return &s;
}

#include "script/folded.h"
// FOLDED: 0x4025a0 Script::BoolMeasurementInl::SetValue
void Script::BoolMeasurementInl::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x402b70 Script::BoolMeasurementInl::GetBaseType
Script::ScriptReference * Script::BoolMeasurementInl::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x402b80 Script::BoolMeasurementInl::GetFloat
float Script::BoolMeasurementInl::GetFloat(Script::ScriptReference * a1) { return Script::Measurement::GetFloat(a1); }
// FOLDED: 0x407ab0 Script::BoolMeasurementInl::SetValue
void Script::BoolMeasurementInl::SetValue(float a1, Script::ScriptReference * a2) { Script::Measurement::SetValue(a1, a2); }
// FOLDED: 0x416530 Script::BoolMeasurementInl::GetInt
int Script::BoolMeasurementInl::GetInt(Script::ScriptReference * a1) { return Fold_Return0_Arg1(a1); }
// FOLDED: 0x436a50 Script::BoolMeasurementInl::GetType
Script::ScriptReference * Script::BoolMeasurementInl::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43efe0 Script::BoolMeasurementInl::GetMax
int Script::BoolMeasurementInl::GetMax() { return Script_AlwaysTrue(); }
