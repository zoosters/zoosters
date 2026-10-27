#include "rw/rpcollis.h"
// Intersection probe over the atomics of a clump.
// `this` is the LosProbe of Vec3::LineOfSightTo (g_vtLosProbe slot 0), `src` the target's Collider; same walk as CameraProbe::ForEachAtomic.
#include "script/losprobe.h"
#include "tfb/collision/collider.h"
#include "tfb/render/tfbatomic.h"
int AtomicIsectHitCB(int, int, int, unsigned char *p);          // 0x42b130 (tfb/collision/losprobe.cpp)

// FOLDED: 0x42c850 LosProbe::ForEachAtomic
void LosProbe::ForEachAtomic(ProbeArgA *src)
{
	RpClump *clump = ((const RWS::TFB::Collider *)src)->m_clump;
	RwLLLink *node = clump->atomicList.link.next;
	while (node != &clump->atomicList.link) {
		RwLLLink *next = node->next;
		RpAtomic *atomic = RpAtomicFromClumpLink(node);
		if (TFBPlugin_FD_TestFlags(atomic, 0x100) && (intptr_t)RpAtomicGetClump(atomic) != m_targetClump)
			RpAtomicForAllIntersections(atomic, reinterpret_cast<RpIntersection *>(&m_lineStart),
				reinterpret_cast<RpIntersectionCallBackGeometryTriangle>(AtomicIsectHitCB), this);
		if (!atomic)
			break;
		node = next;
	}
}
