#ifndef ZOOSTERS_MODULES_FX_PARTICLEPASS_VIEWS_H
#define ZOOSTERS_MODULES_FX_PARTICLEPASS_VIEWS_H

#include "rw/rwviewbase.h"

struct PassAtomicView {
	RwObjectHasFrameView object;       // +0x00
	void *repEntry;                    // +0x14
	void *geometry;                    // +0x18
	RwSphereView boundingSphere;       // +0x1c
	RwSphereView worldBoundingSphere;  // +0x2c
	void *clump;                       // +0x3c
	RwLLLink inClump;                  // +0x40
	void (*renderCallBack)(PassAtomicView *);   // +0x48
};

#endif
