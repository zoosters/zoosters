// Script::ScriptLight (PS2 RTTI; base Placement), the "light" script object of a CFXColorLight, with the member classes
// registered for it (ExposeLightColor with R / G / B / A children). No data: the game only builds its prototype (TFBType_Light
// 0x43c490, a bare 0x40-byte Placement, vtable 0x5caf90: 0 GetName 0x407a20, 4 GetType 0x407a30, 5 GetBaseType 0x4039e0, 7
// GetMembers 0x407a40). The game creates RWS::FX::GameLight (modules/fx/gamelight.h) instead; it is not derived.
#ifndef ZOOSTERS_SCRIPT_TYPES_SCRIPTLIGHT_H
#define ZOOSTERS_SCRIPT_TYPES_SCRIPTLIGHT_H

#include "zoosters/base.h"
#include "script/actor.h"

namespace Script {

// GLOBAL: 0x5caf90 Script::ScriptLight::vftable
class ScriptLight : public Placement { // "light"
public:
	const char *GetName();          // 0: 0x407a20 ("light")
	ScriptReference *GetType();     // 4: 0x407a30
	ScriptReference *GetBaseType(); // 5: 0x4039e0
	void *GetMembers(int *count);   // 7: 0x407a40
	// GLOBAL: 0x5cb038 Script::ScriptLight::ExposeLightColor::vftable
	class ExposeLightColor : public Measurement { // "light color", vtable 0x5cb038
	public:
		void *GetMembers(int *count);   // 7: 0x407af0
		const char *GetName();                            // 0: 0x407ae0
		int GetInt(ScriptReference *owner);               // 13: 0x4075b0
		void SetValue(int value, ScriptReference *owner); // 16: 0x407a00
		// GLOBAL: 0x5cb160 Script::ScriptLight::ExposeLightColor::ExposeR::vftable
		class ExposeR : public Measurement { // "red", vtable 0x5cb160
		public:
			int GetInt(ScriptReference *owner);               // 13: 0x407570
			void SetValue(int value, ScriptReference *owner); // 16: 0x407980
			const char *GetName();                                  // 0: 0x436920
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5cb118 Script::ScriptLight::ExposeLightColor::ExposeG::vftable
		class ExposeG : public Measurement { // "green", vtable 0x5cb118
		public:
			int GetInt(ScriptReference *owner);               // 13: 0x407580
			void SetValue(int value, ScriptReference *owner); // 16: 0x4079a0
			const char *GetName();                                  // 0: 0x436930
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5cb0d0 Script::ScriptLight::ExposeLightColor::ExposeB::vftable
		class ExposeB : public Measurement { // "blue", vtable 0x5cb0d0
		public:
			int GetInt(ScriptReference *owner);               // 13: 0x407590
			void SetValue(int value, ScriptReference *owner); // 16: 0x4079c0
			const char *GetName();                                  // 0: 0x436940
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5cb088 Script::ScriptLight::ExposeLightColor::ExposeA::vftable
		class ExposeA : public Measurement { // "alpha", vtable 0x5cb088
		public:
			int GetInt(ScriptReference *owner);               // 13: 0x4075a0
			void SetValue(int value, ScriptReference *owner); // 16: 0x4079e0
			const char *GetName();                                  // 0: 0x436950
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		ScriptReference *GetType();                             // 4: 0x407aa0
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x407ad0
	};
};
ZOOSTERS_ASSERT_SIZE(ScriptLight, 0x40);   // = Placement: the game build's LightBase mirror (m_04, m_08, m_20, m_3c) has the same 0x40 bytes
} // namespace Script

#endif
