// Sweep of a body's probe against a triangle (the ground probe callback is in ground_probe.cpp).
#define ZOOSTERS_RWS_SENDMSG 1
#include "tfb/corpus/corpus.h"
#include "script/actor.h"
#include "tfb/corpus/corpus_private.h"
#include "modules/tfb/gameactor.h"
#include "tfb/collision/collisionbox.h"
#include "tfb/collision/raytriangle.h"
#include <math.h>


// GLOBAL: 0x454880 CollisionBox::FindTriangleContact
float _rwInvSqrt(float x);                           // 0x4d7850

using RWS::TFB::GameActor;

// The collision flags of a triangle (0 without the world's flag table)
static unsigned char TriangleFlags(const CorpusProbe *p, const RpCollisionTriangle *tri)
{
	return p->m_useTriFlags ? p->m_triFlags[tri->index * 8 + 6] : 0;
}

// A ray hit earlier than the probe's best becomes it (the triangle's trigger flag goes with it)
static void RecordRayHit(CorpusProbe *p, const RpCollisionTriangle *tri, const RwV3d &normal, float t, const RwV3d &point, const float uv[2])
{
	p->m_normal = normal;
	p->m_t = t;
	p->m_hitPos = point;
	p->m_hitCorpus = p->m_corpus;
	p->m_u = uv[0];
	p->m_rayHit = 1;
	p->m_v = uv[1];
	p->m_corpus->m_flags &= ~(CORPUS_HIT_LEDGE | CORPUS_HIT_TRIGGER);
	if (TriangleFlags(p, tri) & 1)
		p->m_corpus->m_flags |= CORPUS_HIT_TRIGGER;
}

// Box sweep for bodies with wall collision against a wall-like triangle the box is in front of. A hit that is not just grazing along the up axis becomes the probe's best; a touching contact (time 0) pushes out along the triangle normal (plus the earlier normal of a touching box hit).
static void SweepBox(CorpusProbe *p, RpCollisionTriangle *tri, const RwV3d &N, float len2, float &dist)
{
	float touch = (p->m_corpus->m_flags & CORPUS_COLLIDED) ? 0.025f : 0.0f;   // a body pushed this tick counts near contacts as touching
	RwV3d d;
	RwV3dSub(&d, &p->m_pos, tri->vertices[0]);
	if (!((N.y * d.y + N.x * d.x) + N.z * d.z >= -0.01f))   // behind the triangle
		return;
	RwV3d H, V;
	if (p->m_box.FindTriangleContact(p->m_t, tri->vertices, &p->m_move, &dist, &H, &V))
		return;
	if (!(dist <= touch)) {
		const RwV3d *up = &p->m_up;
		if (!((V.x * up->x + V.y * up->y) + V.z * up->z < 0.0f)) {
			RwV3d side;
			RwV3dCrossProduct(&side, up, &V);
			if (!((side.x * side.x + side.y * side.y) + side.z * side.z > ((V.y * V.y + V.x * V.x) + V.z * V.z) * 1.0e-06))
				return;   // the contact normal is the up axis itself: no wall hit
		}
	}
	if (!((V.y * N.y + V.x * N.x) + V.z * N.z >= 0.0f))
		return;
	if (!(dist > touch)) {
		dist = 0.0f;
		if (p->m_corpus->m_owner->m_embedded)
			return;
	}
	float dp = (V.y * p->m_up.y + V.x * p->m_up.x) + V.z * p->m_up.z;
	if (!(dp <= 0.0f) && !(dp * dp <= ((V.y * V.y + V.x * V.x) + V.z * V.z) * 0.49999037f))
		V = N;   // a floor-like contact normal: take the triangle's
	if (dist == 0.0f) {
		float f = _rwInvSqrt(len2);
		if ((V.y * N.y + V.x * N.x) + V.z * N.z < 0.0f)   // full dot product V.N
			f = -f;
		V.x = N.x * f;
		V.y = N.y * f;
		V.z = N.z * f;
		if (p->m_t == 0.0f && !p->m_rayHit)
			RwV3dAdd(&V, &V, &p->m_normal);
	}
	p->m_normal = V;
	p->m_t = dist;
	p->m_hitPos = H;
	p->m_hitCorpus = p->m_corpus;
	p->m_rayHit = 0;
	p->m_corpus->m_flags &= ~(CORPUS_HIT_LEDGE | CORPUS_HIT_TRIGGER);
	unsigned char flags = TriangleFlags(p, tri);
	if (flags & 1)
		p->m_corpus->m_flags |= CORPUS_HIT_TRIGGER;
	if (flags & 0x20) {
		p->m_corpus->m_flags |= CORPUS_HIT_LEDGE;
		p->m_ledge = N;
	}
}

// The rays of the move (from the start, nudged 0.1 along the ground ray for a body walking on the ground; also, moving into the triangle on the ground, from the second start) against the triangle.
static void CastMoveRays(CorpusProbe *p, RpCollisionTriangle *tri, const RwV3d &N, float len2, float &dist)
{
	float dp = (N.y * p->m_move.y + N.x * p->m_move.x) + N.z * p->m_move.z;
	if (!(dp * dp > ((p->m_move.y * p->m_move.y + p->m_move.x * p->m_move.x) + p->m_move.z * p->m_move.z) * len2 * 1.0e-04f))
		return;   // moving along the triangle
	float uv[2];
	RwV3d H;
	RwV3d start = p->m_pos;
	if ((p->m_corpus->m_flags & (CORPUS_MOVING | CORPUS_AIRBORNE)) == CORPUS_MOVING) {
		float f = 0.1f;
		if (!(dp < 0.0f))
			f = -0.1f;
		RwV3dIncrementScaled(&start, &p->m_groundDir, f);
	}
	if (RayTriangleIntersect(&start, &p->m_move, tri->vertices[0], tri->vertices[1], tri->vertices[2], &H, uv, &dist) == RAYTRI_INSIDE && dist > 0.0f
	    && dist < p->m_t)
		RecordRayHit(p, tri, N, dist, H, uv);
	if (dp < 0.0f && !(p->m_corpus->m_flags & CORPUS_AIRBORNE)) {
		if (RayTriangleIntersect(&p->m_pos2, &p->m_move, tri->vertices[0], tri->vertices[1], tri->vertices[2], &H, uv, &dist) == RAYTRI_INSIDE
		    && dist > 0.0f && dist < p->m_t)
			RecordRayHit(p, tri, N, dist, H, uv);
	}
}

// After a full move: is there ground under the end position? A ray down (a wall climber in the air: along the triangle normal) within 0.25, or, airborne without ground yet, along the box's up axis within its height, marks the triangle as the body's ground.
static void FindGround(CorpusProbe *p, RpCollisionTriangle *tri, const RwV3d &N, float &dist)
{
	Corpus *body = p->m_corpus;
	GameActor *owner = body->m_owner;
	float uv[2];
	RwV3d H;
	RwV3d end = p->m_pos;
	float rem = p->m_rem;
	end.x = rem * p->m_move.x + end.x;
	end.y = rem * p->m_move.y + end.y;
	end.z = rem * p->m_move.z + end.z;
	const RwV3d *down = ((body->m_flags & CORPUS_AIRBORNE) && (owner->m_params.m_flags & RWS::TFB::ACTOR_WALL_CLIMBER)) ? &tri->normal : &p->m_groundDir;
	if ((RayTriangleIntersect(&end, down, tri->vertices[0], tri->vertices[1], tri->vertices[2], &H, uv, &dist) && fabsf(dist) <= 0.25f)
	    || ((body->m_flags & (CORPUS_GROUND_FOUND | CORPUS_AIRBORNE)) == CORPUS_AIRBORNE
	        && (N.y * p->m_up.y + N.x * p->m_up.x) + N.z * p->m_up.z > 0.01f
	        && RayTriangleIntersect(&end, &p->m_box.m_frame.up, tri->vertices[0], tri->vertices[1], tri->vertices[2], &H, uv, &dist)
	        && dist >= 0.0f && dist < p->m_box.m_extent[1])) {
		p->m_hitCorpus = body;
		body->m_flags |= CORPUS_GROUND_FOUND;
		body->m_link.m_index = tri->index;
		body->m_carrierFrame.pos = H;   // the probe's contact point and normal, read back by HandleEvents
		body->m_link.m_u = uv[0];
		body->m_link.m_v = uv[1];
		body->m_frameStep = N;
		body->m_surface = 0;
		if (TriangleFlags(p, tri) & 1)
			body->m_flags |= CORPUS_HIT_TRIGGER;
	}
}

// The RpCollision callback of a body's physics probe for one triangle (world or an actor's mesh): box sweep against walls, move rays, ground under the end of the move. Degenerate and ignored (flag 4) triangles are skipped. The suffix D stays.
// 0x4229d0
RpCollisionTriangle *Corpus_SphereTriangleSweepD(RpIntersection *isect, RpCollisionTriangle *tri, RwReal dist, void *data)
{
	CorpusProbe *p = (CorpusProbe *)data;
	(void)isect;   // `dist` (RpCollision's distance) is only storage: the tests below write their hit times into it
	RwV3d e1, e2, N;
	RwV3dSub(&e1, tri->vertices[1], tri->vertices[0]);
	RwV3dSub(&e2, tri->vertices[2], tri->vertices[0]);
	RwV3dCrossProduct(&N, &e1, &e2);
	float len2 = (N.y * N.y + N.x * N.x) + N.z * N.z;
	if (len2 <= 1.0e-05f || (TriangleFlags(p, tri) & 4))
		return tri;
	// wall-like: facing down, or more than 45 degrees from the body's up axis
	float up = (N.y * p->m_up.y + N.x * p->m_up.x) + N.z * p->m_up.z;
	bool wall = up <= 0.0f || up * up <= len2 * 0.49999037f;
	unsigned actorFlags = p->m_corpus->m_owner->m_params.m_flags;
	if ((actorFlags & RWS::TFB::ACTOR_WALL_COLLISION) && wall)
		SweepBox(p, tri, N, len2, dist);
	if (p->m_t == 0.0f)
		return tri;
	if (!wall || (actorFlags & (RWS::TFB::ACTOR_WALL_COLLISION | RWS::TFB::ACTOR_WALL_CLIMBER)))
		CastMoveRays(p, tri, N, len2, dist);
	if (p->m_t == p->m_rem
	    && !(p->m_hitActor && !(unsigned char)((Script::Placement *)p->m_hitActor->m_owner)->IsSolidCollider()))
		FindGround(p, tri, N, dist);
	return tri;
}
