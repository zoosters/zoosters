#ifndef ZOOSTERS_MODULES_FX_RPLIGHTVIEW_H
#define ZOOSTERS_MODULES_FX_RPLIGHTVIEW_H

#include "zoosters/base.h"
#include "rw/rwviewbase.h"

// RpLight is opaque in include/rw; the game reads these members directly (the SDK's GetRadius / GetColor / GetFlags macros).
struct RpLightView {
	unsigned char type;            // +0 object.type
	unsigned char subType;         // +1
	unsigned char flags;           // +2 rpLIGHTLIGHTATOMICS = 1, rpLIGHTLIGHTWORLD = 2
	unsigned char privateFlags;    // +3
	RwFrame *parent;               // +4 the frame the light is attached to
	RwLLLink inFrame;              // +8 RwLLLink in the frame's object list
	void *sync;                    // +0x10 RwObjectHasFrameSyncFunction
	float radius;                  // +0x14
	RwRGBAReal color;              // +0x18
};
ZOOSTERS_ASSERT_OFFSET(RpLightView, parent, 4);
ZOOSTERS_ASSERT_OFFSET(RpLightView, radius, 0x14);
ZOOSTERS_ASSERT_OFFSET(RpLightView, color, 0x18);

#endif
