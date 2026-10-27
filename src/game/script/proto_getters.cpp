// Prototype getters (static T s; return &s;): they need the real virtual destructor, so ZOOSTERS_REAL_DTOR comes first.
#include "script/ops_object.h"
#include "script/value.h"

namespace Script {
class IntMeasurement : public Measurement {
public:
	const char *GetName();
	ScriptReference *GetType();
	ScriptReference *GetBaseType();
	int GetInt(ScriptReference *owner);
	float GetFloat(ScriptReference *owner);
	void SetValue(int value, ScriptReference *owner);
	void SetValue(float value, ScriptReference *owner);
};
}  // namespace Script
using namespace Script;

// GLOBAL: 0x5ca670 Script::IntMeasurement::vftable
// 0x42fdb0
ScriptReference *TFBType_Int()
{
	static IntMeasurement s;
	return &s;
}

// GLOBAL: 0x5ce1fc Script::ObjectControl::vftable
// 0x431200
ScriptReference *OpProto_ObjectControl()
{
	static ObjectControl s;
	return &s;
}

#include "script/folded.h"
// FOLDED: 0x4025a0 Script::IntMeasurement::SetValue
void Script::IntMeasurement::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x402b00 Script::IntMeasurement::GetName
char const * Script::IntMeasurement::GetName() { return ValueInterface_GetName(); }
// FOLDED: 0x402b70 Script::IntMeasurement::GetBaseType
Script::ScriptReference * Script::IntMeasurement::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x402b80 Script::IntMeasurement::GetFloat
float Script::IntMeasurement::GetFloat(Script::ScriptReference * a1) { return Script::Measurement::GetFloat(a1); }
// FOLDED: 0x407ab0 Script::IntMeasurement::SetValue
void Script::IntMeasurement::SetValue(float a1, Script::ScriptReference * a2) { Script::Measurement::SetValue(a1, a2); }
// FOLDED: 0x416530 Script::IntMeasurement::GetInt
int Script::IntMeasurement::GetInt(Script::ScriptReference * a1) { return Fold_Return0_Arg1(a1); }
// FOLDED: 0x436a50 Script::IntMeasurement::GetType
Script::ScriptReference * Script::IntMeasurement::GetType() { return IntMeasurement_GetType(); }
