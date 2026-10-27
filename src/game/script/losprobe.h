// Ray probe record of Vec3::LineOfSightTo (built on the stack by lineofsight.cpp): vptr g_vtLosProbe, slot 0 = ProbeUserA::ForEachAtomic.
// Offsets are the 32-bit layout's; on x64 the members move (m_clear +0x30, m_targetClump +0x38), so reach them by name.
#ifndef ZOOSTERS_SCRIPT_LOSPROBE_H
#define ZOOSTERS_SCRIPT_LOSPROBE_H
#include "script/script.h"

struct ProbeArgA;   // really a RWS::TFB::Collider
// GLOBAL: 0x5cd2d8 LosProbe::vftable
struct LosProbe {
	virtual void ForEachAtomic(ProbeArgA *src);   // 0: 0x42c850; the vptr is the compiler's
	Vec3 m_lineStart, m_lineEnd;   // +4: RpIntersection (rpcollis.h): t.line = RwLine start/end, passed to RpCollisionWorldForAllIntersections
	int m_isectType;               // +0x1c: RpIntersection::type (1 = rpINTERSECTLINE)
	Vec3 m_dir;                     // +0x20: lineEnd - lineStart
	bool m_clear;                   // +0x2c: preset 1; the triangle callback 0x42b140 clears it (data+0x2c) and stops the search
	intptr_t m_targetClump;         // +0x30: target actor+0x168 = its clump (Collider::m_clump)
};
ZOOSTERS_ASSERT_SIZE(LosProbe, 0x34);

#endif
