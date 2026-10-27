// Broad phase (sweep and prune): collider corners in three sorted lists, one per axis, and the pairs whose bounds overlap.
#ifndef ZOOSTERS_TFB_COLLISION_BROADPHASE_H
#define ZOOSTERS_TFB_COLLISION_BROADPHASE_H

#include "tfb/collision/collider.h"

struct CorpusProbe;

namespace RWS {
namespace TFB {

enum {
	BROADPHASE_MAX_BOUNDS = 1001, // per axis: the sentinel and two corners per collider
	BROADPHASE_MAX_PAIRS = 1000
};

struct CollisionPair {
	CollisionPair *m_next;
	Collider *m_a;
	Collider *m_b;
};
ZOOSTERS_ASSERT_SIZE(CollisionPair, 0xc);

}  // namespace TFB
}  // namespace RWS

// Empties the lists and the pairs. Level start and end, tick handler reset.
void BroadPhase_Reset();
// Re-sorts the lists and adds the pairs that came to overlap.
void BroadPhase_SortAndPair();
// Probing pass (each physics sub-step): drops pairs that no longer overlap and keeps the earliest hit between two solid
// colliders in `probe`. Message pass: sends HitActor for the other touching pairs.
void BroadPhase_ResolvePairs(CorpusProbe *probe, bool probing);

extern int g_boundsCount; // entries per axis list, the sentinel included
extern RWS::TFB::ColliderBound *g_boundsItems[3][RWS::TFB::BROADPHASE_MAX_BOUNDS];
extern RWS::TFB::ColliderBound g_boundsSentinel; // at -FLT_MAX: stops the insertion sort
extern RWS::TFB::CollisionPair g_pairPool[RWS::TFB::BROADPHASE_MAX_PAIRS];
extern RWS::TFB::CollisionPair *g_pairList;
extern RWS::TFB::CollisionPair *g_pairFreeList;

#endif
