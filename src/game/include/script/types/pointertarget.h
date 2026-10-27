// Script::PointerTarget: the "pointer" slot of a script object (ScriptCamera's look-at / look-from, ActorTurret's look-at), split out of
// scriptcamera.h so the turret can embed it with its real (pointer-sized) members.
#ifndef ZOOSTERS_SCRIPT_TYPES_POINTERTARGET_H
#define ZOOSTERS_SCRIPT_TYPES_POINTERTARGET_H

#include "zoosters/base.h"
#include "script/value.h"

namespace Script {

// A script "pointer" target (0x18 bytes): an actor named in the script, resolved lazily, plus offsets from it. ScriptCamera holds
// two (look at +0x0c, look from +0x24: ExposeTgt* / ExposePos*; CameraData::HandleAttributes (rws/handlers_y_camera.cpp) writes
// name / actor / offsets / flag), ActorTurret one at +0xd0 (ExposeLookAtPointer).
struct PointerTarget {
	PointerTarget() : m_horzOffs(0), m_vertOffs(0), m_headingOffs(0), m_pendingName(0), m_actor(0), m_headingRel(1) {}
	float m_horzOffs;
	float m_vertOffs;
	float m_headingOffs;
	const char *m_pendingName;  // actor name still to resolve (0 = none / "(Current)")
	ScriptReference *m_actor;  // resolved actor (reference counted)
	unsigned char m_headingRel;  // heading offset relative to the actor's facing
	void ClearActor();           // 0x41d6c0: drop the pending name and the resolved actor
	ScriptReference *GetActor(); // 0x41d920: resolve the pending name (TFBType_Actor symbols), return the actor
};
ZOOSTERS_ASSERT_OFFSET(PointerTarget, m_horzOffs, 0x00);
ZOOSTERS_ASSERT_OFFSET(PointerTarget, m_vertOffs, 0x04);
ZOOSTERS_ASSERT_OFFSET(PointerTarget, m_headingOffs, 0x08);
ZOOSTERS_ASSERT_OFFSET(PointerTarget, m_pendingName, 0x0c);
ZOOSTERS_ASSERT_OFFSET(PointerTarget, m_actor, 0x10);
ZOOSTERS_ASSERT_OFFSET(PointerTarget, m_headingRel, 0x14);

}  // namespace Script

#endif
