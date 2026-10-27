// Script::ScriptCamera (PS2 RTTI; base ScriptReference), the "camera" script object: two PointerTarget slots ("look at" / "look from")
// and the Expose* members (field of view, roll, laziness, pitch control, follow settings, read-only status values). One per CameraData
// (inlined in CameraData::CameraData 0x41d7c0, 0x54 bytes, vtable 0x5ccfec; destructors 0x41d8b0 / 0x41d890 are in rws/cameradata_vt.cpp).
// The type prototype (TFBType_Camera 0x4389d0) is a second instance whose constructor sets the first 0x3c bytes only. Members work on
// the object GetCameraObject 0x438af0 returns (compared with the owner for the read-only values), offset members on the owner's fields.
// Vtable (11 slots): 0 GetName, 2 AdjustRefCount, 3 ~ScriptCamera, 4 GetType, 7 GetMembers, 10 below; 5 is the default 0x43e6f0,
// 6 the object default 0x42cbe0.
#ifndef ZOOSTERS_SCRIPT_TYPES_SCRIPTCAMERA_H
#define ZOOSTERS_SCRIPT_TYPES_SCRIPTCAMERA_H

#include "zoosters/base.h"
#include "script/value.h"
#include "script/types/pointertarget.h"
#include "script/actor.h"   // PositionMeasurement: the base of ExposeCameraLocation in a normal build

void *MemAlloc(size_t n); // 0x550170

namespace Script {

// Base of ScriptCamera holding the two fields the game's constructor stores before the vptr (+4 back pointer to the owner's pointer
// member, +8 reference count); inferred from the store order of the inlined constructor.
// NOVTABLE: Script::ScriptCameraBase is declared in rws/cameradata_vt.cpp (the TU that emits ScriptCamera's vtable).
class ScriptCameraBase : public ScriptReference {
public:
	ScriptCameraBase() : m_ownerSlot(0), m_refCount(0) {}
	void *m_ownerSlot;  // back pointer to CameraData::m_cam (0 after its destructor)
	int m_refCount;  // reference count (AdjustRefCount 0x41d490)
};

class ScriptCamera : public ScriptCameraBase { // "camera"
public:
	static void *operator new(size_t n) { return MemAlloc(n); }
	// Inline constructor (the game inlines it into CameraData::CameraData 0x41d7c0).
	ScriptCamera() : m_roll(0), m_flags(0), m_vertFollowAt(0), m_vertFollowFrom(0), m_horzFollow(30) {}
	int AdjustRefCount(int delta);                                     // 2: 0x41d490
	void *GetMembers(int *count);                                      // 7: 0x439200
	void vf10();                                                       // 10: 0x438b10 (Reset: the prototype's reset)
	~ScriptCamera();                                                   // 3: 0x41d890 scalar deleting destructor, body 0x41d8b0
	const char *GetName();                                             // 0: 0x41d6f0
	ScriptReference *GetType();                                        // 4: 0x41d700
	PointerTarget m_lookAt;  // look-at target (ExposeTgt*; default object 0x61ff68)
	PointerTarget m_lookFrom;  // look-from target = camera position (ExposePos*; default object 0x61ff6c)
	float m_fov;  // field of view in degrees (ExposeFOV); not stored by the constructor
	float m_roll;  // roll in degrees (ExposeRoll)
	unsigned m_flags;  // bit 0 lazy, bit 1 pitch control inverted (ExposeLazy / ExposePitchControl); unsigned: ExposePitchControl::GetInt 0x438d60 shifts with shr
	int m_vertFollowAt;  // ExposeVertFollowAt
	int m_vertFollowFrom;  // ExposeVertFollowFrom
	int m_horzFollow;  // ExposeHorzFollow: percent (* 0.01 = UpdateGameCamera's follow lerp factor)
	// GLOBAL: 0x5d0a88 Script::ScriptCamera::ExposePosPointer::vftable
	class ExposePosPointer : public ScriptReference { // "look from", vtable 0x5d0a88 (11 slots: a reference, not a measurement; slot 9 Deref 0x4391c0 returns the resolved actor of m_lookFrom)
	public:
		const char *GetName(); // 0: 0x438940
		ScriptReference *GetType();                                      // 4: 0x438950
		ScriptReference *GetBaseType();                                  // 5: 0x43aa60
		int GetKind();                                                   // 6: 0x42def0
		void SetReferent(ScriptReference *value, ScriptReference *owner); // 8: 0x438db0 (CamPosPointerSetW::SetValue, script/pointerslots.cpp)
		ScriptReference *Deref(ScriptReference *owner, int flags);       // 9: 0x4391c0 (CameraLookFromPointer::Deref)
	};
	// GLOBAL: 0x5d0ac0 Script::ScriptCamera::ExposeTgtPointer::vftable
	class ExposeTgtPointer : public ExposePosPointer { // "look at", vtable 0x5d0ac0: the same slots as ExposePosPointer, the look-at pointer (m_lookAt)
	public:
		const char *GetName();                                           // 0: 0x438960
		void SetReferent(ScriptReference *value, ScriptReference *owner); // 8: 0x438e40 (CamTgtPointerSetW::SetValue)
		ScriptReference *Deref(ScriptReference *owner, int flags);       // 9: 0x4391e0 (CameraLookAtPointer::Deref)
	};
	// GLOBAL: 0x5d0af0 Script::ScriptCamera::ExposePosHorzOffs::vftable
	class ExposePosHorzOffs : public Measurement { // "look from horizontal offset", vtable 0x5d0af0
	public:
		const char *GetName();                              // 0: 0x438ba0
		float GetFloat(ScriptReference *owner);             // 14: 0x438bb0
		void SetValue(float value, ScriptReference *owner); // 15: 0x438bc0
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d0b50 Script::ScriptCamera::ExposePosVertOffs::vftable
	class ExposePosVertOffs : public Measurement { // "look from vertical offset", vtable 0x5d0b50
	public:
		const char *GetName();                              // 0: 0x438bd0
		float GetFloat(ScriptReference *owner);             // 14: 0x438be0
		void SetValue(float value, ScriptReference *owner); // 15: 0x438bf0
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d0bb0 Script::ScriptCamera::ExposePosHeadingRel::vftable
	class ExposePosHeadingRel : public Measurement { // "look from heading relative", vtable 0x5d0bb0
	public:
		const char *GetName();                            // 0: 0x438c00
		int GetInt(ScriptReference *owner);               // 13: 0x438c10
		void SetValue(int value, ScriptReference *owner); // 16: 0x438c20
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43efe0
	};
	// GLOBAL: 0x5d0c10 Script::ScriptCamera::ExposeTgtHorzOffs::vftable
	class ExposeTgtHorzOffs : public Measurement { // "look at horizontal offset", vtable 0x5d0c10
	public:
		float GetFloat(ScriptReference *owner);             // 14: 0x438c50
		void SetValue(float value, ScriptReference *owner); // 15: 0x438c60
		char const *GetName();                                  // 0: 0x438c40
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d0c58 Script::ScriptCamera::ExposeTgtVertOffs::vftable
	class ExposeTgtVertOffs : public Measurement { // "look at vertical offset", vtable 0x5d0c58
	public:
		float GetFloat(ScriptReference *owner);             // 14: 0x438c80
		void SetValue(float value, ScriptReference *owner); // 15: 0x438c90
		char const *GetName();                                  // 0: 0x438c70
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d0ca0 Script::ScriptCamera::ExposeTgtHeadingRel::vftable
	class ExposeTgtHeadingRel : public Measurement { // "look at heading relative", vtable 0x5d0ca0
	public:
		int GetInt(ScriptReference *owner);               // 13: 0x438cb0
		void SetValue(int value, ScriptReference *owner); // 16: 0x438cc0
		char const *GetName();                                  // 0: 0x438ca0
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43efe0
	};
	// GLOBAL: 0x5d0ce8 Script::ScriptCamera::ExposeLazy::vftable
	class ExposeLazy : public Measurement { // "laziness", vtable 0x5d0ce8
	public:
		const char *GetName();                            // 0: 0x438ce0
		int GetInt(ScriptReference *owner);               // 13: 0x438cf0
		void SetValue(int value, ScriptReference *owner); // 16: 0x438d00
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43efe0
	};
	// GLOBAL: 0x5d0d38 Script::ScriptCamera::ExposePercentComplete::vftable
	class ExposePercentComplete : public Measurement { // "percent complete", vtable 0x5d0d38
	public:
		int GetInt(ScriptReference *owner); // 13: 0x41d570
		const char *GetName();                                  // 0: 0x438d30
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x42fbc0
		void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
	};
	// GLOBAL: 0x5d0d90 Script::ScriptCamera::ExposeTimeRemaining::vftable
	class ExposeTimeRemaining : public Measurement { // "time remaining", vtable 0x5d0d90
	public:
		float GetFloat(ScriptReference *owner); // 14: 0x41d5a0
		char const *GetName();                                  // 0: 0x438d40
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
		void SetValue(float /*a1*/, ScriptReference * /*a2*/);  // 15: 0x4025a0
	};
	// GLOBAL: 0x5d0dd8 Script::ScriptCamera::ExposePitchControl::vftable
	class ExposePitchControl : public Measurement { // "pitch control", vtable 0x5d0dd8
	public:
		const char *GetName();                            // 0: 0x438d50
		int GetInt(ScriptReference *owner);               // 13: 0x438d60
		void SetValue(int value, ScriptReference *owner); // 16: 0x438d80
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43efe0
	};
	// GLOBAL: 0x5d0e68 Script::ScriptCamera::ExposePosHeadingOffs::vftable
	class ExposePosHeadingOffs : public Measurement { // "look from heading offset", vtable 0x5d0e68
	public:
		const char *GetName();                              // 0: 0x439080
		float GetFloat(ScriptReference *owner);             // 14: 0x439090
		void SetValue(float value, ScriptReference *owner); // 15: 0x4390a0
		ScriptReference *GetType();                             // 4: 0x403e90
		ScriptReference *GetBaseType();                         // 5: 0x436a50
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d0ec8 Script::ScriptCamera::ExposeTgtHeadingOffs::vftable
	class ExposeTgtHeadingOffs : public Measurement { // "look at heading offset", vtable 0x5d0ec8
	public:
		float GetFloat(ScriptReference *owner);             // 14: 0x4390c0
		void SetValue(float value, ScriptReference *owner); // 15: 0x4390d0
		char const *GetName();                                  // 0: 0x4390b0
		ScriptReference *GetType();                             // 4: 0x403e90
		ScriptReference *GetBaseType();                         // 5: 0x436a50
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d0f10 Script::ScriptCamera::ExposeFOV::vftable
	class ExposeFOV : public Measurement { // "field of view", vtable 0x5d0f10
	public:
		float GetFloat(ScriptReference *owner);             // 14: 0x439100
		void SetValue(float value, ScriptReference *owner); // 15: 0x439110
		char const *GetName();                                  // 0: 0x4390e0
		ScriptReference *GetType();                             // 4: 0x403e90
		ScriptReference *GetBaseType();                         // 5: 0x436a50
		int GetMax();                                           // 11: 0x4390f0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d0f58 Script::ScriptCamera::ExposeVertFollowAt::vftable
	class ExposeVertFollowAt : public Measurement { // "Vertical Follow At", vtable 0x5d0f58
	public:
		const char *GetName();                            // 0: 0x439120
		int GetInt(ScriptReference *owner);               // 13: 0x41d5c0
		void SetValue(int value, ScriptReference *owner); // 16: 0x41d5e0
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x439160
	};
	// GLOBAL: 0x5d0fb0 Script::ScriptCamera::ExposeHorzFollow::vftable
	class ExposeHorzFollow : public Measurement { // "Horizontal Follow", vtable 0x5d0fb0
	public:
		const char *GetName();                            // 0: 0x439130
		int GetInt(ScriptReference *owner);               // 13: 0x41d640
		void SetValue(int value, ScriptReference *owner); // 16: 0x41d660
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x42fbc0
	};
	// GLOBAL: 0x5d1008 Script::ScriptCamera::ExposeFacing::vftable
	class ExposeFacing : public Measurement { // "Current Facing", vtable 0x5d1008
	public:
		const char *GetName();                              // 0: 0x439140
		float GetFloat(ScriptReference *owner);             // 14: 0x41d680
		void SetValue(float value, ScriptReference *owner); // 15: 0x41d6a0
		ScriptReference *GetType();                             // 4: 0x403e90
		ScriptReference *GetBaseType();                         // 5: 0x436a50
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5d1060 Script::ScriptCamera::ExposeVertFollowFrom::vftable
	class ExposeVertFollowFrom : public Measurement { // "Vertical Follow From", vtable 0x5d1060
	public:
		const char *GetName();                            // 0: 0x439150
		int GetInt(ScriptReference *owner);               // 13: 0x41d600
		void SetValue(int value, ScriptReference *owner); // 16: 0x41d620
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x439160
	};
	// GLOBAL: 0x5d10c0 Script::ScriptCamera::ExposeRoll::vftable
	class ExposeRoll : public Measurement { // "roll", vtable 0x5d10c0
	public:
		float GetFloat(ScriptReference *owner);             // 14: 0x439180
		void SetValue(float value, ScriptReference *owner); // 15: 0x439190
		const char *GetName();                                  // 0: 0x439170
		ScriptReference *GetType();                             // 4: 0x403e90
		ScriptReference *GetBaseType();                         // 5: 0x436a50
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// NOTE: the game's base is PositionMeasurement (27 slots, GetVector 17 = 0x4391b0 ExposeCameraLocation_GetVector); declared
	// as Measurement with the inherited overrides spelled out because the prototype getter TFBMember_Camera_CameraLocation
	// (script/proto_430aa0_gen.cpp) emits the vtable from this declaration. normal build: the compiler's vtable must be the
	// game's 27 slots, so the real base
	class ExposeCameraLocation : public PositionMeasurement { // "camera position", vtable 0x5d1108
	public:
		Vec3 *GetVector(ScriptReference *owner);            // 17: 0x4391b0 ExposeCameraLocation_GetVector (a free function there)
	public:
		const char *GetName();                              // 0: 0x4391a0
		ScriptReference *GetType();                         // 4: 0x4039f0
		ScriptReference *GetBaseType();                     // 5: 0x4039a0
		int GetKind();                                      // 6: 0x43e6f0
		void *GetMembers(int *count);                       // 7: 0x430ca0
		int GetValueType();                                 // 12: 0x4034a0
		float GetFloat(ScriptReference *owner);             // 14: 0x42f770
		void SetValue(float value, ScriptReference *owner); // 15: 0x4025a0
	};
};
ZOOSTERS_ASSERT_SIZE(ScriptCamera, 0x54);
ZOOSTERS_ASSERT_OFFSET(ScriptCameraBase, m_ownerSlot, 0x04);
ZOOSTERS_ASSERT_OFFSET(ScriptCameraBase, m_refCount, 0x08);
ZOOSTERS_ASSERT_OFFSET(ScriptCamera, m_lookAt, 0x0c);
ZOOSTERS_ASSERT_OFFSET(ScriptCamera, m_lookFrom, 0x24);
ZOOSTERS_ASSERT_OFFSET(ScriptCamera, m_fov, 0x3c);
ZOOSTERS_ASSERT_OFFSET(ScriptCamera, m_roll, 0x40);
ZOOSTERS_ASSERT_OFFSET(ScriptCamera, m_flags, 0x44);
ZOOSTERS_ASSERT_OFFSET(ScriptCamera, m_vertFollowAt, 0x48);
ZOOSTERS_ASSERT_OFFSET(ScriptCamera, m_vertFollowFrom, 0x4c);
ZOOSTERS_ASSERT_OFFSET(ScriptCamera, m_horzFollow, 0x50);

} // namespace Script

Script::ScriptReference *GetCameraObject();   // 0x438af0: the game camera's script object
Script::ScriptReference *TFBType_Camera();    // ScriptCamera prototype getter

#endif
