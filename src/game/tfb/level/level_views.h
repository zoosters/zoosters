#ifndef ZOOSTERS_TFB_LEVEL_LEVEL_VIEWS_H
#define ZOOSTERS_TFB_LEVEL_LEVEL_VIEWS_H

#include "rw/rwviewbase.h"

// RpWorld: world origin and bounding box (RW names).
struct RpWorldView {
	RwWorldHeadView head;
	RwV3d worldOrigin;
	RwBBox boundingBox;
};
ZOOSTERS_ASSERT_OFFSET(RpWorldView, worldOrigin, 0x44);
ZOOSTERS_ASSERT_OFFSET(RpWorldView, boundingBox, 0x50);

#endif
