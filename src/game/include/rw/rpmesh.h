// Mesh header of a geometry (RpMeshHeader).
#ifndef ZOOSTERS_RW_RPMESH_H
#define ZOOSTERS_RW_RPMESH_H

#include "rw/rpworld.h"

// The mesh header of a geometry (read by AtomicApplyUserDataFlags 0x44e3c0 to sort the meshes). The RpMesh array (12 bytes each)
// follows the header at firstMeshOffset.
struct RpMeshHeader {
	RwUInt32 flags;
	RwUInt16 numMeshes;
	RwUInt16 serialNum;
	RwUInt32 totalIndicesInMesh;
	RwUInt32 firstMeshOffset;
};
ZOOSTERS_ASSERT_SIZE(RpMeshHeader, 0x10);
static inline RpMeshHeader *RpGeometryGetMeshHeader(const RpGeometry *geometry)
{
	return *(RpMeshHeader *const *)((const char *)geometry + rwOff_RpGeometryBody_mesh);
}

#endif
