// Script object sets of the actor prototypes in the normal build: CsSet and its three actor member sets as real
// ReferenceSet subclasses (game vtables 0x5ca608 / 0x5cd380 / 0x5cd3d8 / 0x5cd430). The game view keeps stand-ins in actorprotos.cpp.
#ifndef ZOOSTERS_SCRIPT_CSSET_H
#define ZOOSTERS_SCRIPT_CSSET_H

#include "script/referenceset.h"

namespace Script {

// (vftable annotation: script/actorprotos.cpp)
struct CsSet : ReferenceSet {
	CsSet() {}
};
// GLOBAL: 0x5cd380 Script::CsSetWayPoints::vftable
struct CsSetWayPoints : CsSet {
	CsSetWayPoints() {}
	const char *GetName();                                       // 0: 0x42CDC0
	ScriptReference *GetBaseType();                              // 5: 0x42D4A0
	ScriptReference *Deref(ScriptReference *owner, int flags);   // 9: 0x42CDD0
};
// GLOBAL: 0x5cd3d8 Script::CsSetAttachPoints::vftable
struct CsSetAttachPoints : CsSet {
	CsSetAttachPoints() {}
	const char *GetName();                                       // 0: 0x42CDE0
	ScriptReference *GetBaseType();                              // 5: 0x42D650
	ScriptReference *Deref(ScriptReference *owner, int flags);   // 9: 0x42CDF0
};
// GLOBAL: 0x5cd430 Script::CsSetTurrets::vftable
struct CsSetTurrets : CsSet {
	CsSetTurrets() {}
	const char *GetName();                                       // 0: 0x42CE00
	ScriptReference *GetBaseType();                              // 5: 0x42CED0
	ScriptReference *Deref(ScriptReference *owner, int flags);   // 9: 0x42CE10
};

}  // namespace Script

#endif
