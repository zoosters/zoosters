// Script::ScriptMotionBlur (PS2 RTTI; base ScriptReference), the "MOTION BLUR" script object, and the Expose* members of the
// screen motion-blur effect (amount, offset, scale, iso scale; they read and write g_motionBlur, modules/fx/cfxmotionblur.h).
// No data members: the game only builds its prototype (TFBType_MotionBlur 0x43f760). Vtable 0x5d386c: slot 6 is 0x43e640
// (Script::SysFamilySlot6, returns -1), slot 7 GetMembers is 0x43f470.
#ifndef ZOOSTERS_SCRIPT_TYPES_SCRIPTMOTIONBLUR_H
#define ZOOSTERS_SCRIPT_TYPES_SCRIPTMOTIONBLUR_H

#include "zoosters/base.h"
#include "script/value.h"

namespace Script {

// GLOBAL: 0x5d386c Script::ScriptMotionBlur::vftable
class ScriptMotionBlur : public ScriptReference { // "MOTION BLUR"
public:
	void *GetMembers(int *count);   // 7: 0x43f470
	const char *GetName();                    // 0: 0x43e680
	ScriptReference *GetType();               // 4: 0x43f870
	int GetKind();                               // 6: 0x43e640 (Script::SysFamilySlot6, returns -1; bound in script/normal_bringup.cpp)
	// GLOBAL: 0x5d4a68 Script::ScriptMotionBlur::ExposeScaleX::vftable
	class ExposeScaleX : public Measurement { // "scale x-axis", vtable 0x5d4a68
	public:
		const char *GetName();                              // 0: 0x43f5b0
		ScriptReference *GetType();                         // 4: 0x436a50
		ScriptReference *GetBaseType();                     // 5: 0x402b70
		int GetMax();                                       // 11: 0x43e6f0
		int GetValueType();                                 // 12: 0x4034a0
		float GetFloat(ScriptReference *owner);             // 14: 0x410d00
		void SetValue(float value, ScriptReference *owner); // 15: 0x410d20
	};
	// GLOBAL: 0x5d4ab0 Script::ScriptMotionBlur::ExposeOffsetY::vftable
	class ExposeOffsetY : public Measurement { // "offset-y", vtable 0x5d4ab0
	public:
		const char *GetName();                              // 0: 0x43f5a0
		ScriptReference *GetType();                         // 4: 0x436a50
		ScriptReference *GetBaseType();                     // 5: 0x402b70
		int GetMax();                                       // 11: 0x43e6f0
		int GetValueType();                                 // 12: 0x4034a0
		float GetFloat(ScriptReference *owner);             // 14: 0x410cc0
		void SetValue(float value, ScriptReference *owner); // 15: 0x410ce0
	};
	// GLOBAL: 0x5d4af8 Script::ScriptMotionBlur::ExposeOffsetX::vftable
	class ExposeOffsetX : public Measurement { // "offset-x", vtable 0x5d4af8
	public:
		const char *GetName();                              // 0: 0x43f590
		ScriptReference *GetType();                         // 4: 0x436a50
		ScriptReference *GetBaseType();                     // 5: 0x402b70
		int GetMax();                                       // 11: 0x43e6f0
		int GetValueType();                                 // 12: 0x4034a0
		float GetFloat(ScriptReference *owner);             // 14: 0x410c80
		void SetValue(float value, ScriptReference *owner); // 15: 0x410ca0
	};
	// GLOBAL: 0x5d4b40 Script::ScriptMotionBlur::ExposeAmount::vftable
	class ExposeAmount : public Measurement { // "amount", vtable 0x5d4b40
	public:
		const char *GetName();                              // 0: 0x43f580
		ScriptReference *GetType();                         // 4: 0x436a50
		ScriptReference *GetBaseType();                     // 5: 0x402b70
		int GetMax();                                       // 11: 0x43efe0
		int GetValueType();                                 // 12: 0x4034a0
		float GetFloat(ScriptReference *owner);             // 14: 0x410c40
		void SetValue(float value, ScriptReference *owner); // 15: 0x410c60
	};
	// GLOBAL: 0x5d4bb8 Script::ScriptMotionBlur::ExposeScaleY::vftable
	class ExposeScaleY : public Measurement { // "scale y-axis", vtable 0x5d4bb8
	public:
		ExposeScaleY();                                     // 0x43f5c0
		const char *GetName();                              // 0: 0x43f5d0
		ScriptReference *GetType();                         // 4: 0x436a50
		ScriptReference *GetBaseType();                     // 5: 0x402b70
		int GetMax();                                       // 11: 0x43e6f0
		int GetValueType();                                 // 12: 0x4034a0
		float GetFloat(ScriptReference *owner);             // 14: 0x410d40
		void SetValue(float value, ScriptReference *owner); // 15: 0x410d60
	};
	// GLOBAL: 0x5d4c10 Script::ScriptMotionBlur::ExposeIsoScale::vftable
	class ExposeIsoScale : public Measurement { // vtable 0x5d4c10
	public:
		ExposeIsoScale();                                   // 0x43f5e0
		const char *GetName();                              // 0: 0x43f5f0
		ScriptReference *GetType();                         // 4: 0x436a50
		ScriptReference *GetBaseType();                     // 5: 0x402b70
		int GetMax();                                       // 11: 0x43e6f0
		int GetValueType();                                 // 12: 0x4034a0
		float GetFloat(ScriptReference *owner);             // 14: 0x410d80
		void SetValue(float value, ScriptReference *owner); // 15: 0x410da0
	};
	// GLOBAL: 0x5d4c58 Script::ScriptMotionBlur::ExposeField40::vftable
	class ExposeField40 : public Measurement { // vtable 0x5d4c58
	public:
		ExposeField40();                                    // 0x43f600
		const char *GetName();                              // 0: 0x416660
		ScriptReference *GetType();                         // 4: 0x403e90
		ScriptReference *GetBaseType();                     // 5: 0x436a50
		int GetMax();                                       // 11: 0x43e6f0
		int GetValueType();                                 // 12: 0x4034a0
		float GetFloat(ScriptReference *owner);             // 14: 0x410dc0
		void SetValue(float value, ScriptReference *owner); // 15: 0x410de0
	};
};
ZOOSTERS_ASSERT_SIZE(ScriptMotionBlur, 4);

} // namespace Script

#endif
