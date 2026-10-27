// Script::ScriptActor (PS2 RTTI; base Placement), the script object of an actor, and the Expose* member classes registered for it
// (an Expose class is a Measurement of one actor property: GetInt / GetFloat / SetValue take the owner, the actor the script variable
// resolved to, and ignore `this`). 0x98 bytes; RWS::TFB::GameActor (modules/tfb/gameactor.h) adds the Collider at +0x98 and the actor's
// own members from +0x16c: every accessor below reaches those through a GameActor cast.
#ifndef ZOOSTERS_SCRIPT_TYPES_SCRIPTACTOR_H
#define ZOOSTERS_SCRIPT_TYPES_SCRIPTACTOR_H

#include "zoosters/base.h"
#include "script/actor.h"

namespace Script {

// GLOBAL: 0x5cd5e8 Script::ScriptActor::vftable
// RTTI name Script::ScriptActor; vtable 0x5cd5e8 (40 slots, Placement's).
class ScriptActor : public Placement {
public:
	SetCursorTail m_unk40;
	int m_controlMode;  // 0 player, 1 NPC, 2 parked (the constructor's value), 3 leaving (Placement::SetControlMode 0x42e530)
	// The three member sets (0x18 bytes each, own vtables: CsSetWayPoints 0x5cd380, CsSetAttachPoints 0x5cd3d8, CsSetTurrets
	// 0x5cd430), as the Deref accessors of ExposeWayPoints / ExposeAttachmentPoints / ExposeTurrets return them. CProtoActor attribute 5
	// fills the waypoints.
	// normal builds construct the sets in this storage (script/actorprotos.cpp p4b_ScriptActor_init_sets): a ReferenceSet each, 0x18
	// bytes on x86, more on x64 (vptr and list pointers)
	alignas(ReferenceSet) uint8_t m_wayPoints[sizeof(ReferenceSet)];
	alignas(ReferenceSet) uint8_t m_attachmentPoints[sizeof(ReferenceSet)];
	alignas(ReferenceSet) uint8_t m_turrets[sizeof(ReferenceSet)];

	ScriptActor();                                    // 0x42d540
	~ScriptActor();                                   // 0x42be70 (scalar deleting destructor 0x42d600)
	const char *GetName();                            // 0: 0x42D5F0
	ScriptReference *GetType();                       // 4: 0x43AA60
	ScriptReference *GetBaseType();                   // 5: 0x4039E0
	void *GetMembers(int *count);                     // 7: 0x436F80
	void vf10();                                      // 10: 0x4386C0
	Vec3 *GetVector(ScriptReference *owner);          // 17: 0x429D60 (name GetPosition): the body's position, else the actor's own
	Vec3 *GetVelocity(ScriptReference *owner);        // 25: 0x429D80

	// Not virtual. All three read or write fields of GameActor (+0x1b0 m_params.m_flags, +0x1b4 m_params.m_activationRange, +0x2d0
	// m_unk2D0), so in the original they probably belong to GameActor; they keep the ScriptActor names of the symbol list.
	void SetSolidCollider(bool on);                   // 0x42c900: set/clear the solid-collider flag and update the body
	void SetMeshCollidee(bool on);                    // 0x42c9a0: the same for the mesh-collidee flag
	void SetNoShadow(bool on);                        // 0x42ca50: the same for the no-shadow flag
	float GetActivationRange();                             // 0x429ca0 (activation range, in metres)
	float GetDeactivationRange();                             // 0x429d10
	void SetScriptClockHold(unsigned char v);               // 0x42ab60: stores GameActor::m_unk2D0
	// GLOBAL: 0x5cefd0 Script::ScriptActor::ExposeHitPoints::vftable
	class ExposeHitPoints : public Measurement {
	public:
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		const char *GetName();
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
	};
	// GLOBAL: 0x5cf0d0 Script::ScriptActor::ExposeFadePercent::vftable
	class ExposeFadePercent : public Measurement {
	public:
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		const char *GetName();
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x42fbc0
	};
	// GLOBAL: 0x5cf138 Script::ScriptActor::ExposeActivationRange::vftable
	class ExposeActivationRange : public Measurement {
	public:
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		const char *GetName();                                  // 0: 0x435db0
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5cf190 Script::ScriptActor::ExposeDeactivationRange::vftable
	class ExposeDeactivationRange : public Measurement {
	public:
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		const char *GetName();                                  // 0: 0x435dc0
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5cf2e0 Script::ScriptActor::ExposeGroundAccel::vftable
	class ExposeGroundAccel : public Measurement {
	public:
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		const char *GetName();
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5cf338 Script::ScriptActor::ExposeGroundDecel::vftable
	class ExposeGroundDecel : public Measurement {
	public:
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		const char *GetName();
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5cf390 Script::ScriptActor::ExposeMoveType::vftable
	class ExposeMoveType : public Measurement {
	public:
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		const char *GetName();
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43a6a0
	};
	// GLOBAL: 0x5cf3e0 Script::ScriptActor::ExposeAirSpeed::vftable
	class ExposeAirSpeed : public Measurement {
	public:
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		const char *GetName();
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5cf490 Script::ScriptActor::ExposeAirAccel::vftable
	class ExposeAirAccel : public Measurement {
	public:
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		const char *GetName();
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5cf540 Script::ScriptActor::ExposeJumpDelay::vftable
	class ExposeJumpDelay : public Measurement {
	public:
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		const char *GetName();
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
	};
	// GLOBAL: 0x5cf808 Script::ScriptActor::ExposeWeight::vftable
	class ExposeWeight : public Measurement {
	public:
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		const char *GetName();
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5cf8b0 Script::ScriptActor::ExposeXOffset::vftable
	class ExposeXOffset : public Measurement {
	public:
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		const char *GetName();
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5cf900 Script::ScriptActor::ExposeYOffset::vftable
	class ExposeYOffset : public Measurement {
	public:
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		const char *GetName();
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5cf950 Script::ScriptActor::ExposeZOffset::vftable
	class ExposeZOffset : public Measurement {
	public:
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		const char *GetName();
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5cf9a0 Script::ScriptActor::ExposeXExtent::vftable
	class ExposeXExtent : public Measurement {
	public:
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		const char *GetName();
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5cf9f0 Script::ScriptActor::ExposeYExtent::vftable
	class ExposeYExtent : public Measurement {
	public:
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		const char *GetName();
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5cfa40 Script::ScriptActor::ExposeZExtent::vftable
	class ExposeZExtent : public Measurement {
	public:
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		const char *GetName();
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d04f0 Script::ScriptActor::ExposeConeAngle::vftable
	class ExposeConeAngle : public Measurement {
	public:
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		const char *GetName();
		ScriptReference *GetType();                             // 4: 0x403e90
		ScriptReference *GetBaseType();                         // 5: 0x436a50
		int GetMax();                                           // 11: 0x436b00
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5cf230 Script::ScriptActor::ExposeGroundSpeed::vftable
	class ExposeGroundSpeed : public Measurement {
	public:
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		const char *GetName();
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5cfe40 Script::ScriptActor::ExposeID::vftable
	class ExposeID : public Measurement {
	public:
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		const char *GetName();
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
	};
	// GLOBAL: 0x5cfb30 Script::ScriptActor::ExposeConeColor::vftable
	class ExposeConeColor : public Measurement {
	public:
		void *GetMembers(int *count);   // 7: 0x436820
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		const char *GetName();
		// GLOBAL: 0x5d0100 Script::ScriptActor::ExposeConeColor::ExposeB::vftable
		class ExposeB : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436940
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5d0148 Script::ScriptActor::ExposeConeColor::ExposeG::vftable
		class ExposeG : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436930
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5d0190 Script::ScriptActor::ExposeConeColor::ExposeR::vftable
		class ExposeR : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436920
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5d00b8 Script::ScriptActor::ExposeConeColor::ExposeA::vftable
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
	// GLOBAL: 0x5cff38 Script::ScriptActor::ExposeTintColor::vftable
	class ExposeTintColor : public Measurement {
	public:
		void *GetMembers(int *count);   // 7: 0x4368a0
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		const char *GetName();
		// GLOBAL: 0x5d02b0 Script::ScriptActor::ExposeTintColor::ExposeR::vftable
		class ExposeR : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436920
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5d0268 Script::ScriptActor::ExposeTintColor::ExposeG::vftable
		class ExposeG : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436930
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5d0220 Script::ScriptActor::ExposeTintColor::ExposeB::vftable
		class ExposeB : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436940
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5d01d8 Script::ScriptActor::ExposeTintColor::ExposeA::vftable
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
	// GLOBAL: 0x5cfe88 Script::ScriptActor::ExposeMeshCollider::vftable
	class ExposeMeshCollider : public Measurement {
	public:
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		const char *GetName();
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43efe0
	};
	// GLOBAL: 0x5cfb80 Script::ScriptActor::ExposeSolidCollider::vftable
	class ExposeSolidCollider : public Measurement {
	public:
		int GetInt(ScriptReference *owner);
		const char *GetName();
		void SetValue(int value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43efe0
	};
	// GLOBAL: 0x5cfbd8 Script::ScriptActor::ExposeWallCollision::vftable
	class ExposeWallCollision : public Measurement {
	public:
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		const char *GetName();
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43efe0
	};
	// GLOBAL: 0x5cfc88 Script::ScriptActor::ExposeWallClimber::vftable
	class ExposeWallClimber : public Measurement {
	public:
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		const char *GetName();
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43efe0
	};
	// GLOBAL: 0x5cfce0 Script::ScriptActor::ExposeIgnoresCutScenes::vftable
	class ExposeIgnoresCutScenes : public Measurement {
	public:
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		const char *GetName();                                  // 0: 0x435fd0
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43efe0
	};
	// GLOBAL: 0x5cf7a8 Script::ScriptActor::ExposeAnimationRate::vftable
	class ExposeAnimationRate : public Measurement {
	public:
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		const char *GetName();
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d0498 Script::ScriptActor::ExposeFacingObsolete::vftable
	class ExposeFacingObsolete : public Measurement {
	public:
		float GetFloat(ScriptReference *owner);
		const char *GetName();
		ScriptReference *GetType();                             // 4: 0x403e90
		ScriptReference *GetBaseType();                         // 5: 0x436a50
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
		void SetValue(float /*a1*/, ScriptReference * /*a2*/);  // 15: 0x4025a0
	};
	// GLOBAL: 0x5d0350 Script::ScriptActor::ExposeOrigin::vftable
	class ExposeOrigin : public PositionMeasurement {   // normal build: the game's 27-slot base
	public:
		void *GetMembers(int *count);   // 7: 0x438790
		Vec3 *GetVector(ScriptReference *owner);
		const char *GetName();
		// GLOBAL: 0x5d08d8 Script::ScriptActor::ExposeOrigin::ExposeHeading::vftable
		class ExposeHeading : public Measurement {
		public:
			float GetFloat(ScriptReference *owner);
			const char *GetName();                                  // 0: 0x430b70
			ScriptReference *GetType();                             // 4: 0x403e90
			ScriptReference *GetBaseType();                         // 5: 0x436a50
			int GetKind();                                          // 6: 0x43e6f0
			int GetMax();                                           // 11: 0x43e6f0
			int GetValueType();                                     // 12: 0x4034a0
			void SetValue(float /*a1*/, ScriptReference * /*a2*/);  // 15: 0x4025a0
		};
		// GLOBAL: 0x5d0890 Script::ScriptActor::ExposeOrigin::ExposePitch::vftable
		class ExposePitch : public Measurement {
		public:
			float GetFloat(ScriptReference *owner);
			const char *GetName();                                  // 0: 0x430b80
			ScriptReference *GetType();                             // 4: 0x403e90
			ScriptReference *GetBaseType();                         // 5: 0x436a50
			int GetKind();                                          // 6: 0x43e6f0
			int GetMax();                                           // 11: 0x43e6f0
			int GetValueType();                                     // 12: 0x4034a0
			void SetValue(float /*a1*/, ScriptReference * /*a2*/);  // 15: 0x4025a0
		};
		// GLOBAL: 0x5d0800 Script::ScriptActor::ExposeOrigin::ExposeVertComponent::vftable
		class ExposeVertComponent : public Measurement {
		public:
			float GetFloat(ScriptReference *owner);
			char const *GetName();                                  // 0: 0x403ec0
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetKind();                                          // 6: 0x43e6f0
			int GetValueType();                                     // 12: 0x4034a0
			void SetValue(float /*a1*/, ScriptReference * /*a2*/);  // 15: 0x4025a0
		};
		// GLOBAL: 0x5d0848 Script::ScriptActor::ExposeOrigin::ExposeHorzComponent::vftable
		class ExposeHorzComponent : public Measurement {
		public:
			float GetFloat(ScriptReference *owner);
			char const *GetName();                                  // 0: 0x403eb0
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetKind();                                          // 6: 0x43e6f0
			int GetValueType();                                     // 12: 0x4034a0
			void SetValue(float /*a1*/, ScriptReference * /*a2*/);  // 15: 0x4025a0
		};
		ScriptReference *GetType();                             // 4: 0x4039f0
		ScriptReference *GetBaseType();                         // 5: 0x4039a0
	};
	// GLOBAL: 0x5d03c8 Script::ScriptActor::ExposeDestination::vftable
	class ExposeDestination : public PositionMeasurement {   // normal build: the game's 27-slot base
	public:
		void *GetMembers(int *count);   // 7: 0x438840
		Vec3 *GetVector(ScriptReference *owner);
		const char *GetName();
		void SetPosition(const Vec3 *v, ScriptReference *owner);
		void SetVector(const Vec3 *v, ScriptReference *owner);   // 18: 0x42ae10 (the game's name is SetPosition)
		// "vertical distance" (vtable 0x5d09f8, GetFloat 0x42af40)
		class ExposeVertComponent : public Measurement {
		public:
			ExposeVertComponent();
			const char *GetName();
			ScriptReference *GetType();
			ScriptReference *GetBaseType();
			int GetKind();
			int GetValueType();
			float GetFloat(ScriptReference *owner);
			void SetValue(float value, ScriptReference *owner);
		};
		// GLOBAL: 0x5d09b0 Script::ScriptActor::ExposeDestination::ExposeHeading::vftable
		class ExposeHeading : public Measurement {
		public:
			float GetFloat(ScriptReference *owner);
			const char *GetName();                                  // 0: 0x430b70
			ScriptReference *GetType();                             // 4: 0x403e90
			ScriptReference *GetBaseType();                         // 5: 0x436a50
			int GetKind();                                          // 6: 0x43e6f0
			int GetMax();                                           // 11: 0x43e6f0
			int GetValueType();                                     // 12: 0x4034a0
			void SetValue(float /*a1*/, ScriptReference * /*a2*/);  // 15: 0x4025a0
		};
		// GLOBAL: 0x5d0968 Script::ScriptActor::ExposeDestination::ExposePitch::vftable
		class ExposePitch : public Measurement {
		public:
			float GetFloat(ScriptReference *owner);
			const char *GetName();                                  // 0: 0x430b80
			ScriptReference *GetType();                             // 4: 0x403e90
			ScriptReference *GetBaseType();                         // 5: 0x436a50
			int GetKind();                                          // 6: 0x43e6f0
			int GetMax();                                           // 11: 0x43e6f0
			int GetValueType();                                     // 12: 0x4034a0
			void SetValue(float /*a1*/, ScriptReference * /*a2*/);  // 15: 0x4025a0
		};
		// GLOBAL: 0x5d0920 Script::ScriptActor::ExposeDestination::ExposeHorzComponent::vftable
		class ExposeHorzComponent : public Measurement {
		public:
			float GetFloat(ScriptReference *owner);
			char const *GetName();                                  // 0: 0x403eb0
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetKind();                                          // 6: 0x43e6f0
			int GetValueType();                                     // 12: 0x4034a0
			void SetValue(float /*a1*/, ScriptReference * /*a2*/);  // 15: 0x4025a0
		};
		ScriptReference *GetType();                             // 4: 0x4039f0
		ScriptReference *GetBaseType();                         // 5: 0x4039a0
	};
	// GLOBAL: 0x5cf020 Script::ScriptActor::ExposeSpeed::vftable
	class ExposeSpeed : public VectorMeasurement {
	public:
		void *GetMembers(int *count);   // 7: 0x4382f0
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		Vec3 *GetVector(ScriptReference *owner);
		void SetVector(const Vec3 *v, ScriptReference *owner);
		// GLOBAL: 0x5d06e0 Script::ScriptActor::ExposeSpeed::ExposeVertComponent::vftable
		class ExposeVertComponent : public Measurement {
		public:
			float GetFloat(ScriptReference *owner);
			char const *GetName();                                  // 0: 0x403ec0
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetKind();                                          // 6: 0x43e6f0
			int GetValueType();                                     // 12: 0x4034a0
			void SetValue(float /*a1*/, ScriptReference * /*a2*/);  // 15: 0x4025a0
		};
		// GLOBAL: 0x5d0728 Script::ScriptActor::ExposeSpeed::ExposeHorzComponent::vftable
		class ExposeHorzComponent : public Measurement {
		public:
			float GetFloat(ScriptReference *owner);
			char const *GetName();                                  // 0: 0x403eb0
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetKind();                                          // 6: 0x43e6f0
			int GetValueType();                                     // 12: 0x4034a0
			void SetValue(float /*a1*/, ScriptReference * /*a2*/);  // 15: 0x4025a0
		};
		// GLOBAL: 0x5d0770 Script::ScriptActor::ExposeSpeed::ExposePitch::vftable
		class ExposePitch : public Measurement {
		public:
			float GetFloat(ScriptReference *owner);
			char const *GetName();                                  // 0: 0x43dbf0
			ScriptReference *GetType();                             // 4: 0x403e90
			ScriptReference *GetBaseType();                         // 5: 0x436a50
			int GetKind();                                          // 6: 0x43e6f0
			int GetMax();                                           // 11: 0x43e6f0
			int GetValueType();                                     // 12: 0x4034a0
			void SetValue(float /*a1*/, ScriptReference * /*a2*/);  // 15: 0x4025a0
		};
		ScriptReference *GetBaseType();                         // 5: 0x4039a0
	};
	// GLOBAL: 0x5cf080 Script::ScriptActor::ExposeAltitude::vftable
	class ExposeAltitude : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
		void SetValue(float /*a1*/, ScriptReference * /*a2*/);  // 15: 0x4025a0
	};
	// GLOBAL: 0x5cfa90 Script::ScriptActor::ExposeConeLength::vftable
	class ExposeConeLength : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d0540 Script::ScriptActor::ExposeSweepAngle::vftable
	class ExposeSweepAngle : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x403e90
		ScriptReference *GetBaseType();                         // 5: 0x436a50
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
	};
	class ExposeWayPoints : public Measurement {
	public:
		const char *GetName();
		ScriptReference *Deref(ScriptReference *owner, int flags);
	};
	class ExposeAttachmentPoints : public Measurement {
	public:
		const char *GetName();
		ScriptReference *Deref(ScriptReference *owner, int flags);
	};
	class ExposeTurrets : public Measurement {
	public:
		const char *GetName();
		ScriptReference *Deref(ScriptReference *owner, int flags);
	};
	// The set of the actor's clones (vtable 0x5cf1e8; the member row names it as a vptr-only slot, zoosters::VptrOnly).
	// GLOBAL: 0x5cf1e8 Script::ScriptActor::ExposeClones::vftable
	class ExposeClones : public ReferenceSet {
	public:
		const char *GetName();                                           // 0: 0x416480
		ScriptReference *GetBaseType();                                  // 5: 0x43aa60
		ScriptReference *Deref(ScriptReference *owner, int flags);       // 9: 0x43b150
	};
	// GLOBAL: 0x5cf288 Script::ScriptActor::ExposeGroundTurnRate::vftable
	class ExposeGroundTurnRate : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5cf438 Script::ScriptActor::ExposeAirTurnRate::vftable
	class ExposeAirTurnRate : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5cf4e8 Script::ScriptActor::ExposeAirDecel::vftable
	class ExposeAirDecel : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5cf590 Script::ScriptActor::ExposeInitialJumpVelocity::vftable
	class ExposeInitialJumpVelocity : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5cf5f0 Script::ScriptActor::ExposeCoastFrames::vftable
	class ExposeCoastFrames : public Measurement {
	public:
		const char *GetName();
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
	};
	// GLOBAL: 0x5cf648 Script::ScriptActor::ExposeUpSpeed::vftable
	class ExposeUpSpeed : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5cf6a0 Script::ScriptActor::ExposeUpAccel::vftable
	class ExposeUpAccel : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5cf6f8 Script::ScriptActor::ExposeDownSpeed::vftable
	class ExposeDownSpeed : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5cf750 Script::ScriptActor::ExposeDownAccel::vftable
	class ExposeDownAccel : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5cf858 Script::ScriptActor::ExposeBounceRestitution::vftable
	class ExposeBounceRestitution : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5cfae0 Script::ScriptActor::ExposeBlipColor::vftable
	class ExposeBlipColor : public Measurement {
	public:
		void *GetMembers(int *count);   // 7: 0x4367a0
		const char *GetName();
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		// GLOBAL: 0x5cffe0 Script::ScriptActor::ExposeBlipColor::ExposeB::vftable
		class ExposeB : public Measurement {
		public:
			void SetValue(int value, ScriptReference *owner);
			int GetInt(ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436940
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5d0028 Script::ScriptActor::ExposeBlipColor::ExposeG::vftable
		class ExposeG : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436930
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5d0070 Script::ScriptActor::ExposeBlipColor::ExposeR::vftable
		class ExposeR : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436920
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5cff98 Script::ScriptActor::ExposeBlipColor::ExposeA::vftable
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
	// GLOBAL: 0x5cfc30 Script::ScriptActor::ExposeVisibleOnRadar::vftable
	class ExposeVisibleOnRadar : public Measurement {
	public:
		const char *GetName();
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43efe0
	};
	// GLOBAL: 0x5cfd38 Script::ScriptActor::ExposeIgnoresGroundColor::vftable
	class ExposeIgnoresGroundColor : public Measurement {
	public:
		const char *GetName();
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43efe0
	};
	// GLOBAL: 0x5cfd98 Script::ScriptActor::ExposeGroundTilter::vftable
	class ExposeGroundTilter : public Measurement {
	public:
		const char *GetName();
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43efe0
	};
	// GLOBAL: 0x5cfdf0 Script::ScriptActor::ExposeNoShadow::vftable
	class ExposeNoShadow : public Measurement {
	public:
		const char *GetName();
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43efe0
	};
	// GLOBAL: 0x5cfee0 Script::ScriptActor::ExposeMeshCollidee::vftable
	class ExposeMeshCollidee : public Measurement {
	public:
		const char *GetName();
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43efe0
	};
	// GLOBAL: 0x5d0440 Script::ScriptActor::ExposeHeadingObsolete::vftable
	class ExposeHeadingObsolete : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x403e90
		ScriptReference *GetBaseType();                         // 5: 0x436a50
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
		void SetValue(float /*a1*/, ScriptReference * /*a2*/);  // 15: 0x4025a0
	};
	// the heading member of ExposeSpeed's member table (GetName 0x403ea0 = ExposeHeading_GetName), otherwise ExposeHeadingObsolete
	// GLOBAL: 0x5d07b8 Script::ScriptActor::ExposeSpeedHeading::vftable
	class ExposeSpeedHeading : public ExposeHeadingObsolete {
	public:
		char const *GetName();                                  // 0: 0x403ea0
		ScriptReference *GetType();                             // 4: 0x403e90
		ScriptReference *GetBaseType();                         // 5: 0x436a50
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
		void SetValue(float /*a1*/, ScriptReference * /*a2*/);  // 15: 0x4025a0
	};
};
ZOOSTERS_ASSERT_SIZE(ScriptActor, 0x98);
ZOOSTERS_ASSERT_OFFSET(ScriptActor, m_unk40, 0x40);
ZOOSTERS_ASSERT_OFFSET(ScriptActor, m_controlMode, 0x4c);
ZOOSTERS_ASSERT_OFFSET(ScriptActor, m_wayPoints, 0x50);
ZOOSTERS_ASSERT_OFFSET(ScriptActor, m_attachmentPoints, 0x68);
ZOOSTERS_ASSERT_OFFSET(ScriptActor, m_turrets, 0x80);

}  // namespace Script

#endif
