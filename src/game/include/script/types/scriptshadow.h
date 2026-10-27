// Script::ScriptShadow (PS2 RTTI; base ScriptReference), the "SHADOW" script object, and the Expose* members of the blob-shadow
// settings (area radius, pitch, heading, vertical offset, softness, fading). No data members: the game only builds its
// prototype (TFBType_Shadow 0x43f680); the members read or write the shadow camera (g_shadowCamera). Vtable 0x5d37d0: slot 6 is
// 0x43e640 (Script::SysFamilySlot6, returns -1), slot 7 GetMembers is 0x43eea0.
#ifndef ZOOSTERS_SCRIPT_TYPES_SCRIPTSHADOW_H
#define ZOOSTERS_SCRIPT_TYPES_SCRIPTSHADOW_H

#include "zoosters/base.h"
#include "script/value.h"

namespace Script {

// GLOBAL: 0x5d37d0 Script::ScriptShadow::vftable
class ScriptShadow : public ScriptReference {
public:
	void *GetMembers(int *count);   // 7: 0x43eea0
	const char *GetName();                      // 0: 0x43e650
	ScriptReference *GetType();                 // 4: 0x43f840
	int GetKind();                               // 6: 0x43e640 (Script::SysFamilySlot6, returns -1; bound in script/normal_bringup.cpp)
	// GLOBAL: 0x5d41f0 Script::ScriptShadow::ExposeSoftness::vftable
	class ExposeSoftness : public Measurement { // "softness", vtable 0x5d41f0
	public:
		const char *GetName();                              // 0: 0x43efd0
		float GetFloat(ScriptReference *owner);             // 14: 0x442a70
		void SetValue(float value, ScriptReference *owner); // 15: 0x442a90
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43efe0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d4238 Script::ScriptShadow::ExposeVerticalOffset::vftable
	class ExposeVerticalOffset : public Measurement { // "vertical offset", vtable 0x5d4238
	public:
		const char *GetName();                              // 0: 0x43efc0
		float GetFloat(ScriptReference *owner);             // 14: 0x442a30
		void SetValue(float value, ScriptReference *owner); // 15: 0x442a50
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d4280 Script::ScriptShadow::ExposeHeading::vftable
	class ExposeHeading : public Measurement { // vtable 0x5d4280
	public:
		float GetFloat(ScriptReference *owner);             // 14: 0x4429f0
		void SetValue(float value, ScriptReference *owner); // 15: 0x442a10
		char const *GetName();                                  // 0: 0x403ea0
		ScriptReference *GetType();                             // 4: 0x403e90
		ScriptReference *GetBaseType();                         // 5: 0x436a50
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d42c8 Script::ScriptShadow::ExposePitch::vftable
	class ExposePitch : public Measurement { // vtable 0x5d42c8
	public:
		float GetFloat(ScriptReference *owner);             // 14: 0x4429b0
		void SetValue(float value, ScriptReference *owner); // 15: 0x4429d0
		char const *GetName();                                  // 0: 0x43dbf0
		ScriptReference *GetType();                             // 4: 0x403e90
		ScriptReference *GetBaseType();                         // 5: 0x436a50
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d4310 Script::ScriptShadow::ExposeAreaRadius::vftable
	class ExposeAreaRadius : public Measurement { // "area radius", vtable 0x5d4310
	public:
		const char *GetName();                              // 0: 0x43efb0
		float GetFloat(ScriptReference *owner);             // 14: 0x442970
		void SetValue(float value, ScriptReference *owner); // 15: 0x442990
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	class ExposeEnableFading : public Measurement { // "fading enabled", vtable 0x5d4380
	public:
		ExposeEnableFading();  // 0x43eff0
		const char *GetName(); // 0: 0x43f000
		ScriptReference *GetType();
		ScriptReference *GetBaseType();
		int GetMax();
		int GetInt(ScriptReference *owner);               // 13: 0x442ab0
		void SetValue(int value, ScriptReference *owner); // 16: 0x442ad0
	};
	class ExposeFadeRadius : public Measurement { // "fade radius", vtable 0x5d43d8
	public:
		ExposeFadeRadius();    // 0x43f010
		const char *GetName(); // 0: 0x43f020
		ScriptReference *GetType();
		ScriptReference *GetBaseType();
		int GetMax();
		int GetValueType();
		float GetFloat(ScriptReference *owner);             // 14: 0x442af0
		void SetValue(float value, ScriptReference *owner); // 15: 0x442b10
	};
};
ZOOSTERS_ASSERT_SIZE(ScriptShadow, 4);

} // namespace Script

#endif
