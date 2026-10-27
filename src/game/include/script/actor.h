// Script placement/actor objects: what op targets (actors, sprites, lights, ...) resolve to.
// Hierarchy (PS2 RTTI): Measurement > IntMeasurement > FloatMeasurement > VectorMeasurement (0x5ca6b8, 19 slots) >
// PositionMeasurement (0x5ca760, 27) > Placement (0x5ca7d8, 40) > ScriptActor/ScriptSprite/... > GameActor/...
// IntMeasurement/FloatMeasurement add no slots and are left out here.
#ifndef ZOOSTERS_SCRIPT_ACTOR_H
#define ZOOSTERS_SCRIPT_ACTOR_H

#include "script/value.h"
#include "script/referenceset.h"

namespace Script {

struct ScriptValue;   // script/runtime.h

class VectorMeasurement : public Measurement {
public:
	const char *GetName();    // 0: 0x4034b0
	ScriptReference *GetType();    // 4: 0x4039a0
	ScriptReference *GetBaseType();    // 5: 0x436a50
	int GetKind();    // 6: 0x43e6f0
	void *GetMembers(int *count);    // 7: 0x403e10
	int GetValueType();    // 12: 0x4034a0
	float GetFloat(ScriptReference *owner);    // 14: 0x41aa20
	void SetValue(float value, ScriptReference *owner);    // 15: 0x4025a0
	virtual Vec3 *GetVector(ScriptReference *owner);                   // 17: default 0x4039d0
	virtual void SetVector(const Vec3 *v, ScriptReference *owner);     // 18: default does nothing
	// bearings of the owner relative to the current script actor (0x440820-0x440990)
	// GLOBAL: 0x5cace0 Script::VectorMeasurement::ExposeHeading::vftable
	class ExposeHeading : public Measurement {
	public:
		ExposeHeading();
		const char *GetName();
		ScriptReference *GetType();
		ScriptReference *GetBaseType();
		int GetKind();
		int GetMax();
		int GetValueType();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);   // game slot 15 = 0x4025a0 (empty), not Measurement's 0x407ab0
	};
	// GLOBAL: 0x5cac98 Script::VectorMeasurement::ExposePitch::vftable
	class ExposePitch : public Measurement {
	public:
		ExposePitch();
		const char *GetName();
		ScriptReference *GetType();
		ScriptReference *GetBaseType();
		int GetKind();
		int GetMax();
		int GetValueType();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);   // game slot 15 = 0x4025a0 (empty), not Measurement's 0x407ab0
	};
	// GLOBAL: 0x5cac50 Script::VectorMeasurement::ExposeHorzComponent::vftable
	class ExposeHorzComponent : public Measurement {
	public:
		ExposeHorzComponent();
		const char *GetName();
		ScriptReference *GetType();
		ScriptReference *GetBaseType();
		int GetKind();
		int GetValueType();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);   // game slot 15 = 0x4025a0 (empty), not Measurement's 0x407ab0
	};
	// GLOBAL: 0x5cac08 Script::VectorMeasurement::ExposeVertComponent::vftable
	class ExposeVertComponent : public Measurement {
	public:
		ExposeVertComponent();
		const char *GetName();
		ScriptReference *GetType();
		ScriptReference *GetBaseType();
		int GetKind();
		int GetValueType();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);   // game slot 15 = 0x4025a0 (empty), not Measurement's 0x407ab0
	};
};

class PositionMeasurement : public VectorMeasurement {
public:
	const char *GetName();    // 0: 0x403990
	ScriptReference *GetType();    // 4: 0x4039f0
	ScriptReference *GetBaseType();    // 5: 0x4039a0
	void *GetMembers(int *count);    // 7: 0x430ca0
	float GetFloat(ScriptReference *owner);    // 14: 0x42f770
	virtual float GetFacing();                                         // 19: default 0x4039b0
	virtual bool SetFacing(float v, int flag);  // 20: ActorTurnTo/TeleportTo
	virtual float GetTilt();                                           // 21
	virtual bool SetTilt(float v, ScriptReference *owner);             // 22: default 0x4039c0 (shared with SetFacing/SetRoll) returns true
	virtual float GetRoll();                                           // 23
	virtual bool SetRoll(float v, ScriptReference *owner);             // 24: as SetTilt
	virtual Vec3 *GetVelocity(ScriptReference *owner);  // 25
	virtual intptr_t vf26(intptr_t);                                            // 26: default returns 0
	// 0x409700 (thiscall, PlayerObj::Dist): squared distance of the closest approach of this and `other` within the next tick (1/30 s),
	// the measure the particle shield and the actor wake-up compare with their range
	float Dist(ScriptReference *owner, PositionMeasurement *other, ScriptReference *otherOwner);
	// small_c: the script-visible members Facing/Tilt/Roll of a PositionMeasurement owner (GetFloat/SetValue call slots 19-24)
	// GLOBAL: 0x5cdec0 Script::PositionMeasurement::ExposeViewPyramidDist::vftable
	class ExposeViewPyramidDist : public Measurement {   // 0x41faf0
	public:
		float GetFloat(ScriptReference *owner);
		const char *GetName();                                  // 0: 0x42feb0
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
		void SetValue(float /*a1*/, ScriptReference * /*a2*/);  // 15: 0x4025a0
	};
	// GLOBAL: 0x5cdfc8 Script::PositionMeasurement::ExposeFacing::vftable
	class ExposeFacing : public Measurement {
	public:
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		const char *GetName();                                  // 0: 0x430340
		ScriptReference *GetType();                             // 4: 0x403e90
		ScriptReference *GetBaseType();                         // 5: 0x436a50
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5ce018 Script::PositionMeasurement::ExposeTilt::vftable
	class ExposeTilt : public Measurement {
	public:
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		const char *GetName();                                  // 0: 0x430380
		ScriptReference *GetType();                             // 4: 0x403e90
		ScriptReference *GetBaseType();                         // 5: 0x436a50
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5ce068 Script::PositionMeasurement::ExposeRoll::vftable
	class ExposeRoll : public Measurement {
	public:
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		const char *GetName();                                  // 0: 0x439170
		ScriptReference *GetType();                             // 4: 0x403e90
		ScriptReference *GetBaseType();                         // 5: 0x436a50
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// Normal build only (an unused declaration here moves MenuPointer_Update's code in the cl 13.10 build): the heading / pitch members
	// are vector members with their own names (0x430b70 / 0x430b80); their other 17 slots are the vector member's.
	// GLOBAL: 0x5ce0c0 Script::PositionMeasurement::ExposeHeading::vftable
	class ExposeHeading : public VectorMeasurement::ExposeHeading {
	public:
		const char *GetName();   // 0: 0x430b70
	};
	// GLOBAL: 0x5ce118 Script::PositionMeasurement::ExposePitch::vftable
	class ExposePitch : public VectorMeasurement::ExposePitch {
	public:
		const char *GetName();   // 0: 0x430b80
	};
};

// The base of every script object placed in the level (PS2 RTTI): an id and two sets. 0x40 bytes;
// derived classes inline the constructor (ScriptProp 0x41ca40, GameSprite 0x415c90, ...).
// Selects the constructor a type prototype (the function-local static of a TFBType_* getter) is built with.
enum PrototypeInit { PROTOTYPE };

// GLOBAL: 0x5ca710 Script::Placement::PlacementSet::vftable
// The actor's set cursor fields (script/ops_actor.h SetCursor) after PlacementSet::m_1c: the first member of the Placement
// subclasses (+0x40). 12 bytes on x86; on x64 the cursor stays contiguous because m_1c is pointer-sized. Normal builds only: cl
// 13.10 keeps the byte arrays, and an unused declaration here moves MenuPointer_Update's code.
struct SetCursorTail {
	SetNode *m_node;
	int m_kind;
	int m_stamp;
};

class Placement : public PositionMeasurement {
public:
	// The second set (PS2 RTTI Placement::PlacementSet, vtable 0x5ca710): a ReferenceSet that knows its placement.
	class PlacementSet : public ReferenceSet {
	public:
		inline PlacementSet() { m_1c = 0; }
		ScriptReference *GetBaseType();   // 5: 0x4034c0 the placement's type (m_owner->GetType); slot 4 GetType stays the set's TFBType_Set
		Placement *m_owner;
		intptr_t m_1c;
		                       // +0x1c (+0x3c in the placement) cleared by ObjectRemove / ObjectReset::Execute; the set pointer
		                       // of the actor's set cursor (script/ops_actor.h SetCursor), whose other fields follow the
		                       // placement (SetCursorTail)
	};

	inline Placement() : m_id(-1), m_08(), m_20() { m_20.m_owner = this; }
	~Placement();                                                      // 0x402c00 (scalar deleting destructor 0x403a30)
	ScriptReference *GetType();    // 4: 0x4039E0
	ScriptReference *GetBaseType();    // 5: 0x4039F0
	int m_id;  // -1 until placed
	ReferenceSet m_08;
	PlacementSet m_20;
	virtual void Remove();                                             // 27: ObjectRemove::Execute
	virtual void Reset();                                              // 28: ObjectReset::Execute
	virtual char Control(bool);                                        // 29: ObjectControl::Execute (al = ok)
	virtual intptr_t Spawn(intptr_t, int);                                    // 30: ObjectSpawn::Execute
	virtual ScriptReference *GetSpawnSource();  // 31
	virtual int IsSolidCollider();                                     // 32
	virtual int IsMeshCollider();                                      // 33
	virtual int IsMeshCollidee();                                      // 34
	virtual bool PlayAnimation(int);                                  // 35: MoveTo/MoveFrom/TurnTo/PlayAnim
	virtual int AccessBehavior(int index, int value);  // 36: index -1 = count
	virtual void SetDestination(Vec3 *pos, int flag);                  // 37: MoveTo/MoveFrom/TeleportTo
	virtual void SetDestinationTimed(Vec3 *pos, float seconds);  // 38
	virtual void SetAttachTarget(ScriptReference *target);  // 39
	void SetControlMode(int, int);                                       // 0x42e530 (non-virtual, ret 8): ActorRunAsPlayer::Execute
	// The actor's message queue is m_08 (the messages it has posted; the global sender set holds the placements that have any).
	void PostMessage(ScriptReference *msgType, ScriptReference *receiver, Vec3 *pos, float arg);  // 0x42f660 (ret 0x10): queues a message (type, receiver, position, float; -1 = update this tick's identical one)
	intptr_t FindMessage(ScriptReference *msgType, ScriptValue *value, float range, Vec3 *out);  // 0x42f260 (ret 0x10): MessageCheck's range test (message type, value, range, out position); returns the queued message or 0
	ScriptReference *FinishSpawn(intptr_t pos, float facing, int flag);  // 0x431be0 (ret 0xc): spawns at pos (ObjectSpawn::Execute), files the new object in m_20, sets its facing
};
ZOOSTERS_ASSERT_SIZE(Placement, 0x40);
ZOOSTERS_ASSERT_OFFSET(Placement, m_20, 0x20);
// The splits that reach these through raw offsets in the game build (ops_actor.h ActorCursor, cutscene_reset / gameactor.cpp set walks):
ZOOSTERS_ASSERT_OFFSET(Placement, m_id, 0x04);
ZOOSTERS_ASSERT_OFFSET(Placement, m_08, 0x08);
ZOOSTERS_ASSERT_OFFSET_AT(PlacementSet_owner, offsetof(Placement, m_20) + offsetof(Placement::PlacementSet, m_owner), 0x38);
ZOOSTERS_ASSERT_OFFSET_AT(PlacementSet_1c, offsetof(Placement, m_20) + offsetof(Placement::PlacementSet, m_1c), 0x3c);   // SetCursor::m_set (actor+0x3c)
ZOOSTERS_ASSERT_SIZE(SetCursorTail, 0xc);   // the byte arrays `m_unk40[0xc]` it replaces (ScriptActor, ScriptRumble, GameLight, GameParticle at +0x40)

}  // namespace Script

#endif
