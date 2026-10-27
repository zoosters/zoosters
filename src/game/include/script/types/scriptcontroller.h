// Script::ScriptController (PS2 RTTI; base ScriptReference), the "controller" script object (the pads), with nested ScriptRumble
// (a force-feedback effect) and the Expose* members for buttons, sticks and rumble settings.
// No data members: only the prototype is built (TFBType_Controller 0x4393d0, vptr only). The members are Measurements whose GetInt /
// GetFloat read the owner's pad (index into the player refs of TFB_GetPlayerRefs 0x439460) through GetInputValue 0x402c80 and ignore `this`.
// Vtable 0x5d1190 (11 slots): 0 GetName below; 4 GetType 0x439450 and 7 GetMembers 0x439810 are overrides, declared below;
// 5 is the default 0x43e6f0, 6 the object default 0x42cbe0.
#ifndef ZOOSTERS_SCRIPT_TYPES_SCRIPTCONTROLLER_H
#define ZOOSTERS_SCRIPT_TYPES_SCRIPTCONTROLLER_H

#include "zoosters/base.h"
#include "script/actor.h"

namespace Script {

// The type prototype of the rumble effect (TFBType_Rumble 0x439770): a bare 0x40-byte Placement with its own vtable 0x5ca878,
// whose slots 0, 4 and 7 are the functions ScriptRumble's vtable (0x5cab68) holds too. The class name is ours.
// GLOBAL: 0x5ca878 Script::RumbleProto::vftable
class RumbleProto : public Placement {
public:
	RumbleProto() {}
	const char *GetName();                      // 0: 0x403a50 ("rumble")
	ScriptReference *GetType();                 // 4: 0x403a60
	ScriptReference *GetBaseType();             // 5: 0x4039e0
	void *GetMembers(int *count);               // 7: 0x403a70
	// 30: 0x403d80, the same function as ScriptRumble::Spawn (the game's vtable 0x5ca878 holds it; it only uses `this` as the source).
	// Normal build only: linker alias to ScriptRumble::Spawn in script/rumble.cpp.
	intptr_t Spawn(intptr_t pos, int unused);
};

// vtable 0x5d11c8: the five player-reference objects of TFB_GetPlayerRefs (GetName 0x4395b0 = "controller 1".."no
// controller" by the object's index); the name is ours
class ControllerPlayerRef : public ScriptReference {
public:
	ControllerPlayerRef();
	const char *GetName();
	ScriptReference *GetType();
	void *GetMembers(int *count);
};
ZOOSTERS_ASSERT_SIZE(ControllerPlayerRef, 4);

// GLOBAL: 0x5d1190 Script::ScriptController::vftable
class ScriptController : public ScriptReference { // "controller"
public:
	void *GetMembers(int *count);   // 7: 0x439810
	static void *Members(int *count);   // 0x439810 without an object (the body of GetMembers; ControllerPlayerRef::GetMembers shares it)
	const char *GetName(); // 0: 0x4393c0
	ScriptReference *GetType();     // 4: 0x439450 (ScriptController_GetType); normal build: the prototype is a real ScriptController (controllertype.cpp)
	// One rumble (force-feedback) effect, 0x7c bytes, a Placement the pad code mixes every frame (GameController::HandleEvents 0x403530).
	// Vtable 0x5cab68 = Placement's 40 slots with overrides: 0 GetName 0x403a50, 2 AdjustRefCount, 3 shared scalar deleting dtor 0x403d20,
	// 4 GetType 0x403a60, 7 GetMembers 0x403a70, 17 GetVector 0x403cf0, 18 SetVector, 27 Remove, 28 Reset 0x403cd0, 30 Spawn 0x403d80
	// (W2a::Source::Create), 31 GetSpawnSource 0x403ce0. The prototype (TFBType_Rumble 0x439770, vtable 0x5ca878) is what the Expose*
	// accessors compare their owner with ("no rumble").
	// GLOBAL: 0x5cab68 Script::ScriptController::ScriptRumble::vftable
	class ScriptRumble : public Placement {
	public:
		SetCursorTail m_unk40;
		int m_refCount;  // reference count (AdjustRefCount)
		ScriptReference *m_source;  // the object this rumble belongs to (has a ReferenceSet at +0x20)
		Vec3 m_position;
		unsigned m_slotLevel;  // level of the cut-scene stack slot at creation (HandleEvents compares it)
		int m_actuator1Level;  // ExposeActuator1Level ("high frequency")
		int m_actuator2Level;  // ExposeActuator2Level ("low frequency")
		float m_duration;  // ExposeDuration
		float m_timeRemaining;  // ExposeTimeRemaining: running copy of the duration
		float m_radius;  // ExposeRadius
		int m_attenuation;  // ExposeAttenuation (0 none, 2 quadratic, else linear)

		ScriptRumble(ScriptReference *source, const Vec3 *pos);   // 0x403bf0 (vtable 0x5cab68; Placement's constructor inline)
		// normal build: slots 0, 4, 5 of the game's vtable 0x5cab68 are the same functions as the prototype's (0x403a50
		// ScriptRumble_GetName, 0x403a60 ScriptRumble_GetType, 0x4039e0 Placement_GetType); linker aliases in
		// script/normal_bringup.cpp
		const char *GetName();                                 // 0: 0x403a50
		ScriptReference *GetType();                            // 4: 0x403a60
		ScriptReference *GetBaseType();                        // 5: 0x4039e0
		int AdjustRefCount(int delta);                         // 2: 0x403cb0
		void *GetMembers(int *count);                          // 7: 0x403a70 (the six Expose* prototypes)
		static void *Members(int *count);                      // 0x403a70 without an object (the body of GetMembers; RumbleProto::GetMembers shares it)
		Vec3 *GetVector(ScriptReference *owner);               // 17: 0x403cf0 (returns m_position)
		void Reset();                                          // 28: 0x403cd0 (tail call of slot 27, Remove)
		ScriptReference *GetSpawnSource();                     // 31: 0x403ce0 (returns m_source)
		// 30: 0x403d80 creates a rumble with `this` as its source and puts it into the rumble set. Not an override: Placement::Spawn
		// takes (int, int) in our declaration, the game's slot takes (pos, unused).
		// normal build: the compiler builds the vtable, so slot 30 must be Placement::Spawn(int, int)'s override: same function, the
		// position pointer passed as int (script/rumble.cpp). Returns the new rumble as int.
		intptr_t Spawn(intptr_t pos, int unused);
		// The body of slot 30 (0x403d80), shared with RumbleProto::Spawn (the game folded the two): a new rumble from `source` at pos.
		static intptr_t SpawnFrom(ScriptReference *source, intptr_t pos);
		void SetVector(const Vec3 *v, ScriptReference *owner); // 18: 0x403d00
		void Remove();                                         // 27: 0x403d40 (Placement::Remove; ScriptRumble::Remove)
		// GLOBAL: 0x5ca920 Script::ScriptController::ScriptRumble::ExposeAttenuation::vftable
		class ExposeAttenuation : public Measurement {         // "attenuation", vtable 0x5ca920
		public:
			const char *GetName();                            // 0: 0x403ba0
			int GetInt(ScriptReference *owner);               // 13: 0x402a60
			void SetValue(int value, ScriptReference *owner); // 16: 0x402a80
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
		};
		// GLOBAL: 0x5ca968 Script::ScriptController::ScriptRumble::ExposeRadius::vftable
		class ExposeRadius : public Measurement { // "radius", vtable 0x5ca968
		public:
			const char *GetName();                              // 0: 0x403b90
			float GetFloat(ScriptReference *owner);             // 14: 0x402a20
			void SetValue(float value, ScriptReference *owner); // 15: 0x402a40
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetValueType();                                     // 12: 0x4034a0
		};
		// GLOBAL: 0x5ca9b0 Script::ScriptController::ScriptRumble::ExposeDuration::vftable
		class ExposeDuration : public Measurement { // "duration", vtable 0x5ca9b0
		public:
			const char *GetName();                              // 0: 0x403b80
			float GetFloat(ScriptReference *owner);             // 14: 0x4029c0
			void SetValue(float value, ScriptReference *owner); // 15: 0x4029e0
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetValueType();                                     // 12: 0x4034a0
		};
		// GLOBAL: 0x5ca9f8 Script::ScriptController::ScriptRumble::ExposeActuator1Level::vftable
		class ExposeActuator1Level : public Measurement { // "high frequency", vtable 0x5ca9f8
		public:
			const char *GetName();                            // 0: 0x403b60
			int GetInt(ScriptReference *owner);               // 13: 0x402940
			void SetValue(int value, ScriptReference *owner); // 16: 0x402960
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x42fbc0
		};
		// GLOBAL: 0x5caa40 Script::ScriptController::ScriptRumble::ExposeActuator2Level::vftable
		class ExposeActuator2Level : public Measurement { // "low frequency", vtable 0x5caa40
		public:
			const char *GetName();                            // 0: 0x403b70
			int GetInt(ScriptReference *owner);               // 13: 0x402980
			void SetValue(int value, ScriptReference *owner); // 16: 0x4029a0
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x42fbc0
		};
		class ExposeTimeRemaining : public Measurement { // "time remaining", vtable 0x5caac8
		public:
			ExposeTimeRemaining();                              // 0x403bd0
			const char *GetName();                              // 0: 0x438d40
			ScriptReference *GetType();                         // 4: 0x436a50
			ScriptReference *GetBaseType();                     // 5: 0x402b70
			int GetKind();                                      // 6: 0x43e6f0
			int GetValueType();                                 // 12: 0x4034a0
			float GetFloat(ScriptReference *owner);             // 14: 0x402a00
			void SetValue(float value, ScriptReference *owner); // 15: 0x4025a0
		};
		class ExposePercent5cab20 : public Measurement { // vtable 0x5cab20
		public:
			ExposePercent5cab20();                              // 0x403be0
			ScriptReference *GetType();                         // 4: 0x436a50
			ScriptReference *GetBaseType();                     // 5: 0x402b70
			int GetMax();                                       // 11: 0x42fbc0
			int GetInt(ScriptReference *owner);                 // 13: 0x416530
			float GetFloat(ScriptReference *owner);             // 14: 0x402b80
			void SetValue(float value, ScriptReference *owner); // 15: 0x407ab0
			void SetValue(int value, ScriptReference *owner);   // 16: 0x4025a0
		};
		class ExposeRumbleDisabled : public Measurement { // "rumble disabled", vtable 0x5d2fe8
		public:
			ExposeRumbleDisabled();                           // 0x43c0c0
			const char *GetName();                            // 0: 0x43c120
			ScriptReference *GetType();                       // 4: 0x436a50
			ScriptReference *GetBaseType();                   // 5: 0x402b70
			int GetMax();                                     // 11: 0x43efe0
			int GetInt(ScriptReference *owner);               // 13: 0x402aa0
			void SetValue(int value, ScriptReference *owner); // 16: 0x402ac0
		};
	};
	// GLOBAL: 0x5d11f8 Script::ScriptController::ExposeLeftAnalogL::vftable
	class ExposeLeftAnalogL : public Measurement { // "analog1 left", vtable 0x5d11f8
	public:
		const char *GetName();              // 0: 0x4394d0
		int GetInt(ScriptReference *owner); // 13: 0x4030e0
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x42fbc0
		void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
	};
	// GLOBAL: 0x5d1250 Script::ScriptController::ExposeLeftAnalogR::vftable
	class ExposeLeftAnalogR : public Measurement { // "analog1 right", vtable 0x5d1250
	public:
		const char *GetName();              // 0: 0x4394e0
		int GetInt(ScriptReference *owner); // 13: 0x403120
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x42fbc0
		void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
	};
	// GLOBAL: 0x5d12a8 Script::ScriptController::ExposeLeftAnalogU::vftable
	class ExposeLeftAnalogU : public Measurement { // "analog1 up", vtable 0x5d12a8
	public:
		const char *GetName();              // 0: 0x4394f0
		int GetInt(ScriptReference *owner); // 13: 0x403160
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x42fbc0
		void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
	};
	// GLOBAL: 0x5d12f8 Script::ScriptController::ExposeLeftAnalogD::vftable
	class ExposeLeftAnalogD : public Measurement { // "analog1 down", vtable 0x5d12f8
	public:
		const char *GetName();              // 0: 0x439500
		int GetInt(ScriptReference *owner); // 13: 0x4031a0
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x42fbc0
		void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
	};
	// GLOBAL: 0x5d1350 Script::ScriptController::ExposeRightAnalogL::vftable
	class ExposeRightAnalogL : public Measurement { // "analog2 left", vtable 0x5d1350
	public:
		const char *GetName();              // 0: 0x439510
		int GetInt(ScriptReference *owner); // 13: 0x4031e0
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x42fbc0
		void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
	};
	// GLOBAL: 0x5d13a8 Script::ScriptController::ExposeRightAnalogR::vftable
	class ExposeRightAnalogR : public Measurement { // "analog2 right", vtable 0x5d13a8
	public:
		const char *GetName();              // 0: 0x439520
		int GetInt(ScriptReference *owner); // 13: 0x403220
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x42fbc0
		void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
	};
	// GLOBAL: 0x5d1400 Script::ScriptController::ExposeRightAnalogU::vftable
	class ExposeRightAnalogU : public Measurement { // "analog2 up", vtable 0x5d1400
	public:
		const char *GetName();              // 0: 0x439530
		int GetInt(ScriptReference *owner); // 13: 0x403260
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x42fbc0
		void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
	};
	// GLOBAL: 0x5d1450 Script::ScriptController::ExposeRightAnalogD::vftable
	class ExposeRightAnalogD : public Measurement { // "analog2 down", vtable 0x5d1450
	public:
		const char *GetName();              // 0: 0x439540
		int GetInt(ScriptReference *owner); // 13: 0x4032a0
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x42fbc0
		void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
	};
	// GLOBAL: 0x5d14a8 Script::ScriptController::ExposeLeftAnalogMagnitude::vftable
	class ExposeLeftAnalogMagnitude : public Measurement { // "analog1 magnitude", vtable 0x5d14a8
	public:
		const char *GetName();                  // 0: 0x439550
		float GetFloat(ScriptReference *owner); // 14: 0x403350
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
		void SetValue(float /*a1*/, ScriptReference * /*a2*/);  // 15: 0x4025a0
	};
	// GLOBAL: 0x5d1500 Script::ScriptController::ExposeRightAnalogMagnitude::vftable
	class ExposeRightAnalogMagnitude : public Measurement { // "analog2 magnitude", vtable 0x5d1500
	public:
		const char *GetName();                  // 0: 0x439560
		float GetFloat(ScriptReference *owner); // 14: 0x403410
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
		void SetValue(float /*a1*/, ScriptReference * /*a2*/);  // 15: 0x4025a0
	};
	// GLOBAL: 0x5d1558 Script::ScriptController::ExposeControllerUnplugged::vftable
	class ExposeControllerUnplugged : public Measurement { // "connection", vtable 0x5d1558
	public:
		const char *GetName();              // 0: 0x439570
		int GetInt(ScriptReference *owner); // 13: 0x402910
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x43efe0
		void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
	};
	class ExposeActiveControllers : public Measurement { // "Active Controllers", vtable 0x5d15a8
	public:
		const char *GetName();                            // 0: 0x439580
		ScriptReference *GetType();                       // 4: 0x436a50
		ScriptReference *GetBaseType();                   // 5: 0x402b70
		int GetInt(ScriptReference *owner);               // 13: 0x4395a0
		void SetValue(int value, ScriptReference *owner); // 16: 0x439590
	};
	// GLOBAL: 0x5d1650 Script::ScriptController::ExposeCircle::vftable
	class ExposeCircle : public Measurement { // "circle button", vtable 0x5d1650
	public:
		const char *GetName();              // 0: 0x439600
		int GetInt(ScriptReference *owner); // 13: 0x402e40
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x43efe0
		void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
	};
	// GLOBAL: 0x5d16a8 Script::ScriptController::ExposeTriangle::vftable
	class ExposeTriangle : public Measurement { // "triangle button", vtable 0x5d16a8
	public:
		const char *GetName();              // 0: 0x439610
		int GetInt(ScriptReference *owner); // 13: 0x402e70
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x43efe0
		void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
	};
	// GLOBAL: 0x5d1700 Script::ScriptController::ExposeX::vftable
	class ExposeX : public Measurement { // "X button", vtable 0x5d1700
	public:
		const char *GetName();              // 0: 0x439620
		int GetInt(ScriptReference *owner); // 13: 0x402ea0
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x43efe0
		void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
	};
	// GLOBAL: 0x5d1750 Script::ScriptController::ExposeSquare::vftable
	class ExposeSquare : public Measurement { // "square button", vtable 0x5d1750
	public:
		const char *GetName();              // 0: 0x439630
		int GetInt(ScriptReference *owner); // 13: 0x402ed0
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x43efe0
		void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
	};
	// GLOBAL: 0x5d17a8 Script::ScriptController::ExposeSelect::vftable
	class ExposeSelect : public Measurement { // "select button", vtable 0x5d17a8
	public:
		const char *GetName();              // 0: 0x439640
		int GetInt(ScriptReference *owner); // 13: 0x402f00
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x43efe0
		void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
	};
	// GLOBAL: 0x5d1800 Script::ScriptController::ExposeStart::vftable
	class ExposeStart : public Measurement { // "start button", vtable 0x5d1800
	public:
		const char *GetName();              // 0: 0x439650
		int GetInt(ScriptReference *owner); // 13: 0x402f30
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x43efe0
		void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
	};
	// GLOBAL: 0x5d1858 Script::ScriptController::ExposeL1::vftable
	class ExposeL1 : public Measurement { // "L1 button", vtable 0x5d1858
	public:
		const char *GetName();              // 0: 0x439660
		int GetInt(ScriptReference *owner); // 13: 0x402f60
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x43efe0
		void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
	};
	// GLOBAL: 0x5d18a8 Script::ScriptController::ExposeR1::vftable
	class ExposeR1 : public Measurement { // "R1 button", vtable 0x5d18a8
	public:
		const char *GetName();              // 0: 0x439670
		int GetInt(ScriptReference *owner); // 13: 0x402f90
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x43efe0
		void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
	};
	// GLOBAL: 0x5d18f8 Script::ScriptController::ExposeL2::vftable
	class ExposeL2 : public Measurement { // "L2 button", vtable 0x5d18f8
	public:
		const char *GetName();              // 0: 0x439680
		int GetInt(ScriptReference *owner); // 13: 0x402fc0
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x43efe0
		void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
	};
	// GLOBAL: 0x5d1948 Script::ScriptController::ExposeR2::vftable
	class ExposeR2 : public Measurement { // "R2 button", vtable 0x5d1948
	public:
		const char *GetName();              // 0: 0x439690
		int GetInt(ScriptReference *owner); // 13: 0x402ff0
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x43efe0
		void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
	};
	// GLOBAL: 0x5d1998 Script::ScriptController::ExposeDPadL::vftable
	class ExposeDPadL : public Measurement { // "DPad left", vtable 0x5d1998
	public:
		const char *GetName();              // 0: 0x4396a0
		int GetInt(ScriptReference *owner); // 13: 0x403020
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x43efe0
		void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
	};
	// GLOBAL: 0x5d19e8 Script::ScriptController::ExposeDPadR::vftable
	class ExposeDPadR : public Measurement { // "DPad right", vtable 0x5d19e8
	public:
		const char *GetName();              // 0: 0x4396b0
		int GetInt(ScriptReference *owner); // 13: 0x403050
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x43efe0
		void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
	};
	// GLOBAL: 0x5d1a38 Script::ScriptController::ExposeDPadU::vftable
	class ExposeDPadU : public Measurement { // "DPad up", vtable 0x5d1a38
	public:
		const char *GetName();              // 0: 0x4396c0
		int GetInt(ScriptReference *owner); // 13: 0x403080
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x43efe0
		void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
	};
	// GLOBAL: 0x5d1a88 Script::ScriptController::ExposeDPadD::vftable
	class ExposeDPadD : public Measurement { // "DPad down", vtable 0x5d1a88
	public:
		const char *GetName();              // 0: 0x4396d0
		int GetInt(ScriptReference *owner); // 13: 0x4030b0
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x43efe0
		void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
	};
	// GLOBAL: 0x5d1ad8 Script::ScriptController::ExposeLeftAnalogAngle::vftable
	class ExposeLeftAnalogAngle : public Measurement { // "analog1 heading", vtable 0x5d1ad8
	public:
		const char *GetName();                  // 0: 0x4396e0
		float GetFloat(ScriptReference *owner); // 14: 0x4032e0
		ScriptReference *GetType();                             // 4: 0x403e90
		ScriptReference *GetBaseType();                         // 5: 0x436a50
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
		void SetValue(float /*a1*/, ScriptReference * /*a2*/);  // 15: 0x4025a0
	};
	// GLOBAL: 0x5d1b30 Script::ScriptController::ExposeRightAnalogAngle::vftable
	class ExposeRightAnalogAngle : public Measurement { // "analog2 heading", vtable 0x5d1b30
	public:
		const char *GetName();                  // 0: 0x4396f0
		float GetFloat(ScriptReference *owner); // 14: 0x4033a0
		ScriptReference *GetType();                             // 4: 0x403e90
		ScriptReference *GetBaseType();                         // 5: 0x436a50
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
		void SetValue(float /*a1*/, ScriptReference * /*a2*/);  // 15: 0x4025a0
	};
};

ZOOSTERS_ASSERT_SIZE(ScriptController, 4);

typedef ScriptController::ScriptRumble ScriptControllerRumble;
ZOOSTERS_ASSERT_SIZE(ScriptControllerRumble, 0x7c);
ZOOSTERS_ASSERT_OFFSET(ScriptControllerRumble, m_unk40, 0x40);
ZOOSTERS_ASSERT_OFFSET(ScriptControllerRumble, m_refCount, 0x4c);
ZOOSTERS_ASSERT_OFFSET(ScriptControllerRumble, m_source, 0x50);
ZOOSTERS_ASSERT_OFFSET(ScriptControllerRumble, m_position, 0x54);
ZOOSTERS_ASSERT_OFFSET(ScriptControllerRumble, m_slotLevel, 0x60);
ZOOSTERS_ASSERT_OFFSET(ScriptControllerRumble, m_actuator1Level, 0x64);
ZOOSTERS_ASSERT_OFFSET(ScriptControllerRumble, m_actuator2Level, 0x68);
ZOOSTERS_ASSERT_OFFSET(ScriptControllerRumble, m_duration, 0x6c);
ZOOSTERS_ASSERT_OFFSET(ScriptControllerRumble, m_timeRemaining, 0x70);
ZOOSTERS_ASSERT_OFFSET(ScriptControllerRumble, m_radius, 0x74);
ZOOSTERS_ASSERT_OFFSET(ScriptControllerRumble, m_attenuation, 0x78);

} // namespace Script

// Controller / rumble helpers of 0x402910-0x439770 (callers: GameController::HandleEvents, the Expose* accessors above)
Script::ReferenceSet *RumbleSet_Get();                      // 0x4034d0: the live rumble set (function-local static)
extern unsigned char g_rumbleOff;                          // 0x617988
Script::ControllerPlayerRef *TFB_GetPlayerRefs();              // 0x439460
Script::ScriptReference *TFBType_Rumble();                   // 0x439770: function-local static "no rumble" reference
bool IsWaitingForPad(int player);                       // 0x4057f0
float GetInputValue(int *owner, int input, float threshold);   // 0x402c80 (owner: a ControllerPlayerRef, only the pad index is read)

#endif
