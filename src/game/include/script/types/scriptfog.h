// Script::ScriptFog (PS2 RTTI; base ScriptReference), the "FOG" script object: Expose* members of the vertical fog, pixel fog,
// bloom and depth-of-field settings (colour members have R / G / B / A children). No data members: the game only builds its
// prototype (TFBType_Fog 0x43f6f0). Vtable 0x5d3804: slot 6 is 0x43e640 (Script::SysFamilySlot6, returns -1), slot 7 GetMembers
// is 0x43f030.
#ifndef ZOOSTERS_SCRIPT_TYPES_SCRIPTFOG_H
#define ZOOSTERS_SCRIPT_TYPES_SCRIPTFOG_H

#include "zoosters/base.h"
#include "script/value.h"

namespace Script {

// GLOBAL: 0x5d3804 Script::ScriptFog::vftable
class ScriptFog : public ScriptReference { // "FOG"
public:
	void *GetMembers(int *count);   // 7: 0x43f030
	const char *GetName();                            // 0: 0x43e660
	ScriptReference *GetType();                       // 4: 0x43f850
	int GetKind();                               // 6: 0x43e640 (Script::SysFamilySlot6, returns -1; bound in script/normal_bringup.cpp)
	// GLOBAL: 0x5d4428 Script::ScriptFog::ExposePixelFogEnable::vftable
	class ExposePixelFogEnable : public Measurement { // "pixel fog toggle", vtable 0x5d4428
	public:
		const char *GetName();                            // 0: 0x43f1c0
		int GetInt(ScriptReference *owner);               // 13: 0x41a830
		void SetValue(int value, ScriptReference *owner); // 16: 0x41a840
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43efe0
	};
	// GLOBAL: 0x5d4470 Script::ScriptFog::ExposePixelCutoffIntensity::vftable
	class ExposePixelCutoffIntensity : public Measurement { // "pixel fog/DOF cutoff intensity", vtable 0x5d4470
	public:
		const char *GetName();                            // 0: 0x43f1e0
		int GetInt(ScriptReference *owner);               // 13: 0x41a8b0
		void SetValue(int value, ScriptReference *owner); // 16: 0x41a8c0
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x42fbc0
	};
	// GLOBAL: 0x5d44b8 Script::ScriptFog::ExposePixelFogDistance::vftable
	class ExposePixelFogDistance : public Measurement { // "pixel fog/DOF distance", vtable 0x5d44b8
	public:
		const char *GetName();                              // 0: 0x43f1d0
		float GetFloat(ScriptReference *owner);             // 14: 0x41a880
		void SetValue(float value, ScriptReference *owner); // 15: 0x41a890
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d4500 Script::ScriptFog::ExposeVertFogColor::vftable
	class ExposeVertFogColor : public Measurement { // "vertex fog color", vtable 0x5d4500
	public:
		void *GetMembers(int *count);   // 7: 0x43f270
		const char *GetName();                            // 0: 0x43f1b0
		int GetInt(ScriptReference *owner);               // 13: 0x41a680
		void SetValue(int value, ScriptReference *owner); // 16: 0x41a6b0
		// GLOBAL: 0x5d4760 Script::ScriptFog::ExposeVertFogColor::ExposeA::vftable
		class ExposeA : public Measurement {              // vtable 0x5d4760
		public:
			int GetInt(ScriptReference *owner);               // 13: 0x41a7e0
			void SetValue(int value, ScriptReference *owner); // 16: 0x41a7f0
			const char *GetName();                                  // 0: 0x436950
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5d47a8 Script::ScriptFog::ExposeVertFogColor::ExposeB::vftable
		class ExposeB : public Measurement { // vtable 0x5d47a8
		public:
			int GetInt(ScriptReference *owner);               // 13: 0x41a790
			void SetValue(int value, ScriptReference *owner); // 16: 0x41a7a0
			const char *GetName();                                  // 0: 0x436940
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5d47f0 Script::ScriptFog::ExposeVertFogColor::ExposeG::vftable
		class ExposeG : public Measurement { // vtable 0x5d47f0
		public:
			int GetInt(ScriptReference *owner);               // 13: 0x41a740
			void SetValue(int value, ScriptReference *owner); // 16: 0x41a750
			const char *GetName();                                  // 0: 0x436930
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5d4838 Script::ScriptFog::ExposeVertFogColor::ExposeR::vftable
		class ExposeR : public Measurement { // vtable 0x5d4838
		public:
			int GetInt(ScriptReference *owner);               // 13: 0x41a6f0
			void SetValue(int value, ScriptReference *owner); // 16: 0x41a700
			const char *GetName();                                  // 0: 0x436920
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		ScriptReference *GetType();                             // 4: 0x407aa0
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x407ad0
	};
	// GLOBAL: 0x5d4548 Script::ScriptFog::ExposeVertFogDistance::vftable
	class ExposeVertFogDistance : public Measurement { // "vertex fog distance", vtable 0x5d4548
	public:
		const char *GetName();                              // 0: 0x43f1a0
		float GetFloat(ScriptReference *owner);             // 14: 0x41a650
		void SetValue(float value, ScriptReference *owner); // 15: 0x41a660
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	class ExposePixelFogColor : public Measurement { // "pixel fog/DOF color", vtable 0x5d4600
	public:
		ExposePixelFogColor();                            // 0x43f1f0
		const char *GetName();                            // 0: 0x43f200
		ScriptReference *GetType();                       // 4: 0x407aa0
		ScriptReference *GetBaseType();                   // 5: 0x402b70
		void *GetMembers(int *count);                     // 7: 0x43f2f0
		int GetMax();                                     // 11: 0x43e6f0
		int GetValueType();                               // 12: 0x407ad0
		int GetInt(ScriptReference *owner);               // 13: 0x41a8d0
		void SetValue(int value, ScriptReference *owner); // 16: 0x41a900
		// GLOBAL: 0x5d4880 Script::ScriptFog::ExposePixelFogColor::ExposeA::vftable
		class ExposeA : public Measurement {              // "far plane alpha", vtable 0x5d4880
		public:
			const char *GetName();                            // 0: 0x43f370
			int GetInt(ScriptReference *owner);               // 13: 0x41a990
			void SetValue(int value, ScriptReference *owner); // 16: 0x41a9a0
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5d48c8 Script::ScriptFog::ExposePixelFogColor::ExposeB::vftable
		class ExposeB : public Measurement { // vtable 0x5d48c8
		public:
			int GetInt(ScriptReference *owner);               // 13: 0x41a970
			void SetValue(int value, ScriptReference *owner); // 16: 0x41a980
			const char *GetName();                                  // 0: 0x436940
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5d4910 Script::ScriptFog::ExposePixelFogColor::ExposeG::vftable
		class ExposeG : public Measurement { // vtable 0x5d4910
		public:
			int GetInt(ScriptReference *owner);               // 13: 0x41a950
			void SetValue(int value, ScriptReference *owner); // 16: 0x41a960
			const char *GetName();                                  // 0: 0x436930
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5d4958 Script::ScriptFog::ExposePixelFogColor::ExposeR::vftable
		class ExposeR : public Measurement { // vtable 0x5d4958
		public:
			int GetInt(ScriptReference *owner);               // 13: 0x41a930
			void SetValue(int value, ScriptReference *owner); // 16: 0x41a940
			const char *GetName();                                  // 0: 0x436920
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
	};
	class ExposeBloomEnable : public Measurement { // "bloom toggle", vtable 0x5d4658
	public:
		ExposeBloomEnable();                              // 0x43f210
		const char *GetName();                            // 0: 0x43f220
		ScriptReference *GetType();                       // 4: 0x436a50
		ScriptReference *GetBaseType();                   // 5: 0x402b70
		int GetMax();                                     // 11: 0x43efe0
		int GetInt(ScriptReference *owner);               // 13: 0x41a9b0
		void SetValue(int value, ScriptReference *owner); // 16: 0x41a9c0
	};
	class ExposeBloomAddFactor : public Measurement { // "bloom add factor", vtable 0x5d46b0
	public:
		ExposeBloomAddFactor();                             // 0x43f230
		const char *GetName();                              // 0: 0x43f240
		ScriptReference *GetType();                         // 4: 0x436a50
		ScriptReference *GetBaseType();                     // 5: 0x402b70
		int GetMax();                                       // 11: 0x43efe0
		int GetValueType();                                 // 12: 0x4034a0
		float GetFloat(ScriptReference *owner);             // 14: 0x41a9e0
		void SetValue(float value, ScriptReference *owner); // 15: 0x41a9f0
	};
	class ExposeBloomHaloSize : public Measurement { // "bloom halo size", vtable 0x5d4708
	public:
		ExposeBloomHaloSize();                              // 0x43f250
		const char *GetName();                              // 0: 0x43f260
		ScriptReference *GetType();                         // 4: 0x436a50
		ScriptReference *GetBaseType();                     // 5: 0x402b70
		int GetMax();                                       // 11: 0x43efe0
		int GetValueType();                                 // 12: 0x4034a0
		float GetFloat(ScriptReference *owner);             // 14: 0x41aa00
		void SetValue(float value, ScriptReference *owner); // 15: 0x41aa10
	};
	class ExposeDOFBluriness : public Measurement { // "DOF bluriness factor", vtable 0x5d49b0
	public:
		ExposeDOFBluriness();                             // 0x43f380
		const char *GetName();                            // 0: 0x43f390
		ScriptReference *GetType();                       // 4: 0x436a50
		ScriptReference *GetBaseType();                   // 5: 0x402b70
		int GetMax();                                     // 11: 0x42fbc0
		int GetInt(ScriptReference *owner);               // 13: 0x41a860
		void SetValue(int value, ScriptReference *owner); // 16: 0x41a870
	};
};
ZOOSTERS_ASSERT_SIZE(ScriptFog, 4);

} // namespace Script

#endif
