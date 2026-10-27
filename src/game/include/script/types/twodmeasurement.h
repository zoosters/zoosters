// The script's 2D value type ("TwoD", vtable 0x5cc328, constructor 0x4164f0): two 16-bit components X and Y. A TwoD variable keeps its
// value inline in the user variable's data word (UserReference +0x1c); the members X / Y read and write the two halves.
#ifndef ZOOSTERS_SCRIPT_TYPES_TWODMEASUREMENT_H
#define ZOOSTERS_SCRIPT_TYPES_TWODMEASUREMENT_H

#include "script/value.h"

namespace Script {

class TwoDMeasurement : public Measurement {
public:
	TwoDMeasurement();                                    // 0x4164f0
	const char *GetName();                                // 0x416500
	ScriptReference *GetType();                           // 0x416400
	ScriptReference *GetBaseType();
	void *GetMembers(int *count);
	int GetMax();
	int GetValueType();
	int GetInt(ScriptReference *owner);
	float GetFloat(ScriptReference *owner);
	void SetValue(float value, ScriptReference *owner);
	void SetValue(int value, ScriptReference *owner);
	// GLOBAL: 0x5cc4f8 Script::TwoDMeasurement::ExposeX::vftable
	class ExposeX : public Measurement {
	public:
		const char *GetName();                            // 0x4166e0
		int GetInt(ScriptReference *owner);               // 0x43e2a0
		void SetValue(int value, ScriptReference *owner); // 0x43e2b0
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43e6f0
		float GetFloat(ScriptReference *owner);                 // 14: 0x416630
	};
	// GLOBAL: 0x5cc4b0 Script::TwoDMeasurement::ExposeY::vftable
	class ExposeY : public Measurement {
	public:
		const char *GetName();                            // 0x4166f0
		int GetInt(ScriptReference *owner);               // 0x43e2c0
		void SetValue(int value, ScriptReference *owner); // 0x43e2d0
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43e6f0
		float GetFloat(ScriptReference *owner);                 // 14: 0x416630
	};
};

}  // namespace Script

Script::ScriptReference *TFBType_TwoD();   // TwoDMeasurement prototype getter

#endif
