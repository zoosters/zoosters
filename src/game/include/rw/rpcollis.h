// RpCollision declarations: intersection primitives, the triangle a query reports, the two queries.
#ifndef ZOOSTERS_RW_RPCOLLIS_H
#define ZOOSTERS_RW_RPCOLLIS_H

#include "rw/rpworld.h"
extern "C" {

// What an intersection query tests with. Only 1 (line), 3 (sphere) and 4 (box) are used.
enum RpIntersectType {
	rpINTERSECTNONE = 0,
	rpINTERSECTLINE = 1,
	rpINTERSECTPOINT = 2,
	rpINTERSECTSPHERE = 3,
	rpINTERSECTBOX = 4,
	rpINTERSECTATOMIC = 5
};

#ifndef ZOOSTERS_RPINTERSECTION_DEFINED
#define ZOOSTERS_RPINTERSECTION_DEFINED
// The primitive of an intersection query; which member is valid depends on RpIntersection::type.
union RpIntersectData {
	RwLine line;     // rpINTERSECTLINE
	RwV3d point;     // rpINTERSECTPOINT
	RwSphere sphere; // rpINTERSECTSPHERE
	RwBBox box;      // rpINTERSECTBOX
	void *object;    // rpINTERSECTATOMIC: the atomic
};
struct RpIntersection {
	RpIntersectData t;
	RpIntersectType type;
};
ZOOSTERS_ASSERT_SIZE(RpIntersection, 0x1C);
ZOOSTERS_ASSERT_OFFSET(RpIntersection, type, 0x18);
#endif

#ifndef ZOOSTERS_RPCOLLISIONTRIANGLE_DEFINED
#define ZOOSTERS_RPCOLLISIONTRIANGLE_DEFINED
// A triangle a query reports: the plane (normal and a point on it), the triangle's index and pointers to its three vertices.
struct RpCollisionTriangle {
	RwV3d normal;
	RwV3d point;
	RwInt32 index;
	RwV3d *vertices[3];
};
ZOOSTERS_ASSERT_SIZE(RpCollisionTriangle, 0x28);
ZOOSTERS_ASSERT_OFFSET(RpCollisionTriangle, point, 0x0C);
ZOOSTERS_ASSERT_OFFSET(RpCollisionTriangle, index, 0x18);
ZOOSTERS_ASSERT_OFFSET(RpCollisionTriangle, vertices, 0x1C);
#endif

// --- Collision trees (plugin data of a geometry built with collision data; RW 3.7 calls them RpCollTree).
enum {
	rpCOLLTREE_MAXDEPTH = 32,
	rpCOLLSECTOR_CONTENTS_SPLIT = 0xff, // RpCollSector::contents of a split node (else the leaf's entry count)
	rpCOLLSECTOR_TYPE_AXISMASK = 0x0c   // RpCollSector::type: the split axis times 4 (x 0, y 4, z 8)
};
// A node of a collision tree: a split (contents rpCOLLSECTOR_CONTENTS_SPLIT, index = its RpCollSplit) or a leaf (`contents` entries
// from `index` on in the entry map). `value` is the node's bound on the split axis: the upper one of a left child, the lower one of a
// right child.
struct RpCollSector {
	RwUInt8 type;
	RwUInt8 contents;
	RwUInt16 index;
	RwReal value;
};
ZOOSTERS_ASSERT_SIZE(RpCollSector, 8);
struct RpCollSplit {
	RpCollSector leftSector;
	RpCollSector rightSector;
};
ZOOSTERS_ASSERT_SIZE(RpCollSplit, 0x10);
struct RpCollTree {
	RwUInt32 flags;
	RwBBox bbox;
	RwUInt16 numEntries;
	RwUInt16 numSplits;
	RpCollSplit *splits; // splits[0] is the root
	RwUInt16 *map;       // entry map: leaf entry -> triangle index (0: identity)
};
ZOOSTERS_ASSERT_OFFSET(RpCollTree, bbox, 4);
ZOOSTERS_ASSERT_OFFSET(RpCollTree, numEntries, 0x1c);
ZOOSTERS_ASSERT_OFFSET(RpCollTree, splits, 0x20);
ZOOSTERS_ASSERT_OFFSET(RpCollTree, map, 0x24);

// The plugin's offset in a geometry (RpCollision registers it at start-up).
extern RwInt32 _rpCollisionGeometryDataOffset;
// The collision tree of a geometry (0: built without collision data).
static inline RpCollTree *RpCollisionGeometryGetCollTree(const RpGeometry *geometry)
{
	return *(RpCollTree *const *)((const char *)geometry + _rpCollisionGeometryDataOffset);
}

// Query callbacks: return the triangle to continue, or 0 to stop. `distance` is normalised as the toolkit defines per type.
typedef RpCollisionTriangle *(*RpIntersectionCallBackWorldTriangle)(RpIntersection *intersection, RpWorldSector *sector,
                                                                    RpCollisionTriangle *collTriangle, RwReal distance,
                                                                    void *data);
typedef RpCollisionTriangle *(*RpIntersectionCallBackGeometryTriangle)(RpIntersection *intersection,
                                                                       RpCollisionTriangle *collTriangle, RwReal distance,
                                                                       void *data);

// Calls `callBack` for every triangle of the world (or of one atomic's geometry) that the primitive touches.
RpWorld *RpCollisionWorldForAllIntersections(RpWorld *world, RpIntersection *intersection,
                                             RpIntersectionCallBackWorldTriangle callBack, void *data);
RpAtomic *RpAtomicForAllIntersections(RpAtomic *atomic, RpIntersection *intersection,
                                      RpIntersectionCallBackGeometryTriangle callBack, void *data);

}

#endif
