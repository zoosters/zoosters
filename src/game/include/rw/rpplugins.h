// Small RenderWare world plugins the game uses: DMorph, MatFX, UserData, Skin, PTank, PVS, Random, Spline, Patch, LOD. All opaque.
#ifndef ZOOSTERS_RW_RPPLUGINS_H
#define ZOOSTERS_RW_RPPLUGINS_H

#include "rw/rpworld.h"
#include "rw/rphanim.h"
extern "C" {

// --- RpDMorph: morph targets driven by animation
struct RpDMorphAnimation;
struct RpDMorphTarget;
RpDMorphAnimation *RpDMorphAnimationStreamRead(RwStream *stream);
RpDMorphAnimation *RpDMorphAnimationDestroy(RpDMorphAnimation *animation);
RwUInt32 RpDMorphAnimationGetNumDMorphTargets(RpDMorphAnimation *animation);
RpAtomic *RpDMorphAtomicSetAnimation(RpAtomic *atomic, RpDMorphAnimation *animation);
RpAtomic *RpDMorphAtomicAddTime(RpAtomic *atomic, RwReal time);
RwUInt32 RpDMorphGeometryGetNumDMorphTargets(const RpGeometry *geometry);
RpDMorphTarget *RpDMorphGeometryGetDMorphTarget(const RpGeometry *geometry, RwUInt32 index);

// --- RpMatFX: material effects (environment map, UV transform, ...)
enum RpMatFXMaterialFlags {
	rpMATFXEFFECTNULL = 0,
	rpMATFXEFFECTBUMPMAP = 1,
	rpMATFXEFFECTENVMAP = 2,
	rpMATFXEFFECTBUMPENVMAP = 3,
	rpMATFXEFFECTDUAL = 4,
	rpMATFXEFFECTUVTRANSFORM = 5,
	rpMATFXEFFECTDUALUVTRANSFORM = 6
};
RwBool RpMatFXPluginAttach(void);
RpMatFXMaterialFlags RpMatFXMaterialGetEffects(const RpMaterial *material);
RpMaterial *RpMatFXMaterialSetEffects(RpMaterial *material, RpMatFXMaterialFlags flags);
RpMaterial *RpMatFXMaterialSetEnvMapCoefficient(RpMaterial *material, RwReal coefficient);
RpMaterial *RpMatFXMaterialSetEnvMapFrame(RpMaterial *material, RwFrame *frame);
RpMaterial *RpMatFXMaterialSetUVTransformMatrices(RpMaterial *material, RwMatrix *baseTransform, RwMatrix *dualTransform);

// --- RpUserData: named arrays of ints, reals or strings the artists attach to geometries and frames
enum RpUserDataFormat { rpNAUSERDATAFORMAT = 0, rpINTUSERDATA, rpREALUSERDATA, rpSTRINGUSERDATA };
struct RpUserDataArray;
RwInt32 RpGeometryGetUserDataArrayCount(const RpGeometry *geometry);
RpUserDataArray *RpGeometryGetUserDataArray(const RpGeometry *geometry, RwInt32 index);
RwInt32 RwFrameGetUserDataArrayCount(const RwFrame *frame);
RpUserDataArray *RwFrameGetUserDataArray(const RwFrame *frame, RwInt32 index);
RwChar *RpUserDataArrayGetName(RpUserDataArray *userData);
RpUserDataFormat RpUserDataArrayGetFormat(RpUserDataArray *userData);
RwInt32 RpUserDataArrayGetNumElements(RpUserDataArray *userData);
RwInt32 RpUserDataArrayGetInt(RpUserDataArray *userData, RwInt32 index);

// --- RpSkin: skinned geometry
struct RpSkin;
RpSkin *RpSkinGeometryGetSkin(RpGeometry *geometry);
RwUInt32 RpSkinGetNumBones(RpSkin *skin);
const RwMatrix *RpSkinGetSkinToBoneMatrices(RpSkin *skin);
RpAtomic *RpSkinAtomicSetHAnimHierarchy(RpAtomic *atomic, RpHAnimHierarchy *hierarchy);

// --- RpPTank: particle tank (one atomic that renders many particles)
enum RpPTankDataFlags {
	rpPTANKDFLAGPOSITION = 0x00000001,
	rpPTANKDFLAGCOLOR = 0x00000002,
	rpPTANKDFLAGSIZE = 0x00000004,
	rpPTANKDFLAGVTX2TEXCOORDS = 0x00000080
};
enum RpPTankLockDataFlags {
	rpPTANKLFLAGPOSITION = 0x00000001,
	rpPTANKLFLAGCOLOR = 0x00000002,
	rpPTANKLFLAGSIZE = 0x00000004,
	rpPTANKLFLAGVTX2TEXCOORDS = 0x00000080
};
enum RpPTankLockFlags { rpPTANKLOCKWRITE = 0x40000000, rpPTANKLOCKREAD = -0x7FFFFFFF - 1 /* 0x80000000 */ };
// What RpPTankAtomicLock hands out: the first element of one attribute and the distance between elements.
struct RpPTankLockStruct {
	RwUInt8 *data;
	RwInt32 stride;
};
ZOOSTERS_ASSERT_SIZE(RpPTankLockStruct, 8);
RpAtomic *RpPTankAtomicCreate(RwInt32 maxParticleNum, RwUInt32 dataFlags, RwUInt32 platFlags);
RwBool RpPTankAtomicDestroy(RpAtomic *atomic); // 0x57ba80 (returns a truth value in the game's 3.6 build)
void RpPTankAtomicUnlock(RpAtomic *atomic);  // 0x57bdd0
RwBool RpPTankAtomicLock(RpAtomic *atomic, RpPTankLockStruct *dst, RwUInt32 dataFlags, RpPTankLockFlags lockFlag);
void RpPTankAtomicSetBlendModes(RpAtomic *atomic, RwBlendFunction srcBlendMode, RwBlendFunction dstBlendMode);
void RpPTankAtomicSetTexture(RpAtomic *atomic, RwTexture *texture);
void RpPTankAtomicSetVertexAlpha(RpAtomic *atomic, RwBool vertexAlphaState);

// --- RpPVS: potentially visible set of a world
RpWorld *RpPVSHook(RpWorld *world);
RpWorld *RpPVSUnhook(RpWorld *world);
RwBool RpPVSQuery(RpWorld *world);
RpWorldSector *RpPVSSetViewPosition(RpWorld *world, RwV3d *position);

// --- RpRandom: RW's own pseudo-random generator
RwUInt32 RpRandom(void);
void RpRandomSeed(RwUInt32 seed);

// --- RpSpline
struct RpSpline;
RpSpline *RpSplineStreamRead(RwStream *stream);
RwBool RpSplineDestroy(RpSpline *spline);

// --- RpPatch
struct RpPatchMesh;
RpPatchMesh *RpPatchAtomicGetPatchMesh(const RpAtomic *atomic);

// --- RpLOD: atomic level of detail
void RpLODAtomicSetCamera(RwCamera *camera); // the camera whose distance selects the level of detail

}

#endif
