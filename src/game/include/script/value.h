// Script value objects: what a reference resolves to when an op reads or writes a number (members of actors,
// variables, ...). Hierarchy from the PS2 RTTI: ScriptReference > ValueInterface > Measurement > IntMeasurement
// > FloatMeasurement / AngleMeasurement / BoolMeasurement ... > VectorMeasurement > PositionMeasurement ...
#ifndef ZOOSTERS_SCRIPT_VALUE_H
#define ZOOSTERS_SCRIPT_VALUE_H

#include "script/script.h"

namespace Script {

// vtable 0x5ca5c8 (13 slots)
class ValueInterface : public ScriptReference {
public:
	const char *GetName();       // 0: 0x402b00
	ScriptReference *GetType();  // 4: 0x402b70
	virtual int GetMax();        // 11: default 0x402b10 returns 0x7fffffff; 0 = no limit (ValueInc)
	virtual int GetValueType();  // 12: 0 int, 0x10 float
	bool CheckParsedType();         // calls GetValueType, returns false (ValueInc/Dec::Parse call it on the target type)
};

// Slots 13-16 (owner = the object the value is a member of, from ScriptRef::Resolve). The PS2 build has the two
// setters in the other order (SetInt 15, SetFloat 16): an overloaded SetValue pair, which MSVC lays out in reverse
// declaration order.
// NOVTABLE: Script::Measurement
class Measurement : public ValueInterface {
public:
	virtual int GetInt(ScriptReference *owner);                   // 13: default 0x403460 = (int)GetFloat(owner)
	virtual float GetFloat(ScriptReference *owner);               // 14
	virtual void SetValue(int value, ScriptReference *owner);     // 16: default 0x403480 = SetValue((float)value)
	virtual void SetValue(float value, ScriptReference *owner);   // 15
};

// The "generic value" member (vtable 0x5cdf20, TFBMember_GenericValue 0x430aa0): a value of any type, set and read as an int.
class ExposeGenericValue : public Measurement {
public:
	const char *GetName();                              // 0: 0x430130 "generic value"
	ScriptReference *GetType();                         // 4: 0x436a50
	ScriptReference *GetBaseType();                     // 5: 0x402b70
	int GetKind();                                      // 6: 0x43e6f0
	int GetInt(ScriptReference *owner);                 // 13: 0x416530
	void SetValue(int value, ScriptReference *owner);   // 16: 0x4025a0
};

// vtable 0x5cc468, GetName "value" (inherited): class_map_ambiguous lists it as BoolMeasurement (max 1 at slot 11)
class BoolMeasurement : public Measurement {
public:
	BoolMeasurement();
	ScriptReference *GetType();
	ScriptReference *GetBaseType();
	int GetMax();
	int GetInt(ScriptReference *owner);
	float GetFloat(ScriptReference *owner);
	void SetValue(float value, ScriptReference *owner);
	void SetValue(int value, ScriptReference *owner);
};

}  // namespace Script

#endif
