// Script::ScriptParticle (PS2 RTTI; base Placement), the script object of a particle emitter, with the Expose* member classes
// registered for it (an Expose class is a Measurement of one emitter setting: GetInt / GetFloat / SetValue take the owner, the
// particle the script variable resolved to, and ignore `this`). The settings block +0x4c..+0xd0 is what the script reads and
// what the emitter (CFXPartSpray) mirrors its attributes into; RWS::FX::GameParticle (modules/fx/gameparticle.h) adds the live
// particle buffer from +0xd4, and the setters below push every change into it as well.
#ifndef ZOOSTERS_SCRIPT_TYPES_SCRIPTPARTICLE_H
#define ZOOSTERS_SCRIPT_TYPES_SCRIPTPARTICLE_H

#include "zoosters/base.h"
#include "rw/rwcore.h"
#include "script/actor.h"

namespace Script {

// GLOBAL: 0x5cb370 Script::ScriptParticle::vftable
// RTTI name Script::ScriptParticle; vtable 0x5cb370 (40 slots, Placement's with the overrides below; GameParticle's vtable 0x5cb420
// holds the same functions in these slots).
class ScriptParticle : public Placement {
public:
	ScriptParticle() {}
	// The type prototype (TFBType_Particle 0x43b4f0): the constructor calls vf10 (DetachParticleSetClones) on the new object.
	explicit ScriptParticle(PrototypeInit) { }   // the reset reaches TFBType_Particle again (zoosters/base.h)
	const char *GetName();                                  // 0: 0x40CF90 ("particle")
	ScriptReference *GetType();                             // 4: 0x43B870 (jumps to TFBType_Particle 0x43b4f0)
	ScriptReference *GetBaseType();                         // 5: 0x4039E0
	void *GetMembers(int *count);                           // 7: 0x43B8B0
	void vf10();                                            // 10: 0x43B880 (DetachParticleSetClones: type prototype hook)

	// ExposeClones (the set of spawned copies, vtable 0x5d20c8) is not declared: its class is not decompiled
	// (see the stub in script/deleting_dtors_2.cpp). Normal builds declare the two members the vtable data tables name.
	// GLOBAL: 0x5d20c8 Script::ScriptParticle::ExposeClones::vftable
	class ExposeClones : public ReferenceSet {
	public:
		const char *GetName();                                  // 0: 0x416480
		ScriptReference *GetBaseType();                         // 5: 0x43b870
		ScriptReference *Deref(ScriptReference *owner, int flags);   // 9: 0x43b150
		~ExposeClones();                                    // 0x43b0b0 (linker alias, script/normal_aliases.cpp)
		ExposeClones *scalar_deleting_dtor(unsigned flags); // 0x43b710 (script/deleting_dtors_2.cpp)
	};
	// GLOBAL: 0x5d1f78 Script::ScriptParticle::ExposeOutputRate::vftable
	class ExposeOutputRate : public Measurement {
	public:
		const char *GetName();
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43b110
	};
	// GLOBAL: 0x5d1fc8 Script::ScriptParticle::ExposeOutputRateSpread::vftable
	class ExposeOutputRateSpread : public Measurement {
	public:
		const char *GetName();
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43b2b0
	};
	// GLOBAL: 0x5d2020 Script::ScriptParticle::ExposeLifetime::vftable
	class ExposeLifetime : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d2070 Script::ScriptParticle::ExposeLifetimeSpread::vftable
	class ExposeLifetimeSpread : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d2110 Script::ScriptParticle::ExposeActivationRange::vftable
	class ExposeActivationRange : public Measurement {
	public:
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		const char *GetName();                                  // 0: 0x435db0
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d2158 Script::ScriptParticle::ExposeDeactivationRange::vftable
	class ExposeDeactivationRange : public Measurement {
	public:
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		const char *GetName();                                  // 0: 0x435dc0
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d21a0 Script::ScriptParticle::ExposePrimarySVelocity::vftable
	class ExposePrimarySVelocity : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43b250
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d2200 Script::ScriptParticle::ExposePrimarySVelocitySpread::vftable
	class ExposePrimarySVelocitySpread : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43b2b0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d2268 Script::ScriptParticle::ExposePrimarySGravity::vftable
	class ExposePrimarySGravity : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d22c8 Script::ScriptParticle::ExposePrimarySDrag::vftable
	class ExposePrimarySDrag : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43efe0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d2320 Script::ScriptParticle::ExposePrimarySColor::vftable
	class ExposePrimarySColor : public Measurement {
	public:
		void *GetMembers(int *count);   // 7: 0x43b2d0
		const char *GetName();
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		// GLOBAL: 0x5d29a8 Script::ScriptParticle::ExposePrimarySColor::ExposeR::vftable
		class ExposeR : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436920
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5d2960 Script::ScriptParticle::ExposePrimarySColor::ExposeG::vftable
		class ExposeG : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436930
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5d2918 Script::ScriptParticle::ExposePrimarySColor::ExposeB::vftable
		class ExposeB : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436940
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5d28d0 Script::ScriptParticle::ExposePrimarySColor::ExposeA::vftable
		class ExposeA : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436950
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		int GetValueType();                                     // 12: 0x407ad0
		ScriptReference *GetType();                             // 4: 0x407aa0
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43e6f0
	};
	// GLOBAL: 0x5d2378 Script::ScriptParticle::ExposePrimarySColorSpread::vftable
	class ExposePrimarySColorSpread : public Measurement {
	public:
		void *GetMembers(int *count);   // 7: 0x43b350
		const char *GetName();
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		// GLOBAL: 0x5d2ac8 Script::ScriptParticle::ExposePrimarySColorSpread::ExposeR::vftable
		class ExposeR : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436920
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5d2a80 Script::ScriptParticle::ExposePrimarySColorSpread::ExposeG::vftable
		class ExposeG : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436930
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5d2a38 Script::ScriptParticle::ExposePrimarySColorSpread::ExposeB::vftable
		class ExposeB : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436940
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5d29f0 Script::ScriptParticle::ExposePrimarySColorSpread::ExposeA::vftable
		class ExposeA : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436950
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		int GetValueType();                                     // 12: 0x407ad0
		ScriptReference *GetType();                             // 4: 0x407aa0
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43e6f0
	};
	// GLOBAL: 0x5d23d8 Script::ScriptParticle::ExposePrimarySSize::vftable
	class ExposePrimarySSize : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43b250
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d2430 Script::ScriptParticle::ExposePrimarySSizeSpread::vftable
	class ExposePrimarySSizeSpread : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43b270
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d2490 Script::ScriptParticle::ExposePrimaryEVelocity::vftable
	class ExposePrimaryEVelocity : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43b250
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d24f0 Script::ScriptParticle::ExposePrimaryEVelocitySpread::vftable
	class ExposePrimaryEVelocitySpread : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43b2b0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d2550 Script::ScriptParticle::ExposePrimaryEGravity::vftable
	class ExposePrimaryEGravity : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d25a8 Script::ScriptParticle::ExposePrimaryEDrag::vftable
	class ExposePrimaryEDrag : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43efe0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d2600 Script::ScriptParticle::ExposePrimaryEColor::vftable
	class ExposePrimaryEColor : public Measurement {
	public:
		void *GetMembers(int *count);   // 7: 0x43b3e0
		const char *GetName();
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		// GLOBAL: 0x5d2be8 Script::ScriptParticle::ExposePrimaryEColor::ExposeR::vftable
		class ExposeR : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436920
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5d2ba0 Script::ScriptParticle::ExposePrimaryEColor::ExposeG::vftable
		class ExposeG : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436930
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5d2b58 Script::ScriptParticle::ExposePrimaryEColor::ExposeB::vftable
		class ExposeB : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436940
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5d2b10 Script::ScriptParticle::ExposePrimaryEColor::ExposeA::vftable
		class ExposeA : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436950
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		int GetValueType();                                     // 12: 0x407ad0
		ScriptReference *GetType();                             // 4: 0x407aa0
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43e6f0
	};
	// GLOBAL: 0x5d2658 Script::ScriptParticle::ExposePrimaryEColorSpread::vftable
	class ExposePrimaryEColorSpread : public Measurement {
	public:
		void *GetMembers(int *count);   // 7: 0x43b460
		const char *GetName();
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		// GLOBAL: 0x5d2d08 Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeR::vftable
		class ExposeR : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436920
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5d2cc0 Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeG::vftable
		class ExposeG : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436930
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5d2c78 Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeB::vftable
		class ExposeB : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436940
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5d2c30 Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeA::vftable
		class ExposeA : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436950
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		int GetValueType();                                     // 12: 0x407ad0
		ScriptReference *GetType();                             // 4: 0x407aa0
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43e6f0
	};
	// GLOBAL: 0x5d26b8 Script::ScriptParticle::ExposePrimaryESize::vftable
	class ExposePrimaryESize : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43b250
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d2770 Script::ScriptParticle::ExposeEmitterSpreadX::vftable
	class ExposeEmitterSpreadX : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43b2b0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d27c8 Script::ScriptParticle::ExposeEmitterSpreadY::vftable
	class ExposeEmitterSpreadY : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43b2b0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d2820 Script::ScriptParticle::ExposeEmitterSpreadZ::vftable
	class ExposeEmitterSpreadZ : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43b2b0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d2e90 Script::ScriptParticle::ExposeAngularSpreadZ::vftable
	class ExposeAngularSpreadZ : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x403e90
		ScriptReference *GetBaseType();                         // 5: 0x436a50
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d2878 Script::ScriptParticle::ExposeVisualRadius::vftable
	class ExposeVisualRadius : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d2ee8 Script::ScriptParticle::ExposeIgnoreCutScenes::vftable
	class ExposeIgnoreCutScenes : public Measurement {
	public:
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		const char *GetName();                                  // 0: 0x435fd0
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43efe0
	};
	// GLOBAL: 0x5d2710 Script::ScriptParticle::ExposePrimaryESizeSpread::vftable
	class ExposePrimaryESizeSpread : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43b270
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d2e38 Script::ScriptParticle::ExposeAngularSpreadX::vftable
	class ExposeAngularSpreadX : public Measurement {
	public:
		float GetFloat(ScriptReference *owner);                 // 14: 0x413c10 (folded with ScriptSprite::ExposeRotation::GetFloat)
		const char *GetName();
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x403e90
		ScriptReference *GetBaseType();                         // 5: 0x436a50
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
	};
};
ZOOSTERS_ASSERT_SIZE(ScriptParticle, 0x40);

}  // namespace Script

#endif
