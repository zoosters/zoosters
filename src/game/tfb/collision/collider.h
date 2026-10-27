// Collision part of a game actor (GameActor's second base): broad-phase bounds, collision box, velocity.
#ifndef ZOOSTERS_TFB_COLLISION_COLLIDER_H
#define ZOOSTERS_TFB_COLLISION_COLLIDER_H

#include "zoosters/base.h"
#include "rw/rwcore.h"
#include "tfb/collision/collisionbox.h"

struct RpClump;
namespace Script {
class ScriptReference;
}

namespace RWS {
namespace TFB {

struct Collider;

// One corner of a collider's world bounds. The broad phase sorts pointers to these, one list per axis.
struct ColliderBound {
	RwV3d m_point;        // x == -FLT_MAX: no bounds yet
	Collider *m_collider; // null while out of the broad phase
};
ZOOSTERS_ASSERT_SIZE(ColliderBound, 0x10);

struct Collider {
	Script::ScriptReference *m_owner;
	ColliderBound m_min;
	ColliderBound m_max;
	CollisionBox m_localBox; // in the owner's frame
	CollisionBox m_worldBox;
	RwV3d m_velocity;        // per second
	RwReal m_inverseMass;
	uint8_t m_embedded;      // the body started the step inside a surface: touches at t = 0 don't count
	uint8_t m_padCD[3];
	RpClump *m_clump;

	// Creation order, kept in the padding. 64-bit builds order collision pairs by it: the game orders them by heap address,
	// which is not reproducible there.
	unsigned Serial() const { return m_padCD[0] | (m_padCD[1] << 8) | (m_padCD[2] << 16); }
	void AssignSerial();

	void ClearLinks();
	void Reset();
	void UpdateBounds(const RwMatrix *frame, RwReal dt);
};
ZOOSTERS_ASSERT_SIZE(Collider, 0xd4);
ZOOSTERS_ASSERT_OFFSET(Collider, m_localBox, 0x24);
ZOOSTERS_ASSERT_OFFSET(Collider, m_worldBox, 0x70);
ZOOSTERS_ASSERT_OFFSET(Collider, m_velocity, 0xbc);
ZOOSTERS_ASSERT_OFFSET(Collider, m_clump, 0xd0);

} // namespace TFB
} // namespace RWS

#endif
