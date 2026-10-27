// Script actor ops. Address comments: the game function in that slot. Members past the base are sized from CloneToPool (m_XX = offset).
#ifndef ZOOSTERS_SCRIPT_OPS_ACTOR_H
#define ZOOSTERS_SCRIPT_OPS_ACTOR_H

#include "script/runtime.h"
#include "script/referenceset.h"
#include "script/actor.h"   // ActorCursor: Placement::m_20.m_1c

namespace Script {


// The actor's cursor into a set, at actor+0x3c (accessed through ActorCursor).
struct SetCursor {
	ReferenceSet *m_set;
	SetNode *m_node;
	int m_kind;
	int m_stamp;
};
inline SetCursor *ActorCursor(ScriptReference *actor)
{
	return (SetCursor *)&reinterpret_cast<Placement *>(actor)->m_20.m_1c;   // m_1c and the SetCursorTail after the placement (x64: 8 + 16 B)
}

// vtable 0x5cecd4, "cut-scene", 12 B
class ActorCutScene : public OpBlock {
public:
	const char *GetName();                      // 0: 0x435220
	OpCode *Clone();                            // 12: 0x436d90
	int Execute();                              // 13: 0x4383b0
	int Parse();                                // 14: 0x435300
	__declspec(noinline) static ActorCutScene *CloneToPool(const ActorCutScene *src);
	unsigned char m_mode;  // cut-scene state to enter (2, 3, ...; Execute compares it with the slot's m_state)
	ActorCutScene() : m_mode(1) {}
};
ZOOSTERS_ASSERT_SIZE(ActorCutScene, 0xc);

// vtable 0x5ced1c, "play animation", 12 B
class ActorPlayAnim : public OpBlock {
public:
	const char *GetName();                      // 0: 0x435280
	OpCode *Clone();                            // 12: 0x4372d0
	int Execute();                              // 13: 0x435290
	int Parse();                                // 14: 0x435300
	__declspec(noinline) static ActorPlayAnim *CloneToPool(const ActorPlayAnim *src);
	unsigned char m_anim;  // animation (Execute plays m_anim - 1)
	ActorPlayAnim() : m_anim(0) {}
};
ZOOSTERS_ASSERT_SIZE(ActorPlayAnim, 0xc);

// vtable 0x5ced68, "move to", 28 B
class ActorMoveTo : public OpBlock {
public:
	const char *GetName();                      // 0: 0x435320
	OpCode *Clone();                            // 12: 0x437320
	int Execute();                              // 13: 0x4373a0
	int Parse();                                // 14: 0x435330
	__declspec(noinline) static ActorMoveTo *CloneToPool(const ActorMoveTo *src);
	ScriptRef m_target;  // position (or set) to move to
	ScriptValue m_limit;  // distance; its opArg = set walk direction (g_forEachOrderNames)
	unsigned char m_anim;  // animation (Execute plays m_anim - 2)
	// distance; its opArg = set walk direction (g_forEachOrderNames)
	ActorMoveTo() { ZeroScriptValue(m_limit); m_anim = 4; }
};
ZOOSTERS_ASSERT_SIZE(ActorMoveTo, 0x1c);

// vtable 0x5cedcc, "move from", 28 B (same layout as ActorMoveTo)
class ActorMoveFrom : public OpBlock {
public:
	const char *GetName();                      // 0: 0x4353d0
	OpCode *Clone();                            // 12: 0x437730
	int Execute();                              // 13: 0x4377b0
	int Parse();                                // 14: 0x4353e0
	__declspec(noinline) static ActorMoveFrom *CloneToPool(const ActorMoveFrom *src);
	ScriptRef m_target;  // position to move away from
	ScriptValue m_limit;  // distance (Execute stops when it is below the target's GetFloat)
	unsigned char m_anim;  // animation (Execute plays m_anim - 2)
	ActorMoveFrom() { ZeroScriptValue(m_limit); m_anim = 4; }
};
ZOOSTERS_ASSERT_SIZE(ActorMoveFrom, 0x1c);

// vtable 0x5cee14, "teleport to", 36 B
class ActorTeleportTo : public OpCode {
public:
	const char *GetName();                      // 0: 0x435600
	OpCode *Clone();                            // 12: 0x436b20
	int Execute();                              // 13: 0x435610
	int Parse();                                // 14: 0x4358d0
	__declspec(noinline) static ActorTeleportTo *CloneToPool(const ActorTeleportTo *src);
	// Explicit + __forceinline: the implicit copy constructor (2 Vec3 members) is not inlined by cl 13.10, the game's is.
	ActorTeleportTo() { ZeroScriptValue(m_facing); ZeroScriptValue(m_seconds); }   // prototype instance (OpProto_ActorTeleportTo)
	__forceinline ActorTeleportTo(const ActorTeleportTo &o) throw() : OpCode(o), m_target(o.m_target), m_facing(o.m_facing), m_seconds(o.m_seconds) {}
	ScriptRef m_target;  // position (or set) to teleport to
	ScriptValue m_facing;  // heading angle after the teleport (SetFacing)
	ScriptValue m_seconds;  // "seconds" (timed move when != 0); its opArg = set walk direction (g_forEachOrderNames)
};
ZOOSTERS_ASSERT_SIZE(ActorTeleportTo, 0x24);

// vtable 0x5cee5c, "turn to", 20 B
class ActorTurnTo : public OpBlock {
public:
	const char *GetName();                      // 0: 0x435980
	OpCode *Clone();                            // 12: 0x437620
	int Execute();                              // 13: 0x437690
	int Parse();                                // 14: 0x435990
	__declspec(noinline) static ActorTurnTo *CloneToPool(const ActorTurnTo *src);
	ScriptValue m_angle;  // heading to turn to; its opArg = animation (Execute plays it - 1)
	ActorTurnTo() { ZeroScriptValue(m_angle); }   // prototype instance (OpProto_ActorTurnTo)
};
ZOOSTERS_ASSERT_SIZE(ActorTurnTo, 0x14);

// vtable 0x5ceea0, "run as player", 12 B
class ActorRunAsPlayer : public OpCode {
public:
	const char *GetName();                      // 0: 0x4359d0
	OpCode *Clone();                            // 12: 0x436bb0
	int Execute();                              // 13: 0x437890
	int Parse();                                // 14: 0x4359e0
	__declspec(noinline) static ActorRunAsPlayer *CloneToPool(const ActorRunAsPlayer *src);
	ScriptRef m_player;  // the player (controller) reference; its index in TFB_GetPlayerRefs = control mode
};
ZOOSTERS_ASSERT_SIZE(ActorRunAsPlayer, 0xc);

// vtable 0x5ceeec, "check fov", 48 B
class CheckFOV : public OpBlock {
public:
	const char *GetName();                      // 0: 0x435a10
	int IsBlock();                              // 11: 0x43d070
	OpCode *Clone();                            // 12: 0x437900
	int Execute();                              // 13: 0x436050
	int Parse();                                // 14: 0x435a20
	__declspec(noinline) static CheckFOV *CloneToPool(const CheckFOV *src);
	// Explicit + __forceinline: the implicit copy constructor (3 Vec3 members) is not inlined by cl 13.10, the game's is.
	__forceinline CheckFOV(const CheckFOV &o) throw() : OpBlock(o), m_target(o.m_target), m_heading(o.m_heading), m_halfFov(o.m_halfFov), m_range(o.m_range) {}
	ScriptRef m_target;  // the object or set to test
	ScriptValue m_heading;  // view direction (angle)
	ScriptValue m_halfFov;  // half the field of view (angle); its opArg = 1: also require line of sight
	ScriptValue m_range;  // "feet"; its opArg = compare operator (g_compareOpNames) of distance vs range
	CheckFOV() { ZeroScriptValue(m_heading); ZeroScriptValue(m_halfFov); ZeroScriptValue(m_range); }   // prototype instance (OpProto_CheckFOV)
};
ZOOSTERS_ASSERT_SIZE(CheckFOV, 0x30);

}  // namespace Script

#endif
