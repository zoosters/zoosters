// Script::ActorTurret (PS2 RTTI), the "turret" script type: an ActorAttachment (vtable 0x5cd480, 35 slots) that also looks at a
// target (members at +0xd0.., field of view +0x100; ~ActorTurret 0x42ce50 releases +0xe0). The prototype instance
// of the type (TFBType_Turret 0x436f10) has vtable 0x5cdb30, whose GetName 0x42cec0 also returns "turret".
#ifndef ZOOSTERS_SCRIPT_TYPES_ACTORTURRET_H
#define ZOOSTERS_SCRIPT_TYPES_ACTORTURRET_H

#include "script/types/actorattachment.h"
#include "script/types/pointertarget.h"
#include "script/types/partentry.h"

namespace RWS { namespace TFB { class GameActor; } }
struct RwV3d;
void *MemAlloc(size_t n);   // 0x550170

namespace Script {

class ActorTurret : public ActorAttachment {
public:
	ActorTurret();                  // scaffold (no game constructor written)
	// 0x42e0c0 (GameActor::CreateParts builds every part of an actor with it; the game names it PartObj::PartObj): normal
	// builds construct a real turret from the owner actor and the model's part entry (script/partobj.cpp)
	ActorTurret(RWS::TFB::GameActor *owner, PartEntry *entry);
	static void *operator new(size_t n) { return MemAlloc(n); }
	static void operator delete(void *p) { ::operator delete(p); }
	~ActorTurret();                 // 0x42ce50
	ActorTurret *scalar_deleting_dtor(unsigned flags);   // 0x42d620 (script/deleting_dtors_2.cpp: the vtable data tables name it)
	const char *GetName();          // 0: 0x42cec0 "turret"
	ScriptReference *GetType();     // 4: 0x42ced0
	ScriptReference *GetBaseType(); // 5: 0x4039f0
	int GetKind();                  // 6: 0x43e6f0
	void *GetMembers(int *count);   // 7: 0x42dd50
	intptr_t vf26(intptr_t v);             // 26: 0x42cee0 (m_2c unless it equals v)
	// Normal build: the turret's overrides of slots 18 and 31, the game's 0x42cf00 (ActorLocalPoint::SetLocalPointFromWorld,
	// modules/tfb/ gameactor_localpoint.cpp) and 0x42cf90 (GameActor::SetLocalOffset, script/gameactor_refcount.cpp), both
	// written on stand-in classes with the same layout; bound by linker aliases in script/normal_bringup.cpp.
	void SetVector(const Vec3 *v, ScriptReference *owner);          // 18: 0x42cf00 (world point -> local offset, sets the dirty flag)
	void SetLocalOffset(const Vec3 *v, ScriptReference *owner);     // 31: 0x42cf90 (stores +0xbc..+0xc4, sets the dirty flag)
	// 0x429860 (tfb/corpus/footpoint.cpp; the game build keeps it on the FootPoint stand-in): foot points are parts of this
	// class (Corpus::m_feet)
	float CrossGround(RwV3d *at) const;
	static void *Members(int *count);                               // 0x42dd50 without an object (script/member_tables.cpp): the body of GetMembers, which AttachProtoB::GetMembers shares
	bool SetLocalFacing(float v);   // 32: 0x429800 (stores +0xb8, sets the dirty flag +0xcc)
	bool SetLocalTilt(float v);     // 33: 0x429820 (+0xb4)
	bool SetLocalRoll(float v);     // 34: 0x429840 (+0xb0)

	bool m_localDirty;  // set by the SetLocal* setters
	char m_cd[3];
	// +0xd0: a PointerTarget (look-at offsets, pending name, resolved actor, heading flag): the Expose* members below work on its fields
	PointerTarget m_lookAt;
	// +0xe8..: what the part keeps after the look-at slot (the aim update 0x42b2d0 and the foot-step code read them; names from there)
	Vec3 m_prevPos;  // where the part was (body space), one aim update ago
	Vec3 m_curPos;  // where it is now (the foot point's m_from / m_to)
	float m_fov;
	float m_aimRate;  // LerpFloat factor towards the wanted angles (1 - m_14 / 100 of the entry)
	PartEntry *m_entry;  // the model's part entry
	float m_pitch;  // current aim pitch (degrees)
	float m_yaw;  // current aim yaw (degrees)
	int m_114;
	int m_effectSurface;  // foot points: surface of the running dust effect
	unsigned m_effectEnd;  // its end (0x461cb0 clock)

	class ExposeLookAtPointer : public ScriptReference {
	public:
		ExposeLookAtPointer();
		const char *GetName();
		ScriptReference *GetType();
		ScriptReference *GetBaseType();
		int GetKind();
		void SetReferent(ScriptReference *value, ScriptReference *owner);
		ScriptReference *Deref(ScriptReference *owner, int flags);
	};
	class ExposeLookAtHorzOffs : public Measurement {
	public:
		ExposeLookAtHorzOffs();
		const char *GetName();
		ScriptReference *GetType();
		ScriptReference *GetBaseType();
		int GetValueType();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
	};
	class ExposeLookAtVertOffs : public Measurement {
	public:
		ExposeLookAtVertOffs();
		const char *GetName();
		ScriptReference *GetType();
		ScriptReference *GetBaseType();
		int GetMax();
		int GetValueType();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
	};
	class ExposeLookAtHeadingOffs : public Measurement {
	public:
		ExposeLookAtHeadingOffs();
		const char *GetName();
		ScriptReference *GetType();
		ScriptReference *GetBaseType();
		int GetMax();
		int GetValueType();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
	};
	class ExposeFOV : public Measurement {
	public:
		ExposeFOV();
		const char *GetName();
		ScriptReference *GetType();
		ScriptReference *GetBaseType();
		int GetMax();
		int GetValueType();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
	};
	class ExposeLookAtHeadingRel : public Measurement {
	public:
		ExposeLookAtHeadingRel();
		const char *GetName();
		ScriptReference *GetType();
		ScriptReference *GetBaseType();
		int GetMax();
		void SetValue(float value, ScriptReference *owner);
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
	};
};
ZOOSTERS_ASSERT_SIZE(ActorTurret, 0x120);
ZOOSTERS_ASSERT_OFFSET(ActorTurret, m_localDirty, 0xcc);
ZOOSTERS_ASSERT_OFFSET(ActorTurret, m_lookAt, 0xd0);
ZOOSTERS_ASSERT_OFFSET(ActorTurret, m_prevPos, 0xe8);
ZOOSTERS_ASSERT_OFFSET(ActorTurret, m_fov, 0x100);
ZOOSTERS_ASSERT_OFFSET(ActorTurret, m_entry, 0x108);
ZOOSTERS_ASSERT_OFFSET(ActorTurret, m_effectEnd, 0x11c);

}  // namespace Script

#endif
