// Corpus::HandleEvents: the physics, shadow and camera messages of a body.
#define ZOOSTERS_RWS_SENDMSG 1
#include "modules/tfb/renderqueue.h"
#include "tfb/level/level.h"
#include "tfb/corpus/messagemembers.h"
#include "tfb/corpus/corpus.h"
#include "tfb/corpus/footpoint.h"
#include "tfb/render/shadow.h"
#include "modules/tfb/cameraprobe.h"
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptactor.h"
#include "modules/tfb/gameactor.h"
#include "tfb/corpus/corpus_private.h"
#include "modules/tfb/ctfbmodel.h"
#include "rw/rwcore.h"
#include "tfb/render/tfbclump.h"
#include "script/ops_message.h"

// GLOBAL: 0x61831c RWS::ERROR_NOT_A_VALID_MESSAGE_ID
// GLOBAL: 0x618308 g_corpusCount
// GLOBAL: 0x618328 g_msgCorpusPhysics
// GLOBAL: 0x618330 g_msgCorpusShadow
// GLOBAL: 0x618338 g_msgCorpusCamera
extern int g_corpusCount;
extern RWS::CEventId g_msgCorpusPhysics;
extern RWS::CEventId g_msgCorpusShadow;
extern RWS::CEventId g_msgCorpusCamera;
namespace RWS { namespace MainLoop { namespace Logic { extern unsigned Rate; } } }


// GLOBAL: 0x42f660 CpActor::PostMessage
// GLOBAL: 0x458e70 CpBox::TestPoint
struct CpActor { void PostMessage(void *msgType, void *actor, const RwV3d *pos, float t); };
struct CpBox { int TestPoint(const RwV3d *p, const RwV3d *d, int z); };
#include "script/types/scriptsound.h"
typedef Script::ScriptSoundObj CpSnd;   // ScriptSoundObj::Play (vtable slot 11 in the game)
extern float g_frameRate;                            // GLOBAL: 0x618314 g_frameRate
namespace Script { class ScriptReference; }
extern Script::ScriptReference *g_voiceDefaultRef;                      // GLOBAL: 0x621f34 g_voiceDefaultRef
Script::ScriptReference *TFBType_Actor();            // 0x436e30
float GetPlayerInput(int input, float threshold, int player);   // 0x402880 input query
int *CutScene_StackSlot(int n);                      // 0x436c00
RpClump *RpClumpClone(RpClump *c);                   // 0x4bc200

// Posts script message `message` from this body's actor to `target` (the actor itself, or the other actor of a hit) at `pos`.
void Corpus::Post(void *message, void *target, const RwV3d *pos, float t)
{
	((CpActor *)m_owner)->PostMessage(message, target, pos, t);
}

// CORPUS_PHYSICS_PROCESS (each tick: data 0 the tick, then the physics step's records), CORPUS_SHADOW_PROCESS and CORPUS_CAMERA_PROCESS (visitors that want the bodies on the ground / in the camera's way).
// 0x4283e0
void Corpus::HandleEvents(RWS::CMsg &msg)
{
	if (msg.Id == g_msgCorpusPhysics.pEventId)
		HandlePhysics(msg.pData);
	else if (msg.Id == g_msgCorpusShadow.pEventId)
		HandleShadowProbe((ShadowPass *)msg.pData);
	else if (msg.Id == g_msgCorpusCamera.pEventId)
		HandleCameraProbe((CameraProbe *)msg.pData);
}

// CORPUS_PHYSICS_PROCESS: data -1 = queue the clump for rendering, 0 = the tick, else a record of the physics step.
void Corpus::HandlePhysics(void *data)
{
	if ((intptr_t)data == -1) {
		QueueAtomics();
		return;
	}
	if (m_flags & CORPUS_NO_PHYSICS)
		return;
	if (!data) {
		Tick();
		return;
	}
	CorpusProbe *rec = (CorpusProbe *)data;
	if (rec->m_firstPass) {
		if (rec->m_t != 0.0f)
			ResolveGroundContact(rec);
		return;
	}
	if (rec->m_rem != 0.0f) {
		if (!(m_flags & CORPUS_STEP_DONE)) {
			RwV3d part;
			RwV3dScale(&part, &m_owner->m_velocity, rec->m_t);
			RwFrameTranslate(m_frame, &part, rwCOMBINEPOSTCONCAT);
		}
		return;
	}
	FinishStep(rec->m_t);
}

void Corpus::QueueAtomics()
{
	RwLLLink *end = &m_owner->m_clump->atomicList.link;
	RwLLLink *cur = end->next;
	if (cur == end)
		return;
	RwLLLink *next;
	do {
		RwCamera *camera = RwCameraGetCurrentCamera();
		next = cur->next;
		RpAtomic *atomic = RpAtomicFromClumpLink(cur);
		PORT_RQ(19);
		RenderQueue_AddAtomic(camera, atomic);
		if (!atomic)
			break;
		cur = next;
	} while (next != end);
}

// The final record of the physics step: the rest of the move (fraction t), landing or leaving the ground, then the end of the step. A body whose step was already done only keeps its touch of a trigger.
void Corpus::FinishStep(float t)
{
	m_moveInput.x = 0.0f;
	if (m_flags & CORPUS_STEP_DONE) {
		m_frameStep.z = 0.0f;
		m_frameStep.y = 0.0f;
		m_frameStep.x = 0.0f;
		if (m_flags & CORPUS_HIT_TRIGGER)
			Post(TFBMember_HitTriggerMessage(), m_owner,
			        (m_flags & CORPUS_AIRBORNE) ? &m_owner->m_worldBox.m_frame.pos : &m_contactPoint, -1.0f);
		UpdateTint();
		return;
	}
	m_flags |= CORPUS_STEP_DONE;
	if (m_flags & CORPUS_GROUND_FOUND) {   // the probe's triangle normal, in m_frameStep during the step
		RwV3dScale(&m_groundNormal, &m_frameStep, 0.0001f);
		RwV3dNormalize(&m_groundNormal, &m_groundNormal);
	}
	RwV3d rest;
	RwV3dScale(&rest, &m_owner->m_velocity, t);
	RwFrameTranslate(m_frame, &rest, rwCOMBINEPOSTCONCAT);
	if ((m_flags & CORPUS_GROUND_FOUND) && !BouncesOff())
		Land();
	else
		NoGround();
	EndStep();
}

// An airborne bouncy body hitting the ground fast enough bounces off instead of landing.
bool Corpus::BouncesOff()
{
	const RwV3d &move = m_owner->m_velocity;
	float restitution = m_owner->m_params.m_bounceRestitution;
	return (m_flags & CORPUS_AIRBORNE) && restitution > 0.0f
	       && fabsf((move.y * m_groundNormal.y + move.x * m_groundNormal.x + move.z * m_groundNormal.z) * (restitution * 0.5f)) > 0.25f;
}

// On the ground after the step: the frame goes to the probe's contact point; a landing reports HitGround (and BounceNormal unless a collision changed the velocity) and keeps only the velocity along the ground; the body stands on its carrier or the world.
void Corpus::Land()
{
	m_contactPoint = m_carrierFrame.pos;   // the probe's contact point (Corpus_RunPhysicsStep stores it there)
	RwMatrix *frame = Modelling();
	RwV3d snap;
	RwV3dSub(&snap, &m_contactPoint, &frame->pos);
	RwFrameTranslate(m_frame, &snap, rwCOMBINEPOSTCONCAT);
	if (m_flags & CORPUS_HIT_TRIGGER)
		Post(TFBMember_HitTriggerMessage(), m_owner, &m_contactPoint, -1.0f);
	if (m_flags & CORPUS_AIRBORNE) {
		if (!(m_flags & CORPUS_LEFT_GROUND)) {
			m_stateTime = 0.0f;
			Post(TFBMember_HitGroundMessage(), m_owner, &m_contactPoint, 0.0f);
		}
		if (!(m_flags & CORPUS_COLLIDED)) {
			const RwV3d *move = &m_owner->m_velocity;
			float into = move->y * m_groundNormal.y + move->x * m_groundNormal.x + move->z * m_groundNormal.z;
			if (into < 0.0f) {   // the move mirrored at the ground
				RwV3d bounce;
				RwV3dScale(&bounce, &m_groundNormal, into * -2.0f);
				RwV3dAdd(&bounce, &bounce, move);
				Post(TFBMember_BounceNormalMessage(), m_owner, &bounce, -1.0f);
			}
		}
		m_lateJumpTime = 0.0f;
		m_riseTime = 0.0f;
		float normalSpeed = m_groundNormal.y * m_velocity.y + m_groundNormal.x * m_velocity.x + m_groundNormal.z * m_velocity.z;
		RwV3d along;
		RwV3dScale(&along, &m_groundNormal, -normalSpeed);
		RwV3dAdd(&along, &along, &m_velocity);
		RwMatrix level;
		if (RotationFromTo(&m_groundNormal, &g_corpusYAxis, &level))
			RwV3dTransformVectors(&along, &along, 1, &level);
		m_velocity.x = along.x;
		m_velocity.z = along.z;
		m_velocity.y = 0.0f;
	}
	m_flags &= ~(CORPUS_FELL_OUT | CORPUS_JUMP_STARTED | CORPUS_LEFT_GROUND | CORPUS_AIRBORNE);
	if (!m_hitCarrier) {
		DetachFromCarrier();
		return;
	}
	if (!m_link.m_carrier || m_link.m_atomic != m_hitAtomic) {   // a new carrier (or another atomic of it): remember its frame
		m_link.m_atomic = m_hitAtomic;
		RwMatrixInvert(&m_carrierFrame, RwFrameGetLTM(RpAtomicGetFrame(m_link.m_atomic)));
		AttachToCarrier(static_cast<GameActor *>(m_hitCarrier));
	}
	Script::ScriptReference *carrier = m_link.m_carrier->m_owner;
	if (carrier->GetType() == TFBType_Actor()) {   // both actors hear of it
		Post(TFBMember_HitActorMessage(), carrier, &m_contactPoint, -1.0f);
		((CpActor *)carrier)->PostMessage(TFBMember_HitActorMessage(), m_owner, &m_contactPoint, -1.0f);
	}
}

// No ground after the step (or bounced off it): a body that stood leaves the ground (walking off an edge allows a late jump), reporting LeftGround and JumpStarted. Without ground contact it rides its attachment point, and a probe straight down finds the ground for the shadow and the ground colour.
void Corpus::NoGround()
{
	RwMatrix *frame = Modelling();
	if (!(m_flags & CORPUS_AIRBORNE)) {
		LeaveGround(0, true);
		m_velocity = m_owner->m_velocity;
	}
	if (m_flags & CORPUS_LEFT_GROUND) {
		m_stateTime = 0.0f;
		Post(TFBMember_LeftGroundMessage(), m_owner, &frame->pos, 0.0f);
	}
	if (m_flags & CORPUS_JUMP_STARTED)
		Post(TFBMember_JumpStartedMessage(), m_owner, &m_lastFrame.pos, 0.0f);
	m_flags &= ~(CORPUS_JUMP_STARTED | CORPUS_LEFT_GROUND);
	if (m_flags & CORPUS_GROUND_FOUND) {   // bounced: to the contact point, a little above the ground
		m_contactPoint = m_carrierFrame.pos;
		RwV3d snap;
		RwV3dSub(&snap, &m_contactPoint, &frame->pos);
		RwV3dIncrementScaled(&snap, &m_groundNormal, 0.01f);
		RwFrameTranslate(m_frame, &snap, rwCOMBINEPOSTCONCAT);
		if (m_flags & CORPUS_HIT_TRIGGER)
			Post(TFBMember_HitTriggerMessage(), m_owner, &m_contactPoint, -1.0f);
		return;
	}
	if (m_link.m_carrier && m_link.m_index == -1) {   // riding: move as the attachment point's body moved
		m_velocity = RWS::TFB::ActorOf(*m_link.m_carrier).m_corpus->m_frameStep;
		m_owner->m_velocity = m_velocity;
		RwV3d target;   // two steps as the game: the sum is rounded to float first
		RwV3dAdd(&target, &m_owner->m_velocity, &m_contactPoint);
		RwV3dSub(&target, &target, &frame->pos);
		RwFrameTranslate(m_frame, &target, rwCOMBINEPOSTCONCAT);
	}
	ProbeGroundBelow();
}

// A line from just above the body 6000 units down: the nearest world triangle gives the contact point and normal; on a prelit floor, the prelight colour at the hit (halved towards grey) colours the actor.
void Corpus::ProbeGroundBelow()
{
	const RwV3d *pos = &Modelling()->pos;
	NearestTriHit hit;
	RpIntersection line;
	hit.dist = 3.4028235e+38f;
	line.type = rpINTERSECTLINE;
	line.t.line.start.x = pos->x;
	line.t.line.start.y = pos->y + 0.25f;
	line.t.line.start.z = pos->z;
	line.t.line.end.x = pos->x;
	line.t.line.end.y = pos->y - 6000.0f;
	line.t.line.end.z = pos->z;
	if (!RWS::CLevel::sm_pActiveWorld)
		return;
	RpCollisionWorldForAllIntersections(RWS::CLevel::sm_pActiveWorld, &line, Collision_NearestWorldTriangleCB, &hit);
	if (hit.dist == 3.4028235e+38f)
		return;
	m_flags |= CORPUS_GROUND_FOUND;
	m_contactPoint.x = line.t.line.start.x;
	m_contactPoint.y = line.t.line.start.y - hit.dist * 6000.0f;
	m_contactPoint.z = line.t.line.start.z;
	m_groundNormal = hit.normal;
	if (!hit.preLit || !(m_groundNormal.y >= 0.7071f))
		return;
	const unsigned short *tri = (const unsigned short *)hit.triangle;
	const unsigned char *preLit = (const unsigned char *)hit.preLit;
	RwV3d c0, c1, c2, d1, d2;
	c0.x = (float)preLit[tri[0] * 4];
	c0.y = (float)preLit[tri[0] * 4 + 1];
	c0.z = (float)preLit[tri[0] * 4 + 2];
	c1.x = (float)preLit[tri[1] * 4];
	c1.y = (float)preLit[tri[1] * 4 + 1];
	c1.z = (float)preLit[tri[1] * 4 + 2];
	c2.x = (float)preLit[tri[2] * 4];
	c2.y = (float)preLit[tri[2] * 4 + 1];
	c2.z = (float)preLit[tri[2] * 4 + 2];
	d1 = V3Sub(&c1, &c0);
	d2 = V3Sub(&c2, &c0);
	m_link.m_u = hit.uv[0];
	m_link.m_v = hit.uv[1];
	m_groundColor[0] = (unsigned char)(int)(((d2.x * hit.uv[1] + d1.x * hit.uv[0] + c0.x) - 128.0f) * 0.5f + 128.0f);
	m_groundColor[1] = (unsigned char)(int)(((d2.y * hit.uv[1] + d1.y * hit.uv[0] + c0.y) - 128.0f) * 0.5f + 128.0f);
	m_groundColor[2] = (unsigned char)(int)(((d2.z * hit.uv[1] + d1.z * hit.uv[0] + c0.z) - 128.0f) * 0.5f + 128.0f);
}

// The end of the physics step: velocity back to units per second (along the slope when standing), the frame step, the frame kept for the next tick, the bounds, the colour.
void Corpus::EndStep()
{
	RwV3dScale(&m_velocity, &m_velocity, g_frameRate);
	m_owner->m_velocity = m_velocity;
	if (!(m_flags & CORPUS_AIRBORNE)) {
		RwMatrix slope;
		if (RotationFromTo(&g_corpusYAxis, &m_groundNormal, &slope))
			RwV3dTransformVectors(&m_owner->m_velocity, &m_owner->m_velocity, 1, &slope);
	}
	RwMatrix *frame = Modelling();
	m_frameStep.x = frame->pos.x - m_lastFrame.pos.x;
	m_frameStep.y = frame->pos.y - m_lastFrame.pos.y;
	m_frameStep.z = frame->pos.z - m_lastFrame.pos.z;
	m_lastFrame = *frame;
	m_owner->UpdateBounds(frame, 0.0f);
	UpdateTint();
}

// The actor's colour: the ground colour (or its own tint when it ignores that), with its alpha (times the alpha of the body it rides).
void Corpus::UpdateTint()
{
	GameActor *owner = m_owner;
	unsigned char alpha = owner->m_tint[3];
	m_groundColor[3] = alpha;
	if (m_link.m_carrier && m_link.m_index == -1)
		m_groundColor[3] = (unsigned char)(((int)RWS::TFB::ActorOf(*m_link.m_carrier).m_corpus->m_groundColor[3] * (int)alpha + 0x7f) / 255);
	if (!(owner->m_params.m_flags & RWS::TFB::ACTOR_IGNORES_GROUND_COLOR)) {
		ClumpSetAtomicColour(owner->m_clump, (RwRGBA *)m_groundColor);
	} else {
		unsigned char rgba[4];
		rgba[0] = owner->m_tint[0];
		rgba[1] = owner->m_tint[1];
		rgba[2] = owner->m_tint[2];
		rgba[3] = m_groundColor[3];
		ClumpSetAtomicColour(owner->m_clump, (RwRGBA *)rgba);
	}
}

// The tick (data 0, before the physics step): the wanted direction (scripted move, else the stick), the jump button, animation, ApplyMovement, the model update, footsteps.
void Corpus::Tick()
{
	RwV3d dir;
	dir.z = 0.0f;
	dir.y = 0.0f;
	dir.x = 0.0f;
	if (!(m_flags & CORPUS_PLAYER_CONTROLLED)) {
		if (m_moveInput.x != 0.0f)
			SteerToTarget(&dir);
	} else {
		m_jumpPressed = 0;
		if (m_moveInput.x != 0.0f)
			SteerToTarget(&dir);
		else if (m_controller >= 0)
			ReadStick(&dir);
		ReadJumpButton();
		if (m_animOverride == -1 && m_riseTime >= 0.0f)
			UpdateAnimation(&dir);
	}
	m_stateTime = g_frameDt + m_stateTime;
	ApplyMovement(&dir);
	// a cut-scene frame the clump was not rendered in moves the animation on a little
	float step = 0.0f;
	if (*CutScene_StackSlot(0) == 2 && TFBPlugin_B1_GetRenderFrame(m_owner->m_clump) != RWS::MainLoop::Render::GetFrameCount())
		step = 0.1f;
	m_model.SetUpdateInterval(step);
	if (((Script::ReferenceSet *)m_owner->m_turrets)->m_count != 0 || m_feet.m_count != 0) {   // aim the attachments after the animation
		m_model.SetUpdateMatricesCallback((void *)&AimAttachmentsCB, m_owner);
	}
	m_model.Advance(g_frameDt);
	if (m_feet.m_count != 0 && (m_flags & (CORPUS_JUMP_STARTED | CORPUS_AIRBORNE)) != CORPUS_AIRBORNE)
		Footsteps();
}

// The player's stick: a dead zone of 0.4 per axis, rescaled to 0..1; m_moveInput.x = the push (0 below 0.05, at most 1); the direction is turned into the camera's view, level, scaled by the push.
void Corpus::ReadStick(RwV3d *dir)
{
	dir->x = GetPlayerInput(0x0e, 0.4f, m_controller);
	dir->z = GetPlayerInput(0x0f, 0.4f, m_controller);
	if (dir->x < 0.0f)
		dir->x = (dir->x + 0.4f) * 1.6666666f;
	else if (dir->x > 0.0f)
		dir->x = (dir->x - 0.4f) * 1.6666666f;
	if (dir->z < 0.0f)
		dir->z = (dir->z + 0.4f) * 1.6666666f;
	else if (dir->z > 0.0f)
		dir->z = (dir->z - 0.4f) * 1.6666666f;
	float push = RwV3dLength(dir);
	m_moveInput.x = push;
	if (push <= 0.05f) {
		m_moveInput.x = 0.0f;
		dir->z = 0.0f;
		dir->x = 0.0f;
		return;
	}
	if (push >= 1.0f)
		m_moveInput.x = 1.0f;
	RwV3dTransformVectors(dir, dir, 1, RwFrameGetLTM(RwCameraGetFrame(GetActiveGameCamera())));
	dir->y = 0.0f;
	RwV3dNormalize(dir, dir);
	dir->x = dir->x * m_moveInput.x;
	dir->z = dir->z * m_moveInput.x;
}

// The jump button (read once per tick: m_controller is reset): m_jumpHoldTime counts the hold; a press counts for the first 0.25 s of a hold and for the whole rise of a jump.
void Corpus::ReadJumpButton()
{
	if (m_controller == -1) {
		m_jumpPressed = 0;
		m_jumpHoldTime = 3.4028235e+38f;
		return;
	}
	unsigned char held;
	if (GetPlayerInput(4, (float)m_controller, 0) > 0.15f) {
		held = 1;
		m_jumpHoldTime = g_frameDt + m_jumpHoldTime;
	} else {
		held = 0;
		m_jumpHoldTime = 0.0f;
	}
	m_controller = -1;
	if (m_jumpHoldTime < 0.25f || m_riseTime > 0.0f)
		m_jumpPressed = held;
}

// A step plays the next footstep sound of the surface, a print posts FootPrint and (on dusty surfaces) leaves a dust clump. Only while a ground animation (slots 1..0x3e except 0x2d) plays and the actor is visible enough (alpha above 10).
void Corpus::Footsteps()
{
	int slot = m_model.GetLastSlot();
	if (slot <= 0 || slot >= 0x3f || slot == 0x2d || m_groundColor[3] <= 0x0a)
		return;
	m_modelPtr->ResolveSurfaceEffects();
	SurfaceEffects *effects = m_modelPtr->m_surfaces;   // per surface: a dust clump and four footstep sounds
	for (Script::SetNode *node = m_feet.m_head; node; node = node->m_next) {
		FootPoint *foot = static_cast<FootPoint *>(node->m_object);
		RwV3d at;
		float event = foot->CrossGround(&at);
		if (event == 0.0f)
			continue;
		if (event < 0.0f) {   // a step: the next sound of the surface's four (empty slots skipped)
			CpSnd *sound = (CpSnd *)effects[m_surface].m_steps[m_footstepSound];
			if (!sound) {
				m_footstepSound = 0;
				sound = (CpSnd *)effects[m_surface].m_steps[0];
				if (!sound)
					continue;
			}
			g_voiceDefaultRef = m_owner;
			sound->Play(m_owner);
			do
				m_footstepSound = (m_footstepSound + 1) & 3;
			while (!effects[m_surface].m_steps[m_footstepSound]);
			continue;
		}
		if (m_link.m_carrier)   // prints only on the world
			continue;
		unsigned now = TFBClock_GetRawTime();
		int surface = m_surface;
		if (foot->m_effectEnd <= now || !SurfaceHoldsFootprint(surface)) {
			foot->m_effectEnd = FootprintLifetimeMs(surface);
			if (foot->m_effectEnd) {
				foot->m_effectEnd += now;
				foot->m_effectSurface = surface;
			}
		}
		RwV3d world;
		RwV3dTransformPoint(&world, &at, Modelling());
		Post(TFBMember_FootPrintMessage(), m_owner, &world, 0.0f);
		if (foot->m_effectEnd > now && foot->m_effectSurface != surface && SurfaceHoldsFootprint(surface))
			surface = foot->m_effectSurface;
		else if (!SurfaceRaisesDust(surface))
			continue;
		if (!effects[surface].m_dust)
			continue;
		at.y = at.y + 0.5f;
		Script::ClumpCloneRef *dust = new Script::ClumpCloneRef(RpClumpClone(effects[surface].m_dust), (const RwV3d *)Modelling(), &at,
		                                                        (intptr_t)&m_groundNormal);
		CloneList_Get()->Insert(dust, 3.4028235e+38f);
	}
}

// CORPUS_SHADOW_PROCESS: a body standing on something (not riding) casts a shadow: the visitor gets its collider.
void Corpus::HandleShadowProbe(ShadowPass *visitor)
{
	if ((m_flags & CORPUS_GROUND_FOUND) && m_owner->m_clump && !(m_link.m_carrier && m_link.m_index == -1))
		visitor->Visit(static_cast<Collider *>(m_owner));
}

// CORPUS_CAMERA_PROCESS: an actor that blocks the camera and is hit by the camera's probe (a segment through its box, or a sphere touching its bounds) goes to the visitor.
void Corpus::HandleCameraProbe(CameraProbe *probe)
{
	GameActor *owner = m_owner;
	if (!owner->m_clump || !(owner->m_params.m_flags & RWS::TFB::ACTOR_BLOCKS_CAMERA))
		return;
	if (probe->m_isect.type == rpINTERSECTLINE) {
		RwV3d d;
		RwV3dSub(&d, &probe->m_isect.t.line.end, &probe->m_isect.t.line.start);
		if (((CpBox *)&owner->m_worldBox)->TestPoint(&probe->m_isect.t.line.start, &d, 0) < 3)
			return;
	} else if (probe->m_isect.type == rpINTERSECTSPHERE) {   // distance from the sphere's centre to the bounds, per axis
		const RwV3d &c = probe->m_isect.t.sphere.center;
		const RwV3d &lo = owner->m_min.m_point;
		const RwV3d &hi = owner->m_max.m_point;
		float dx, dy, dz;
		if (c.x < lo.x)
			dx = lo.x - c.x;
		else if (c.x > hi.x)
			dx = c.x - hi.x;
		else
			dx = 0.0f;
		if (c.y < lo.y)
			dy = lo.y - c.y;
		else if (c.y > hi.y)
			dy = c.y - hi.y;
		else
			dy = 0.0f;
		if (c.z < lo.z)
			dz = lo.z - c.z;
		else if (c.z > hi.z)
			dz = c.z - hi.z;
		else
			dz = 0.0f;
		if (!(probe->m_isect.t.sphere.radius * probe->m_isect.t.sphere.radius >= dx * dx + dy * dy + dz * dz))
			return;
	}
	probe->ForEachAtomic(static_cast<Collider *>(owner));
}

inline void CpActor::PostMessage(void *msgType, void *actor, const RwV3d *pos, float t)
{
	((Script::Placement *)this)->PostMessage((Script::ScriptReference *)msgType, (Script::ScriptReference *)actor, (Vec3 *)pos, t);
}
inline int CpBox::TestPoint(const RwV3d *p, const RwV3d *d, int z) { return ((const CollisionBox *)this)->TestSegment(p, d, (RwReal *)(intptr_t)z); }
