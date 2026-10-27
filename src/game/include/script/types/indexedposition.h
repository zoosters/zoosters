// The common base of the position objects that carry an "index" (+4): waypoints (ActorWayPoint) and the actor's attachment
// points / turrets (ActorAttachment). The game has ONE "index" script member (vtable 0x5cd598, ActorWayPoint::ExposeIndex) for
// all three, which reads +4 of its owner. Normal builds only: the game build declares the +4 int in each class (cl 13.10 bytes
// must not change).
#ifndef ZOOSTERS_SCRIPT_TYPES_INDEXEDPOSITION_H
#define ZOOSTERS_SCRIPT_TYPES_INDEXEDPOSITION_H

#include "zoosters/base.h"
#include "script/actor.h"

namespace Script {

class IndexedPosition : public PositionMeasurement {
public:
	int m_index;  // (-1 at construction; a part's sort key after GameActor::CreateParts)
	Vec3 m_pos;  // the position (waypoint: its point; attachment: the cached world position, ActorAttachment::GetVector). Part of the base so that
	               // the layout stays the one of the classes' own members on x64 too (a base's tail padding is not reused under the MS ABI)
};
ZOOSTERS_ASSERT_OFFSET(IndexedPosition, m_index, 4);
ZOOSTERS_ASSERT_OFFSET(IndexedPosition, m_pos, 8);

}  // namespace Script

#endif
