// Corpus: the per-actor physics body, with MainTickHandler and ClumpCloneRef.
#ifndef ZOOSTERS_G_CORPUS_H
#define ZOOSTERS_G_CORPUS_H

#include "tfb/anim/tfbmodelstate.h"   // first: <vector> before the framework headers
#include "script/script.h"
#include "framework/core/rwsguid.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/attributemacros.h"
#include "framework/core/cclassfactory.h"
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/linearallocationpolicy.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/ceventhandler.h"
#include "rw/rwcore.h"
#include "script/ops_value.h"
#include "script/referenceset.h"
#include "tfb/collision/collider.h"
#include "modules/fx/attachpoint.h"
#include "framework/toolkits/world/smartptrs/cclumpptr.h"

struct RwFrame;
struct RpClump;
struct RpAtomic;
#ifndef ZOOSTERS_RWV3D_DEFINED   // the same struct is defined in rws/rwfx.h
#define ZOOSTERS_RWV3D_DEFINED
struct RwV3d { float x, y, z; };
#endif

#define CA_AT(T, p, off) (*(T *)((char *)(p) + (off)))
// Corpus +0x40 / +0x41: TFBModelState::auxAActive_ / auxBActive_ (protected in CAnimSet)
#define CORPUS_AUX_ACTIVE(c) ((c)->m_model.IsAuxActive())

class CTFBModel;
namespace RWS { namespace TFB { class GameActor; } }
class Corpus;
class ShadowPass;          // the payload of CORPUS_SHADOW_PROCESS (tfb/render/shadow.h)
struct CameraProbe;        // the payload of CORPUS_CAMERA_PROCESS (modules/tfb/cameraprobe.h)

// The link of a Corpus to the actor that carries it: standing on a triangle of the carrier's mesh (m_index >= 0) or riding an attachment point (m_index -1, SetAttachTarget).
class CorpusLink {
public:
	CorpusLink() : m_carrier(0) {}
	~CorpusLink() { Release(); }
	bool Release();
	RWS::TFB::Collider *m_carrier;                    // the carrier (GameActor's collider base)
	int m_id;                                         // corpus id of the carrier at link time (a newer body means the link is stale)
	int m_index;                                      // ground triangle index of the probe hit (-1: riding m_attachPoint)
	AttachPoint *m_attachPoint;
	RpAtomic *m_atomic;                               // the carrier's atomic the body stands on (compared with the probe's hit)
	RwFrame *m_frame;                                 // carrier frame (RwFrameGetLTM; SetAttachTarget stores the target's frame)
	float m_u, m_v;                                   // barycentric u/v of the ground hit on the carrier triangle
	void *m_owner;
};
ZOOSTERS_ASSERT_SIZE(CorpusLink, 0x24);

namespace Script {
class ClumpCloneRef : public ScriptReference {
public:
	ClumpCloneRef(RpClump *clump, const RwV3d *a, const RwV3d *b, intptr_t c);
	~ClumpCloneRef();
	virtual int AdjustRefCount(int delta);
	RWS::CClumpPtr m_clump;
	int m_08;
};
ZOOSTERS_ASSERT_SIZE(ClumpCloneRef, 0xc);
}

enum CorpusFlags {
	CORPUS_NO_PHYSICS = 0x00000001,        // HandleEvents ignores the physics records
	CORPUS_AIRBORNE = 0x00000200,
	CORPUS_GROUND_FOUND = 0x00000400,      // the physics probe found ground under the body this tick
	CORPUS_LEFT_GROUND = 0x00000800,       // HandleEvents posts LeftGround, not HitGround
	CORPUS_HIT_TRIGGER = 0x00001000,       // touched a triangle with collision flag 1: HandleEvents posts message 0x439e60
	CORPUS_JUMP_STARTED = 0x00002000,      // HandleEvents posts JumpStarted
	CORPUS_STEP_DONE = 0x00004000,         // this tick's physics record was applied
	CORPUS_MOVING = 0x00008000,            // horizontal velocity after ApplyMovement
	CORPUS_HIT_LEDGE = 0x00010000,         // touched a ledge triangle (collision flag 0x20): a falling player probes for it (LedgeNearby)
	CORPUS_COLLIDED = 0x04000000,          // the collision response changed the velocity this tick
	CORPUS_PLAYER_CONTROLLED = 0x08000000, // moved by pad m_controller (SetPlayerControl)
	CORPUS_FELL_OUT = 0x80000000           // fell out of the world and was put back at its origin (reported once)
};

// Animation slots (TFBModelState) the Corpus plays; the names are ours.
enum CorpusAnimSlot {
	CORPUS_ANIM_RISE = 4,         // airborne, moving up; also the start of a jump
	CORPUS_ANIM_FALL = 5,         // airborne, moving down
	CORPUS_ANIM_JUMP_WINDUP = 0x33 // a jump from standing still: the launch waits for its end
};

struct RwV3dZero : public RwV3d { RwV3dZero() { x = 0.0f; y = 0.0f; z = 0.0f; } };

class Corpus : public RWS::CEventHandler {
public:
	Corpus(void *model, RWS::TFB::GameActor *owner);
	~Corpus();
	virtual void HandleEvents(RWS::CMsg &msg);
	virtual void SetEnabled(int enable);
	void SetPlayerControl(bool on, int pad);
	void PlayAnim(int slot, bool keep);
	void SteerToTarget(RwV3d *out);
	void LeaveGround(const RwV3d *newPosition, bool allowLateJump);
	void DetachFromCarrier();
	void AttachToCarrier(RWS::TFB::GameActor *carrier);
	void UpdateAnimation(const RwV3d *dir);
	void ResolveGroundContact(void *rec);  // fills the physics record, probes the ground

	TFBModelState m_model;                            // animation state of the actor's model (0xf0 bytes)
	CTFBModel *m_modelPtr;                            // the actor's CTFBModel (footstep sounds and effects per surface)
	RwFrame *m_frame;                                 // the body's frame (its modelling matrix is the actor's placement)
	unsigned m_flags;                                 // CorpusFlags
	int m_controller;                                 // pad to read this tick (-1: none; reset after reading)
	RwV3dZero m_moveInput;                            // x: stick magnitude 0..1 (after the 0.4 dead zone), y: turn this tick (degrees)
	RwV3d m_velocity;                                 // per tick while moving, per second after the physics step (Scale by g_frameRate)
	RwMatrix m_lastFrame;                             // the frame's modelling matrix at the end of the last step
	RWS::TFB::GameActor *m_owner;
	RwV3d m_groundNormal;                             // normal under the body (floor test: .y >= 0.7071)
	RwV3d m_contactPoint;                             // ground / carrier contact point (world)
	int m_id;
	CorpusLink m_link;                                // the carrier the body stands on
	unsigned char m_groundColor[4];                   // prelight colour of the ground under it (RGBA; alpha from the owner's tint)
	RpAtomic *m_hitAtomic;                            // atomic the physics probe hit this tick (a carrier candidate)
	RWS::TFB::Collider *m_hitCarrier;                 // its actor's collider (0: world geometry)
	RwMatrix m_carrierFrame;                          // inverse of the carrier's frame when attached; its pos also receives the probe's contact point (physics step)
	RwV3d m_frameStep;                                // how far the frame moved in the last step; the probe's ground normal during the physics step
	unsigned short m_linkPriority;
	unsigned char m_jumpPressed;                      // jump button pressed this tick (latched for the first 0.25 s of a hold)
	char m_pad1ff;
	float m_riseTime;                                 // seconds left of a held jump's rise; below 0: time spent falling at top speed
	float m_lateJumpTime;                             // seconds after walking off an edge in which a jump still starts
	float m_jumpHoldTime;                             // how long the jump button has been held (FLT_MAX: no pad)
	float m_jumpSpeed;                                // launch speed of the jump that starts next
	float m_stateTime;                                // seconds since the body landed or left the ground
	float m_standSpeed;                                   // 0.05: UpdateAnimation stands when the forward speed is not above it; the walk-slowly cycle starts above
	int m_animOverride;                               // animation slot a script forced (-1: none)
	int m_unk21c;                                     // unused
	int m_idleSlot;                                   // the idle variant slot played last (UpdateAnimation)
	int m_unk224;                                     // unused
	int m_idleSet;                                    // the idle variant set 0..2 ChooseIdleVariant plays (slots set * 5 + 0x3f ...), cleared when the cut-scene idle ends
	int m_prevIdleSet;                                // the set the last switch came from; ChooseIdleVariant compares it to stop after a full circle; cleared when a run / walk / jump / fall animation starts
	int m_surface;                                    // surface type under the body (index of the footstep tables)
	int m_footstepSound;                              // next of the surface's four footstep sounds
	Script::ReferenceSet m_feet;                                    // the model's foot effect points
	void ApplyMovement(RwV3d *dir);

private:
	// ApplyMovement's steps (one function in the game)
	void UpdateAirborne(float fallLimit);
	void ReturnToOrigin();
	void FollowCarrier();
	void RideAttachPoint();
	void StandOnCarrierMesh();
	void UpdateJumpStart();
	bool MayWindUpJump() const;
	void Jump(float speed);
	void Steer(RwV3d *dir, float fallLimit);
	float Turn(float speed, float maxSpeed);
	void FollowSlope();
	void LimitTilt();
	void FinishMovement();
	RwMatrix *Modelling();
	// HandleEvents' steps (one function in the game)
	void Post(void *message, void *target, const RwV3d *pos, float t);
	void HandlePhysics(void *data);
	void QueueAtomics();
	void Tick();
	void ReadStick(RwV3d *dir);
	void ReadJumpButton();
	void Footsteps();
	void FinishStep(float t);
	bool BouncesOff();
	void Land();
	void NoGround();
	void ProbeGroundBelow();
	void EndStep();
	void UpdateTint();
	void HandleShadowProbe(ShadowPass *visitor);
	void HandleCameraProbe(CameraProbe *probe);
	// UpdateAnimation's steps (one function in the game)
	void Run(const RwV3d *dir, float speed, float fwd);
	void Walk(float fwd);
	void WalkSlowly(float fwd);
	void Stand(float speed, float fwd);
	int ChooseIdleVariant(float speed);
	void UpdateLean(float fwd);
};
ZOOSTERS_ASSERT_SIZE(Corpus, 0x250);
ZOOSTERS_ASSERT_OFFSET(Corpus, m_model, 0x08);
ZOOSTERS_ASSERT_OFFSET(Corpus, m_modelPtr, 0xf8);
ZOOSTERS_ASSERT_OFFSET(Corpus, m_frame, 0xfc);
ZOOSTERS_ASSERT_OFFSET(Corpus, m_flags, 0x100);
ZOOSTERS_ASSERT_OFFSET(Corpus, m_controller, 0x104);
ZOOSTERS_ASSERT_OFFSET(Corpus, m_moveInput, 0x108);
ZOOSTERS_ASSERT_OFFSET(Corpus, m_velocity, 0x114);
ZOOSTERS_ASSERT_OFFSET(Corpus, m_lastFrame, 0x120);
ZOOSTERS_ASSERT_OFFSET(Corpus, m_owner, 0x160);
ZOOSTERS_ASSERT_OFFSET(Corpus, m_groundNormal, 0x164);
ZOOSTERS_ASSERT_OFFSET(Corpus, m_contactPoint, 0x170);
ZOOSTERS_ASSERT_OFFSET(Corpus, m_link, 0x180);
ZOOSTERS_ASSERT_OFFSET(Corpus, m_groundColor, 0x1a4);
ZOOSTERS_ASSERT_OFFSET(Corpus, m_hitAtomic, 0x1a8);
ZOOSTERS_ASSERT_OFFSET(Corpus, m_carrierFrame, 0x1b0);
ZOOSTERS_ASSERT_OFFSET(Corpus, m_frameStep, 0x1f0);
ZOOSTERS_ASSERT_OFFSET(Corpus, m_linkPriority, 0x1fc);
ZOOSTERS_ASSERT_OFFSET(Corpus, m_riseTime, 0x200);
ZOOSTERS_ASSERT_OFFSET(Corpus, m_standSpeed, 0x214);
ZOOSTERS_ASSERT_OFFSET(Corpus, m_animOverride, 0x218);
ZOOSTERS_ASSERT_OFFSET(Corpus, m_idleSlot, 0x220);
ZOOSTERS_ASSERT_OFFSET(Corpus, m_idleSet, 0x228);
ZOOSTERS_ASSERT_OFFSET(Corpus, m_surface, 0x230);
ZOOSTERS_ASSERT_OFFSET(Corpus, m_feet, 0x238);

namespace RWS {
// NOVTABLE: RWS::CEventHandlerBase
class CEventHandlerBase : public CEventHandler {     // stand-in for the base constructor
public:
	CEventHandlerBase() { m_Active = 1; }
};
// The static tick handler: frame timing, clone list limit and the physics step.
class MainTickHandler : public CEventHandlerBase {
public:
	~MainTickHandler();
	static CEventHandler *GetInstance();
	virtual void HandleEvents(CMsg &msg);
};
}

#endif
