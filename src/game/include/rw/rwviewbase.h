// Building blocks of the views of RenderWare objects (the per-module Rp...View / Rw...View structs). A view lists the members of
// an RW 3.6 object in RW's order with RW's types, so it has the object's layout at both pointer sizes (pointer members, not `int`).
// port/shim/rwviewcheck.cpp compares every view with the object the port runs on.
#include <stddef.h>
#include "rw/rwcore.h"   // (it includes this header again once its value types are defined)
#ifndef ZOOSTERS_RW_RWVIEWBASE_H
#define ZOOSTERS_RW_RWVIEWBASE_H

// RwObject: type, subType, flags, privateFlags, parent (8 bytes with 4-byte pointers).
struct RwObjectView {
	RwUInt8 type;
	RwUInt8 subType;
	RwUInt8 flags;
	RwUInt8 privateFlags;
	void *parent;
};

// RwObjectHasFrame: the object, its link in the frame's object list and the sync callback (0x14 bytes with 4-byte pointers).
struct RwObjectHasFrameView {
	RwObjectView object;
	RwLLLink inFrame;
	void *sync;
};

// RwSphere (centre, radius).
struct RwSphereView {
	RwV3d center;
	RwReal radius;
};

// RpWorldSector's leading members (RW 3.6 rpworld.h) up to the two link lists: +0x00 .. +0x38 with 4-byte pointers.
struct RwSectorHeadView {
	RwInt32 type;
	void *triangles;
	void *vertices;
	void *normals;
	void *texCoords[8];
	void *preLitLum;
	void *repEntry;
};

// RpWorld's leading members up to worldOrigin (+0x00 .. +0x44 with 4-byte pointers).
struct RwWorldHeadView {
	RwObjectView object;
	RwUInt32 flags;
	RwInt32 renderOrder;
	struct { void **materials; RwInt32 numMaterials; RwInt32 space; } matList;
	void *rootSector;
	RwInt32 numTexCoordSets;
	RwInt32 numClumpsInWorld;
	RwLLLink *currentClumpLink;
	RwLinkList clumpList;
	RwLinkList lightList;
	RwLinkList directionalLightList;
};

// The bodies of the objects the accessors of rw/*.h reach into (RW 3.6 order, up to the last member an accessor reads).
// The accessors use offsetof on these, so they hold at both pointer sizes.
struct RwRasterBody {
	void *parent;
	RwUInt8 *cpPixels;
	RwUInt8 *palette;
	RwInt32 width;
	RwInt32 height;
	RwInt32 depth;
	RwInt32 stride;
	RwInt16 nOffsetX, nOffsetY;
	RwUInt8 cType, cFlags, privateFlags, cFormat;
};
struct RwTextureBody {
	void *raster;
	void *dict;
	RwLLLink lInDictionary;
	char name[32];
	char mask[32];
	RwUInt32 filterAddressing;
};
struct RwCameraBody {
	RwObjectHasFrameView object;
	RwInt32 projectionType;
	void *beginUpdate;
	void *endUpdate;
	RwMatrix viewMatrix;
	void *frameBuffer;
	void *zBuffer;
	RwV2d viewWindow;
	RwV2d recipViewWindow;
	RwV2d viewOffset;
	RwReal nearPlane;
	RwReal farPlane;
	RwReal fogPlane;
	RwReal zScale;
	RwReal zShift;
	char frustumPlanes[6][0x14];  // RwFrustumPlane x 6
};
struct RpAtomicBody {
	RwObjectHasFrameView object;
	void *repEntry;
	void *geometry;
	RwSphereView boundingSphere;
	RwSphereView worldBoundingSphere;
	void *clump;
	RwLLLink inClump;
	void *renderCallBack;
	char interpolator[0x14];  // RpInterpolator
	RwUInt32 renderFrame;
	RwLinkList llWorldSectorsInAtomic;
	void *pipeline;
};
struct RpMaterialBody {
	void *texture;
	RwRGBA color;
};
struct RpMeshBody {
	void *indices;
	RwUInt32 numIndices;
	void *material;
};
struct RpGeometryBody {
	RwObjectView object;
	RwUInt32 flags;
	RwUInt16 lockedSinceLastInst;
	RwInt32 numTriangles;
	RwInt32 numVertices;
	RwInt32 numMorphTargets;
	RwInt32 numTexCoordSets;
	void **materials;  // matList
	RwInt32 numMaterials;
	RwInt32 spaceMaterials;
	void *triangles;
	void *preLitLum;
	void *texCoords[8];
	void *mesh;
	void *repEntry;
	void *morphTarget;
};
struct RpMorphTargetBody {
	void *parentGeom;
	RwSphereView boundingSphere;
	RwV3d *verts;
	RwV3d *normals;
};
// The offsets the accessors use, as enumerators: integer constants like the literals they replace (an offsetof expression in an accessor
// changed the code cl 13.10 generates for a game function that was only compiled with them, MenuPointer_Update).
enum RwBodyOffsets {
	rwOff_RwObjectView_parent = offsetof(RwObjectView, parent),
	rwOff_RwCameraBody_frustumPlanes = offsetof(RwCameraBody, frustumPlanes),
	rwSize_RpMorphTargetBody = sizeof(RpMorphTargetBody),
	rwOff_RwCameraBody_projectionType = offsetof(RwCameraBody, projectionType),
	rwOff_RwCameraBody_frameBuffer = offsetof(RwCameraBody, frameBuffer),
	rwOff_RwCameraBody_zBuffer = offsetof(RwCameraBody, zBuffer),
	rwOff_RwCameraBody_viewWindow = offsetof(RwCameraBody, viewWindow),
	rwOff_RwRasterBody_width = offsetof(RwRasterBody, width),
	rwOff_RwRasterBody_height = offsetof(RwRasterBody, height),
	rwOff_RwRasterBody_depth = offsetof(RwRasterBody, depth),
	rwOff_RwTextureBody_name = offsetof(RwTextureBody, name),
	rwOff_RpGeometryBody_flags = offsetof(RpGeometryBody, flags),
	rwOff_RpGeometryBody_mesh = offsetof(RpGeometryBody, mesh),
	rwOff_RpAtomicBody_geometry = offsetof(RpAtomicBody, geometry),
	rwOff_RpGeometryBody_numTriangles = offsetof(RpGeometryBody, numTriangles),
	rwOff_RpGeometryBody_triangles = offsetof(RpGeometryBody, triangles),
	rwOff_RpGeometryBody_morphTarget = offsetof(RpGeometryBody, morphTarget),
	rwOff_RpMorphTargetBody_verts = offsetof(RpMorphTargetBody, verts),
	rwOff_RpGeometryBody_numVertices = offsetof(RpGeometryBody, numVertices),
	rwOff_RpGeometryBody_numMorphTargets = offsetof(RpGeometryBody, numMorphTargets),
	rwOff_RpMorphTargetBody_normals = offsetof(RpMorphTargetBody, normals),
	rwOff_RpAtomicBody_interpolator = offsetof(RpAtomicBody, interpolator),
	rwOff_RpAtomicBody_clump = offsetof(RpAtomicBody, clump),
	rwOff_RpAtomicBody_inClump = offsetof(RpAtomicBody, inClump),
	rwOff_RpAtomicBody_renderCallBack = offsetof(RpAtomicBody, renderCallBack),
	rwOff_RpGeometryBody_numMaterials = offsetof(RpGeometryBody, numMaterials),
	rwOff_RpGeometryBody_materials = offsetof(RpGeometryBody, materials),
	rwOff_RpGeometryBody_preLitLum = offsetof(RpGeometryBody, preLitLum),
	rwOff_RpAtomicBody_pipeline = offsetof(RpAtomicBody, pipeline),
	rwOff_RpMorphTargetBody_boundingSphere = offsetof(RpMorphTargetBody, boundingSphere),
	rwOff_RpMaterialBody_color = offsetof(RpMaterialBody, color),
	rwOff_RpMeshBody_numIndices = offsetof(RpMeshBody, numIndices),
	rwOff_RpMeshBody_material = offsetof(RpMeshBody, material),
	rwSize_RpMeshBody = sizeof(RpMeshBody)
};

#endif
