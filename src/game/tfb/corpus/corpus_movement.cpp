// Corpus::ApplyMovement: the movement of one tick.
#define ZOOSTERS_RWS_SENDMSG 1
#include "tfb/corpus/corpus.h"
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptactor.h"
#include "script/types/actorturret.h"
#include "script/types/partentry.h"
#include "modules/tfb/gameactor.h"
#include "tfb/corpus/corpus_private.h"
#include <math.h>
#include <string.h>

using RWS::TFB::ActorParams;
using RWS::TFB::Collider;
using RWS::TFB::GameActor;

// GameActor's Is* slots return a bool in al; script/actor.h still declares them int, so only the low byte counts.
static bool LowByte(int v)
{
	return (unsigned char)v != 0;
}

// |v| < 0.01 (NaN is not tiny)
static bool IsTiny(float v)
{
	return v < 0.01f && v > -0.01f;
}

// The body's frame matrix: the actor's placement, rebuilt by every ApplyMovement.
RwMatrix *Corpus::Modelling()
{
	return &m_frame->modelling;
}

// The movement of one tick, before the physics step: jump and fall state, carrier, steering towards `dir` and gravity, alignment with the ground. Writes the displacement into the owner's Collider::m_velocity for the physics step to sweep. `dir` is the wanted direction (world axes, length = the stick's push); it is normalised and scaled to this tick's push. m_velocity is per tick while this runs.
// 0x426920
void Corpus::ApplyMovement(RwV3d *dir)
{
	RwV3d &move = m_owner->m_velocity;
	move.x = 0.0f;
	move.y = 0.0f;
	move.z = 0.0f;
	RwV3dScale(&m_velocity, &m_velocity, g_frameDt);   // per second -> per tick
	const float fallLimit = -(g_corpusGravityDt * m_owner->m_params.m_downSpeed);   // fastest fall per tick
	if ((m_flags & CORPUS_AIRBORNE) && !m_link.m_carrier) {
		UpdateAirborne(fallLimit);
	} else {
		if (m_link.m_carrier)
			FollowCarrier();
		UpdateJumpStart();
	}
	m_animOverride = -1;
	m_moveInput.y = 0.0f;
	if (!(m_link.m_carrier && m_link.m_index < 0)) {   // a body riding an attachment point only follows it
		Steer(dir, fallLimit);
		if (m_owner->m_params.m_flags & RWS::TFB::ACTOR_GROUND_TILTER)
			LimitTilt();
	}
	FinishMovement();
}

// Airborne and not carried: the rise of a held jump, the late-jump window after walking off an edge, and the fall (ends with ground under a body of move type 0 or 1, or puts the body back at its origin after falling at top speed too long).
void Corpus::UpdateAirborne(float fallLimit)
{
	if (m_riseTime > 0.0f) {   // a jump is rising: until its time is up or the button is let go
		m_riseTime -= g_frameDt;
		if (m_riseTime <= 0.0f || (m_lateJumpTime == 0.0f && !m_jumpPressed)) {
			m_lateJumpTime = 0.0f;
			m_riseTime = 0.0f;
		}
		return;
	}
	if (m_lateJumpTime != 0.0f) {   // walked off an edge a moment ago: a jump still starts
		m_lateJumpTime -= g_frameDt;
		if (m_lateJumpTime <= 0.0f)
			m_lateJumpTime = 0.0f;
		if (m_jumpPressed && m_jumpSpeed != 0.0f)
			Jump(m_jumpSpeed);
		return;
	}
	const ActorParams &params = m_owner->m_params;
	if (m_flags & CORPUS_GROUND_FOUND) {
		if (params.m_moveType < 2)
			m_riseTime = 0.0f;
	} else if (m_velocity.y - 1.0f <= fallLimit && fallLimit != 0.0f) {   // at top fall speed: m_riseTime counts the time down
		m_riseTime -= g_frameDt;
		if (m_riseTime < -10.0f) {
			ReturnToOrigin();
			return;
		}
	}
	if (params.m_downSpeed * m_riseTime <= -50.0f)   // fell 50 units at top speed: out of the world
		ReturnToOrigin();
}

// Fell out of the world: back to its origin, at rest. The first time it reports (reduced to the name lookup).
void Corpus::ReturnToOrigin()
{
	m_riseTime = 0.0f;
	m_velocity.x = 0.0f;
	m_velocity.y = 0.0f;
	m_velocity.z = 0.0f;
	m_owner->PlaceAt(&m_owner->m_origin);
	if (!(m_flags & CORPUS_FELL_OUT)) {
		m_flags |= CORPUS_FELL_OUT;
		static_cast<Script::ScriptReference *>(m_owner)->GetName();
	}
}

// Carried by another actor: moves with it, or falls off when it no longer carries (clump gone, body rebuilt since the link was made, mesh no longer solid).
void Corpus::FollowCarrier()
{
	Collider *carrier = m_link.m_carrier;
	GameActor *carrierActor = (GameActor *)carrier->m_owner;
	bool carries = carrier->m_clump && (unsigned)m_link.m_id >= (unsigned)RWS::TFB::ActorOf(*carrier).m_corpus->m_id;
	if (carries && m_link.m_index >= 0)
		carries = LowByte(carrierActor->IsSolidCollider()) && LowByte(carrierActor->IsMeshCollidee());
	if (!carries) {
		LeaveGround(0, false);
		RwV3dAdd(&m_velocity, &m_velocity, &m_owner->m_velocity);
		return;
	}
	AttachToCarrier(static_cast<GameActor *>(carrier));
	if (m_link.m_index == -1)
		RideAttachPoint();
	else
		StandOnCarrierMesh();
	// carried this tick: from the last position to the new contact point, plus the carrier's own displacement
	RwV3d fromLast = V3Sub(&m_contactPoint, &m_lastFrame.pos);
	RwV3d carried = V3Add(&fromLast, &m_link.m_carrier->m_velocity);
	RwV3dAdd(&m_owner->m_velocity, &m_owner->m_velocity, &carried);
}

// Riding an attachment point of the carrier: the frame takes the point's world matrix, the facing its yaw; the body does not move by itself.
void Corpus::RideAttachPoint()
{
	RwFrameTransform(m_frame, RwFrameGetLTM(m_link.m_frame), rwCOMBINEREPLACE);
	if (((Script::ActorTurret *)m_link.m_attachPoint)->m_entry->m_kind != 1)   // entry kind: not a turret
		RwFrameTransform(m_frame, m_link.m_attachPoint->GetLocalMatrix(), rwCOMBINEPRECONCAT);
	const RwMatrix *ltm = RwFrameGetLTM(m_frame);
	m_contactPoint = ltm->pos;
	RwMatrix *frame = Modelling();
	frame->pos.z = 0.0f;   // FinishMovement puts the position back
	frame->pos.y = 0.0f;
	frame->pos.x = 0.0f;
	float yaw = (float)atan2(ltm->at.x, ltm->at.z);
	m_owner->m_heading = 0.0f;
	m_owner->m_facing = yaw * kRadToDeg;
	m_velocity.x = 0.0f;
	m_velocity.y = 0.0f;
	m_velocity.z = 0.0f;
}

// Standing on triangle m_link.m_index of the carrier's mesh: contact point (barycentric m_u, m_v) and ground normal move with the carrier; if the carrier turned since the body got on, the heading follows its yaw.
void Corpus::StandOnCarrierMesh()
{
	RpAtomic *atomic = m_link.m_atomic;
	const RwMatrix *ltm = RwFrameGetLTM(RpAtomicGetFrame(atomic));
	RpGeometry *geometry = RpAtomicGetGeometry(atomic);
	const RpTriangle *tri = &RpGeometryGetTriangles(geometry)[m_link.m_index];
	const RwV3d *verts = RpMorphTargetGetVertices(RpGeometryGetMorphTarget(geometry, 0));
	const RwV3d *v0 = &verts[tri->vertIndex[0]];
	const RwV3d *v1 = &verts[tri->vertIndex[1]];
	const RwV3d *v2 = &verts[tri->vertIndex[2]];
	RwV3d edge1 = V3Sub(v1, v0);
	RwV3d edge2 = V3Sub(v2, v0);
	m_contactPoint.x = edge1.x * m_link.m_u + edge2.x * m_link.m_v + v0->x;
	m_contactPoint.y = edge1.y * m_link.m_u + edge2.y * m_link.m_v + v0->y;
	m_contactPoint.z = edge1.z * m_link.m_u + edge2.z * m_link.m_v + v0->z;
	RwV3dTransformPoint(&m_contactPoint, &m_contactPoint, ltm);
	RwV3dCrossProduct(&m_groundNormal, &edge1, &edge2);
	RwV3dScale(&m_groundNormal, &m_groundNormal, 0.0001f);   // scaled down before normalising (large triangles)
	RwV3dNormalize(&m_groundNormal, &m_groundNormal);
	RwV3dTransformVectors(&m_groundNormal, &m_groundNormal, 1, ltm);
	RwMatrix turned;   // the carrier's turn since the body got on
	RwMatrixMultiply(&turned, &m_carrierFrame, ltm);
	float heading = m_owner->m_heading * kDegToRad;
	float misalign = sin(heading) * turned.at.z - cos(heading) * turned.at.x;
	if (!(fabsf(misalign) < 0.015625f))
		m_owner->m_heading = (float)atan2(turned.at.x, turned.at.z) * kRadToDeg;
}

// On the ground or carried: a wound-up standing jump launches, or a press starts a jump (a player standing still with a wind-up animation winds up first: m_riseTime -1 until it ends).
void Corpus::UpdateJumpStart()
{
	const float jumpSpeed = m_owner->m_params.m_initialJumpVelocity;
	if (m_riseTime < 0.0f) {   // winding up
		if (m_jumpHoldTime != 0.0f && MayWindUpJump() && m_model.IsSlotPlaying(CORPUS_ANIM_JUMP_WINDUP)
		    && !m_model.WillFinish(g_frameDt))
			return;
		if (m_animOverride == -1) {
			m_model.PlaySlot(CORPUS_ANIM_RISE, 0.0f);
			m_prevIdleSet = 0;
		}
		Jump(m_jumpSpeed);
		return;
	}
	if (!m_jumpPressed || jumpSpeed == 0.0f)
		return;
	if ((m_flags & CORPUS_PLAYER_CONTROLLED) && MayWindUpJump() && m_model.HasSlotAnimation(CORPUS_ANIM_JUMP_WINDUP)) {
		m_riseTime = -1.0f;
		m_jumpSpeed = jumpSpeed;
		m_model.PlaySlot(CORPUS_ANIM_JUMP_WINDUP, 0.0f);
		m_prevIdleSet = 0;
		return;
	}
	Jump(jumpSpeed);
}

// No script animation forced, no stick push and no horizontal velocity: a jump from here winds up first.
bool Corpus::MayWindUpJump() const
{
	return m_animOverride == -1 && m_moveInput.x == 0.0f && m_velocity.x == 0.0f && m_velocity.z == 0.0f;
}

// Starts a jump with launch speed `speed` (units per second): leaves the ground keeping this tick's carried motion, rises for the actor's jump coast time while the button is held.
void Corpus::Jump(float speed)
{
	LeaveGround(0, false);
	RwV3dAdd(&m_velocity, &m_velocity, &m_owner->m_velocity);
	m_velocity.y = g_corpusGravityDt * speed + m_velocity.y;
	m_jumpSpeed = 0.0f;
	m_lateJumpTime = 0.0f;
	m_riseTime = (float)m_owner->m_params.m_coastFrames * (1.0f / 60.0f);
	m_flags |= CORPUS_JUMP_STARTED;
}

// Turns towards the target facing, accelerates (along the facing, or `dir` for free movers), decelerates, limits the speed, then gravity while airborne or the ground's slope.
void Corpus::Steer(RwV3d *dir, float fallLimit)
{
	const ActorParams &params = m_owner->m_params;
	const bool airborne = (m_flags & CORPUS_AIRBORNE) != 0;
	const bool player = (m_flags & CORPUS_PLAYER_CONTROLLED) != 0;
	float maxSpeed, accel, decel, verticalSpeed;
	if (airborne) {   // steering is horizontal: the vertical speed is set aside until gravity below
		maxSpeed = g_corpusGravityDt * params.m_airSpeed;
		verticalSpeed = m_velocity.y;
		accel = params.m_airAccel;
		decel = params.m_airDecel * g_corpusGravityDt2;
		m_velocity.y = 0.0f;
	} else {
		maxSpeed = g_corpusGravityDt * params.m_groundSpeed;
		accel = params.m_groundAccel;
		verticalSpeed = 0.0f;
		decel = g_corpusGravityDt2 * params.m_groundDecel;
	}
	GameActor *owner = m_owner;
	if (params.m_moveType != RWS::TFB::ACTOR_MOVE_FREE && !(IsTiny(dir->x) && IsTiny(dir->z)))
		owner->m_targetFacing = (float)atan2(dir->x, dir->z) * kRadToDeg;
	else if (owner->m_targetFacing == kNoFacing)
		owner->m_targetFacing = owner->GameActor::GetFacing();   // a direct call in the game, not through the vtable
	float speed = RwV3dLength(&m_velocity);
	maxSpeed = Turn(speed, maxSpeed);
	RwFrameRotate(m_frame, &g_corpusYAxis, owner->m_targetFacing, rwCOMBINEREPLACE);   // FinishMovement adds the position
	RwMatrix *frame = Modelling();

	// the push of this tick: none, along the facing, or (free movers) along `dir`
	float lengthSq = (dir->y * dir->y + dir->x * dir->x) + dir->z * dir->z;
	if (lengthSq <= 1.0000001e-06f) {
		dir->z = 0.0f;
		dir->x = 0.0f;
	} else {
		if (lengthSq < 0.99979997f) {   // a partial push: a unit direction, the top speed scaled by the push
			float length = sqrtf(lengthSq);
			float inverse = 1.0f / length;
			dir->x *= inverse;
			dir->z *= inverse;
			maxSpeed *= length;
		}
		// walkers push only once they face `dir` closely enough: within 45 degrees, players from rest within 5, moving within 135
		float cosLimit = !player ? 0.7071f : speed != 0.0f ? -0.7071f : 0.9962f;
		int moveType = params.m_moveType;
		bool push = accel != 0.0f
		            && (!(moveType == RWS::TFB::ACTOR_MOVE_WALK || moveType == 3)
		                || !(frame->at.z * dir->z + frame->at.x * dir->x < cosLimit));
		if (push) {
			if (moveType != RWS::TFB::ACTOR_MOVE_FREE) {
				if (player && moveType == RWS::TFB::ACTOR_MOVE_WALK && m_moveInput.x != 0.0f) {   // a walking player keeps its speed through the turn
					m_velocity.x = speed * frame->at.x;
					m_velocity.y = speed * frame->at.y;
					m_velocity.z = speed * frame->at.z;
				}
				dir->x = frame->at.x;
				dir->z = frame->at.z;
			}
			RwV3dScale(dir, dir, g_corpusGravityDt2 * accel);
		} else {
			if (player && params.m_moveType == RWS::TFB::ACTOR_MOVE_WALK && m_moveInput.x != 0.0f && accel != 0.0f)
				speed = 0.0f;   // a walking player facing away stops and turns round
			dir->x = 0.0f;
			dir->y = 0.0f;
			dir->z = 0.0f;
		}
	}

	// decelerate, add the push, limit the speed (a body already faster than its top speed is only slowed down)
	float slowed = speed - decel;
	if (slowed <= 0.0f) {
		m_velocity = *dir;
	} else {
		if (maxSpeed + 0.1f < slowed)
			maxSpeed = slowed;
		float keep = slowed / (decel + slowed);
		m_velocity.x = m_velocity.x * keep + dir->x;
		m_velocity.y = m_velocity.y * keep + dir->y;
		m_velocity.z = m_velocity.z * keep + dir->z;
	}
	float speedSq = m_velocity.y * m_velocity.y + m_velocity.x * m_velocity.x + m_velocity.z * m_velocity.z;
	if (speedSq > maxSpeed * maxSpeed)
		RwV3dScale(&m_velocity, &m_velocity, maxSpeed / sqrtf(speedSq));
	m_velocity.y = 0.0f;

	if (airborne) {   // gravity; a held jump keeps rising
		float falling = verticalSpeed - g_corpusGravityDt2 * params.m_downAccel;
		m_velocity.y = falling;
		if (m_riseTime > 0.0f) {
			float limit = g_corpusGravityDt * params.m_upSpeed;
			float rising = g_corpusGravityDt2 * params.m_upAccel + falling;
			m_velocity.y = rising;
			if (rising > limit)
				m_velocity.y = limit;
		}
		if (m_velocity.y < fallLimit)
			m_velocity.y = fallLimit;
		owner->m_velocity = m_velocity;
	} else {
		FollowSlope();
	}
}

// Turns the facing towards m_targetFacing by at most this tick's turn rate; m_moveInput.y is the turn (degrees, -180..180). A player turning while pushing the stick turns faster the further it has to turn, and its top speed drops with the turn; returns the top speed to use.
float Corpus::Turn(float speed, float maxSpeed)
{
	GameActor *owner = m_owner;
	float heading = owner->m_heading + owner->m_facing;
	if (heading < 0.0f)
		heading += 360.0f;
	else if (heading >= 360.0f)
		heading -= 360.0f;
	float &turn = m_moveInput.y;
	turn = owner->m_targetFacing - heading;
	if (turn == 0.0f)
		return maxSpeed;
	float wrapped = (float)fmod((double)turn, 360.0);
	turn = wrapped;
	if (wrapped < -180.0f)
		turn = wrapped + 360.0f;
	else if (wrapped > 180.0f)
		turn = wrapped - 360.0f;
	if (turn == 0.0f)
		return maxSpeed;
	const ActorParams &params = owner->m_params;
	float limit = ((m_flags & CORPUS_AIRBORNE) ? params.m_airTurnRate : params.m_groundTurnRate) * g_frameDt;
	if ((m_flags & CORPUS_PLAYER_CONTROLLED) && m_moveInput.x != 0.0f) {
		float rate = m_moveInput.x * m_moveInput.x;
		if (speed != 0.0f || rate < 0.5f)
			rate = rate * fabsf(turn) * (1.0f / 45.0f);
		if (rate < 0.1f)
			rate = 0.1f;
		else if (rate > 1.0f)
			rate = 1.0f;
		limit = rate * limit;
		maxSpeed = (180.0f - fabsf(turn)) * maxSpeed * (1.0f / 180.0f);
	}
	if (turn < -limit) {
		turn = -limit;
		owner->m_targetFacing = owner->GameActor::GetFacing() - limit;
		if (owner->m_targetFacing < 0.0f)
			owner->m_targetFacing += 360.0f;
	} else if (turn > limit) {
		turn = limit;
		owner->m_targetFacing = owner->GameActor::GetFacing() + limit;
		if (owner->m_targetFacing >= 360.0f)
			owner->m_targetFacing -= 360.0f;
	}
	owner->m_facing = owner->m_targetFacing - owner->m_heading;
	return maxSpeed;
}

// On the ground: the velocity follows the slope. Ground tilters tilt with the ground (at most 45 degrees unless they climb walls); one taller than wide or deep only pitches, its right axis stays level.
void Corpus::FollowSlope()
{
	GameActor *owner = m_owner;
	RwMatrix slope;
	if (!RotationFromTo(&g_corpusYAxis, &m_groundNormal, &slope)) {   // level ground
		RwV3dAdd(&owner->m_velocity, &owner->m_velocity, &m_velocity);
		return;
	}
	RwV3d along;
	RwV3dTransformVectors(&along, &m_velocity, 1, &slope);
	RwV3dAdd(&owner->m_velocity, &owner->m_velocity, &along);
	unsigned actorFlags = owner->m_params.m_flags;
	if (!(actorFlags & RWS::TFB::ACTOR_GROUND_TILTER))
		return;
	RwMatrix *frame = Modelling();
	if (!(actorFlags & RWS::TFB::ACTOR_WALL_CLIMBER) && m_groundNormal.y < 0.7071f) {   // steeper than 45 degrees
		RwV3d axis;
		RwV3dCrossProduct(&axis, &g_corpusYAxis, &m_groundNormal);
		RwV3dNormalize(&axis, &axis);
		RwMatrixRotate(&slope, &axis, 45.0f, rwCOMBINEREPLACE);
	}
	RwMatrixTransform(frame, &slope, rwCOMBINEPOSTCONCAT);
	const RwReal *extent = owner->m_worldBox.m_extent;
	if (extent[1] > extent[0] || extent[1] > extent[2]) {
		RwV3d level;
		RwV3dCrossProduct(&level, &g_corpusYAxis, &frame->at);
		RwV3dNormalize(&level, &level);
		if (level.y * frame->right.y + level.x * frame->right.x + level.z * frame->right.z < 0.0f)
			RwV3dNegate(&level, &level);
		if (RotationFromTo(&frame->right, &level, &slope))
			RwMatrixTransform(frame, &slope, rwCOMBINEPOSTCONCAT);
	}
}

// Ground tilters: the up axis turns at most 2 degrees per tick from the last tick's frame (without the script's tilt and roll).
void Corpus::LimitTilt()
{
	GameActor *owner = m_owner;
	RwMatrix *frame = Modelling();
	RwMatrix last = m_lastFrame;
	if (owner->m_roll != 0.0f)
		RwMatrixRotate(&last, &last.at, -owner->m_roll, rwCOMBINEPOSTCONCAT);
	if (owner->m_tilt != 0.0f)
		RwMatrixRotate(&last, &last.right, -owner->m_tilt, rwCOMBINEPOSTCONCAT);
	const float cosAngle = RwV3dDotProduct(&last.up, &frame->up);
	if (cosAngle < 0.99939f) {   // more than 2 degrees
		RwV3d axis;
		RwV3dCrossProduct(&axis, &last.up, &frame->up);
		if (axis.x != 0.0f || axis.y != 0.0f || axis.z != 0.0f) {
			RwV3dNormalize(&axis, &axis);
			RwFrameRotate(m_frame, &axis, 2.0f - (float)acos(cosAngle) * kRadToDeg, rwCOMBINEPOSTCONCAT);
		}
	}
}

// End of the tick's movement: the script's tilt and roll, the position, the bounds of a body that turned on the spot, the tick's flags.
void Corpus::FinishMovement()
{
	GameActor *owner = m_owner;
	RwMatrix *frame = Modelling();
	if (owner->m_tilt != 0.0f)
		RwFrameRotate(m_frame, &frame->right, owner->m_tilt, rwCOMBINEPOSTCONCAT);
	if (owner->m_roll != 0.0f)
		RwFrameRotate(m_frame, &frame->at, owner->m_roll, rwCOMBINEPOSTCONCAT);
	owner->m_targetFacing = kNoFacing;
	RwFrameTranslate(m_frame, &m_lastFrame.pos, rwCOMBINEPOSTCONCAT);
	// A body that does not move skips the physics step: this tick's record counts as applied. One that turned on the spot updates its bounds here, and takes the step anyway if it has wall collision.
	const RwV3d &move = owner->m_velocity;
	bool skipStep = false;
	if (!(m_flags & CORPUS_COLLIDED) && move.x == 0.0f && move.y == 0.0f && move.z == 0.0f && owner->m_embedded == 0) {
		const RwMatrix &last = m_lastFrame;
		bool turned = last.at.x != frame->at.x || frame->at.y != last.at.y || frame->at.z != last.at.z
		              || frame->right.x != last.right.x || frame->right.y != last.right.y || frame->right.z != last.right.z
		              || last.up.x != frame->up.x || last.up.y != frame->up.y || last.up.z != frame->up.z;
		if (turned) {
			owner->UpdateBounds(frame, 0.0f);
			skipStep = !(owner->m_params.m_flags & RWS::TFB::ACTOR_WALL_COLLISION);
		} else {
			skipStep = true;
		}
	}
	if (!skipStep)
		m_flags &= ~(CORPUS_STEP_DONE | CORPUS_HIT_TRIGGER);
	memcpy(owner->m_worldBox.m_extent, owner->m_localBox.m_extent, sizeof owner->m_worldBox.m_extent);
	owner->m_embedded = 0;
	m_flags &= ~(CORPUS_COLLIDED | CORPUS_HIT_LEDGE | CORPUS_MOVING);
	if (m_velocity.x != 0.0f || m_velocity.z != 0.0f)
		m_flags |= CORPUS_MOVING;
}

// Unit direction from the corpus to the actor's move target (xz), or the facing.
// 0x4243d0
void Corpus::SteerToTarget(RwV3d *out)
{
	RwV3d d;
	RWS::TFB::GameActor *o = m_owner;
	d.x = o->m_destination.x - m_lastFrame.pos.x;
	d.y = 0.0f;
	d.z = o->m_destination.z - m_lastFrame.pos.z;
	if (m_moveInput.x < 0.0f) {
		m_moveInput.x = -m_moveInput.x;
		if (d.x == 0.0f && d.z == 0.0f) {
			float a;
			if (o->m_targetFacing == kNoFacing)
				a = o->RWS::TFB::GameActor::GetFacing();
			else
				a = o->m_targetFacing;
			a *= kDegToRad;
			out->y = 0.0f;
			out->x = (float)sin(a);
			out->z = (float)cos(a);
			return;
		}
		d.x = -d.x;
		d.z = -d.z;
	}
	if (d.x == 0.0f && d.z == 0.0f) {
		m_moveInput.x = 0.0f;
		out->x = 0.0f;
		out->y = 0.0f;
		out->z = 0.0f;
		return;
	}
	RwV3dNormalize(out, &d);
}
