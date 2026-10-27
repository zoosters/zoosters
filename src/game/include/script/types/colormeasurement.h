// Script::ColorMeasurement (PS2 RTTI; vtable 0x5cc370): the colour value, 4 bytes at +0x1c of the object itself; its members R, G, B, A
// are byte accessors (the member objects come from GetMembers 0x416880). The class has other members; only these are declared.
#ifndef ZOOSTERS_SCRIPT_TYPES_COLORMEASUREMENT_H
#define ZOOSTERS_SCRIPT_TYPES_COLORMEASUREMENT_H

#include "zoosters/base.h"
#include "script/value.h"

namespace Script {

class ColorMeasurement : public Measurement {
public:
	ColorMeasurement();
	const char *GetName();                     // 0: 0x416520
	ScriptReference *GetType();                // 4: thunk to TFBType_Color
	ScriptReference *GetBaseType();
	void *GetMembers(int *count);              // 7: 0x416880
	int GetMax();
	int GetValueType();
	int GetInt(ScriptReference *owner);
	float GetFloat(ScriptReference *owner);
	void SetValue(float value, ScriptReference *owner);
	void SetValue(int value, ScriptReference *owner);
	char m_04[0x18];
	unsigned char m_r, m_g, m_b, m_a;
	// GLOBAL: 0x5cc950 Script::ColorMeasurement::ExposeR::vftable
	class ExposeR : public Measurement {
	public:
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		const char *GetName();                                  // 0: 0x436920
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43b4e0
	};
	// GLOBAL: 0x5cc908 Script::ColorMeasurement::ExposeG::vftable
	class ExposeG : public Measurement {
	public:
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		const char *GetName();                                  // 0: 0x436930
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43b4e0
	};
	// GLOBAL: 0x5cc8c0 Script::ColorMeasurement::ExposeB::vftable
	class ExposeB : public Measurement {
	public:
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		const char *GetName();                                  // 0: 0x436940
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43b4e0
	};
	// GLOBAL: 0x5cc878 Script::ColorMeasurement::ExposeA::vftable
	class ExposeA : public Measurement {
	public:
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		const char *GetName();                                  // 0: 0x436950
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43b4e0
	};
};
ZOOSTERS_ASSERT_SIZE(ColorMeasurement, 0x20);
ZOOSTERS_ASSERT_OFFSET(ColorMeasurement, m_r, 0x1c);

}  // namespace Script

#endif
