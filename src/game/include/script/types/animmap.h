// Script::AnimMapMeasurement: the "animation mapping" value; its members (ExposeAnimIndex, an array of 4-byte objects) index the
// behaviours of the current script actor (g_scriptSelf). Prototype getter TFBType_AnimMap 0x4384c0.
#ifndef ZOOSTERS_SCRIPT_TYPES_ANIMMAP_H
#define ZOOSTERS_SCRIPT_TYPES_ANIMMAP_H

#include "zoosters/base.h"
#include "script/value.h"

namespace Script {

class AnimMapMeasurement : public Measurement {
public:
	const char *GetName();                // 0: 0x436a30
	ScriptReference *GetType();           // 4: 0x438900 (jmp 0x4384c0)
	void *GetMembers(int *count);         // 7: 0x436440 (63 ExposeAnimIndex prototypes)
	int GetInt(ScriptReference *owner);   // 13: 0x4351e0
	// normal build: the prototype (TFBType_AnimMap) is a real AnimMapMeasurement, so the class carries every override of the
	// game's vtable 0x5d02f8 (the cl 13.10 view binds the slots by address); bodies in script/animmap_proto.cpp
	ScriptReference *GetBaseType();                     // 5: 0x436a50 (IntMeasurement's type)
	int GetKind();                                      // 6: 0x43e6f0 (a value)
	int GetMax();                                       // 11: 0x4386a0 (AnimMapMemberCount)
	int GetValueType();                                 // 12: 0x43e6f0 (int)
	using Measurement::SetValue;
	void SetValue(int value, ScriptReference *owner);   // 16: 0x438630 (ExposeAnimIndex::SetValue's body)
};
ZOOSTERS_ASSERT_SIZE(AnimMapMeasurement, 4);

// One of the 63 members of AnimMapMeasurement: an empty 4-byte object (vptr only); GetInt / SetValue index the behaviours of g_scriptSelf.
// GLOBAL: 0x5cef88 Script::ExposeAnimIndex::vftable
class ExposeAnimIndex : public Measurement {
public:
	int GetInt(ScriptReference *owner);                  // 0x4385c0
	void SetValue(int value, ScriptReference *owner);    // 0x438630
	ScriptReference *GetType();                             // 4: 0x436a50
	ScriptReference *GetBaseType();                         // 5: 0x402b70
	int GetMax();                                           // 11: 0x4386a0
	const char *GetName();                                  // 0: 0x438540
};
ZOOSTERS_ASSERT_SIZE(ExposeAnimIndex, 4);

}  // namespace Script

int AnimMapMemberCount();                           // 0x4386a0
Script::AnimMapMeasurement *TFBType_AnimMap();      // 0x4384c0: the AnimMapMeasurement prototype (function-local static)

#endif
