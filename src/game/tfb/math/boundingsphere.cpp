#include "tfb/math/boundingsphere.h"

// 0x44e5e0
RpAtomic *AtomicGrowSphereCB(RpAtomic *atomic, RwSphere *sphere)
{
	const RwSphere *other = RpAtomicGetWorldBoundingSphere(atomic);
	if (sphere->radius == 0.0f) {
		*sphere = *other;
		return atomic;
	}
	RwV3d toOther;
	RwV3dSub(&toOther, &other->center, &sphere->center);
	float distance = RwV3dLength(&toOther);
	if (sphere->radius >= other->radius) {
		// the union of the two: its far side is the other sphere's far side
		float reach = distance + other->radius;
		if (reach > sphere->radius) {
			float radius = (reach + sphere->radius) * 0.5f;
			sphere->radius = radius;
			RwV3dIncrementScaled(&sphere->center, &toOther, (reach - radius) / distance);
		}
		return atomic;
	}
	float reach = distance + sphere->radius;
	if (reach <= other->radius) { // the other sphere holds this one
		*sphere = *other;
		return atomic;
	}
	float radius = (reach + other->radius) * 0.5f;
	sphere->radius = radius;
	// the centre goes (reach - other radius) from the other sphere's centre towards this one: twice as far as the union's centre, so the result does not quite hold both spheres (the game's code; kept)
	RwV3dScale(&sphere->center, &toOther, -((reach - other->radius) / distance));
	RwV3dAdd(&sphere->center, &sphere->center, &other->center);
	return atomic;
}
