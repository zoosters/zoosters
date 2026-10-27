#ifndef ZOOSTERS_RW_RPSECTORVIEW_H
#define ZOOSTERS_RW_RPSECTORVIEW_H

#include "rw/rwviewbase.h"
#include "rw/rwcore.h"

struct RwResEntry;
struct RxPipeline;

// The one view of an RpWorldSector (RW 3.7 layout, 8 texture coordinate sets). Members are RW's own types, so the layout
// follows the pointer size (offsets in the comments are for 4-byte pointers).
struct RpWorldSectorView {
	RwInt32 type;
	char *triangles;  // RpTriangle array (8 bytes each: 3 vertex indices, material index, flags byte at +6)
	RwV3d *vertices;
	void *normals;
	void *texCoords[8];
	RwRGBA *preLitLum;  // per vertex (0: not prelit)
	RwResEntry *repEntry;
	RwLinkList collAtomicsInWorldSector;
	RwLinkList lightsInWorldSector;
	RwBBox boundingBox;
	RwBBox tightBoundingBox;
	void *mesh;
	RxPipeline *pipeline;
	RwUInt16 matListWindowBase;
	RwUInt16 numVertices;
	RwUInt16 numTriangles;
};
ZOOSTERS_ASSERT_OFFSET(RpWorldSectorView, preLitLum, 0x30);
ZOOSTERS_ASSERT_OFFSET(RpWorldSectorView, repEntry, 0x34);
ZOOSTERS_ASSERT_OFFSET(RpWorldSectorView, tightBoundingBox, 0x60);
ZOOSTERS_ASSERT_OFFSET(RpWorldSectorView, pipeline, 0x7c);
ZOOSTERS_ASSERT_OFFSET(RpWorldSectorView, numTriangles, 0x84);

#endif
