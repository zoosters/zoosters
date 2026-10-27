// Script::ScriptMathOps (PS2 RTTI; base ScriptReference), the "MATH OPS" script object, and its one member ExposeNormalizeAngle
// ("Normalized Angle": setting it wraps an angle in degrees into -180..180 and keeps it in g_normalizedAngle 0x6221ac, reading
// returns that value). No data members: the game only builds its prototype (TFBType_MathOps 0x43f610). Vtable 0x5d3834: slot 6
// is 0x43e640 (Script::SysFamilySlot6, returns -1), slot 7 GetMembers is 0x43f3a0.
#ifndef ZOOSTERS_SCRIPT_TYPES_SCRIPTMATHOPS_H
#define ZOOSTERS_SCRIPT_TYPES_SCRIPTMATHOPS_H

#include "zoosters/base.h"
#include "script/value.h"

namespace Script {

// GLOBAL: 0x5d3834 Script::ScriptMathOps::vftable
class ScriptMathOps : public ScriptReference { // "MATH OPS"
public:
	void *GetMembers(int *count);   // 7: 0x43f3a0
	const char *GetName();                       // 0: 0x43e670
	ScriptReference *GetType();                  // 4: 0x43f860 (returns TFBType_MathOps)
	int GetKind();                               // 6: 0x43e640 (Script::SysFamilySlot6, returns -1; bound in script/normal_bringup.cpp)
	// GLOBAL: 0x5d4a10 Script::ScriptMathOps::ExposeNormalizeAngle::vftable
	class ExposeNormalizeAngle : public Measurement { // "Normalized Angle", vtable 0x5d4a10
	public:
		const char *GetName();                              // 0: 0x43f400
		float GetFloat(ScriptReference *owner);             // 14: 0x43f410
		void SetValue(float value, ScriptReference *owner); // 15: 0x43f420
		ScriptReference *GetType();                             // 4: 0x403e90
		ScriptReference *GetBaseType();                         // 5: 0x436a50
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
	};
};
ZOOSTERS_ASSERT_SIZE(ScriptMathOps, 4);

} // namespace Script

extern float g_normalizedAngle;                  // 0x6221ac

#endif
