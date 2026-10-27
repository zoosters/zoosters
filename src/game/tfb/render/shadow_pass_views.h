#ifndef ZOOSTERS_TFB_RENDER_SHADOW_PASS_VIEWS_H
#define ZOOSTERS_TFB_RENDER_SHADOW_PASS_VIEWS_H

#include "zoosters/base.h"
#include "rw/rwviewbase.h"

struct RpMaterial;

// The world plugin's data in an RwCamera (RpWorldCameraExt): the sectors the camera's frustum touches.
struct RpWorldCameraExtView {
	void **sectors;
	int space;
	int count;
};

// The part of an RxD3D8 resource entry the callback reads: the mesh count, then the instance data (RW 3.7 d3d8 RxD3D8InstanceData, 0x2c bytes each with 4-byte pointers).
struct RxD3D8InstanceDataView {
	RwUInt32 minVert;
	RwInt32 stride;         // vertex stride (SetStreamSource)
	RwInt32 numVertices;
	RwInt32 numIndices;     // index count of the indexed draw
	RpMaterial *material;
	RwUInt32 vertexShader;
	RwUInt32 primType;
	void *indexBuffer;      // 0: a plain draw
	void *vertexBuffer;
	RwUInt32 baseIndex;     // base vertex / index
	RwUInt32 flags;         // vertexAlpha, managed, remapped (bytes)
};
struct RxD3D8ResEntryView {
	char pad00[0x1a];
	unsigned short numMeshes;
	RxD3D8InstanceDataView meshes[1];
};

#endif
