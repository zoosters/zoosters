#ifndef ZOOSTERS_MODULES_TFB_CTFBWORLD_VIEWS_H
#define ZOOSTERS_MODULES_TFB_CTFBWORLD_VIEWS_H

#include "zoosters/base.h"
#include "rw/rwviewbase.h"

struct RpWorldSector;
struct RpAtomic;

namespace RWS { namespace TFB {
// The camera's plugin data for world rendering (RW 3.7 RpWorldCameraExt, names RW's): the sectors inside the frustum.
struct RpWorldCameraExt {
	RpWorldSector **frustumSectors;
	RwInt32 space;
	RwInt32 position;   // the number of sectors in frustumSectors
};
} }

// The tie of an RpWorldSector's collision atomic list (RpTie: next link at +0, the atomic after the two links of the list node)
// and the list head in the sector (collAtomicsInWorldSector, +0x38 with 4-byte pointers).
struct RpTieView {
	RpTieView *next;
	RpTieView *prev;
	RpAtomic *atomic;
	RwLLLink lWorldSectorInAtomic;
	void *worldSector;
};
struct RpWorldSectorTies {
	RwSectorHeadView head;
	RpTieView collAtomicsInWorldSector;   // the list head
};
// The part of an RpAtomic read here: the object flags (+2, 4 = rpATOMICRENDER) and the render frame stamp (+0x60 with 4-byte pointers).
struct RpAtomicRenderView {
	RwObjectHasFrameView object;       // +0x00 (flags: object.object.flags)
	void *repEntry;                    // +0x14
	void *geometry;                    // +0x18
	RwSphereView boundingSphere;       // +0x1c
	RwSphereView worldBoundingSphere;  // +0x2c
	void *clump;                       // +0x3c
	RwLLLink inClump;                  // +0x40
	void *renderCallBack;              // +0x48
	char interpolator[0x14];           // +0x4c RpInterpolator
	unsigned short renderFrame;        // +0x60
};
ZOOSTERS_ASSERT_OFFSET(RpAtomicRenderView, renderFrame, 0x60);

#endif
