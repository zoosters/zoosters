// The physics step of a tick (MainTickHandler post-tick).
#define ZOOSTERS_RWS_SENDMSG 1
#include "tfb/corpus/corpus.h"
#include "tfb/level/level.h"
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptactor.h"
#include "modules/tfb/gameactor.h"
#include "script/ops_message.h"
#include "tfb/corpus/corpus_private.h"
#include "tfb/collision/broadphase.h"
#include "tfb/corpus/messagemembers.h"

extern RWS::CEventId g_msgCorpusPhysics;
namespace RWS { namespace MainLoop { namespace Logic { extern unsigned Rate; } } }



// The actor as the target of a script message
static Script::Placement *Messages(GameActor *actor)
{
	return (Script::Placement *)actor;
}

// Where a hit happened: the sweep's hit point, or (no sweep hit) the centre of the body's box
static Vec3 *ContactPosition(CorpusProbe &probe, Corpus *body)
{
	return (Vec3 *)(probe.m_t != 0.0f ? &probe.m_hitPos : &body->m_owner->m_worldBox.m_frame.pos);
}

// The nearest triangle on `line`: in the world, or (hit != 0) on the atomics of that actor's clump (their normals in world space)
static void FindNearest(RpIntersection *line, Collider *hit, NearestTriHit *nearest)
{
	if (!hit) {
		RpCollisionWorldForAllIntersections(RWS::CLevel::sm_pActiveWorld, line, Collision_NearestWorldTriangleCB, nearest);
		return;
	}
	RwLLLink *end = &hit->m_clump->atomicList.link;
	RwLLLink *cur = end->next;
	if (cur == end)
		return;
	RwLLLink *next;
	do {
		next = cur->next;
		RpAtomic *atomic = RpAtomicFromClumpLink(cur);
		float before = nearest->dist;
		RpAtomicForAllIntersections(atomic, line, Collision_NearestGeometryTriangleCB, nearest);
		if (nearest->dist < before)
			RwV3dTransformVectors(&nearest->normal, &nearest->normal, 1, RwFrameGetLTM(RpAtomicGetFrame(atomic)));
		if (!atomic)
			break;
		cur = next;
	} while (next != end);
}

// A velocity change dv for a body after a collision (the other body of an actor-actor hit, or the one that hit). On the ground, a push away from it lifts the body off; else only the part along the ground counts, turned level. Airborne, an upward push is dropped when nothing bounces. Very slow horizontal speeds stop.
static void PushBody(Corpus *body, RwV3d *dv, float restitution)
{
	RwV3d &move = body->m_owner->m_velocity;
	RwV3dAdd(&move, &move, dv);
	if (!(body->m_flags & CORPUS_AIRBORNE)) {
		const RwV3d &n = body->m_groundNormal;
		float away = ((dv->y * n.y) + (dv->x * n.x)) + (dv->z * n.z);
		if (away > 0.01f) {   // lifted off
			body->LeaveGround(0, false);
			body->m_velocity = move;
			body->m_contactPoint = body->m_lastFrame.pos;
		} else {   // only the part along the ground, turned level
			RwV3dSub(&move, &move, dv);
			away = -away;
			dv->x = away * n.x + dv->x;
			dv->y = away * n.y + dv->y;
			dv->z = away * n.z + dv->z;
			RwV3dAdd(&move, &move, dv);
			RwMatrix level;
			if (RotationFromTo(&n, &g_corpusYAxis, &level))
				RwV3dTransformVectors(dv, dv, 1, &level);
			dv->y = 0.0f;
		}
	} else if (dv->y > 0.0f && restitution <= 0.0f) {
		dv->y = 0.0f;
	}
	RwV3dAdd(&body->m_velocity, &body->m_velocity, dv);
	if (body->m_velocity.x * body->m_velocity.x + body->m_velocity.z * body->m_velocity.z <= 0.09f) {
		body->m_velocity.z = 0.0f;
		body->m_velocity.x = 0.0f;
	}
}

// After a collision response: the body's step is no longer done (it runs again in the next sub-step) and it collided; an almost
// still actor stops.
static void MarkCollided(Corpus *body)
{
	body->m_flags = (body->m_flags & ~CORPUS_STEP_DONE) | CORPUS_COLLIDED;
	RwV3d &move = body->m_owner->m_velocity;
	if (((move.y * move.y) + (move.x * move.x)) + (move.z * move.z) < 0.001f) {
		move.x = 0.0f;
		move.y = 0.0f;
		move.z = 0.0f;
	}
}

// A standing body's ground ray hit a surface: a steep one it is moving into (or a step up too high) stops a player who has let
// go of the stick, else the body walks off; otherwise the move follows the new surface. The body moves to the hit point.
static void FollowGroundHit(CorpusProbe &probe, Corpus *body)
{
	GameActor *owner = body->m_owner;
	if (probe.m_hitAtomic && body->m_link.m_atomic == probe.m_hitAtomic)   // on its carrier: no carried motion this sub-step
		body->m_contactPoint = body->m_lastFrame.pos;
	bool follow = true;
	if (!(owner->m_params.m_flags & RWS::TFB::ACTOR_WALL_CLIMBER) && body->m_groundNormal.y >= 0.7071f) {
		const RwV3d &move = owner->m_velocity;
		float into = ((probe.m_normal.y * move.y) + (probe.m_normal.x * move.x)) + (probe.m_normal.z * move.z);
		if ((into > 0.01f && probe.m_normal.y < 0.7071f) || (probe.m_normal.y >= 0.7071f && into / probe.m_normal.y > 2.5f)) {
			if ((body->m_flags & CORPUS_PLAYER_CONTROLLED) && probe.m_normal.y < 0.7071f && owner->m_params.m_groundAccel != 0.0f
			    && body->m_moveInput.x == 0.0f) {
				probe.m_normal = body->m_groundNormal;   // stays on its ground, stopped a unit back
				RwV3dNormalize(&owner->m_velocity, &owner->m_velocity);
				RwV3dSub(&probe.m_hitPos, &probe.m_hitPos, &owner->m_velocity);
				owner->m_velocity.z = 0.0f;
				owner->m_velocity.y = 0.0f;
				owner->m_velocity.x = 0.0f;
			} else {
				body->LeaveGround(0, true);
				body->m_contactPoint = body->m_lastFrame.pos;
			}
			body->m_velocity = owner->m_velocity;
			follow = false;
		}
	}
	RwMatrix turn;
	if (follow && RotationFromTo(&body->m_groundNormal, &probe.m_normal, &turn))
		RwV3dTransformVectors(&owner->m_velocity, &owner->m_velocity, 1, &turn);
	body->m_groundNormal = probe.m_normal;
	probe.m_pos = probe.m_hitPos;
}

// Any other hit (airborne, or the box sweep hit a wall or an actor): reports it to the scripts, bounces the body off with its
// restitution (an actor-actor hit pushes both by their inverse masses) and moves it out of the surface.
static void Bounce(CorpusProbe &probe, Corpus *body)
{
	GameActor *owner = body->m_owner;
	float otherInverseMass = 0.0f;
	float restitution = owner->m_params.m_bounceRestitution;
	if (probe.m_rayHit)
		restitution *= 0.5f;
	probe.m_move = owner->m_velocity;
	probe.m_pos = body->m_frame->modelling.pos;
	Collider *hit = probe.m_hit;
	if (!hit) {   // the world
		float into = ((probe.m_move.y * probe.m_normal.y) + (probe.m_move.x * probe.m_normal.x)) + (probe.m_move.z * probe.m_normal.z);
		if (into < 0.0f) {
			if (!probe.m_rayHit) {
				Messages(owner)->PostMessage(TFBMember_HitWallMessage(), Messages(owner), ContactPosition(probe, body), -1.0f);
				Messages(owner)->PostMessage(TFBMember_WallNormalMessage(), Messages(owner), (Vec3 *)&probe.m_normal, -1.0f);
			}
			if (body->m_flags & CORPUS_HIT_TRIGGER)
				Messages(owner)->PostMessage(TFBMember_HitTriggerMessage(), Messages(owner), ContactPosition(probe, body), -1.0f);
			// the move mirrored at the surface; k*n.y and k*n.z are rounded to float before the adds, k*n.x is not (as in the game)
			RwV3d bounce;
			bounce.y = probe.m_normal.y * (into * -2.0f);
			bounce.z = probe.m_normal.z * (into * -2.0f);
			bounce.x = probe.m_move.x + probe.m_normal.x * (into * -2.0f);
			bounce.y = probe.m_move.y + bounce.y;
			bounce.z = probe.m_move.z + bounce.z;
			Messages(owner)->PostMessage(TFBMember_BounceNormalMessage(), Messages(owner), (Vec3 *)&bounce, -1.0f);
		}
	} else {   // an actor
		Script::ScriptReference *other = hit->m_owner;
		if (other->GetType() == TFBType_Actor()) {   // both hear of it
			Messages(owner)->PostMessage(TFBMember_HitActorMessage(), other, ContactPosition(probe, body), 0.0f);
			((Script::Placement *)other)->PostMessage(TFBMember_HitActorMessage(), Messages(owner), ContactPosition(probe, body), 0.0f);
		}
		RwV3dSub(&probe.m_move, &probe.m_move, &hit->m_velocity);   // relative velocity
		if ((unsigned char)((Script::Placement *)owner->m_owner)->IsMeshCollidee() && RWS::TFB::ActorOf(*hit).m_corpus) {
			// two bodies: push apart along the line between the box centres, elastic-ish
			probe.m_normal = V3Sub(&owner->m_worldBox.m_frame.pos, &hit->m_worldBox.m_frame.pos);
			RwV3dNormalize(&probe.m_normal, &probe.m_normal);
			restitution = 0.7f;
			otherInverseMass = hit->m_inverseMass;
		}
		if ((body->m_flags & (CORPUS_LEFT_GROUND | CORPUS_AIRBORNE)) != CORPUS_AIRBORNE && probe.m_hitAtomic
		    && body->m_link.m_atomic == probe.m_hitAtomic) {
			// on its carrier: the carried motion of this sub-step goes into the position, not the velocity
			RwV3d carried;
			RwV3dSub(&carried, &body->m_contactPoint, &body->m_lastFrame.pos);
			body->m_contactPoint = body->m_lastFrame.pos;
			RwV3dAdd(&probe.m_pos, &probe.m_pos, &carried);
			RwV3dSub(&probe.m_move, &probe.m_move, &carried);
		}
	}
	if (!(body->m_flags & CORPUS_AIRBORNE) && otherInverseMass == 0.0f) {
		const RwV3d &ground = body->m_groundNormal;
		float cosine = ((probe.m_normal.y * ground.y) + (probe.m_normal.x * ground.x)) + (probe.m_normal.z * ground.z);
		if (cosine < 0.93969f) {   // more than 20 degrees from the ground: the wall normal is turned perpendicular to the ground
			if (cosine < -1.0f)
				cosine = -1.0f;
			RwV3d axis;
			RwV3dCrossProduct(&axis, &ground, &probe.m_normal);
			RwV3dNormalize(&axis, &axis);
			RwMatrix turn;
			RwMatrixRotate(&turn, &axis, (float)(90.0f - acos(cosine) * 57.295776f), rwCOMBINEREPLACE);   // one rounding, at the argument store
			RwV3dTransformVectors(&probe.m_normal, &probe.m_normal, 1, &turn);
		} else {   // about the ground's own direction: the body leaves the ground (a late jump stays possible)
			body->LeaveGround(0, true);
			body->m_velocity = owner->m_velocity;
			body->m_contactPoint = body->m_lastFrame.pos;
		}
	}
	if (probe.m_t != 0.0f) {   // out of the surface
		RwV3dIncrementScaled(&probe.m_pos, &probe.m_normal, 0.1f);
	} else {   // it started inside: a larger push out
		RwV3dIncrementScaled(&probe.m_pos, &probe.m_normal, 1.5f);
		owner->m_embedded = 1;
	}
	RwV3d dv;
	if (restitution < 0.0f) {   // no bounce at all: the body stops
		RwV3dNegate(&dv, &owner->m_velocity);
	} else {
		// impulse along the normal: j = -(1 + e) v.n / (1/m1 + 1/m2), only for an approaching body
		float approach = ((probe.m_move.y * probe.m_normal.y) + (probe.m_move.x * probe.m_normal.x)) + (probe.m_move.z * probe.m_normal.z);
		if (approach > 0.0f)
			approach = 0.0f;
		float impulse = -(approach * (restitution + 1.0f) / (otherInverseMass + owner->m_inverseMass));
		RwV3dScale(&probe.m_move, &probe.m_normal, impulse);
		if (otherInverseMass != 0.0f) {   // the other body takes its share
			Corpus *other = RWS::TFB::ActorOf(*probe.m_hit).m_corpus;
			RwV3d otherDv;
			RwV3dScale(&otherDv, &probe.m_move, -otherInverseMass);
			PushBody(other, &otherDv, restitution);
			MarkCollided(other);
		}
		RwV3dScale(&dv, &probe.m_move, owner->m_inverseMass);
	}
	PushBody(body, &dv, restitution);
}

// A falling player that touched a ledge triangle (flag 0x20): two probes find the ledge's top edge and LedgeNearby is posted there.
static void ProbeLedge(CorpusProbe &probe, Corpus *body)
{
	GameActor *owner = body->m_owner;
	const Collider *box = owner;
	NearestTriHit nearest;
	RpIntersection line;
	nearest.dist = 3.4028235e38f;
	line.type = rpINTERSECTLINE;
	owner->UpdateBounds(&body->m_frame->modelling, probe.m_rem);
	// back from the ledge (its normal, level) by the box's extent in that direction plus 6
	RwV3d back;
	back.x = probe.m_ledge.x;
	back.y = 0.0f;
	back.z = probe.m_ledge.z;
	RwV3dNormalize(&back, &back);
	{
		const RwMatrix &axes = box->m_worldBox.m_frame;
		const RwReal *e = box->m_worldBox.m_extent;
		float a = ((back.y * axes.right.y) + (back.x * axes.right.x)) + (back.z * axes.right.z);
		float b = fabsf(((back.y * axes.up.y) + (back.x * axes.up.x)) + (back.z * axes.up.z));
		float c = fabsf(((back.y * axes.at.y) + (back.x * axes.at.x)) + (back.z * axes.at.z));
		float reach = -((((c * e[2]) + (fabsf(a) * e[0])) + (b * e[1])) + 6.0f);
		RwV3dScale(&back, &back, reach);
	}
	// first probe: down from there, half the box's height: the top of the ledge
	{
		RwV3d start = V3Add(&box->m_worldBox.m_frame.pos, &back);
		line.t.line.start = start;
		line.t.line.end = start;
	}
	line.t.line.end.y = line.t.line.end.y - box->m_worldBox.m_extent[1] * 0.5f;
	FindNearest(&line, probe.m_hit, &nearest);
	if (nearest.dist == 3.4028235e+38f || !(nearest.normal.y > 0.7071f))
		return;
	RwMatrix turn;
	if (RotationFromTo(&g_corpusYAxis, &nearest.normal, &turn))
		RwV3dTransformVectors(&back, &back, 1, &turn);
	// second probe: from just above the found top back towards the body: the ledge's edge
	{
		float dx = line.t.line.end.x - line.t.line.start.x;
		float dy = line.t.line.end.y - line.t.line.start.y;
		line.t.line.end.z = line.t.line.end.z - line.t.line.start.z;   // dz is spilled to the end.z slot (rounded) before the multiply, as in the game
		float d = nearest.dist;
		nearest.dist = 3.4028235e+38f;
		nearest.normal.x = nearest.normal.x * -0.1f;
		nearest.normal.y = nearest.normal.y * -0.1f;
		nearest.normal.z = nearest.normal.z * -0.1f;
		line.t.line.end.x = dx * d;   // dx*d is stored to float first, then start.x is added
		line.t.line.end.x = line.t.line.start.x + line.t.line.end.x;
		line.t.line.end.y = line.t.line.start.y + d * dy;
		float sz = line.t.line.start.z + line.t.line.end.z * d;   // kept unrounded until the add below, as in the game
		line.t.line.end.x = nearest.normal.x + line.t.line.end.x;
		line.t.line.end.y = nearest.normal.y + line.t.line.end.y;
		line.t.line.end.z = nearest.normal.z + sz;
		line.t.line.start.x = line.t.line.end.x - back.x;
		line.t.line.start.y = line.t.line.end.y - back.y;
		line.t.line.start.z = line.t.line.end.z - back.z;
	}
	FindNearest(&line, probe.m_hit, &nearest);
	if (nearest.dist == 3.4028235e+38f)
		return;
	// the edge point, 2 units further in
	RwV3d edge, in;
	in.x = nearest.dist * back.x;
	in.y = nearest.dist * back.y;
	in.z = nearest.dist * back.z;
	edge.x = line.t.line.start.x + in.x;
	edge.y = line.t.line.start.y + in.y;
	edge.z = line.t.line.start.z + in.z;
	RwV3dNormalize(&in, &in);
	in.x = in.x + in.x;
	in.y = in.y + in.y;
	in.z = in.z + in.z;
	RwV3dAdd(&edge, &edge, &in);
	Messages(owner)->PostMessage(TFBMember_LedgeNearbyMessage(), Messages(owner), (Vec3 *)&edge, -1.0f);
}

// The response to the hit of one sub-step: the body that hit something follows the ground or bounces off, moves to the new
// position, and a falling player near a ledge looks for it.
static void RespondToHit(CorpusProbe &probe, Corpus *body)
{
	RwV3dScale(&probe.m_normal, &probe.m_normal, 0.0001f);   // scaled down before normalising
	RwV3dNormalize(&probe.m_normal, &probe.m_normal);
	if (!(body->m_flags & CORPUS_AIRBORNE) && probe.m_rayHit)
		FollowGroundHit(probe, body);
	else
		Bounce(probe, body);
	MarkCollided(body);
	RwV3d to;
	to.x = probe.m_pos.x - body->m_frame->modelling.pos.x;
	to.y = probe.m_pos.y - body->m_frame->modelling.pos.y;
	to.z = probe.m_pos.z - body->m_frame->modelling.pos.z;
	RwFrameTranslate(body->m_frame, &to, rwCOMBINEPOSTCONCAT);
	const RwV3d &move = body->m_owner->m_velocity;
	const unsigned falling = CORPUS_PLAYER_CONTROLLED | CORPUS_HIT_LEDGE | CORPUS_AIRBORNE;
	if ((body->m_flags & falling) == falling && move.y < 0.0f && body->m_owner->m_params.m_groundAccel != 0.0f
	    && (!probe.m_hit || (probe.m_hit->m_velocity.x == 0.0f && probe.m_hit->m_velocity.y == 0.0f && probe.m_hit->m_velocity.z == 0.0f)))
		ProbeLedge(probe, body);
}

// The physics step of a tick: up to 100 sub-steps. In each, every body's handler first probes its move (the broad phase finds the
// earliest hit among all of them), then all bodies move up to that time; the body that hit something gets its collision
// response, and the rest of the tick runs as the next sub-step. Sub-step 100 skips the probe and moves everything to the end.
// 0x424c40 Corpus_RunPhysicsStep
void Corpus_RunPhysicsStep()
{
	CorpusProbe probe;
	RWS::CMsg msg(g_msgCorpusPhysics, &probe);
	// The broad phase's message pass gets a probe of its own: only its time and remaining time are set; the mesh sweeps of that pass write its hit fields.
	CorpusProbe messageProbe;
	int subSteps = 0;
	probe.m_rem = 1.0f;
	for (;;) {
		probe.m_t = probe.m_rem;
		probe.m_hitCorpus = 0;
		probe.m_hitActor = 0;
		probe.m_hit = 0;
		subSteps++;
		if (subSteps != 100) {   // probe: how far can everything move?
			probe.m_firstPass = 1;
			RWS::SendMsg(msg);
			BroadPhase_ResolvePairs(&probe, true);
		}
		if (probe.m_t != 0.0f) {   // move everything that far
			messageProbe.m_t = probe.m_t;
			messageProbe.m_rem = probe.m_rem;
			probe.m_rem = probe.m_rem - probe.m_t;
			BroadPhase_ResolvePairs(&messageProbe, false);
			probe.m_firstPass = 0;
			RWS::SendMsg(msg);
		}
		if (probe.m_rem == 0.0f)
			break;
		if (probe.m_hitCorpus)
			RespondToHit(probe, probe.m_hitCorpus);
	}
}
