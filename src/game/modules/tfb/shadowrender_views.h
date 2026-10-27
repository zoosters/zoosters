#ifndef ZOOSTERS_MODULES_TFB_SHADOWRENDER_VIEWS_H
#define ZOOSTERS_MODULES_TFB_SHADOWRENDER_VIEWS_H

#include "rw/rwviewbase.h"

// The render order member of an RpWorld (+0xc with 4-byte pointers), as CTFBWorld::RenderWorldPass sets it.
struct RpWorldRenderOrderView {
	RwObjectView object;   // +0x00
	RwUInt32 flags;        // +0x08 (RpWorld::flags)
	RwInt32 renderOrder;   // +0x0c
};

#endif
