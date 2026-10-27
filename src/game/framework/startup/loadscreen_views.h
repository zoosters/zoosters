#ifndef ZOOSTERS_FRAMEWORK_STARTUP_LOADSCREEN_VIEWS_H
#define ZOOSTERS_FRAMEWORK_STARTUP_LOADSCREEN_VIEWS_H

#include "rw/rwcore.h"

// RwImage head.
struct RwImageView {
	char pad00[4];
	RwInt32 width;    // +4
	RwInt32 height;   // +8
	RwInt32 depth;    // +0xc
};

#endif
