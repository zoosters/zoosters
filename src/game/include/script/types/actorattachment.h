// Script::ActorAttachment (PS2 RTTI; vtable 0x5cd690, 35 slots), the "attachment" script type: PositionMeasurement's 27 slots
// plus its own 27-34, the local transform relative to the parent frame. Slots 31-34 are defaults here, the turret
// (types/actorturret.h) overrides them. The set at +0x14 (AttacherSet, vtable 0x5cd328) holds the objects attached to it.
// Names of the Expose* members are the game's strings ("local offset", "local facing", "local tilt", "local roll").
#ifndef ZOOSTERS_SCRIPT_TYPES_ACTORATTACHMENT_H
#define ZOOSTERS_SCRIPT_TYPES_ACTORATTACHMENT_H

#include "zoosters/base.h"
#include "script/actor.h"
#include "script/referenceset.h"
#include "script/types/indexedposition.h"

struct RwFrame;

namespace Script {

class ActorAttachment : public IndexedPosition {
public:
	~ActorAttachment();             // 0x42bf20 (clears the attacher set)
	const char *GetName();          // 0: 0x42d640 "attachment"
	ScriptReference *GetType();     // 4: 0x42d650 (bound in script/normal_bringup.cpp)
	ScriptReference *GetBaseType(); // 5: 0x4039f0
	void *GetMembers(int *count);   // 7: 0x42db40
	Vec3 *GetVector(ScriptReference *owner);    // 17: 0x429760 (world position of the owner's local matrix under its frame +0xc8)
	virtual Vec3 *GetLocalOffset(ScriptReference *owner);                    // 27
	virtual float GetLocalFacing();                                          // 28
	virtual float GetLocalTilt();                                            // 29
	virtual float GetLocalRoll();                                            // 30
	virtual void SetLocalOffset(const Vec3 *v, ScriptReference *owner);      // 31
	virtual bool SetLocalFacing(float v);                                    // 32 (returns the dirty flag in the turret)
	virtual bool SetLocalTilt(float v);                                      // 33
	virtual bool SetLocalRoll(float v);                                      // 34

	// Members (offsets from GetVector 0x429760 and the GetLocal* accessors).
	// The set at +0x14 (vtable 0x5cd328; ActorAttachment::AttacherSet is our name); its destructor 0x42be20 is
	// ReferenceSet's (clears the set). Its constructor is not written (stand-in factories only).
	// GLOBAL: 0x5cd328 Script::ActorAttachment::AttacherSet::vftable
	class AttacherSet : public ReferenceSet {
	public:
		~AttacherSet();                 // 0x42be20
		AttacherSet *scalar_deleting_dtor(unsigned flags);   // 0x42ce20 (script/deleting_dtors_2.cpp: the vtable data tables name it)
		const char *GetName();          // 0: 0x42cda0 "attachers"
		ScriptReference *GetBaseType();   // 5: 0x4039e0
		ScriptReference *Deref(ScriptReference *owner, int flags);   // 9: 0x42cdb0 (the set inside the owner)
	};
	AttacherSet m_attachers;
	intptr_t m_2c;
	char m_localMatrix[0x40];  // RwMatrix: local transform under the parent frame
	char m_70[0x40];
	float m_localRoll;
	float m_localTilt;
	float m_localFacing;
	Vec3 m_localOffset;
	RwFrame *m_parentFrame;  // parent frame (GetVector: RwFrameGetLTM)

	// GLOBAL: 0x5cd8a0 Script::ActorAttachment::ExposeLocalOffset::vftable
	class ExposeLocalOffset : public VectorMeasurement {
	public:
		const char *GetName();          // 0: 0x42dc60 "local offset"
		Vec3 *GetVector(ScriptReference *owner);
		void SetVector(const Vec3 *v, ScriptReference *owner);
	};
	// GLOBAL: 0x5cd858 Script::ActorAttachment::ExposeLocalFacing::vftable
	class ExposeLocalFacing : public Measurement {
	public:
		const char *GetName();          // 0: 0x42dc90 "local facing"
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x403e90
		ScriptReference *GetBaseType();                         // 5: 0x436a50
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5cd810 Script::ActorAttachment::ExposeLocalTilt::vftable
	class ExposeLocalTilt : public Measurement {
	public:
		const char *GetName();          // 0: 0x42dcd0 "local tilt"
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x403e90
		ScriptReference *GetBaseType();                         // 5: 0x436a50
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5cd7c8 Script::ActorAttachment::ExposeLocalRoll::vftable
	class ExposeLocalRoll : public Measurement {
	public:
		ExposeLocalRoll();
		const char *GetName();          // 0: 0x42dd10 "local roll"
		ScriptReference *GetType();
		ScriptReference *GetBaseType();
		int GetMax();
		int GetValueType();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
	};
};
ZOOSTERS_ASSERT_SIZE(ActorAttachment, 0xcc);
ZOOSTERS_ASSERT_OFFSET(ActorAttachment, m_index, 0x04);
ZOOSTERS_ASSERT_OFFSET(ActorAttachment, m_pos, 0x08);
ZOOSTERS_ASSERT_OFFSET(ActorAttachment, m_attachers, 0x14);
ZOOSTERS_ASSERT_OFFSET(ActorAttachment, m_2c, 0x2c);
ZOOSTERS_ASSERT_OFFSET(ActorAttachment, m_localMatrix, 0x30);
ZOOSTERS_ASSERT_OFFSET(ActorAttachment, m_70, 0x70);
ZOOSTERS_ASSERT_OFFSET(ActorAttachment, m_localRoll, 0xb0);
ZOOSTERS_ASSERT_OFFSET(ActorAttachment, m_localTilt, 0xb4);
ZOOSTERS_ASSERT_OFFSET(ActorAttachment, m_localFacing, 0xb8);
ZOOSTERS_ASSERT_OFFSET(ActorAttachment, m_localOffset, 0xbc);
ZOOSTERS_ASSERT_OFFSET(ActorAttachment, m_parentFrame, 0xc8);

}  // namespace Script

#endif
