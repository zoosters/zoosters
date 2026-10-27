// RenderWare world API: clumps, atomics, geometry, materials, lights, the world. Only RpClump has a (partial) layout.
#ifndef ZOOSTERS_RW_RPWORLD_H
#define ZOOSTERS_RW_RPWORLD_H

#include "rw/rwcore.h"
#include "rw/rwviewbase.h"
extern "C" {

#ifndef ZOOSTERS_RPCLUMP_DEFINED
#define ZOOSTERS_RPCLUMP_DEFINED
// A clump: a frame plus the atomics, lights and cameras attached to it. PARTIAL (to the end of inWorldLink, +0x28): never allocate or copy.
struct RpClump {
	char object[4];        // RwObject header (type, subType, flags, privateFlags)
	void *parent;  // the clump's frame
	RwLinkList atomicList;
	RwLinkList lightList;
	RwLinkList cameraList;
	RwLLLink inWorldLink;  // the link in the world's clump list
};
ZOOSTERS_ASSERT_OFFSET(RpClump, parent, 0x04);
ZOOSTERS_ASSERT_OFFSET(RpClump, atomicList, 0x08);
ZOOSTERS_ASSERT_OFFSET(RpClump, lightList, 0x10);
ZOOSTERS_ASSERT_OFFSET(RpClump, cameraList, 0x18);
ZOOSTERS_ASSERT_OFFSET(RpClump, inWorldLink, 0x20);
#endif

// --- Opaque types
struct RpAtomic;
struct RpGeometry;
struct RpMaterial;
struct RpLight;
// A world. PARTIAL: the leading members up to the pipeline (bounding box: sup +0x50, inf +0x5c). Never allocate or copy.
struct RpWorld {
	RwWorldHeadView head;  // .. +0x44
	RwV3d worldOrigin;
	RwBBox boundingBox;
	void *renderCallBack;
	void *pipeline;
};
enum RpWorldOffsets {
	rwOff_RpWorld_head_matList_numMaterials = offsetof(RpWorld, head.matList.numMaterials),
	rwOff_RpWorld_head_matList_materials = offsetof(RpWorld, head.matList.materials),
	rwOff_RpWorld_pipeline = offsetof(RpWorld, pipeline)
};
ZOOSTERS_ASSERT_OFFSET(RpWorld, boundingBox, 0x50);
struct RpWorldSector;
struct RpMesh;
struct RpMeshHeader;
struct RpTriangle;
struct RwSphere;

// --- Accessors that are macros in RenderWare
#ifndef RpClumpGetFrame // a header may still define the RW macro form: same operation
// The frame a clump hangs from (the parent of its object header, +4).
static inline RwFrame *RpClumpGetFrame(const RpClump *clump)
{
	return (RwFrame *)clump->parent;
}
#endif
#ifndef RpAtomicGetFrame // rws/rwfx.h still defines the RW macro form on its own RpAtomic view: same operation
// The frame an atomic hangs from (the parent of its object header, +4).
static inline RwFrame *RpAtomicGetFrame(const RpAtomic *atomic)
{
	return *(RwFrame *const *)((const char *)atomic + rwOff_RwObjectView_parent);
}
#endif
#ifndef ZOOSTERS_RPTRIANGLE_DEFINED
#define ZOOSTERS_RPTRIANGLE_DEFINED
// A triangle of a geometry (8 bytes). The game keeps collision flags in the low byte of matIndex.
struct RpTriangle {
	RwUInt16 vertIndex[3];
	RwInt16 matIndex;
};
ZOOSTERS_ASSERT_SIZE(RpTriangle, 8);
#endif
struct RpMorphTarget;
#ifndef RpAtomicGetGeometry
// The geometry of an atomic (+0x18).
static inline RpGeometry *RpAtomicGetGeometry(const RpAtomic *atomic)
{
	return *(RpGeometry *const *)((const char *)atomic + rwOff_RpAtomicBody_geometry);
}
#endif
#ifndef RpGeometryGetNumTriangles
// The triangle count of a geometry (+0x10).
static inline RwInt32 RpGeometryGetNumTriangles(const RpGeometry *geometry)
{
	return *(const RwInt32 *)((const char *)geometry + rwOff_RpGeometryBody_numTriangles);
}
#endif
#ifndef RpGeometryGetTriangles
// The triangles of a geometry (+0x2c).
static inline RpTriangle *RpGeometryGetTriangles(const RpGeometry *geometry)
{
	return *(RpTriangle *const *)((const char *)geometry + rwOff_RpGeometryBody_triangles);
}
#endif
#ifndef RpGeometryGetMorphTarget
// Morph target `index` of a geometry (an array at +0x5c, 0x1c bytes per target).
static inline RpMorphTarget *RpGeometryGetMorphTarget(const RpGeometry *geometry, RwInt32 index)
{
	return (RpMorphTarget *)(*(char *const *)((const char *)geometry + rwOff_RpGeometryBody_morphTarget) + index * rwSize_RpMorphTargetBody);
}
#endif
#ifndef RpMorphTargetGetVertices
// The vertex positions of a morph target (+0x14).
static inline RwV3d *RpMorphTargetGetVertices(const RpMorphTarget *morphTarget)
{
	return *(RwV3d *const *)((const char *)morphTarget + rwOff_RpMorphTargetBody_verts);
}
#endif
// More geometry / morph target accessors.
static inline RwInt32 RpGeometryGetNumVertices(const RpGeometry *geometry)
{
	return *(const RwInt32 *)((const char *)geometry + rwOff_RpGeometryBody_numVertices);
}
static inline RwInt32 RpGeometryGetNumMorphTargets(const RpGeometry *geometry)
{
	return *(const RwInt32 *)((const char *)geometry + rwOff_RpGeometryBody_numMorphTargets);
}
// The normals of a morph target (+0x18); its bounding sphere is the RwSphere at +4.
static inline RwV3d *RpMorphTargetGetVertexNormals(const RpMorphTarget *morphTarget)
{
	return *(RwV3d *const *)((const char *)morphTarget + rwOff_RpMorphTargetBody_normals);
}
static inline RwSphere *RpMorphTargetGetBoundingSphere(const RpMorphTarget *morphTarget)
{
	return (RwSphere *)((const char *)morphTarget + rwOff_RpMorphTargetBody_boundingSphere);
}
// The morph interpolator of an atomic (0x14 bytes at +0x4c): flags, start / end morph target, time, 1 / time, position.
struct RpInterpolator {
	RwInt32 flags;
	RwInt16 startMorphTarget;
	RwInt16 endMorphTarget;
	RwReal time;
	RwReal recipTime;
	RwReal position;
};
ZOOSTERS_ASSERT_SIZE(RpInterpolator, 0x14);
static inline RpInterpolator *RpAtomicGetInterpolator(const RpAtomic *atomic)
{
	return (RpInterpolator *)((const char *)atomic + rwOff_RpAtomicBody_interpolator);
}
static inline RwReal RpInterpolatorGetValue(const RpInterpolator *interpolator)
{
	return interpolator->position;
}
static inline RwReal RpInterpolatorGetScale(const RpInterpolator *interpolator)
{
	return interpolator->time;
}
static inline RwInt32 RpInterpolatorGetStartMorphTarget(const RpInterpolator *interpolator)
{
	return interpolator->startMorphTarget;
}
static inline RwInt32 RpInterpolatorGetEndMorphTarget(const RpInterpolator *interpolator)
{
	return interpolator->endMorphTarget;
}
// The atomic of a link in its clump's atomicList (the atomic's inClump link is at +0x40 in the 32-bit layout).
static inline RpAtomic *RpAtomicFromClumpLink(RwLLLink *link)
{
	return (RpAtomic *)((char *)link - rwOff_RpAtomicBody_inClump);
}

#ifndef RpAtomicGetClump
// The clump an atomic belongs to (+0x3c).
static inline RpClump *RpAtomicGetClump(const RpAtomic *atomic)
{
	return *(RpClump *const *)((const char *)atomic + rwOff_RpAtomicBody_clump);
}
#endif
// The render callback of an atomic (+0x48). Setting 0 installs RW's default callback.
typedef RpAtomic *(*RpAtomicCallBackRender)(RpAtomic *atomic);
RpAtomic *AtomicDefaultRenderCallBack(RpAtomic *atomic); // 0x4bb2d0
#ifndef RpAtomicGetRenderCallBack
static inline RpAtomicCallBackRender RpAtomicGetRenderCallBack(const RpAtomic *atomic)
{
	return *(const RpAtomicCallBackRender *)((const char *)atomic + rwOff_RpAtomicBody_renderCallBack);
}
#endif
#ifndef RpMaterialGetTexture
// The texture of a material (+0).
static inline RwTexture *RpMaterialGetTexture(const RpMaterial *material)
{
	return *(RwTexture *const *)material;
}
#endif
#ifndef RpAtomicSetRenderCallBack
static inline void RpAtomicSetRenderCallBack(RpAtomic *atomic, RpAtomicCallBackRender callback)
{
	RpAtomicCallBackRender *slot = (RpAtomicCallBackRender *)((char *)atomic + rwOff_RpAtomicBody_renderCallBack);
	*slot = callback;
	if (!*slot)
		*slot = AtomicDefaultRenderCallBack;
}
#endif
// Geometry flags (+8) and material list (array +0x20, count +0x24).
// The values are RW's (rw 3.7 bageomet.h); the game's TFB atomic renderer sets PRELIT-dependent relocks and LIGHT / MODULATEMATERIALCOLOR.
enum RpGeometryFlag { rpGEOMETRYTRISTRIP = 0x01, rpGEOMETRYNORMALS = 0x10, rpGEOMETRYNATIVE = 0x01000000, rpGEOMETRYPRELIT = 0x08, rpGEOMETRYLIGHT = 0x20, rpGEOMETRYMODULATEMATERIALCOLOR = 0x40 };
#ifndef RpGeometryGetFlags
static inline RwUInt32 RpGeometryGetFlags(const RpGeometry *geometry)
{
	return *(const RwUInt32 *)((const char *)geometry + rwOff_RpGeometryBody_flags);
}
#endif
#ifndef RpGeometrySetFlags
static inline RpGeometry *RpGeometrySetFlags(RpGeometry *geometry, RwUInt32 flags)
{
	*(RwUInt32 *)((char *)geometry + rwOff_RpGeometryBody_flags) = flags;
	return geometry;
}
#endif
#ifndef RpGeometryGetNumMaterials
static inline RwInt32 RpGeometryGetNumMaterials(const RpGeometry *geometry)
{
	return *(const RwInt32 *)((const char *)geometry + rwOff_RpGeometryBody_numMaterials);
}
#endif
// The material array of a geometry's material list (+0x20), for code that walks it by pointer (TFBGeometry_RelockPrelight 0x45d8e0).
static inline RpMaterial *const *RpGeometryGetMaterialArray(const RpGeometry *geometry)
{
	return *(RpMaterial *const *const *)((const char *)geometry + rwOff_RpGeometryBody_materials);
}
#ifndef RpGeometryGetMaterial
static inline RpMaterial *RpGeometryGetMaterial(const RpGeometry *geometry, RwInt32 index)
{
	return (*(RpMaterial *const *const *)((const char *)geometry + rwOff_RpGeometryBody_materials))[index];
}
#endif
#ifndef RpMaterialSetColor
// The colour of a material (an RwRGBA at +4).
static inline RpMaterial *RpMaterialSetColor(RpMaterial *material, const RwRGBA *color)
{
	*(RwRGBA *)((char *)material + rwOff_RpMaterialBody_color) = *color;
	return material;
}
#endif

// Lock modes of RpGeometryLock (RW's values; the TFB code locks the pre-light colours (8) and everything (0xFFF)).
enum RpGeometryLockMode { rpGEOMETRYLOCKPRELIGHT = 0x0008, rpGEOMETRYLOCKALL = 0x0FFF };

// --- More accessors that are macros in RenderWare
// The pre-light colours of a geometry (+0x30; the TFB sector render passes the same member of a world sector to the triangle sort).
#ifndef RpGeometryGetPreLightColors
static inline RwRGBA *RpGeometryGetPreLightColors(const RpGeometry *geometry)
{
	return *(RwRGBA *const *)((const char *)geometry + rwOff_RpGeometryBody_preLitLum);
}
#endif
// The colour of a material (an RwRGBA at +4). RW 3.6's macro yields a writable pointer.
#ifndef RpMaterialGetColor
static inline const RwRGBA *RpMaterialGetColor(const RpMaterial *material)
{
	return (const RwRGBA *)((const char *)material + rwOff_RpMaterialBody_color);
}
#endif
// The colour of a material for in-place changes (a writable pointer, as RW 3.6's macro).
static inline RwRGBA *RpMaterialGetColorRef(RpMaterial *material)
{
	return (RwRGBA *)((char *)material + rwOff_RpMaterialBody_color);
}
// Object flags of an atomic (the RwObject header byte at +2); rpATOMICRENDER is checked by TFBClump_Render 0x45ccc0.
enum RpAtomicFlag { rpATOMICCOLLISIONTEST = 0x01, rpATOMICRENDER = 0x04 };   // collision flag: AtomicHelper::SetCanCollide 0x44e260 range
#ifndef RpAtomicGetFlags
static inline RwUInt32 RpAtomicGetFlags(const RpAtomic *atomic)
{
	return *(const RwUInt8 *)((const char *)atomic + 2);
}
#endif
static inline RwUInt32 RpAtomicSetFlags(RpAtomic *atomic, RwUInt32 flags)
{
	return *(RwUInt8 *)((char *)atomic + 2) = (RwUInt8)flags;
}
// The frame of a light (+4) and its link (RpLightSetFrame is the RwObjectHasFrame set-frame).
static inline RwFrame *RpLightGetFrame(const RpLight *light)
{
	return *(RwFrame *const *)((const char *)light + rwOff_RwObjectView_parent);
}
static inline void RpLightSetFrame(RpLight *light, RwFrame *frame)
{
	_rwObjectHasFrameSetFrame(light, frame);
}
static inline void RpClumpSetFrame(RpClump *clump, RwFrame *frame)
{
	clump->parent = frame;
}
// The materials of a world: array at +0x10, count at +0x14 (TFBWorld_UpdateMaterials 0x45feb0).
#ifndef RpWorldGetNumMaterials
static inline RwInt32 RpWorldGetNumMaterials(const RpWorld *world)
{
	return *(const RwInt32 *)((const char *)world + rwOff_RpWorld_head_matList_numMaterials);
}
#endif
#ifndef RpWorldGetMaterial
static inline RpMaterial *RpWorldGetMaterial(const RpWorld *world, RwInt32 matNum)
{
	return (*(RpMaterial *const *const *)((const char *)world + rwOff_RpWorld_head_matList_materials))[matNum];
}
#endif
// A mesh of a geometry or world sector (RpMesh: index array +0, index count +4, material +8; seen by TFBSector_FadeMeshCB 0x4604d0).
static inline RwUInt32 RpMeshGetNumIndices(const RpMesh *mesh)
{
	return *(const RwUInt32 *)((const char *)mesh + rwOff_RpMeshBody_numIndices);
}
static inline RpMaterial *RpMeshGetMaterial(const RpMesh *mesh)
{
	return *(RpMaterial *const *)((const char *)mesh + rwOff_RpMeshBody_material);
}

// --- Pipelines (RxPipeline stays opaque). An atomic with no pipeline of its own (+0x6c is 0) uses the engine's default one, kept
// in the pipeline globals (plugin data at _rxPipelineGlobalsOffset: atomic pipeline +0x3c, world sector pipeline +0x40;
// TFBAtomic::RenderCallback 0x45bce0, TFBSector_Render 0x4605a0). The world's own sector pipeline is at +0x6c of the RpWorld.
struct RxPipeline;
extern int _rxPipelineGlobalsOffset; // GLOBAL: 0x631f10 _rxPipelineGlobalsOffset
RxPipeline *RxPipelineExecute(RxPipeline *pipeline, void *data, RwBool heapReset); // 0x4e5e00
// Building a pipeline (CreateShadowPipeline 0x444a50): lock a new one, add the fragment of a node definition, unlock it, find
// the node by name and install a render callback. RxNodeDefinition is partial: only its name (first member) is read.
struct RxLockedPipe;
struct RxPipelineNode;
struct RxNodeDefinition {
	const RwChar *name;
};
typedef void (*RxD3D8AllInOneRenderCallBack)(RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags);
RxPipeline *RxPipelineCreate(void);                                                   // 0x4e5ef0
RxLockedPipe *RxPipelineLock(RxPipeline *pipeline);                                   // 0x4ed200
RxNodeDefinition *RxNodeDefinitionGetD3D8WorldSectorAllInOne(void);                   // 0x4c5a00
RxLockedPipe *RxLockedPipeAddFragment(RxLockedPipe *pipeline, RwUInt32 *firstIndex, RxNodeDefinition *nodeDefinition, ...); // 0x4ed5d0: the list ends with 0
RxPipeline *RxLockedPipeUnlock(RxLockedPipe *pipeline);                               // 0x4ebf90
RxPipelineNode *RxPipelineFindNodeByName(RxPipeline *pipeline, const RwChar *name, RxPipelineNode *start, RwInt32 *nodeIndex); // 0x4ed490
void RxD3D8AllInOneSetRenderCallBack(RxPipelineNode *node, RxD3D8AllInOneRenderCallBack callBack); // 0x4c51e0
void _rxPipelineDestroy(RxPipeline *pipeline);                                        // 0x4e5f70
static inline const RpAtomic *RpAtomicGetPipeline(const RpAtomic *atomic, RxPipeline **pipeline)
{
	*pipeline = *(RxPipeline *const *)((const char *)atomic + rwOff_RpAtomicBody_pipeline);
	return atomic;
}
// normal builds: the port has no RW pipeline globals (librw draws through its own pipelines; RxPipelineExecute only looks the pointer up
// among the game-built custom pipelines), so the shim answers (port/shim/rwshim.cpp: null) instead of a read of the engine block
extern "C" RxPipeline *port_RpAtomicGetDefaultPipeline(void);
extern "C" RxPipeline *port_RpWorldGetDefaultSectorPipeline(void);
static inline RxPipeline *RpAtomicGetDefaultPipeline() { return port_RpAtomicGetDefaultPipeline(); }
static inline RxPipeline *RpWorldGetDefaultSectorPipeline() { return port_RpWorldGetDefaultSectorPipeline(); }
static inline const RpWorld *RpWorldGetSectorPipeline(const RpWorld *world, RxPipeline **pipeline)
{
	*pipeline = *(RxPipeline *const *)((const char *)world + rwOff_RpWorld_pipeline);
	return world;
}

// --- Callbacks (a callback returns its object to continue the enumeration, or 0 to stop it)
typedef RpAtomic *(*RpAtomicCallBack)(RpAtomic *atomic, void *data);
typedef RpClump *(*RpClumpCallBack)(RpClump *clump, void *data);
typedef RpLight *(*RpLightCallBack)(RpLight *light, void *data);
typedef RpMaterial *(*RpMaterialCallBack)(RpMaterial *material, void *data);
typedef RpMesh *(*RpMeshCallBack)(RpMesh *mesh, RpMeshHeader *meshHeader, void *data);
typedef RpWorldSector *(*RpWorldSectorCallBack)(RpWorldSector *worldSector, void *data);

// --- Plugins: each RW object type can carry plugin data (see RwPluginObjectConstructor and friends in rw/rwcore.h).
// Registration returns the plugin data's offset in the object, or -1.
RwInt32 RpAtomicRegisterPlugin(RwInt32 size, RwUInt32 pluginId, RwPluginObjectConstructor constructCallBack,
                               RwPluginObjectDestructor destructCallBack, RwPluginObjectCopy copyCallBack);
RwInt32 RpAtomicRegisterPluginStream(RwUInt32 pluginId, RwPluginDataChunkReadCallBack readCallBack,
                                     RwPluginDataChunkWriteCallBack writeCallBack,
                                     RwPluginDataChunkGetSizeCallBack getSizeCallBack);
RwInt32 RpAtomicSetStreamAlwaysCallBack(RwUInt32 pluginId, RwPluginDataChunkAlwaysCallBack alwaysCallBack);
RwInt32 RpAtomicSetStreamRightsCallBack(RwUInt32 pluginId, RwPluginDataChunkRightsCallBack rightsCallBack);
RwInt32 RpClumpRegisterPlugin(RwInt32 size, RwUInt32 pluginId, RwPluginObjectConstructor constructCallBack,
                              RwPluginObjectDestructor destructCallBack, RwPluginObjectCopy copyCallBack);
RwInt32 RpClumpRegisterPluginStream(RwUInt32 pluginId, RwPluginDataChunkReadCallBack readCallBack,
                                    RwPluginDataChunkWriteCallBack writeCallBack,
                                    RwPluginDataChunkGetSizeCallBack getSizeCallBack);
RwInt32 RpGeometryRegisterPlugin(RwInt32 size, RwUInt32 pluginId, RwPluginObjectConstructor constructCallBack,
                                 RwPluginObjectDestructor destructCallBack, RwPluginObjectCopy copyCallBack);
RwInt32 RpGeometryRegisterPluginStream(RwUInt32 pluginId, RwPluginDataChunkReadCallBack readCallBack,
                                       RwPluginDataChunkWriteCallBack writeCallBack,
                                       RwPluginDataChunkGetSizeCallBack getSizeCallBack);
RwInt32 RpLightRegisterPlugin(RwInt32 size, RwUInt32 pluginId, RwPluginObjectConstructor constructCallBack,
                              RwPluginObjectDestructor destructCallBack, RwPluginObjectCopy copyCallBack);
RwInt32 RpLightRegisterPluginStream(RwUInt32 pluginId, RwPluginDataChunkReadCallBack readCallBack,
                                    RwPluginDataChunkWriteCallBack writeCallBack,
                                    RwPluginDataChunkGetSizeCallBack getSizeCallBack);
RwInt32 RpMaterialRegisterPlugin(RwInt32 size, RwUInt32 pluginId, RwPluginObjectConstructor constructCallBack,
                                 RwPluginObjectDestructor destructCallBack, RwPluginObjectCopy copyCallBack);
RwInt32 RpMaterialRegisterPluginStream(RwUInt32 pluginId, RwPluginDataChunkReadCallBack readCallBack,
                                       RwPluginDataChunkWriteCallBack writeCallBack,
                                       RwPluginDataChunkGetSizeCallBack getSizeCallBack);
RwInt32 RpWorldRegisterPlugin(RwInt32 size, RwUInt32 pluginId, RwPluginObjectConstructor constructCallBack,
                              RwPluginObjectDestructor destructCallBack, RwPluginObjectCopy copyCallBack);
RwInt32 RpWorldRegisterPluginStream(RwUInt32 pluginId, RwPluginDataChunkReadCallBack readCallBack,
                                    RwPluginDataChunkWriteCallBack writeCallBack,
                                    RwPluginDataChunkGetSizeCallBack getSizeCallBack);
RwInt32 RpWorldSectorRegisterPlugin(RwInt32 size, RwUInt32 pluginId, RwPluginObjectConstructor constructCallBack,
                                    RwPluginObjectDestructor destructCallBack, RwPluginObjectCopy copyCallBack);
RwInt32 RpWorldSectorRegisterPluginStream(RwUInt32 pluginId, RwPluginDataChunkReadCallBack readCallBack,
                                          RwPluginDataChunkWriteCallBack writeCallBack,
                                          RwPluginDataChunkGetSizeCallBack getSizeCallBack);

// --- Clumps
RpClump *RpClumpCreate(void);
RpClump *RpClumpClone(RpClump *clump);
RwBool RpClumpDestroy(RpClump *clump);
RpClump *RpClumpStreamRead(RwStream *stream);
RpClump *RpClumpAddAtomic(RpClump *clump, RpAtomic *atomic);
RpClump *RpClumpRemoveAtomic(RpClump *clump, RpAtomic *atomic);
RpClump *RpClumpForAllAtomics(RpClump *clump, RpAtomicCallBack callBack, void *data);
RwInt32 RpClumpGetNumAtomics(RpClump *clump);
RpWorld *RpClumpGetWorld(const RpClump *clump);

// --- Atomics
RpAtomic *RpAtomicClone(RpAtomic *atomic);
RwBool RpAtomicDestroy(RpAtomic *atomic);
RpAtomic *RpAtomicStreamRead(RwStream *stream);
RpAtomic *RpAtomicSetFrame(RpAtomic *atomic, RwFrame *frame);
RpWorld *RpAtomicGetWorld(const RpAtomic *atomic);
const RwSphere *RpAtomicGetWorldBoundingSphere(RpAtomic *atomic);

// --- Geometry and materials
RpGeometry *RpGeometryLock(RpGeometry *geometry, RwInt32 lockMode);
RpGeometry *RpGeometryUnlock(RpGeometry *geometry);
RpGeometry *RpGeometryForAllMaterials(RpGeometry *geometry, RpMaterialCallBack callBack, void *data);
const RpGeometry *RpGeometryForAllMeshes(const RpGeometry *geometry, RpMeshCallBack callBack, void *data);
const RpGeometry *RpGeometryTriangleGetVertexIndices(const RpGeometry *geometry, const RpTriangle *triangle, RwUInt16 *vertex1,
                                                     RwUInt16 *vertex2, RwUInt16 *vertex3);
RwBool RpMaterialDestroy(RpMaterial *material);
RpMaterial *RpMaterialSetTexture(RpMaterial *material, RwTexture *texture);

// --- Lights
RpLight *RpLightCreate(RwInt32 type);
RwBool RpLightDestroy(RpLight *light);
RpLight *RpLightSetColor(RpLight *light, const RwRGBAReal *color);
RpLight *RpLightSetRadius(RpLight *light, RwReal radius);
RpLight *RpLightSetConeAngle(RpLight *light, RwReal angle);
RwReal RpLightGetConeAngle(const RpLight *light);
RpWorld *RpLightGetWorld(const RpLight *light);

// --- World
RwBool RpWorldDestroy(RpWorld *world);
RpWorld *RpWorldStreamRead(RwStream *stream);
RpWorld *RpWorldRender(RpWorld *world);
RpWorld *RpWorldAddAtomic(RpWorld *world, RpAtomic *atomic);
RpWorld *RpWorldRemoveAtomic(RpWorld *world, RpAtomic *atomic);
RpWorld *RpWorldRemoveClump(RpWorld *world, RpClump *clump);
RpWorld *RpWorldAddLight(RpWorld *world, RpLight *light);
RpWorld *RpWorldRemoveLight(RpWorld *world, RpLight *light);
RpWorld *RpWorldAddCamera(RpWorld *world, RwCamera *camera);
RpWorld *RpWorldRemoveCamera(RpWorld *world, RwCamera *camera);
RpWorld *RpWorldForAllClumps(RpWorld *world, RpClumpCallBack callBack, void *data);
RpWorld *RpWorldForAllLights(RpWorld *world, RpLightCallBack callBack, void *data);
RpWorld *RpWorldForAllMaterials(RpWorld *world, RpMaterialCallBack callBack, void *data);
RpWorld *RpWorldForAllWorldSectors(RpWorld *world, RpWorldSectorCallBack callBack, void *data);
RpWorldSector *RpWorldSectorForAllAtomics(RpWorldSector *sector, RpAtomicCallBack callBack, void *data);
const RpWorldSector *RpWorldSectorForAllMeshes(const RpWorldSector *sector, RpMeshCallBack callBack, void *data);
RpWorld *
RwCameraGetWorld(const RwCamera *camera); // the world a camera was added to (an RwCamera function, declared with the world)

}

#endif
