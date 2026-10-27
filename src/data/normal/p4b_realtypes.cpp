// Normal build only: the data whose type holds pointers, defined with the real struct. p4b_bss.cpp keeps byte-array stand-ins of the
// game's 32-bit sizes, too small on x64 for the code that indexes them.
#include "script/scriptassets.h"
#include "script/poolnode.h"
#include "tfb/collision/broadphase.h"
#include <float.h>

namespace Script {
AssetEntry g_scriptAssets[0x320];     // GLOBAL: 0x618390 Script::g_scriptAssets
PoolNodeW g_nodePool[kPoolNodes];     // GLOBAL: 0x620170 Script::g_nodePool
}
RWS::TFB::CollisionPair g_pairPool[RWS::TFB::BROADPHASE_MAX_PAIRS];     // GLOBAL: 0x626560 g_pairPool
// The broad phase's three axis lists and their sentinel corner (0x603668 g_boundsItems, 0x606558 g_boundsSentinel). One real array
// spans the game's three 1001-pointer rows, each starting with a pointer to the sentinel; the sentinel is a whole ColliderBound.
extern RWS::TFB::ColliderBound g_boundsSentinel;
RWS::TFB::ColliderBound *g_boundsItems[3][RWS::TFB::BROADPHASE_MAX_BOUNDS] = { { &g_boundsSentinel }, { &g_boundsSentinel }, { &g_boundsSentinel } };
RWS::TFB::ColliderBound g_boundsSentinel = { { -FLT_MAX, 0.0f, 0.0f }, 0 };
