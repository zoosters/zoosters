#include "script/actor.h"
#include "rw/rwcore.h"

namespace Script {

// Squared closest approach within the next tick (1/30 s): d is the relative displacement of one tick, e the relative position.
// Slot 25 (GetVelocity) of both objects is read before slot 17 (GetVector), as the game does.
// 0x409700
float PositionMeasurement::Dist(ScriptReference *owner, PositionMeasurement *other, ScriptReference *otherOwner)
{
	const Vec3 *myVel = GetVelocity(owner);
	const Vec3 *otherVel = other->GetVelocity(otherOwner);
	float dx = (otherVel->x - myVel->x) * 0.033333335f;
	float dy = (otherVel->y - myVel->y) * 0.033333335f;
	float dz = (otherVel->z - myVel->z) * 0.033333335f;
	const Vec3 *myPos = GetVector(owner);
	const Vec3 *otherPos = other->GetVector(otherOwner);
	float ex = otherPos->x - myPos->x;
	float ey = otherPos->y - myPos->y;
	float ez = otherPos->z - myPos->z;
	float dot = dz * ez + dy * ey + dx * ex;   // first term dz * ez, as the game
	// Add d only when the objects approach (dot < 0).
	if (dot < 0.0f) {
		float distSq = dz * dz + dy * dy + dx * dx;
		if (dot + distSq > 0.0f) {
			float scale = -(dot / distSq);
			dx *= scale;
			dy *= scale;
			dz *= scale;
		}
		ex += dx;
		ey += dy;
		ez += dz;
	}
	return ez * ez + ey * ey + ex * ex;
}
}
