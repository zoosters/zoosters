// Script::AngleMeasurement (PS2 RTTI; vtable 0x5cc548): the angle value, a Measurement; only its constructor, name and accessors so far.
#ifndef ZOOSTERS_SCRIPT_TYPES_ANGLEMEASUREMENT_H
#define ZOOSTERS_SCRIPT_TYPES_ANGLEMEASUREMENT_H

#include "zoosters/base.h"
#include "script/value.h"

namespace Script {

class AngleMeasurement : public Measurement {
public:
	AngleMeasurement();                         // 0x416650
	const char *GetName();                      // 0: 0x416660
	ScriptReference *GetType();
	ScriptReference *GetBaseType();
	int GetMax();
	int GetValueType();
	int GetInt(ScriptReference *owner);
	float GetFloat(ScriptReference *owner);
	void SetValue(float value, ScriptReference *owner);
	void SetValue(int value, ScriptReference *owner);
};

}  // namespace Script

#endif
