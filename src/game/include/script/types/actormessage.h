// Script::ActorMessage (vtable 0x5cdc08, 27 slots, base PositionMeasurement): a queued script message (0x1c bytes) posted by
// Placement::PostMessage 0x42f660: the message type, the position and a float argument; it holds a reference to its sender.
#ifndef ZOOSTERS_SCRIPT_TYPES_ACTORMESSAGE_H
#define ZOOSTERS_SCRIPT_TYPES_ACTORMESSAGE_H

#include "zoosters/base.h"
#include "script/actor.h"

void *FreeListAlloc(size_t n);               // 0x439cf0

namespace Script {

class ActorMessage : public PositionMeasurement {
public:
	ActorMessage(intptr_t msgType, ScriptReference *receiver, const Vec3 *pos, int arg);    // 0x42f4e0 (Placement::PostMessage 0x42f660)
	~ActorMessage();    // 0x43b3d0 (folded ~ScriptReference)
	static void *operator new(size_t n) { return FreeListAlloc(n); }
	static void operator delete(void *p);    // 0x439d10 (pool free)
	const char *GetName();    // 0: 0x432c90
	int AdjustRefCount(int delta);    // 2: 0x42f560
	Vec3 *GetVector(ScriptReference *owner);    // 17: 0x42f590
	Vec3 m_pos;
	union { float f; int i; } m_arg;  // PostMessage's float argument (the constructor stores its bits)
	intptr_t m_msgType;  // the message type (a TFBMember_*Message prototype, compared by address)
	ScriptReference *m_receiver;  // object or set the message is for (reference counted); FindMessage tests g_scriptSelf against it
};
ZOOSTERS_ASSERT_SIZE(ActorMessage, 0x1c);

}  // namespace Script

// normal build: the pool-aware operator delete of the script nodes is the real function of 0x439d10
// (script/nodepool_helpers.cpp; no /alternatename bridge)
void ScriptNodePool_Free(void *node);
inline void Script::ActorMessage::operator delete(void *p) { ScriptNodePool_Free(p); }

#endif
