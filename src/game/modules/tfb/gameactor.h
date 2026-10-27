// 0x33c bytes (Spawn 0x42da70 allocates 0x33c): ScriptActor +0x00, Collider +0x98, own members from +0x16c. Created by its CProtoActor,
// which fills ActorParams from its stream attribute 1.
#ifndef ZOOSTERS_MODULES_TFB_GAMEACTOR_H
#define ZOOSTERS_MODULES_TFB_GAMEACTOR_H

#include "zoosters/base.h"
#include "rw/rwcore.h"
#include "script/types/scriptactor.h"
#include "script/runtime.h"
#include "tfb/collision/collider.h"

class Corpus;
void *MemAlloc(size_t size);

namespace RWS {
namespace TFB {

class CProtoActor;

// ActorParams::m_flags. Bit numbers from the script accessors (Script::ScriptActor::Expose*::GetInt) and GameActor::Is*.
enum ActorFlags {
	ACTOR_SOLID_COLLIDER = 0x001,      // solid collision
	ACTOR_WALL_CLIMBER = 0x002,
	ACTOR_WALL_COLLISION = 0x004,
	ACTOR_NO_SHADOW = 0x008,
	ACTOR_VISIBLE_ON_RADAR = 0x010,
	ACTOR_IGNORES_CUT_SCENES = 0x020,  // cut-scene ignore
	ACTOR_IGNORES_GROUND_COLOR = 0x040,
	ACTOR_GROUND_TILTER = 0x080,       // tilts with ground
	ACTOR_MESH_COLLIDER = 0x100,       // with a Corpus
	ACTOR_MESH_COLLIDEE = 0x200,       // "triangles collide", with a Corpus
	ACTOR_BLOCKS_CAMERA = 0x800        // the camera's probe (CORPUS_CAMERA_PROCESS) sees it; no script accessor
};

// ActorParams::m_moveType: how Corpus::ApplyMovement steers the body. Types 1-3 also exist: 3 steers like a walker, types below 2
// end a fall when ground is found under them.
enum ActorMoveType {
	ACTOR_MOVE_WALK = 0, // turns towards the stick direction first, accelerates along its facing
	ACTOR_MOVE_FREE = 4  // accelerates along the stick direction and keeps its facing
};

// The actor's tuning values (0x114 bytes): CProtoActor stream attribute 1, copied as one block into each actor it creates and
// by Spawn into each clone. Names from the script accessor classes (Script::ScriptActor::Expose<Name>); their script labels in
// quotes where they differ. Speeds and accelerations are in world units per second (the scripts give them per frame).
// The extents and offsets are in the editor's axes (Z up): CProtoActor feeds the editor's Z into RenderWare's Y (see m_zExtent).
// Vectors the scripts read and write are Script's Vec3 (layout of RwV3d) until the two vector types are unified.
struct ActorParams {
	int32_t m_unk00;                // +0x00 (actor +0x1ac)
	uint32_t m_flags;               // +0x04 ActorFlags
	float m_activationRange;        // +0x08
	float m_deactivationRange;      // +0x0c
	uint32_t m_coneColor;           // +0x10 RGBA, red in the low byte
	int32_t m_unk14;                // +0x14
	union {
		uint32_t m_blipColor;       // +0x18 RGBA
		uint8_t m_blip[4];          // red, green, blue, alpha
	};
	int32_t m_fadePercent;          // +0x1c "activation range fade percent"
	uint8_t m_unk20[0x24];          // +0x20
	int32_t m_coastFrames;          // +0x44 "jump coast frames": how long a held jump keeps rising, in 60ths of a second
	int32_t m_jumpDelay;            // +0x48
	int32_t m_hitPoints;            // +0x4c "health"
	int32_t m_moveType;             // +0x50 ActorMoveType
	float m_groundSpeed;            // +0x54 "ground top speed"
	float m_groundTurnRate;         // +0x58
	float m_coneAngle;              // +0x5c
	float m_coneLength;             // +0x60
	float m_sweepAngle;             // +0x64 "cone sweep offset" (CProtoActor zeroes it after copying the attribute)
	int32_t m_unk68;                // +0x68
	float m_initialJumpVelocity;    // +0x6c
	float m_upSpeed;                // +0x70 "up top speed": the most a held jump adds while rising
	float m_upAccel;                // +0x74 rise acceleration while the jump is held
	float m_downAccel;              // +0x78 gravity while airborne
	float m_downSpeed;              // +0x7c "down top speed": fastest fall
	float m_bounceRestitution;      // +0x80
	float m_weight;                 // +0x84
	float m_groundAccel;            // +0x88
	float m_groundDecel;            // +0x8c
	float m_airSpeed;               // +0x90 "air top speed"
	float m_airAccel;               // +0x94
	float m_airDecel;               // +0x98
	float m_zExtent;                // +0x9c "L extent": box extent along RenderWare's Y (up) after the x12 scale
	float m_xExtent;                // +0xa0 "W extent": along X
	float m_yExtent;                // +0xa4 "H extent": along Z
	float m_xOffset;                // +0xa8 box offset along X
	float m_yOffset;                // +0xac along Z (editor Y)
	float m_zOffset;                // +0xb0 along Y (editor Z, up)
	float m_airTurnRate;            // +0xb4
	float m_walkSpeed;              // +0xb8 speed above which the walk cycle plays (Corpus::UpdateAnimation; name is ours)
	float m_runSpeed;               // +0xbc speed above which the run cycle plays (name is ours)
	int32_t m_idleAnims[2];         // +0xc0 first/last animation index of the slow cycle (name is ours)
	int32_t m_walkAnims[2];         // +0xc8 first/last of the walk cycle
	int32_t m_runAnims[2];          // +0xd0 first/last of the run cycle
	int32_t m_id;                   // +0xd8 "ID"
	uint8_t m_unkDC[0x38];          // +0xdc
};
ZOOSTERS_ASSERT_SIZE(ActorParams, 0x114);
ZOOSTERS_ASSERT_OFFSET(ActorParams, m_flags, 0x04);
ZOOSTERS_ASSERT_OFFSET(ActorParams, m_activationRange, 0x08);
ZOOSTERS_ASSERT_OFFSET(ActorParams, m_deactivationRange, 0x0c);
ZOOSTERS_ASSERT_OFFSET(ActorParams, m_coneColor, 0x10);
ZOOSTERS_ASSERT_OFFSET(ActorParams, m_blipColor, 0x18);
ZOOSTERS_ASSERT_OFFSET(ActorParams, m_coastFrames, 0x44);
ZOOSTERS_ASSERT_OFFSET(ActorParams, m_groundTurnRate, 0x58);
ZOOSTERS_ASSERT_OFFSET(ActorParams, m_coneLength, 0x60);
ZOOSTERS_ASSERT_OFFSET(ActorParams, m_sweepAngle, 0x64);
ZOOSTERS_ASSERT_OFFSET(ActorParams, m_initialJumpVelocity, 0x6c);
ZOOSTERS_ASSERT_OFFSET(ActorParams, m_upSpeed, 0x70);
ZOOSTERS_ASSERT_OFFSET(ActorParams, m_upAccel, 0x74);
ZOOSTERS_ASSERT_OFFSET(ActorParams, m_downAccel, 0x78);
ZOOSTERS_ASSERT_OFFSET(ActorParams, m_downSpeed, 0x7c);
ZOOSTERS_ASSERT_OFFSET(ActorParams, m_bounceRestitution, 0x80);
ZOOSTERS_ASSERT_OFFSET(ActorParams, m_airDecel, 0x98);
ZOOSTERS_ASSERT_OFFSET(ActorParams, m_airTurnRate, 0xb4);
ZOOSTERS_ASSERT_OFFSET(ActorParams, m_zExtent, 0x9c);
ZOOSTERS_ASSERT_OFFSET(ActorParams, m_id, 0xd8);

class GameActor : public Script::ScriptActor, public Collider {
public:
	GameActor(CProtoActor *proto);
	~GameActor();
	static void *operator new(size_t n) { return MemAlloc(n); }

	// Script::Placement virtuals the game actor overrides (slot numbers of the vtable 0x5cd728)
	int AdjustRefCount(int delta);                             // slot 2
	void SetVector(const Vec3 *v, Script::ScriptReference *owner); // slot 18, sets the position
	float GetFacing();                                         // slot 19
	bool SetFacing(float v, int snap);                         // slot 20
	float GetTilt();                                           // slot 21
	bool SetTilt(float v, Script::ScriptReference *owner);     // slot 22
	float GetRoll();                                           // slot 23
	bool SetRoll(float v, Script::ScriptReference *owner);     // slot 24
	void Remove();                                             // slot 27
	void Reset();                                              // slot 28
	char Control(bool on);                                     // slot 29
	intptr_t Spawn(intptr_t pos, int flag);                           // slot 30
	ScriptReference *GetSpawnSource();                         // slot 31
	int IsSolidCollider();                                     // slot 32
	int IsMeshCollider();                                      // slot 33
	int IsMeshCollidee();                                      // slot 34
	bool PlayAnimation(int index);                             // slot 35
	int AccessBehavior(int index, int value);                  // slot 36
	void SetDestination(Vec3 *pos, int flag);                  // slot 37
	void SetDestinationTimed(Vec3 *pos, float seconds);        // slot 38
	void SetAttachTarget(Script::ScriptReference *target);     // slot 39

	bool UpdateTimedMove();   // one frame of SetDestinationTimed's move; true on a frame the actor moved
	void PlaceAt(const Vec3 *pos);   // moves the actor, through its Corpus if any
	void DetachProto();   // clears the creating CProtoActor in this actor and its children
	void CreateBody();   // builds the Corpus from the proto's model
	void CreateParts();   // builds the attachment points / turrets
	void *GetBodyPosition();   // the Corpus's contact point, else the placement matrix position
	void UpdateCollisionExtents(bool pinned);   // box extents and offset from the parameters (x12); pinned: only the flag of a pinned body
	void ReleaseBody();   // takes the placement from the proto / body, then deletes the member sets and the Corpus
	void DeactivateActor(bool park);   // the actor leaves play (messages, script context reset, parked in the inactive / dead set)
	void ActivateActor(bool park);   // the actor enters play (control mode 1; park: into the NPC set)
	void SetSolidCollider(bool on);
	void SetMeshCollidee(bool on);
	void SetNoShadow(bool on);
	Script::OpContext *GetVariableList(int create);   // the actor's script context, instantiated from its proto's script asset when create != 0

	RwMatrix m_matrix;              // +0x16c placement while the actor has no Corpus (identity at construction)
	ActorParams m_params;           // +0x1ac
	int32_t m_refCount;             // +0x2c0
	CProtoActor *m_proto;           // +0x2c4 the CProtoActor that created the actor
	Corpus *m_corpus;               // +0x2c8 the physics body (0 without one)
	GameActor *m_spawnSource;       // +0x2cc the actor this one was spawned from (Spawn)
	uint8_t m_scriptClockHold;      // +0x2d0 "the actor was moved / reactivated this frame, hold its script clock"; UpdateTimedMove (the only reader) clears it and stamps m_script.m_0c, else shifts m_script.m_startTime by the time stood still
	uint8_t m_pad2D1[3];
	int32_t m_warpTime;               // +0x2d4 script time of the last jump of more than 12 units (SetVector; read by Camera_PostTickFollowUpdate)
	int32_t m_timedMoveStart;       // +0x2d8 script time of SetDestinationTimed
	int32_t m_timedMoveMs;          // +0x2dc its duration in ms (0 = none)
	Vec3 m_timedMoveFrom;           // +0x2e0 its start position
	union {
		uint32_t m_tintColor;       // +0x2ec CProtoActor attribute 6
		uint8_t m_tint[4];          // red, green, blue, alpha
	};
	Script::OpContext m_script;     // +0x2f0 the actor's own script context (0x1c bytes)
	float m_facing;                 // +0x30c degrees
	float m_heading;                // +0x310 degrees, added to m_facing for the frame's yaw
	float m_tilt;                   // +0x314
	float m_roll;                   // +0x318
	float m_targetFacing;           // +0x31c facing to turn to; FLT_MAX bits (0x7f7fffff) = none
	float m_initialFacing;          // +0x320 yaw of the placement matrix (CProtoActor system attribute 1)
	Vec3 m_origin;                  // +0x324
	Vec3 m_destination;             // +0x330
};
ZOOSTERS_ASSERT_SIZE(GameActor, 0x33c);
ZOOSTERS_ASSERT_OFFSET(GameActor, m_matrix, 0x16c);
ZOOSTERS_ASSERT_OFFSET(GameActor, m_params, 0x1ac);
ZOOSTERS_ASSERT_OFFSET(GameActor, m_corpus, 0x2c8);
ZOOSTERS_ASSERT_OFFSET(GameActor, m_scriptClockHold, 0x2d0);
ZOOSTERS_ASSERT_OFFSET(GameActor, m_timedMoveMs, 0x2dc);
ZOOSTERS_ASSERT_OFFSET(GameActor, m_script, 0x2f0);
ZOOSTERS_ASSERT_OFFSET(GameActor, m_facing, 0x30c);
ZOOSTERS_ASSERT_OFFSET(GameActor, m_heading, 0x310);
ZOOSTERS_ASSERT_OFFSET(GameActor, m_targetFacing, 0x31c);
ZOOSTERS_ASSERT_OFFSET(GameActor, m_origin, 0x324);
ZOOSTERS_ASSERT_OFFSET(GameActor, m_destination, 0x330);

// The actor whose collider `c` is (Collider is GameActor's second base). A reference cast: no null check.
static inline GameActor &ActorOf(Collider &c)
{
	return static_cast<GameActor &>(c);
}

} // namespace TFB
} // namespace RWS

// The placement id view of an actor (script/gameactor.cpp 0x42f470)
struct PlacementIdW {
	void *m_vptr;
	int m_placementId;   // +4
	bool IsPlacementRemoved();
};
#endif
