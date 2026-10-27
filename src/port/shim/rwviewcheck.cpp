// Compile-time check of the game's views of RenderWare objects (src/game) against librw (port/patches/librw) and rwlayouts.h: every
// member the game reads is compared with offsetof at both pointer sizes (x86: RW 3.6's offsets; x64: the natural widening). Nothing
// runs; a mismatch is a compile error. The RWA objects are checked at the end, against the audio shim's layouts. Not covered: D3DCAPS8
// (no pointers) and views of game-owned objects.
#include <stddef.h>
#include <rw.h>
#include "rwlayouts.h"
#include "rwaudio_layouts.h"

// Game headers. Views that exist more than once under one name are renamed around their include.
#include "rw/rwcore.h"
#include "rw/rpworld.h"
#include "rw/rphanim.h"
#include "rw/rtanim.h"
#include "rw/rpcollis.h"
#include "rw/rpmesh.h"
#include "modules/tfb/camera_views.h"
#include "modules/tfb/ctfbworld_views.h"
#include "modules/tfb/shadowrender_views.h"
#include "modules/fx/ptankext.h"
#include "modules/fx/particlepass_views.h"
#include "tfb/level/level_views.h"
#include "tfb/level/resourcehandlers_views.h"
#include "tfb/render/atomicflags_views.h"
#include "tfb/render/camera_stub.h"
#include "tfb/render/shadow_pass_views.h"
#include "tfb/render/yonbox_views.h"
#include "framework/startup/loadscreen_views.h"
#include "rw/rpsectorview.h"   // the one RpWorldSectorView (tight box, plugin, probe and corpus users all use it)
#define RpLightView RpLightViewFx
#include "modules/fx/rplightview.h"
#undef RpLightView
#include "tfb/level/level.h"     // RpLightView (the world link)

#define SAME(V, vm, O, om) static_assert(offsetof(V, vm) == offsetof(O, om), "layout: " #V "::" #vm " vs " #O "::" #om)
#define SAMESIZE(V, O) static_assert(sizeof(V) == sizeof(O), "size: " #V " vs " #O)

// ---------------------------------------- RwObject, frames, clumps
SAME(RwObjectView, type, rw::Object, type);
SAME(RwObjectView, flags, rw::Object, flags);
SAME(RwObjectView, privateFlags, rw::Object, privateFlags);
SAME(RwObjectView, parent, rw::Object, parent);
SAMESIZE(RwObjectView, rw::Object);
SAMESIZE(RwObjectHasFrameView, rw::ObjectWithFrame);
SAME(RwObjectHasFrameView, inFrame, rw::ObjectWithFrame, inFrame);
SAME(RwObjectHasFrameView, sync, rw::ObjectWithFrame, syncCB);

SAME(RwFrame, parent, rw::Frame, object.parent);
SAME(RwFrame, inDirtyListLink, rw::Frame, inDirtyList);
SAME(RwFrame, modelling, rw::Frame, matrix);
SAME(RwFrame, ltm, rw::Frame, ltm);
SAME(RwFrame, objectList, rw::Frame, objectList);
SAMESIZE(RwMatrix, rw::Matrix);

SAME(RpClump, parent, rw::Clump, object.parent);
SAME(RpClump, atomicList, rw::Clump, atomics);
SAME(RpClump, lightList, rw::Clump, lights);
SAME(RpClump, cameraList, rw::Clump, cameras);
SAME(RpClump, inWorldLink, rw::Clump, inWorld);

// ---------------------------------------- rasters, textures, images
SAME(RwRasterBody, parent, rw::Raster, parent);
SAME(RwRasterBody, cpPixels, rw::Raster, pixels);
SAME(RwRasterBody, palette, rw::Raster, palette);
SAME(RwRasterBody, width, rw::Raster, width);
SAME(RwRasterBody, height, rw::Raster, height);
SAME(RwRasterBody, depth, rw::Raster, depth);
SAME(RwRasterBody, stride, rw::Raster, stride);
SAME(RwRasterBody, nOffsetX, rw::Raster, rwOffsetX);
SAME(RwRasterBody, cType, rw::Raster, cType);
SAME(RwRasterBody, cFormat, rw::Raster, cFormat);
SAME(RwRasterView, parent, rw::Raster, parent);
SAME(RwRasterView, pixels, rw::Raster, pixels);
SAME(RwRasterView, palette, rw::Raster, palette);
SAME(RwRasterView, width, rw::Raster, width);
SAME(RwRasterView, height, rw::Raster, height);
SAME(RwRasterView, depth, rw::Raster, depth);
SAME(RwRasterView, stride, rw::Raster, stride);
SAME(RwRasterView, pad1c, rw::Raster, rwOffsetX);
SAME(RwRasterView, type, rw::Raster, cType);
SAME(RwRasterView, flags, rw::Raster, cFlags);
SAME(RwRasterView, privateFlags, rw::Raster, cPrivateFlags);
SAME(RwRasterView, format, rw::Raster, cFormat);
SAME(RasterCA, m_parent, rw::Raster, parent);
SAME(RasterCA, m_width, rw::Raster, width);
SAME(RasterCA, m_height, rw::Raster, height);

SAME(RwTextureBody, raster, rw::Texture, raster);
SAME(RwTextureBody, dict, rw::Texture, dict);
SAME(RwTextureBody, lInDictionary, rw::Texture, inDict);
SAME(RwTextureBody, name, rw::Texture, name);
SAME(RwTextureBody, mask, rw::Texture, mask);
SAME(RwTextureBody, filterAddressing, rw::Texture, filterAddressing);
SAME(RwTextureView, raster, rw::Texture, raster);
SAME(RwTextureView, dictionary, rw::Texture, dict);
SAME(RwTextureView, link, rw::Texture, inDict);
SAME(RwTextureView, name, rw::Texture, name);
SAME(RwTextureView, mask, rw::Texture, mask);
SAME(RwTextureView, filter, rw::Texture, filterAddressing);          // byte 0: filter
static_assert(offsetof(RwTextureView, addressing) == offsetof(RwTextureView, filter) + 1, "addressing follows the filter byte");
static_assert(sizeof(TexturePadBase) == offsetof(rw::Texture, name), "TextureNameView: the name follows the raster, dictionary and link");

SAME(RwImageView, width, rw::Image, width);
SAME(RwImageView, height, rw::Image, height);
SAME(RwImageView, depth, rw::Image, depth);

// ---------------------------------------- cameras
SAME(RwCameraView, type, rw::Camera, object);
SAME(RwCameraView, projection, rw::Camera, projection);
SAME(RwCameraView, beginUpdate, rw::Camera, beginUpdateCB);
SAME(RwCameraView, endUpdate, rw::Camera, endUpdateCB);
SAME(RwCameraView, viewMatrix, rw::Camera, viewMatrix);
SAME(RwCameraView, frame, rw::Camera, object.object.parent);
SAME(RwCameraView, inFrame, rw::Camera, object.inFrame);
SAME(RwCameraView, sync, rw::Camera, object.syncCB);
SAME(RwCameraView, frameBuffer, rw::Camera, frameBuffer);
SAME(RwCameraView, zBuffer, rw::Camera, zBuffer);
SAME(RwCameraView, viewWindow, rw::Camera, viewWindow);
SAME(RwCameraView, recipViewWindow, rw::Camera, recipViewWindow);
SAME(RwCameraView, viewOffset, rw::Camera, viewOffset);
SAME(RwCameraView, nearPlane, rw::Camera, nearPlane);
SAME(RwCameraView, farPlane, rw::Camera, farPlane);
SAME(RwCameraView, fogPlane, rw::Camera, fogPlane);
SAME(RwCameraView, zScale, rw::Camera, zScale);
SAME(RwCameraView, zShift, rw::Camera, zShift);
SAME(RwCameraView, frustumPlanes, rw::Camera, frustumPlanes);
SAMESIZE(RwFrustumPlaneView, rw::FrustumPlane);
SAME(RwFrustumPlaneView, distance, rw::FrustumPlane, plane.distance);
SAME(RwFrustumPlaneView, closestX, rw::FrustumPlane, closestX);
SAME(RwCameraBody, projectionType, rw::Camera, projection);
SAME(RwCameraBody, beginUpdate, rw::Camera, beginUpdateCB);
SAME(RwCameraBody, endUpdate, rw::Camera, endUpdateCB);
SAME(RwCameraBody, viewMatrix, rw::Camera, viewMatrix);
SAME(RwCameraBody, frameBuffer, rw::Camera, frameBuffer);
SAME(RwCameraBody, zBuffer, rw::Camera, zBuffer);
SAME(RwCameraBody, viewWindow, rw::Camera, viewWindow);
SAME(RwCameraBody, recipViewWindow, rw::Camera, recipViewWindow);
SAME(RwCameraBody, viewOffset, rw::Camera, viewOffset);
SAME(RwCameraBody, nearPlane, rw::Camera, nearPlane);
SAME(RwCameraBody, farPlane, rw::Camera, farPlane);
SAME(RwCameraBody, fogPlane, rw::Camera, fogPlane);
SAME(RwCameraBody, zScale, rw::Camera, zScale);
SAME(RwCameraBody, zShift, rw::Camera, zShift);
SAME(RwCameraBody, frustumPlanes, rw::Camera, frustumPlanes);
SAME(RwCameraBuffersView, m_projection, rw::Camera, projection);
SAME(RwCameraBuffersView, m_viewMatrix, rw::Camera, viewMatrix);
SAME(RwCameraBuffersView, m_frameBuffer, rw::Camera, frameBuffer);
SAME(RwCameraBuffersView, m_zBuffer, rw::Camera, zBuffer);
SAME(RwCameraStub, m_projection, rw::Camera, projection);
SAME(RwCameraStub, m_viewMatrix, rw::Camera, viewMatrix);
SAME(RwCameraStub, m_frameBuffer, rw::Camera, frameBuffer);
SAME(RwCameraStub, m_nearPlane, rw::Camera, nearPlane);
// the camera's world plugin data (RpWorldCameraExt): sectors, capacity, count
SAME(RWS::TFB::RpWorldCameraExt, frustumSectors, PortCameraExt, frustumSectors);
SAME(RWS::TFB::RpWorldCameraExt, space, PortCameraExt, space);
SAME(RWS::TFB::RpWorldCameraExt, position, PortCameraExt, position);
SAME(RpWorldCameraExtView, sectors, PortCameraExt, frustumSectors);
SAME(RpWorldCameraExtView, space, PortCameraExt, space);
SAME(RpWorldCameraExtView, count, PortCameraExt, position);

// ---------------------------------------- atomics, geometry, lights
#define CHECK_ATOMIC_PREFIX(V, vObj, vRep, vGeom, vClump, vInClump, vRender, vInterp, vFrame) 	SAME(V, vObj, rw::Atomic, object); 	SAME(V, vRep, rw::Atomic, rwRepEntry); 	SAME(V, vGeom, rw::Atomic, geometry); 	SAME(V, vClump, rw::Atomic, clump); 	SAME(V, vInClump, rw::Atomic, inClump); 	SAME(V, vRender, rw::Atomic, renderCB); 	SAME(V, vInterp, rw::Atomic, rwInterpolator); 	SAME(V, vFrame, rw::Atomic, rwRenderFrame)
CHECK_ATOMIC_PREFIX(RpAtomicBody, object, repEntry, geometry, clump, inClump, renderCallBack, interpolator, renderFrame);
SAME(RpAtomicBody, boundingSphere, rw::Atomic, boundingSphere);
SAME(RpAtomicBody, worldBoundingSphere, rw::Atomic, worldBoundingSphere);
SAME(RpAtomicBody, llWorldSectorsInAtomic, rw::Atomic, rwSectors);
SAME(RpAtomicBody, pipeline, rw::Atomic, pipeline);
SAME(RpAtomicView, object, rw::Atomic, object);
SAME(RpAtomicView, repEntry, rw::Atomic, rwRepEntry);
SAME(RpAtomicView, geometry, rw::Atomic, geometry);
SAME(RpAtomicView, boundingSphere, rw::Atomic, boundingSphere);
SAME(RpAtomicView, worldBoundingSphere, rw::Atomic, worldBoundingSphere);
SAME(RpAtomicView, clump, rw::Atomic, clump);
CHECK_ATOMIC_PREFIX(RpAtomicRenderView, object, repEntry, geometry, clump, inClump, renderCallBack, interpolator, renderFrame);
SAME(RpAtomicRenderView, object.object.flags, rw::Atomic, object.object.flags);
CHECK_ATOMIC_PREFIX(AtomicStampView, m_object, m_repEntry, m_geometry, m_clump, m_inClump, m_renderCallBack, m_interpolator, m_renderFrame);
SAME(PassAtomicView, object, rw::Atomic, object);
SAME(PassAtomicView, geometry, rw::Atomic, geometry);
SAME(PassAtomicView, clump, rw::Atomic, clump);
SAME(PassAtomicView, inClump, rw::Atomic, inClump);
SAME(PassAtomicView, renderCallBack, rw::Atomic, renderCB);

SAME(RpGeometryBody, flags, rw::Geometry, flags);
SAME(RpGeometryBody, lockedSinceLastInst, rw::Geometry, lockedSinceInst);
SAME(RpGeometryBody, numTriangles, rw::Geometry, numTriangles);
SAME(RpGeometryBody, numVertices, rw::Geometry, numVertices);
SAME(RpGeometryBody, numMorphTargets, rw::Geometry, numMorphTargets);
SAME(RpGeometryBody, numTexCoordSets, rw::Geometry, numTexCoordSets);
SAME(RpGeometryBody, materials, rw::Geometry, matList.materials);
SAME(RpGeometryBody, numMaterials, rw::Geometry, matList.numMaterials);
SAME(RpGeometryBody, triangles, rw::Geometry, triangles);
SAME(RpGeometryBody, preLitLum, rw::Geometry, colors);
SAME(RpGeometryBody, texCoords, rw::Geometry, texCoords);
SAME(RpGeometryBody, mesh, rw::Geometry, meshHeader);
SAME(RpGeometryBody, repEntry, rw::Geometry, instData);
SAME(RpGeometryBody, morphTarget, rw::Geometry, morphTargets);
SAME(RpMorphTargetBody, parentGeom, rw::MorphTarget, parent);
SAME(RpMorphTargetBody, boundingSphere, rw::MorphTarget, boundingSphere);
SAME(RpMorphTargetBody, verts, rw::MorphTarget, vertices);
SAME(RpMorphTargetBody, normals, rw::MorphTarget, normals);
SAMESIZE(RpMorphTargetBody, rw::MorphTarget);
SAMESIZE(RpTriangle, rw::Triangle);
SAMESIZE(RpMeshHeader, rw::MeshHeader);
SAME(RpMeshHeader, totalIndicesInMesh, rw::MeshHeader, totalIndices);
SAME(RpMeshHeader, firstMeshOffset, rw::MeshHeader, pad);
SAME(RpMeshBody, indices, rw::Mesh, indices);
SAME(RpMeshBody, numIndices, rw::Mesh, numIndices);
SAME(RpMeshBody, material, rw::Mesh, material);
SAME(RpMaterialBody, texture, rw::Material, texture);
SAME(RpMaterialBody, color, rw::Material, color);

SAME(RpLightViewFx, parent, rw::Light, object.object.parent);
SAME(RpLightViewFx, inFrame, rw::Light, object.inFrame);
SAME(RpLightViewFx, sync, rw::Light, object.syncCB);
SAME(RpLightViewFx, radius, rw::Light, radius);
SAME(RpLightViewFx, color, rw::Light, color);
SAME(RpLightView, object, rw::Light, object);
SAME(RpLightView, radius, rw::Light, radius);
SAME(RpLightView, color, rw::Light, color);
SAME(RpLightView, minusCosAngle, rw::Light, minusCosAngle);
SAME(RpLightView, worldSectorsInLight, rw::Light, rwSectorsInLight);
SAME(RpLightView, inWorld, rw::Light, inWorld);

// ---------------------------------------- animation (RtAnim / RpHAnim)
SAME(RtAnimInterpolatorInfo, typeID, rw::AnimInterpolatorInfo, id);
SAME(RtAnimInterpolatorInfo, interpKeyFrameSize, rw::AnimInterpolatorInfo, interpKeyFrameSize);
SAME(RtAnimInterpolatorInfo, animKeyFrameSize, rw::AnimInterpolatorInfo, animKeyFrameSize);
SAME(RtAnimKeyFrameHeader, prevFrame, rw::KeyFrameHeader, prev);
SAME(RtAnimKeyFrameHeader, time, rw::KeyFrameHeader, time);
SAME(RtAnimAnimation, interpInfo, rw::Animation, interpInfo);
SAME(RtAnimAnimation, numFrames, rw::Animation, numFrames);
SAME(RtAnimAnimation, flags, rw::Animation, flags);
SAME(RtAnimAnimation, duration, rw::Animation, duration);
SAME(RtAnimAnimation, pFrames, rw::Animation, keyframes);
SAME(RtAnimInterpolator, pCurrentAnim, rw::AnimInterpolator, currentAnim);
SAME(RtAnimInterpolator, currentTime, rw::AnimInterpolator, currentTime);
SAME(RtAnimInterpolator, pNextFrame, rw::AnimInterpolator, nextFrame);
SAME(RtAnimInterpolator, pAnimCallBack, rw::AnimInterpolator, rwAnimCallBack);
SAME(RtAnimInterpolator, pAnimCallBackData, rw::AnimInterpolator, rwAnimCallBackData);
SAME(RtAnimInterpolator, animCallBackTime, rw::AnimInterpolator, rwAnimCallBackTime);
SAME(RtAnimInterpolator, pAnimLoopCallBack, rw::AnimInterpolator, rwAnimLoopCallBack);
SAME(RtAnimInterpolator, pAnimLoopCallBackData, rw::AnimInterpolator, rwAnimLoopCallBackData);
SAME(RtAnimInterpolator, maxInterpKeyFrameSize, rw::AnimInterpolator, maxInterpKeyFrameSize);
SAME(RtAnimInterpolator, currentInterpKeyFrameSize, rw::AnimInterpolator, currentInterpKeyFrameSize);
SAME(RtAnimInterpolator, currentAnimKeyFrameSize, rw::AnimInterpolator, currentAnimKeyFrameSize);
SAME(RtAnimInterpolator, numNodes, rw::AnimInterpolator, numNodes);
SAME(RtAnimInterpolator, isSubInterpolator, rw::AnimInterpolator, rwIsSubInterpolator);
SAME(RtAnimInterpolator, offsetInParent, rw::AnimInterpolator, rwOffsetInParent);
SAME(RtAnimInterpolator, pParentAnimation, rw::AnimInterpolator, rwParentAnimation);
SAME(RtAnimInterpolator, keyFrameApplyCB, rw::AnimInterpolator, applyCB);
SAME(RtAnimInterpolator, keyFrameBlendCB, rw::AnimInterpolator, blendCB);
SAME(RtAnimInterpolator, keyFrameInterpolateCB, rw::AnimInterpolator, interpCB);
SAME(RtAnimInterpolator, keyFrameAddCB, rw::AnimInterpolator, addCB);
SAMESIZE(RtAnimInterpolator, rw::AnimInterpolator);    // the interpolated key frames follow the structure
SAME(RpHAnimInterpFrame, keyFrame1, rw::HAnimInterpFrame, keyFrame1);
SAME(RpHAnimInterpFrame, keyFrame2, rw::HAnimInterpFrame, keyFrame2);
SAME(RpHAnimInterpFrame, orientation, rw::HAnimInterpFrame, q);
SAME(RpHAnimInterpFrame, translation, rw::HAnimInterpFrame, t);
SAMESIZE(RpHAnimInterpFrame, rw::HAnimInterpFrame);
SAME(RpHAnimNodeInfo, nodeID, rw::HAnimNodeInfo, id);
SAME(RpHAnimNodeInfo, nodeIndex, rw::HAnimNodeInfo, index);
SAME(RpHAnimNodeInfo, flags, rw::HAnimNodeInfo, flags);
SAME(RpHAnimNodeInfo, pFrame, rw::HAnimNodeInfo, frame);
SAMESIZE(RpHAnimNodeInfo, rw::HAnimNodeInfo);
SAME(RpHAnimHierarchy, flags, rw::HAnimHierarchy, flags);
SAME(RpHAnimHierarchy, numNodes, rw::HAnimHierarchy, numNodes);
SAME(RpHAnimHierarchy, pMatrixArray, rw::HAnimHierarchy, matrices);
SAME(RpHAnimHierarchy, pMatrixArrayUnaligned, rw::HAnimHierarchy, matricesUnaligned);
SAME(RpHAnimHierarchy, pNodeInfo, rw::HAnimHierarchy, nodeInfo);
SAME(RpHAnimHierarchy, parentFrame, rw::HAnimHierarchy, parentFrame);
SAME(RpHAnimHierarchy, parentHierarchy, rw::HAnimHierarchy, parentHierarchy);
SAME(RpHAnimHierarchy, rootParentOffset, rw::HAnimHierarchy, rwRootParentOffset);
SAME(RpHAnimHierarchy, currentAnim, rw::HAnimHierarchy, interpolator);

// ---------------------------------------- worlds, sectors, ties, collision
#define CHECK_WORLD_HEAD(V, h) \
	SAME(V, h.flags, ShimWorld, flags); \
	SAME(V, h.renderOrder, ShimWorld, renderOrder); \
	SAME(V, h.matList, ShimWorld, matlist); \
	SAME(V, h.rootSector, ShimWorld, rootSector); \
	SAME(V, h.numTexCoordSets, ShimWorld, numTexCoordSets); \
	SAME(V, h.numClumpsInWorld, ShimWorld, numClumpsInWorld); \
	SAME(V, h.currentClumpLink, ShimWorld, currentClumpLink); \
	SAME(V, h.clumpList, ShimWorld, clumpList); \
	SAME(V, h.lightList, ShimWorld, lightList); \
	SAME(V, h.directionalLightList, ShimWorld, directionalLightList)
CHECK_WORLD_HEAD(RpWorld, head);
SAME(RpWorld, worldOrigin, ShimWorld, worldOrigin);
SAME(RpWorld, boundingBox.sup, ShimWorld, sup);
SAME(RpWorld, boundingBox.inf, ShimWorld, inf);
SAME(RpWorld, renderCallBack, ShimWorld, renderCallBack);
SAME(RpWorld, pipeline, ShimWorld, pipeline);
CHECK_WORLD_HEAD(RpWorldView, head);
SAME(RpWorldView, worldOrigin, ShimWorld, worldOrigin);
SAME(RpWorldView, boundingBox.sup, ShimWorld, sup);
SAME(RpWorldView, boundingBox.inf, ShimWorld, inf);
SAME(RpWorldRenderOrderView, renderOrder, ShimWorld, renderOrder);
SAME(RpWorldRenderOrderView, flags, ShimWorld, flags);
SAME(RwWorldHeadView, object, ShimWorld, object);
SAMESIZE(RwObjectView, decltype(ShimWorld::object));

#define CHECK_SECTOR(V, ty, tri, vert, norm, tex, lum, rep, coll, lights, box, tight, mesh, pipe, win, nv, nt) 	SAME(V, ty, ShimSector, type); 	SAME(V, tri, ShimSector, triangles); 	SAME(V, vert, ShimSector, vertices); 	SAME(V, norm, ShimSector, normals); 	SAME(V, tex, ShimSector, texCoords); 	SAME(V, lum, ShimSector, preLitLum); 	SAME(V, rep, ShimSector, repEntry); 	SAME(V, coll, ShimSector, collAtomicsInWorldSector); 	SAME(V, lights, ShimSector, lightsInWorldSector); 	SAME(V, box, ShimSector, sup); 	SAME(V, tight, ShimSector, tsup); 	SAME(V, mesh, ShimSector, mesh); 	SAME(V, pipe, ShimSector, pipeline); 	SAME(V, win, ShimSector, matListWindowBase); 	SAME(V, nv, ShimSector, numVertices); 	SAME(V, nt, ShimSector, numTriangles)
CHECK_SECTOR(RpWorldSectorView, type, triangles, vertices, normals, texCoords, preLitLum, repEntry, collAtomicsInWorldSector, lightsInWorldSector,
             boundingBox.sup, tightBoundingBox.sup, mesh, pipeline, matListWindowBase, numVertices, numTriangles);
SAME(RwSectorHeadView, type, ShimSector, type);
SAME(RwSectorHeadView, triangles, ShimSector, triangles);
SAME(RwSectorHeadView, vertices, ShimSector, vertices);
SAME(RwSectorHeadView, normals, ShimSector, normals);
SAME(RwSectorHeadView, texCoords, ShimSector, texCoords);
SAME(RwSectorHeadView, preLitLum, ShimSector, preLitLum);
SAME(RwSectorHeadView, repEntry, ShimSector, repEntry);
static_assert(sizeof(RwSectorHeadView) == offsetof(ShimSector, collAtomicsInWorldSector), "RwSectorHeadView ends where the link lists start");
SAME(RpWorldSectorTies, collAtomicsInWorldSector, ShimSector, collAtomicsInWorldSector);
static_assert(sizeof(RwSectorHeadView) == offsetof(RpWorldSectorTies, collAtomicsInWorldSector), "ties: the list head follows the sector head");
SAME(RpTieView, next, PortTie, lAtomicInWorldSector.next);
SAME(RpTieView, prev, PortTie, lAtomicInWorldSector.prev);
SAME(RpTieView, atomic, PortTie, apAtom);
SAME(RpTieView, lWorldSectorInAtomic, PortTie, lWorldSectorInAtomic);
SAME(RpTieView, worldSector, PortTie, worldSector);

SAME(RpCollTree, flags, PortCollTree, flags);
SAME(RpCollTree, bbox.sup, PortCollTree, sup);
SAME(RpCollTree, bbox.inf, PortCollTree, inf);
SAME(RpCollTree, numEntries, PortCollTree, numEntries);
SAME(RpCollTree, numSplits, PortCollTree, numSplits);
SAME(RpCollTree, splits, PortCollTree, splits);
SAME(RpCollTree, map, PortCollTree, map);
SAMESIZE(RpCollSector, PortCollSector);
SAMESIZE(RpCollSplit, PortCollSplit);

// ---------------------------------------- engine instance
SAME(RwGlobals, curCamera, RwGlobalsShim, curCamera);
SAME(RwGlobals, curWorld, RwGlobalsShim, curWorld);
SAME(RwGlobals, renderFrame, RwGlobalsShim, renderFrame);
SAME(RwGlobals, lightFrame, RwGlobalsShim, lightFrame);
SAME(RwGlobals, dOpenDevice.gammaCorrection, RwGlobalsShim, gammaCorrection);
SAME(RwGlobals, dOpenDevice.fpSystem, RwGlobalsShim, fpSystem);
SAME(RwGlobals, dOpenDevice.zBufferNear, RwGlobalsShim, zBufferNear);
SAME(RwGlobals, dOpenDevice.zBufferFar, RwGlobalsShim, zBufferFar);
SAME(RwGlobals, dOpenDevice.fpRenderStateSet, RwGlobalsShim, fpRenderStateSet);
SAME(RwGlobals, dOpenDevice.fpRenderStateGet, RwGlobalsShim, fpRenderStateGet);
SAME(RwGlobals, dOpenDevice.fpIm2DRenderLine, RwGlobalsShim, fpIm2DRenderLine);
SAME(RwGlobals, dOpenDevice.fpIm2DRenderIndexedPrimitive, RwGlobalsShim, fpIm2DRenderIndexedPrimitive);
SAME(RwGlobals, dOpenDevice.fpIm3DRenderLine, RwGlobalsShim, fpIm3DRenderLine);
SAME(RwGlobals, dOpenDevice.fpIm3DRenderIndexedPrimitive, RwGlobalsShim, fpIm3DRenderIndexedPrimitive);
SAME(RwGlobals, stdFunc, RwGlobalsShim, stdFunc);
static_assert(offsetof(RwGlobals, dirtyFrameList) == offsetof(RwGlobalsShim, stdFunc) + 29 * sizeof(void *), "dirtyFrameList follows stdFunc[29]");
SAME(RwGlobals, stringFuncs.vecSprintf, RwGlobalsShim, rwsprintf);
SAME(RwGlobals, stringFuncs.vecStrcmp, RwGlobalsShim, rwstrcmp);
SAME(RwGlobals, stringFuncs.vecSscanf, RwGlobalsShim, rwsscanf);
SAME(RwGlobals, memoryFuncs.rwmalloc, RwGlobalsShim, rwmalloc);
SAME(RwGlobals, memoryFuncs.rwcalloc, RwGlobalsShim, rwcalloc);
SAME(RwGlobals, memoryAlloc, RwGlobalsShim, freeListAlloc);
SAME(RwGlobals, memoryFree, RwGlobalsShim, freeListFree);
SAME(RwGlobals, metrics, RwGlobalsShim, metrics);
SAME(RwGlobals, engineStatus, RwGlobalsShim, engineStatus);
SAME(RwGlobals, resArenaInitSize, RwGlobalsShim, resArenaInitSize);
static_assert(sizeof(RwGlobals) <= sizeof(RwGlobalsShim), "the shim's engine block holds every member of RwGlobals");

// ---------------------------------------- RpPTank, 2D fonts, D3D8 resource entry
SAME(RpPTankExtView, maxPCount, RpPTankAtomicExtPrv, maxPCount);
SAME(RpPTankExtView, actPCount, RpPTankAtomicExtPrv, actPCount);
SAME(RpPTankExtView, isAStructure, RpPTankAtomicExtPrv, isAStructure);
SAME(RpPTankExtView, rawdata, RpPTankAtomicExtPrv, rawdata);
SAME(RpPTankExtView, defaultRenderCB, RpPTankAtomicExtPrv, defaultRenderCB);
SAME(RpPTankExtView, ptankCallBacks, RpPTankAtomicExtPrv, ptankCallBacks);
SAME(RpPTankExtView, insCB, RpPTankAtomicExtPrv, insCB);
SAME(RpPTankExtView, lockFlags, RpPTankAtomicExtPrv, lockFlags);
SAME(RpPTankExtView, instFlags, RpPTankAtomicExtPrv, instFlags);
SAME(RpPTankExtView, platFlags, RpPTankAtomicExtPrv, platFlags);
SAME(RpPTankExtView, data, RpPTankAtomicExtPrv, publicData.data);
SAME(RpPTankExtView, clusters, RpPTankAtomicExtPrv, publicData.clusters);
SAME(RpPTankExtView, userData, RpPTankAtomicExtPrv, publicData.userData);
SAME(RpPTankExtView, numClusters, RpPTankAtomicExtPrv, publicData.format);
SAME(RpPTankExtView, srcBlend, RpPTankAtomicExtPrv, publicData.srcBlend);
SAME(RpPTankExtView, dstBlend, RpPTankAtomicExtPrv, publicData.dstBlend);
SAME(RpPTankExtView, vertexAlpha, RpPTankAtomicExtPrv, publicData.vertexAlphaBlend);
static_assert(sizeof(RpPTankExtView().clusters) == sizeof(RpPTankData::clusters), "RpPTankLockStruct x 9");

SAME(Rt2dFontView, height, Font, height);
SAME(Rt2dFontView, texture, Font, tex);

SAME(RxD3D8ResEntryView, numMeshes, PortRecvEntry, numMeshes);
SAME(RxD3D8ResEntryView, meshes, PortRecvEntry, meshes);
SAMESIZE(RxD3D8InstanceDataView, PortRecvMesh);
SAME(RxD3D8InstanceDataView, minVert, PortRecvMesh, minVert);
SAME(RxD3D8InstanceDataView, stride, PortRecvMesh, stride);
SAME(RxD3D8InstanceDataView, numVertices, PortRecvMesh, numVertices);
SAME(RxD3D8InstanceDataView, numIndices, PortRecvMesh, numIndices);
SAME(RxD3D8InstanceDataView, material, PortRecvMesh, material);
SAME(RxD3D8InstanceDataView, vertexShader, PortRecvMesh, vertexShader);
SAME(RxD3D8InstanceDataView, primType, PortRecvMesh, primType);
SAME(RxD3D8InstanceDataView, indexBuffer, PortRecvMesh, indexBuffer);
SAME(RxD3D8InstanceDataView, vertexBuffer, PortRecvMesh, vertexBuffer);
SAME(RxD3D8InstanceDataView, baseIndex, PortRecvMesh, baseIndex);
SAME(RxD3D8InstanceDataView, flags, PortRecvMesh, flags);

// ---------------------------------------- RWA audio (rwaudio.h, rwaudio_layouts.h)
// P = the pointer size: RWA's x86 offsets are the P == 4 column of each pair, the second value is what the same members give with
// 8-byte pointers.
#define RWA_P ((int)sizeof(void *))
#define RWA_AT(T, m, x86, x64) static_assert((int)offsetof(T, m) == (RWA_P == 4 ? (x86) : (x64)), "RWA layout: " #T "::" #m)
// RwaWaveFormat: dataType and miscData are the pointers
RWA_AT(RwaWaveFormat, sampleRate, 0x00, 0x00);
RWA_AT(RwaWaveFormat, dataType, 0x04, 0x08);
RWA_AT(RwaWaveFormat, length, 0x08, 0x10);
RWA_AT(RwaWaveFormat, bitsPerSample, 0x0c, 0x14);
RWA_AT(RwaWaveFormat, numChannels, 0x0d, 0x15);
RWA_AT(RwaWaveFormat, miscData, 0x10, 0x18);
RWA_AT(RwaWaveFormat, miscSize, 0x14, 0x20);
RWA_AT(RwaWaveFormat, flags, 0x18, 0x24);
static_assert((int)sizeof(RwaWaveFormat) == (RWA_P == 4 ? 0x1c : 0x28), "RWA layout: RwaWaveFormat size");
RWA_AT(RwaWave, format, 0x10, 0x10);
RWA_AT(RwaWave, targetFormat, 0x2c, 0x38);
// the wave object of the shim: the game's RwaWave first, our tail behind it
RWA_AT(RwaWaveObj, pub, 0, 0);
static_assert(sizeof(RwaWave) == offsetof(RwaWave, targetFormat) + sizeof(RwaWaveFormat), "RwaWave ends with its target format");
// RwaVirtualVoice: handle and wave are the pointers, everything between them is 4 bytes wide
RWA_AT(RwaVirtualVoice, handle, 0x30, 0x30);
RWA_AT(RwaVirtualVoice, flags, 0x34, 0x38);
RWA_AT(RwaVirtualVoice, frequency, 0x3c, 0x40);
RWA_AT(RwaVirtualVoice, gain, 0x40, 0x44);
RWA_AT(RwaVirtualVoice, pan, 0x44, 0x48);
RWA_AT(RwaVirtualVoice, wavePos, 0x48, 0x4c);
RWA_AT(RwaVirtualVoice, wave, 0x50, 0x58);
RWA_AT(RwaVirtualVoice, startStamp, 0x54, 0x60);
RWA_AT(RwaVirtualVoice, position, 0x58, 0x64);
RWA_AT(RwaVirtualVoice, velocity, 0x64, 0x70);
RWA_AT(RwaVirtualVoice, minDistance, 0x70, 0x7c);
RWA_AT(RwaVirtualVoice, maxDistance, 0x74, 0x80);
RWA_AT(RwaVirtualVoice, handleOwnsRate, 0x84, 0x90);
static_assert(sizeof(RwV3d) == 12, "RwV3d is three floats (memcpy'd by the audio shim)");
RWA_AT(RwaVoiceObj, pub, 0, 0);
static_assert((int)offsetof(RwaVoiceObj, clock) == kVoiceClockOffset, "voice clock behind the voice object");
// output and listener hold no pointers: the offsets are the same at both sizes
#define RWA_SAME(T, m, off) static_assert((int)offsetof(T, m) == (off), "RWA layout: " #T "::" #m)
RWA_SAME(RwaOsOutput, gain, 0x38);
RWA_SAME(RwaOsOutput, environment, 0x44);
RWA_SAME(RwaOsOutput, dirtyFlags, 0x78);
static_assert(sizeof(RwaEnvironment) == 12 * sizeof(int), "the environment is 12 words (AlaOutput / the shim copy it as int[12])");
RWA_SAME(RwaListener, position, 0x28);
RWA_SAME(RwaListener, up, 0x40);
RWA_SAME(RwaListener, at, 0x4c);
RWA_SAME(RwaListener, right, 0x58);
RWA_SAME(RwaListener, rolloffFactor, 0x64);
RWA_SAME(RwaListener, dopplerScale, 0x68);
RWA_SAME(RwaListener, distanceFactor, 0x6c);
RWA_SAME(RwaListener, dirtyFlags, 0x70);
// stream blocks the game fills in and the shim reads: one pointer, then 4-byte members
RWA_AT(RwaStreamMedia, path, 0, 0);
RWA_AT(RwaStreamMedia, mediaType, 4, 8);
RWA_AT(RwaStreamMedia, loop, 0x20, 0x24);
RWA_AT(RwaStreamItem, name, 0, 0);
RWA_AT(RwaStreamItem, type, 4, 8);
RWA_AT(RwaStreamItem, start, 8, 12);
RWA_AT(RwaStreamItems, items, 0, 0);
RWA_AT(RwaStreamItems, count, 4, 8);
RWA_AT(StreamLoadB, name, 0, 0);
RWA_AT(StreamLoadB, type, 4, 8);
RWA_AT(StreamLoadB, length, 8, 12);
RWA_AT(StreamLoadB, unit, 0xc, 0x10);
RWA_AT(StreamLoadB, arg4, 0x10, 0x14);
RWA_SAME(RwaStreamPosQuery, seconds, 8);
static_assert(sizeof(float) == sizeof(int), "the stream position query reports samples in the float word");
RWA_SAME(RwaStreamGain, pair, 4);
RWA_SAME(RwaStreamGain, side, 8);
RWA_SAME(RwaStreamGain, gain, 12);
// the shim's stream object: observed play state at +0xd6c and our clock at +0xe00 (the offsets RWA's stream object has on x86)
RWA_SAME(RwaStream, playState, 0xd6c);
RWA_SAME(RwaStream, clock, 0xe00);
