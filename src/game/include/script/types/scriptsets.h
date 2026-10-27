// The set members of the script types (Script::ExposePlayerSet ... _SpriteShield::ExposeRemovedSet): objects whose constructor
// builds a ReferenceSet and registers it against the type of its elements (ResolveSymbol 0x434350). Names and vtables are PS2
// RTTI; the "active" / "inactive" / "removed" sets of the particle and sprite shields are assigned to the vtables by order and
// by the strings (ambiguous). The destructor of all of them is the folded ~ReferenceSet-derived one (0x435230 actor sets,
// 0x43a640 senders, 0x43b0b0 particle, ...); the scalar deleting destructors are in script/deleting_dtors_2.cpp.
#ifndef ZOOSTERS_SCRIPT_TYPES_SCRIPTSETS_H
#define ZOOSTERS_SCRIPT_TYPES_SCRIPTSETS_H

#include "zoosters/base.h"
#include "script/referenceset.h"

namespace Script {

class ExposePlayerSet : public ReferenceSet {   // vtable 0x5d0598, "players"
public:
	ExposePlayerSet();                          // 0x4379a0
	~ExposePlayerSet();                         // 0x435230
	ExposePlayerSet *scalar_deleting_dtor(unsigned flags);   // 0x437cc0 (script/deleting_dtors_2.cpp: the vtable data tables name it)
	const char *GetName();                      // 0x437a10
	ScriptReference *GetBaseType();
};
ZOOSTERS_ASSERT_SIZE(ExposePlayerSet, 0x18);

class ExposeNPCsSet : public ReferenceSet {     // vtable 0x5d05e8, "NPCs"
public:
	ExposeNPCsSet();                            // 0x437a80
	~ExposeNPCsSet();                           // 0x435230
	const char *GetName();                      // 0x437af0
	ScriptReference *GetBaseType();
};
ZOOSTERS_ASSERT_SIZE(ExposeNPCsSet, 0x18);

class ExposeInactiveSet : public ReferenceSet { // vtable 0x5d0638, "inactive"
public:
	ExposeInactiveSet();                        // 0x437b60
	~ExposeInactiveSet();                       // 0x435230
	const char *GetName();
	ScriptReference *GetBaseType();
};
ZOOSTERS_ASSERT_SIZE(ExposeInactiveSet, 0x18);

class ExposeDeadSet : public ReferenceSet {     // vtable 0x5d0690, "dead"
public:
	ExposeDeadSet();                            // 0x437c40
	~ExposeDeadSet();                           // 0x435230
	const char *GetName();                      // 0x437cb0
	ScriptReference *GetBaseType();
};
ZOOSTERS_ASSERT_SIZE(ExposeDeadSet, 0x18);

class ExposeSendersSet : public ReferenceSet {  // vtable 0x5d1ee0, "message senders"
public:
	ExposeSendersSet();                         // 0x43a9e0
	~ExposeSendersSet();                        // 0x43a640
	const char *GetName();                      // 0x43aa50
	ScriptReference *GetBaseType();
};
ZOOSTERS_ASSERT_SIZE(ExposeSendersSet, 0x18);

// PS2 RTTI: Script::_ParticleShield::ExposeActiveSet/InactiveSet/RemovedSet (vtables 0x5d2d50/0x5d2da0/0x5d2de8, bases
// ParticleSet) and Script::_SpriteShield::ExposeActiveSet/RemovedSet (0x5d3708/0x5d3750, base SpriteSet).
class _ParticleShield {
public:
	class ExposeActiveSet : public ReferenceSet {
	public:
		ExposeActiveSet();                      // 0x43b5c0
		~ExposeActiveSet();                     // 0x43b0b0
		const char *GetName();
		ScriptReference *GetBaseType();
	};
	class ExposeInactiveSet : public ReferenceSet {
	public:
		ExposeInactiveSet();                    // 0x43b630
		~ExposeInactiveSet();
		const char *GetName();
		ScriptReference *GetBaseType();
	};
	class ExposeRemovedSet : public ReferenceSet {
	public:
		ExposeRemovedSet();
		~ExposeRemovedSet();
		const char *GetName();
		ScriptReference *GetBaseType();
	};
};

class _SpriteShield {
public:
	class ExposeActiveSet : public ReferenceSet {
	public:
		ExposeActiveSet();
		~ExposeActiveSet();
		const char *GetName();
		ScriptReference *GetBaseType();
	};
	class ExposeRemovedSet : public ReferenceSet {
	public:
		ExposeRemovedSet();
		~ExposeRemovedSet();
		const char *GetName();
		ScriptReference *GetBaseType();
	};
};

}  // namespace Script

Script::ScriptReference *TFBType_Actor();      // 0x436e30
Script::ScriptReference *TFBType_Particle();   // 0x43b4f0: type getter of _ParticleShield
Script::ScriptReference *TFBType_Sprite();     // 0x43e330: type getter of _SpriteShield

#endif
