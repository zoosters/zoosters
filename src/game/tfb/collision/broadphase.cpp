#include "tfb/corpus/messagemembers.h"
#include "tfb/collision/broadphase.h"
#include "tfb/collision/meshsweep.h"
#include "tfb/corpus/corpus.h"
#include "tfb/corpus/corpusprobe.h"
#include "modules/tfb/gameactor.h"
#include "script/ops_message.h"
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <set>

// GLOBAL: 0x603660 g_boundsCount
// GLOBAL: 0x603668 g_boundsItems
// GLOBAL: 0x606558 g_boundsSentinel
// GLOBAL: 0x626560 g_pairPool
// GLOBAL: 0x629440 g_pairList
// GLOBAL: 0x606554 g_pairFreeList

Script::ScriptReference *TFBType_Actor();             // 0x436e30

using RWS::TFB::Collider;
using RWS::TFB::ColliderBound;
using RWS::TFB::CollisionPair;
using RWS::TFB::GameActor;
using RWS::TFB::ActorOf;
using RWS::TFB::BROADPHASE_MAX_PAIRS;

static inline RwReal Coordinate(const ColliderBound *bound, int axis)
{
	return (&bound->m_point.x)[axis];
}

// Touching counts.
static bool BoundsOverlap(const Collider *a, const Collider *b)
{
	return b->m_min.m_point.x <= a->m_max.m_point.x && b->m_max.m_point.x >= a->m_min.m_point.x &&
	       b->m_min.m_point.z <= a->m_max.m_point.z && b->m_max.m_point.z >= a->m_min.m_point.z &&
	       b->m_min.m_point.y <= a->m_max.m_point.y && b->m_max.m_point.y >= a->m_min.m_point.y;
}

// Parked by Collider::Reset until its next UpdateBounds; sorts to the end of the lists.
static bool IsParked(const Collider *collider)
{
	return collider->m_min.m_point.x == FLT_MAX;
}

// 0x4476f0
void BroadPhase_Reset()
{
	g_boundsSentinel.m_point.x = -FLT_MAX;
	g_boundsSentinel.m_point.y = -FLT_MAX;
	g_boundsSentinel.m_point.z = -FLT_MAX;
	for (int i = 0; i < BROADPHASE_MAX_PAIRS - 1; i++)
		g_pairPool[i].m_next = &g_pairPool[i + 1];
	g_pairPool[BROADPHASE_MAX_PAIRS - 1].m_next = 0;
	g_pairList = 0;
	g_pairFreeList = g_pairPool;
	for (int i = 1; i < g_boundsCount; i++)
		g_boundsItems[0][i]->m_collider->m_owner->AdjustRefCount(-1);
	g_boundsCount = 1;
}

// Pair order: by address on x86, as the game; by creation order on x64, where heap addresses differ per run.
// Test aids for comparing x86 and x64 runs: PORT_PAIRORDER_SERIAL=1 (x86 uses creation order), PORT_PAIRORDER_REC=<file>
// (x86 records the pairs whose two orders differ), PORT_PAIRORDER_USE=<file> (x64 replays them).
static std::set<unsigned long long> g_pairFlips;
static bool g_pairFlipsLoaded;
static void OrderPair(Collider *first, Collider *second, Collider **a, Collider **b)
{
	bool firstLower = first->Serial() < second->Serial();
	unsigned lo = firstLower ? first->Serial() : second->Serial();
	unsigned hi = firstLower ? second->Serial() : first->Serial();
	unsigned long long key = ((unsigned long long)lo << 32) | hi;
#ifndef ZOOSTERS_PTR64
	static const char *serialEnv = getenv("PORT_PAIRORDER_SERIAL");
	if (serialEnv && *serialEnv == '1') {
		*a = firstLower ? first : second;
		*b = firstLower ? second : first;
		return;
	}
	bool addrFirst = first < second;
	if (addrFirst != firstLower) {
		static const char *rec = getenv("PORT_PAIRORDER_REC");
		if (rec && g_pairFlips.insert(key).second) {
			FILE *f = fopen(rec, "a");
			if (f) { fprintf(f, "%u %u\n", lo, hi); fclose(f); }
		}
	}
	*a = addrFirst ? first : second;
	*b = addrFirst ? second : first;
#else
	if (!g_pairFlipsLoaded) {
		g_pairFlipsLoaded = true;
		const char *use = getenv("PORT_PAIRORDER_USE");
		FILE *f = use ? fopen(use, "r") : 0;
		unsigned l, h;
		while (f && fscanf(f, "%u %u", &l, &h) == 2)
			g_pairFlips.insert(((unsigned long long)l << 32) | h);
		if (f) fclose(f);
	}
	if (!g_pairFlips.empty() && g_pairFlips.count(key))
		firstLower = !firstLower;
	*a = firstLower ? first : second;
	*b = firstLower ? second : first;
#endif
}

// Only pairs with a clump on at least one side. Does nothing when the pool is used up.
static void AddPair(Collider *first, Collider *second)
{
	if (!BoundsOverlap(first, second) || !(first->m_clump || second->m_clump))
		return;
	Collider *a, *b;
	OrderPair(first, second, &a, &b);
	for (CollisionPair *pair = g_pairList; pair; pair = pair->m_next) {
		if (pair->m_a == a && pair->m_b == b)
			return;
	}
	CollisionPair *pair = g_pairFreeList;
	if (!pair)
		return;
	g_pairFreeList = pair->m_next;
	pair->m_next = g_pairList;
	g_pairList = pair;
	pair->m_a = a;
	pair->m_b = b;
}

// Insertion sort: the order changes little between steps. A min corner moving down past another collider's max corner
// may start an overlap.
// 0x448590
void BroadPhase_SortAndPair()
{
	for (int axis = 0; axis < 3; axis++) {
		ColliderBound **list = g_boundsItems[axis];
		for (int i = 2; i < g_boundsCount; i++) {
			ColliderBound *bound = list[i];
			RwReal key = Coordinate(bound, axis);
			Collider *collider = bound->m_collider;
			bool isMin = bound == &collider->m_min;
			int j = i;
			while (key <= Coordinate(list[j - 1], axis)) {
				ColliderBound *other = list[j - 1];
				if (isMin && other == &other->m_collider->m_max && key != FLT_MAX)
					AddPair(collider, other->m_collider);
				list[j] = other;
				j--;
			}
			list[j] = bound;
		}
	}
}

static void FreePair(CollisionPair **link)
{
	CollisionPair *pair = *link;
	*link = pair->m_next;
	pair->m_next = g_pairFreeList;
	g_pairFreeList = pair;
}

// A body attached to the carrier (link index -1) doesn't collide with it.
static bool RidesOn(const Corpus *body, const Collider *carrier)
{
	return body && body->m_link.m_carrier == carrier && body->m_link.m_index == -1;
}

// Message pass: a mesh hit earlier than the box contact replaces it. The probe keeps its own time.
static void SweepMesh(CorpusProbe *probe, GameActor &prober, Collider *meshOwner, float *tHit, RwV3d *point, RwV3d *normal)
{
	float saved = probe->m_t;
	probe->m_corpus = prober.m_corpus;
	ProbeSweepColliderMesh(meshOwner, probe);
	if (probe->m_t < saved) {
		*tHit = probe->m_t;
		*point = probe->m_hitPos;
		*normal = probe->m_normal;
		probe->m_t = saved;
	}
}

// Probing pass. A hit at t <= 0.01 counts as touching (t = 0); embedded colliders ignore touches.
static void KeepEarliestHit(CorpusProbe *probe, Collider *hitter, Collider *hit, float tHit, const RwV3d &point, const RwV3d &normal)
{
	if (probe->m_t == 0.0f)
		return;
	if (tHit <= 0.01f) {
		tHit = 0.0f;
		if (hitter->m_embedded)
			return;
	}
	probe->m_normal = normal;
	probe->m_t = tHit;
	probe->m_hitPos = point;
	probe->m_rayHit = 0;
	probe->m_hitCorpus = ActorOf(*hitter).m_corpus;
	if (probe->m_hitCorpus)
		probe->m_hitCorpus->m_flags &= ~(CORPUS_HIT_TRIGGER | CORPUS_HIT_LEDGE);
	probe->m_hit = hit;
	probe->m_hitAtomic = 0;
}

// HitActor to each side that has a clump, at the contact point, or at its own box centre when touching from the start.
static void PostHitActor(Collider *a, Collider *b, float tHit, RwV3d *point)
{
	GameActor &actorA = ActorOf(*a);
	GameActor &actorB = ActorOf(*b);
	if (a->m_clump) {
		RwV3d *pos = tHit != 0.0f ? point : &a->m_worldBox.m_frame.pos;
		actorA.PostMessage(TFBMember_HitActorMessage(), &actorB, (Vec3 *)pos, -1.0f);
	}
	if (b->m_clump) {
		RwV3d *pos = tHit != 0.0f ? point : &b->m_worldBox.m_frame.pos;
		actorB.PostMessage(TFBMember_HitActorMessage(), &actorA, (Vec3 *)pos, -1.0f);
	}
}

// Probing pass: pairs of two solid colliders; message pass: the rest. Box against box, or, for a mesh collider against a mesh
// collidee, the body swept against the meshes. The normal points away from the second collider; the pair is turned so that
// the first one has the body.
static void ResolvePair(CorpusProbe *probe, CollisionPair *pair, bool probing)
{
	Collider *a = pair->m_a;
	Collider *b = pair->m_b;
	GameActor &actorA = ActorOf(*a);
	GameActor &actorB = ActorOf(*b);
	bool bothSolid = actorA.IsSolidCollider() && actorB.IsSolidCollider();
	if (bothSolid != probing)
		return;
	if (RidesOn(actorA.m_corpus, b) || RidesOn(actorB.m_corpus, a))
		return;

	float tHit = -1.0f;
	RwV3d point, normal;
	CollisionBox::FindBoxContact(probe->m_t, &a->m_worldBox, &a->m_velocity, &b->m_worldBox, &b->m_velocity, &tHit, &point, &normal);
	bool turn = false;
	if (actorA.IsMeshCollider() && actorB.IsMeshCollidee()) {
		if (probing) {
			probe->m_corpus = actorA.m_corpus;
			ProbeSweepColliderMesh(b, probe);
			return;
		}
		SweepMesh(probe, actorA, b, &tHit, &point, &normal);
	} else if (!actorB.IsMeshCollider() && actorA.m_corpus) {
		// a has the body; turn when only a has meshes or a is not a plain actor
		turn = (actorA.IsMeshCollidee() && !actorB.IsMeshCollidee()) || actorA.GetType() != TFBType_Actor();
	} else {
		if (actorA.IsMeshCollidee()) {
			if (probing) {
				probe->m_corpus = actorB.m_corpus;
				ProbeSweepColliderMesh(a, probe);
				return;
			}
			SweepMesh(probe, actorB, a, &tHit, &point, &normal);
		}
		turn = true;
	}
	if (turn) {
		RwV3dNegate(&normal, &normal);
		a = pair->m_b;
		b = pair->m_a;
	}
	if (!(tHit >= 0.0f))
		return;
	if (probing)
		KeepEarliestHit(probe, a, b, tHit, point, normal);
	else
		PostHitActor(a, b, tHit, &point);
}

// 0x448800
void BroadPhase_ResolvePairs(CorpusProbe *probe, bool probing)
{
	if (probing)
		BroadPhase_SortAndPair();
	CollisionPair **link = &g_pairList;
	while (*link) {
		CollisionPair *pair = *link;
		if (probing && !(BoundsOverlap(pair->m_a, pair->m_b) && !IsParked(pair->m_a) && !IsParked(pair->m_b))) {
			FreePair(link);
			continue;
		}
		link = &pair->m_next;
		ResolvePair(probe, pair, probing);
	}
	// parked colliders, sorted to the end, leave the lists
	int last = g_boundsCount - 1;
	while (last > 0 && g_boundsItems[0][last]->m_point.x == FLT_MAX) {
		ColliderBound *bound = g_boundsItems[0][last];
		bound->m_point.x = -FLT_MAX;
		g_boundsItems[0][last]->m_collider->m_owner->AdjustRefCount(-1);
		last--;
	}
	g_boundsCount = last + 1;
}

