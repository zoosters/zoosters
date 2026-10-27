// RenderWare 3.6 core API the game uses: types, enums, vector helpers, the engine instance (RwEngineInstance, RwGlobals) with RW's
// macros as inline functions, and every core function the tree calls. A struct is defined only where the tree touches its members;
// other RW types are opaque. The vector helpers keep RW's operation order with explicit parentheses so cl 13.10 cannot regroup them.
// Other src/game headers still define RwV3d / RwMatrix / RwRGBA and RW's vector macros; the guards let both coexist.
#ifndef ZOOSTERS_RW_RWCORE_H
#define ZOOSTERS_RW_RWCORE_H

#include <stdarg.h>
#include <stddef.h>
#include "zoosters/base.h"

typedef float RwReal;
typedef int32_t RwInt32;
typedef uint32_t RwUInt32;
typedef int16_t RwInt16;
typedef uint16_t RwUInt16;
typedef uint8_t RwUInt8;
typedef char RwChar;
typedef int32_t RwBool; // a 4-byte truth value

// --- Value types
#ifndef ZOOSTERS_RWV2D_DEFINED
#define ZOOSTERS_RWV2D_DEFINED
struct RwV2d {
	RwReal x, y;
};
#endif

#ifndef ZOOSTERS_RWV3D_DEFINED
#define ZOOSTERS_RWV3D_DEFINED
struct RwV3d {
	RwReal x, y, z;
};
#endif

#ifndef ZOOSTERS_RWCOMBINE_DEFINED
#define ZOOSTERS_RWCOMBINE_DEFINED
// How a transform combines with a matrix or frame: replace it, apply before it, or apply after it.
enum RwOpCombineType { rwCOMBINEREPLACE = 0, rwCOMBINEPRECONCAT, rwCOMBINEPOSTCONCAT };
#endif

#ifndef ZOOSTERS_RWRGBA_DEFINED
#define ZOOSTERS_RWRGBA_DEFINED
// An 8-bit-per-channel colour.
struct RwRGBA {
	uint8_t red, green, blue, alpha;
};
#endif

#ifndef ZOOSTERS_RWRGBAREAL_DEFINED
#define ZOOSTERS_RWRGBAREAL_DEFINED
// A floating-point colour (0..1 per channel).
struct RwRGBAReal {
	RwReal red, green, blue, alpha;
};
#endif

#ifndef ZOOSTERS_RWMATRIX_DEFINED
#define ZOOSTERS_RWMATRIX_DEFINED
// A 4x3 transform: the rows right/up/at are the basis vectors, pos the translation (RW 3.6 layout, 0x40 bytes).
struct RwMatrix {
	RwV3d right;
	RwUInt32 flags;
	RwV3d up;
	RwUInt32 pad1;
	RwV3d at;
	RwUInt32 pad2;
	RwV3d pos;
	RwUInt32 pad3;
};
#endif

#ifndef ZOOSTERS_RWBBOX_DEFINED
#define ZOOSTERS_RWBBOX_DEFINED
// An axis-aligned box: sup is the corner with the largest coordinates, inf the smallest.
struct RwBBox {
	RwV3d sup;
	RwV3d inf;
};
#endif

#ifndef ZOOSTERS_RWLINE_DEFINED
#define ZOOSTERS_RWLINE_DEFINED
// A line segment from start to end.
struct RwLine {
	RwV3d start;
	RwV3d end;
};
#endif

#ifndef ZOOSTERS_RWLINKLIST_DEFINED
#define ZOOSTERS_RWLINKLIST_DEFINED
// RW's intrusive doubly linked list: a list is one sentinel link, an object embeds its link.
struct RwLLLink {
	RwLLLink *next;
	RwLLLink *prev;
};
struct RwLinkList {
	RwLLLink link;
};
#endif

#ifndef ZOOSTERS_RWFRAME_DEFINED
#define ZOOSTERS_RWFRAME_DEFINED
// A frame (a node of the transform hierarchy). PARTIAL: members up to the end of objectList (+0x98); never allocate or copy.
struct RwFrame {
	char object[4];           // RwObject header (type, subType, flags, privateFlags)
	void *parent;  // the object's parent (an RwObject carries it; for a frame, the parent frame)
	RwLLLink inDirtyListLink;
	RwMatrix modelling;
	RwMatrix ltm;  // the local-to-world matrix, valid after the frame was updated
	RwLinkList objectList;  // the objects attached to this frame
};
ZOOSTERS_ASSERT_OFFSET(RwFrame, parent, 0x04);
ZOOSTERS_ASSERT_OFFSET(RwFrame, inDirtyListLink, 0x08);
ZOOSTERS_ASSERT_OFFSET(RwFrame, modelling, 0x10);
ZOOSTERS_ASSERT_OFFSET(RwFrame, ltm, 0x50);
ZOOSTERS_ASSERT_OFFSET(RwFrame, objectList, 0x90);
#endif

// The views / bodies of RW objects (RwObjectView, RwCameraBody, ...): the accessors below use their offsets.
#include "rw/rwviewbase.h"

// --- Enums
// Projection of a camera.
enum RwCameraProjection { rwNACAMERAPROJECTION = 0, rwPERSPECTIVE = 1, rwPARALLEL = 2 };

// What RwCameraClear clears (flags, combinable).
enum RwCameraClearMode { rwCAMERACLEARIMAGE = 0x1, rwCAMERACLEARZ = 0x2, rwCAMERACLEARSTENCIL = 0x4 };

// Result of testing a bounding sphere against a camera frustum.
enum RwFrustumTestResult { rwSPHEREOUTSIDE = 0, rwSPHEREBOUNDARY = 1, rwSPHEREINSIDE = 2 };

// Primitive types of the immediate-mode 3D renderer.
enum RwPrimitiveType {
	rwPRIMTYPENAPRIMTYPE = 0,
	rwPRIMTYPELINELIST = 1,
	rwPRIMTYPEPOLYLINE = 2,
	rwPRIMTYPETRILIST = 3,
	rwPRIMTYPETRISTRIP = 4,
	rwPRIMTYPETRIFAN = 5,
	rwPRIMTYPEPOINTLIST = 6
};

// Where a stream reads from or writes to.
enum RwStreamType { rwNASTREAM = 0, rwSTREAMFILE, rwSTREAMFILENAME, rwSTREAMMEMORY, rwSTREAMCUSTOM };

// How a stream is opened.
enum RwStreamAccessType { rwNASTREAMACCESS = 0, rwSTREAMREAD, rwSTREAMWRITE, rwSTREAMAPPEND };

// What a raster is (RwRasterCreate's flags, low byte; the pixel format goes in the next byte).
enum RwRasterType { rwRASTERTYPENORMAL = 0x00, rwRASTERTYPEZBUFFER = 0x01, rwRASTERTYPECAMERA = 0x02, rwRASTERTYPETEXTURE = 0x04, rwRASTERTYPECAMERATEXTURE = 0x05 };
// The pixel format byte of RwRasterCreate's flags (the one the game uses).
enum RwRasterFormat { rwRASTERFORMAT8888 = 0x0500 };

// Lock modes of a raster (flags).
enum RwRasterLockMode { rwRASTERLOCKWRITE = 0x01, rwRASTERLOCKREAD = 0x02, rwRASTERLOCKNOFETCH = 0x04, rwRASTERLOCKRAW = 0x08 };

#ifndef ZOOSTERS_RWBLENDFUNCTION_DEFINED
#define ZOOSTERS_RWBLENDFUNCTION_DEFINED
// Blend factors of the alpha-blend render states.
enum RwBlendFunction {
	rwBLENDNABLEND = 0,
	rwBLENDZERO,
	rwBLENDONE,
	rwBLENDSRCCOLOR,
	rwBLENDINVSRCCOLOR,
	rwBLENDSRCALPHA,
	rwBLENDINVSRCALPHA,
	rwBLENDDESTALPHA,
	rwBLENDINVDESTALPHA,
	rwBLENDDESTCOLOR,
	rwBLENDINVDESTCOLOR,
	rwBLENDSRCALPHASAT
};
#endif

// Render states (RwRenderStateSet / Get) and their values, in RW's enumerator order.
enum RwRenderState {
	rwRENDERSTATENARENDERSTATE = 0,
	rwRENDERSTATETEXTURERASTER,
	rwRENDERSTATETEXTUREADDRESS,
	rwRENDERSTATETEXTUREADDRESSU,
	rwRENDERSTATETEXTUREADDRESSV,
	rwRENDERSTATETEXTUREPERSPECTIVE,
	rwRENDERSTATEZTESTENABLE,
	rwRENDERSTATESHADEMODE,
	rwRENDERSTATEZWRITEENABLE,
	rwRENDERSTATETEXTUREFILTER,
	rwRENDERSTATESRCBLEND,
	rwRENDERSTATEDESTBLEND,
	rwRENDERSTATEVERTEXALPHAENABLE,
	rwRENDERSTATEBORDERCOLOR,
	rwRENDERSTATEFOGENABLE,
	rwRENDERSTATEFOGCOLOR,
	rwRENDERSTATEFOGTYPE,
	rwRENDERSTATEFOGDENSITY,
	rwRENDERSTATECULLMODE = 20,
	rwRENDERSTATESTENCILENABLE,
	rwRENDERSTATESTENCILFAIL,
	rwRENDERSTATESTENCILZFAIL,
	rwRENDERSTATESTENCILPASS,
	rwRENDERSTATESTENCILFUNCTION,
	rwRENDERSTATESTENCILFUNCTIONREF,
	rwRENDERSTATESTENCILFUNCTIONMASK,
	rwRENDERSTATESTENCILFUNCTIONWRITEMASK,
	rwRENDERSTATEALPHATESTFUNCTION,
	rwRENDERSTATEALPHATESTFUNCTIONREF
};
// Values of the texture-address, shade-mode and fog-type render states (RW's enumerator order; the game passes 3 = clamp, 2 = Gouraud,
// 0 / 1 for the fog type).
enum RwTextureAddressMode {
	rwTEXTUREADDRESSNATEXTUREADDRESS = 0,
	rwTEXTUREADDRESSWRAP,
	rwTEXTUREADDRESSMIRROR,
	rwTEXTUREADDRESSCLAMP,
	rwTEXTUREADDRESSBORDER
};
enum RwShadeMode { rwSHADEMODENASHADEMODE = 0, rwSHADEMODEFLAT, rwSHADEMODEGOURAUD };
enum RwFogType { rwFOGTYPENAFOGTYPE = 0, rwFOGTYPELINEAR, rwFOGTYPEEXPONENTIAL, rwFOGTYPEEXPONENTIAL2 };
// Values of the stencil render states (RwStencilOperation for the fail / z-fail / pass states, RwStencilFunction for the function).
enum RwStencilOperation {
	rwSTENCILOPERATIONNASTENCILOPERATION = 0,
	rwSTENCILOPERATIONKEEP,
	rwSTENCILOPERATIONZERO,
	rwSTENCILOPERATIONREPLACE,
	rwSTENCILOPERATIONINCRSAT,
	rwSTENCILOPERATIONDECRSAT,
	rwSTENCILOPERATIONINVERT,
	rwSTENCILOPERATIONINCR,
	rwSTENCILOPERATIONDECR
};
enum RwStencilFunction {
	rwSTENCILFUNCTIONNASTENCILFUNCTION = 0,
	rwSTENCILFUNCTIONNEVER,
	rwSTENCILFUNCTIONLESS,
	rwSTENCILFUNCTIONEQUAL,
	rwSTENCILFUNCTIONLESSEQUAL,
	rwSTENCILFUNCTIONGREATER,
	rwSTENCILFUNCTIONNOTEQUAL,
	rwSTENCILFUNCTIONGREATEREQUAL,
	rwSTENCILFUNCTIONALWAYS
};
enum RwCullMode { rwCULLMODENACULLMODE = 0, rwCULLMODECULLNONE, rwCULLMODECULLBACK, rwCULLMODECULLFRONT };
enum RwTextureFilterMode {
	rwFILTERNAFILTERMODE = 0,
	rwFILTERNEAREST,
	rwFILTERLINEAR,
	rwFILTERMIPNEAREST,
	rwFILTERMIPLINEAR,
	rwFILTERLINEARMIPNEAREST,
	rwFILTERLINEARMIPLINEAR
};
enum RwAlphaTestFunction {
	rwALPHATESTFUNCTIONNAALPHATESTFUNCTION = 0,
	rwALPHATESTFUNCTIONNEVER,
	rwALPHATESTFUNCTIONLESS,
	rwALPHATESTFUNCTIONEQUAL,
	rwALPHATESTFUNCTIONLESSEQUAL,
	rwALPHATESTFUNCTIONGREATER,
	rwALPHATESTFUNCTIONNOTEQUAL,
	rwALPHATESTFUNCTIONGREATEREQUAL,
	rwALPHATESTFUNCTIONALWAYS
};

// --- Opaque types: the tree only passes pointers to these
struct RwCamera;
struct RwRaster;
struct RwTexture;
struct RwTexDictionary;
struct RwImage;
struct RwStream;
struct RwIm3DVertex;
struct RwRect;
struct RwFreeList;
struct RwResEntry;
struct RwSubSystemInfo;
struct RwVideoMode;
struct RwMemoryFunctions;  // defined with the engine instance at the end of this file (its memory function table)
struct RwEngineOpenParams;
struct RwChunkHeaderInfo;
struct RwMatrixTolerance;
struct RwSphere {     // a sphere (the bounding sphere of an atomic / clump, a sphere query)
	RwV3d center;
	RwReal radius;
};

// --- Stream records (RwChunkHeaderInfo, RwStream): rw/rwstream.h
#include "rw/rwstream.h"
extern "C" {

// --- Callbacks
typedef RwFrame *(*RwFrameCallBack)(RwFrame *frame, void *data);
typedef RwImage *(*RwImageCallBackRead)(const RwChar *imageName);
typedef RwImage *(*RwImageCallBackWrite)(RwImage *image, const RwChar *imageName);
// Plugin hooks: attach per-object data to an RW object type (constructor, destructor, copy, stream read/write/size).
typedef void *(*RwPluginObjectConstructor)(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject);
typedef void *(*RwPluginObjectDestructor)(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject);
typedef void *(*RwPluginObjectCopy)(void *dstObject, const void *srcObject, RwInt32 offsetInObject, RwInt32 sizeInObject);
typedef RwStream *(*RwPluginDataChunkReadCallBack)(RwStream *stream, RwInt32 binaryLength, void *object, RwInt32 offsetInObject,
                                                   RwInt32 sizeInObject);
typedef RwStream *(*RwPluginDataChunkWriteCallBack)(RwStream *stream, RwInt32 binaryLength, const void *object,
                                                    RwInt32 offsetInObject, RwInt32 sizeInObject);
typedef RwInt32 (*RwPluginDataChunkGetSizeCallBack)(const void *object, RwInt32 offsetInObject, RwInt32 sizeInObject);
typedef RwBool (*RwPluginDataChunkAlwaysCallBack)(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject);
typedef RwBool (*RwPluginDataChunkRightsCallBack)(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject, RwUInt32 extraData);

// --- Engine: start-up, shutdown, display sub-systems and video modes
RwBool RwEngineInit(const RwMemoryFunctions *memFuncs, RwUInt32 initFlags, RwUInt32 resArenaSize);
RwBool RwEngineOpen(RwEngineOpenParams *initParams);
RwBool RwEngineStart(void);
RwBool RwEngineStop(void);
RwBool RwEngineClose(void);
RwBool RwEngineTerm(void);
RwInt32 RwEngineGetNumSubSystems(void);
RwSubSystemInfo *RwEngineGetSubSystemInfo(RwSubSystemInfo *subSystemInfo, RwInt32 subSystemIndex);
RwBool RwEngineSetSubSystem(RwInt32 subSystemIndex);
RwInt32 RwEngineGetNumVideoModes(void);
RwInt32 RwEngineGetCurrentVideoMode(void);
RwVideoMode *RwEngineGetVideoModeInfo(RwVideoMode *modeInfo, RwInt32 modeIndex);
RwBool RwEngineSetVideoMode(RwInt32 modeIndex);
// Reserves `size` bytes in every engine instance for a plugin; returns the offset, or -1.
RwInt32 RwEngineRegisterPlugin(RwInt32 size, RwUInt32 pluginId, RwPluginObjectConstructor initCallBack,
                               RwPluginObjectDestructor termCallBack);

// --- Free lists and engine resources
RwFreeList *RwFreeListCreate(RwInt32 entrySize, RwInt32 entriesPerBlock, RwInt32 alignment, RwUInt32 hint);
RwBool RwFreeListDestroy(RwFreeList *freeList);
RwInt32 RwFreeListPurgeAllFreeLists(void);
RwBool RwResourcesFreeResEntry(RwResEntry *entry);

// --- Streams
RwStream *RwStreamOpen(RwStreamType type, RwStreamAccessType accessType, const void *data);
RwBool RwStreamClose(RwStream *stream, void *data);
RwUInt32 RwStreamRead(RwStream *stream, void *buffer, RwUInt32 length);
RwStream *RwStreamSkip(RwStream *stream, RwUInt32 offset);
RwStream *RwStreamReadInt32(RwStream *stream, RwInt32 *ints, RwUInt32 numBytes);
RwStream *RwStreamReadReal(RwStream *stream, RwReal *reals, RwUInt32 numBytes);
RwBool RwStreamFindChunk(RwStream *stream, RwUInt32 type, RwUInt32 *lengthOut, RwUInt32 *versionOut);
RwStream *RwStreamReadChunkHeaderInfo(RwStream *stream, RwChunkHeaderInfo *chunkHeaderInfo);

// --- Vectors (the arithmetic helpers further down are macros in RW's release build)
RwReal RwV3dLength(const RwV3d *vector);
RwReal RwV3dNormalize(RwV3d *out, const RwV3d *in); // returns the length of `in`
RwV3d *RwV3dTransformPoint(RwV3d *pointOut, const RwV3d *pointIn, const RwMatrix *matrix);
RwV3d *RwV3dTransformPoints(RwV3d *pointsOut, const RwV3d *pointsIn, RwInt32 numPoints, const RwMatrix *matrix);
RwV3d *RwV3dTransformVectors(RwV3d *vectorsOut, const RwV3d *vectorsIn, RwInt32 numPoints, const RwMatrix *matrix);
RwReal _rwSqrt(RwReal value);
RwReal _rwInvSqrt(RwReal value);

// --- Matrices
RwMatrix *RwMatrixCreate(void);
RwBool RwMatrixDestroy(RwMatrix *matrix);
RwMatrix *RwMatrixUpdate(RwMatrix *matrix);
RwMatrix *RwMatrixOptimize(RwMatrix *matrix, const RwMatrixTolerance *tolerance);
RwMatrix *RwMatrixInvert(RwMatrix *matrixOut, const RwMatrix *matrixIn);
RwMatrix *RwMatrixMultiply(RwMatrix *matrixOut, const RwMatrix *matrixIn1, const RwMatrix *matrixIn2);
RwMatrix *RwMatrixTransform(RwMatrix *matrix, const RwMatrix *transform, RwOpCombineType combineOp);
RwMatrix *RwMatrixTranslate(RwMatrix *matrix, const RwV3d *translation, RwOpCombineType combineOp);
RwMatrix *RwMatrixRotate(RwMatrix *matrix, const RwV3d *axis, RwReal angle, RwOpCombineType combineOp);
RwMatrix *RwMatrixScale(RwMatrix *matrix, const RwV3d *scale, RwOpCombineType combineOp);
const RwMatrix *RwMatrixQueryRotate(const RwMatrix *matrix, RwV3d *unitAxis, RwReal *angle, RwV3d *center);

// --- Frames
RwFrame *RwFrameCreate(void);
RwBool RwFrameDestroy(RwFrame *frame);
RwFrame *RwFrameForAllChildren(RwFrame *frame, RwFrameCallBack callBack, void *data);
RwFrame *RwFrameRemoveChild(RwFrame *child); // detaches `child` from its parent
RwMatrix *RwFrameGetLTM(RwFrame *frame);     // brings the frame's ltm up to date first
// The frame a camera hangs from (RwCameraGetFrame: the parent of its object header, +4). A macro as in the RW API: the game's code
// compiles differently with an inline function here (ShadowCameraRenderTree 0x443680 changes register allocation).
#ifndef RwCameraGetFrame
#define RwCameraGetFrame(_c) (*(RwFrame **)((char *)(_c) + rwOff_RwObjectView_parent))
#endif
#ifndef RwCameraGetProjection
// The projection of a camera (RwCameraGetProjection: a macro on the camera's projectionType member, +0x14 in the game's RW 3.6.0.2).
static inline RwCameraProjection RwCameraGetProjection(const RwCamera *camera)
{
	return *(const RwCameraProjection *)((const char *)camera + rwOff_RwCameraBody_projectionType);
}
#endif
// Macros of the RW API as inline accessors, at the offsets of the game's RW 3.6.0.2 (CameraHelper::Destroy 0x44e850,
// FrameHelper 0x44f000 / 0x44f030).
static inline RwFrame *RwFrameGetParent(const RwFrame *frame)
{
	return (RwFrame *)frame->parent;
}
static inline RwMatrix *RwFrameGetMatrix(RwFrame *frame)
{
	return &frame->modelling;
}
static inline void RwMatrixCopy(RwMatrix *dst, const RwMatrix *src)
{
	*dst = *src;
}
// The raster a camera renders into (+0x60) and its depth raster (+0x64).
// normal build: the shim's functions read the real object (the +0x60 offset is the 32-bit layout)
extern "C" RwRaster *RwCameraGetRaster(const RwCamera *camera);
extern "C" RwRaster *RwCameraGetZRaster(const RwCamera *camera);
extern "C" RwCamera *RwCameraSetRaster(RwCamera *camera, RwRaster *raster);    // the shim returns the camera (RW API)
extern "C" RwCamera *RwCameraSetZRaster(RwCamera *camera, RwRaster *raster);
extern "C" RwInt32 RwRasterGetWidth(const RwRaster *raster);
extern "C" RwInt32 RwRasterGetHeight(const RwRaster *raster);
extern "C" RwInt32 RwRasterGetDepth(const RwRaster *raster);
// The view window of a camera (RwV2d at +0x68; the game's code reads x).
static inline const RwV2d *RwCameraGetViewWindow(const RwCamera *camera)
{
	return (const RwV2d *)((const char *)camera + rwOff_RwCameraBody_viewWindow);
}
// Width (+0x0c) and height (+0x10) of a raster.
// Bits per pixel of a raster (+0x14).
// The raster of a texture (+0).
static inline RwRaster *RwTextureGetRaster(const RwTexture *texture)
{
	return *(RwRaster *const *)texture;
}
RwFrame *RwFrameUpdateObjects(RwFrame *frame);
RwFrame *RwFrameSetIdentity(RwFrame *frame);
RwFrame *RwFrameOrthoNormalize(RwFrame *frame);
RwFrame *RwFrameTransform(RwFrame *frame, const RwMatrix *matrix, RwOpCombineType combineOp);
RwFrame *RwFrameTranslate(RwFrame *frame, const RwV3d *translation, RwOpCombineType combineOp);
RwFrame *RwFrameRotate(RwFrame *frame, const RwV3d *axis, RwReal angle, RwOpCombineType combineOp);
RwFrame *RwFrameScale(RwFrame *frame, const RwV3d *scale, RwOpCombineType combineOp);
RwBool _rwFrameSyncDirty(void);
// Links an RwObjectHasFrame object (camera, light, atomic) to a frame: RwCameraSetFrame / RpLightSetFrame are macros on it.
void _rwObjectHasFrameSetFrame(void *object, RwFrame *frame);
static inline void RwCameraSetFrame(RwCamera *camera, RwFrame *frame)
{
	_rwObjectHasFrameSetFrame(camera, frame);
}

// --- Cameras (RwCameraGetWorld is declared in rw/rpworld.h with the world)
RwCamera *RwCameraCreate(void);
RwCamera *RwCameraClone(RwCamera *camera);
RwBool RwCameraDestroy(RwCamera *camera);
RwCamera *RwCameraBeginUpdate(RwCamera *camera);
RwCamera *RwCameraEndUpdate(RwCamera *camera);
RwCamera *RwCameraClear(RwCamera *camera, RwRGBA *colour, RwInt32 clearMode); // clearMode: RwCameraClearMode flags
// RwCameraShowRaster flags (RwRasterFlipMode)
enum RwRasterFlipMode {
	rwRASTERFLIPDONTWAIT = 0,
	rwRASTERFLIPWAITVSYNC = 1
};
RwCamera *RwCameraShowRaster(RwCamera *camera, void *device, RwUInt32 flags);
RwCamera *RwCameraSetNearClipPlane(RwCamera *camera, RwReal nearClip);
RwCamera *RwCameraSetFarClipPlane(RwCamera *camera, RwReal farClip);
RwCamera *RwCameraSetProjection(RwCamera *camera, RwCameraProjection projection);
RwCamera *RwCameraSetViewWindow(RwCamera *camera, const RwV2d *viewWindow);
RwFrustumTestResult RwCameraFrustumTestSphere(const RwCamera *camera, const RwSphere *sphere);

// --- Rasters, images, textures
RwRaster *RwRasterCreate(RwInt32 width, RwInt32 height, RwInt32 depth, RwInt32 flags);
RwBool RwRasterDestroy(RwRaster *raster);
RwRaster *RwRasterSubRaster(RwRaster *subRaster, RwRaster *raster, RwRect *rect);
RwUInt8 *RwRasterLock(RwRaster *raster, RwUInt8 level, RwInt32 lockMode); // lockMode: RwRasterLockMode flags
RwRaster *RwRasterUnlock(RwRaster *raster);
RwRaster *RwRasterPushContext(RwRaster *raster);
RwRaster *RwRasterPopContext(void);
RwBool RwRasterClear(RwInt32 pixelValue);
RwRaster *RwRasterRenderFast(RwRaster *raster, RwInt32 x, RwInt32 y);
RwRaster *RwRasterRenderScaled(RwRaster *raster, RwRect *rect);
RwRaster *RwRasterSetFromImage(RwRaster *raster, RwImage *image);
RwUInt32 RwRGBAToPixel(RwRGBA *colour, RwInt32 rasterFormat);
RwImage *RwImageCreate(RwInt32 width, RwInt32 height, RwInt32 depth);
RwBool RwImageDestroy(RwImage *image);
RwImage *RwImageAllocatePixels(RwImage *image);
RwImage *RwImageSetFromRaster(RwImage *image, RwRaster *raster);
RwImage *RwImageRead(const RwChar *imageName);
RwImage *RwImageWrite(RwImage *image, const RwChar *imageName);
const RwChar *RwImageSetPath(const RwChar *path);
RwBool RwImageRegisterImageFormat(const RwChar *extension, RwImageCallBackRead imageRead, RwImageCallBackWrite imageWrite);
#ifndef RwTextureGetName
// The name of a texture (a 32-byte string at +0x10).
static inline RwChar *RwTextureGetName(RwTexture *texture)
{
	return (RwChar *)texture + rwOff_RwTextureBody_name;
}
#endif
RwTexture *RwTextureCreate(RwRaster *raster);
RwBool RwTextureDestroy(RwTexture *texture);
RwTexture *RwTextureRead(const RwChar *name, const RwChar *maskName);
RwTexture *RwTextureSetRaster(RwTexture *texture, RwRaster *raster);
RwUInt32 RwTextureStreamGetSize(const RwTexture *texture);
RwTexture *RwTextureStreamRead(RwStream *stream);
RwBool RwTexDictionaryDestroy(RwTexDictionary *dict);
RwTexDictionary *RwTexDictionaryGetCurrent(void);
RwTexDictionary *RwTexDictionarySetCurrent(RwTexDictionary *dict);
RwTexDictionary *RwTexDictionaryStreamRead(RwStream *stream);

// --- Immediate-mode 3D
void *RwIm3DTransform(RwIm3DVertex *vertices, RwUInt32 numVertices, RwMatrix *ltm, RwUInt32 flags);
RwBool RwIm3DRenderPrimitive(RwPrimitiveType primType);
RwBool RwIm3DEnd(void);

#ifndef RwV3dAdd // a header may still define the RW macro form: same operation
// out = a + b
static inline void RwV3dAdd(RwV3d *out, const RwV3d *a, const RwV3d *b)
{
	out->x = a->x + b->x;
	out->y = a->y + b->y;
	out->z = a->z + b->z;
}
#endif

#ifndef RwV3dSub // a header may still define the RW macro form: same operation
// out = a - b
static inline void RwV3dSub(RwV3d *out, const RwV3d *a, const RwV3d *b)
{
	out->x = a->x - b->x;
	out->y = a->y - b->y;
	out->z = a->z - b->z;
}
#endif

#ifndef RwV3dScale // a header may still define the RW macro form: same operation
// out = a * s
static inline void RwV3dScale(RwV3d *out, const RwV3d *a, RwReal s)
{
	out->x = a->x * s;
	out->y = a->y * s;
	out->z = a->z * s;
}
#endif

#ifndef RwV3dIncrementScaled // a header may still define the RW macro form: same operation
// out += a * s
static inline void RwV3dIncrementScaled(RwV3d *out, const RwV3d *a, RwReal s)
{
	out->x += a->x * s;
	out->y += a->y * s;
	out->z += a->z * s;
}
#endif

#ifndef RwV3dNegate // a header may still define the RW macro form: same operation
// out = -a
static inline void RwV3dNegate(RwV3d *out, const RwV3d *a)
{
	out->x = -a->x;
	out->y = -a->y;
	out->z = -a->z;
}
#endif

#ifndef RwV3dDotProduct // a header may still define the RW macro form: same operation
// a . b, summed x, y, then z
static inline RwReal RwV3dDotProduct(const RwV3d *a, const RwV3d *b)
{
	return (a->x * b->x + a->y * b->y) + a->z * b->z;
}
#endif

#ifndef RwV3dCrossProduct // a header may still define the RW macro form: same operation
// out = a x b (out must not be a or b)
static inline void RwV3dCrossProduct(RwV3d *out, const RwV3d *a, const RwV3d *b)
{
	out->x = a->y * b->z - a->z * b->y;
	out->y = a->z * b->x - a->x * b->z;
	out->z = a->x * b->y - a->y * b->x;
}
#endif

// ---- Engine instance (RwEngineInstance 0x62ac18, RW's RwGlobals) and the public macros that read it, as inline functions
// RW's release build uses macros on RWSRCGLOBAL(...); the inline functions compile to the same calls (arguments are evaluated
// before the instance is loaded, the macro loaded it first).
// Layout is RW 3.7's RwGlobals (plcore/balibtyp.h), which matches the game's 3.6.0.2 for every member used:
// device +0x10 (zBufferNear +0x18, fpRenderStateSet +0x20, fpRenderStateGet +0x24, fpIm2DRenderPrimitive +0x30),
// stringFuncs (rwstrcat +0xd4, rwstrcmp +0xe8, rwstrncmp +0xec), memoryFuncs +0x108 (rwmalloc, rwfree +0x10c, rwrealloc +0x110,
// rwcalloc +0x114), memoryAlloc / memoryFree +0x118 / +0x11c, renderFrame +0x08.
// Members past resArenaInitSize (debug builds only) do not exist in the game. Plugin data sits at offsets into the instance
// (RwEngineRegisterPlugin's result): RwEngineGetPluginData(offset).

// One vertex of the D3D8 Im2D pipeline (screen space).
struct RwIm2DVertex {
	RwReal x, y, z;  // screen position and depth (RwIm2DGetNearScreenZ .. FarScreenZ)
	RwReal rhw;  // reciprocal of the view-space depth (1 / the camera's near plane for an overlay)
	RwUInt32 emissiveColor;  // ARGB
	RwReal u, v;
};
ZOOSTERS_ASSERT_SIZE(RwIm2DVertex, 0x1c);
typedef RwUInt16 RwImVertexIndex;

// One vertex of the Im3D pipeline (RW 3.7 d3d8 layout: RxObjSpace3DLitVertex): object-space position and normal, colour, texture
// coordinates (RW's member names). The tree writes position, colour and u / v (ShadowDrawHead 0x4432c0).
struct RwIm3DVertex {
	RwV3d objVertex;
	RwV3d objNormal;
	RwUInt32 color;  // ARGB
	RwReal u, v;
};
ZOOSTERS_ASSERT_SIZE(RwIm3DVertex, 0x24);
// Which members of the source vertices RwIm3DTransform uses (flags, combinable).
enum RwIm3DTransformFlags {
	rwIM3D_VERTEXUV = 0x01,
	rwIM3D_ALLOPAQUE = 0x02,
	rwIM3D_NOCLIP = 0x04,
	rwIM3D_VERTEXXYZ = 0x08,
	rwIM3D_VERTEXRGBA = 0x10
};

// A rectangle of a raster (RwRasterSubRaster: position and size).
struct RwRect {
	RwInt32 x, y, w, h;
};
ZOOSTERS_ASSERT_SIZE(RwRect, 0x10);

typedef RwBool (*RwSystemFunc)(RwInt32 option, void *out, void *inOut, RwInt32 nIn);
typedef RwBool (*RwRenderStateSetFunction)(RwRenderState state, void *value);
typedef RwBool (*RwRenderStateGetFunction)(RwRenderState state, void *value);
typedef RwBool (*RwIm2DRenderLineFunction)(RwIm2DVertex *vertices, RwInt32 numVertices, RwInt32 vert1, RwInt32 vert2);
typedef RwBool (*RwIm2DRenderTriangleFunction)(RwIm2DVertex *vertices, RwInt32 numVertices, RwInt32 vert1, RwInt32 vert2,
                                               RwInt32 vert3);
typedef RwBool (*RwIm2DRenderPrimitiveFunction)(RwPrimitiveType primType, RwIm2DVertex *vertices, RwInt32 numVertices);
typedef RwBool (*RwIm2DRenderIndexedPrimitiveFunction)(RwPrimitiveType primType, RwIm2DVertex *vertices, RwInt32 numVertices,
                                                       RwImVertexIndex *indices, RwInt32 numIndices);
typedef RwBool (*RwIm3DRenderLineFunction)(RwInt32 vert1, RwInt32 vert2);
typedef RwBool (*RwIm3DRenderTriangleFunction)(RwInt32 vert1, RwInt32 vert2, RwInt32 vert3);
typedef RwBool (*RwIm3DRenderPrimitiveFunction)(RwPrimitiveType primType);
typedef RwBool (*RwIm3DRenderIndexedPrimitiveFunction)(RwPrimitiveType primType, RwImVertexIndex *indices, RwInt32 numIndices);

// The open device's entry points (RwGlobals::dOpenDevice).
struct RwDevice {
	RwReal gammaCorrection;
	RwSystemFunc fpSystem;
	RwReal zBufferNear;
	RwReal zBufferFar;
	RwRenderStateSetFunction fpRenderStateSet;
	RwRenderStateGetFunction fpRenderStateGet;
	RwIm2DRenderLineFunction fpIm2DRenderLine;
	RwIm2DRenderTriangleFunction fpIm2DRenderTriangle;
	RwIm2DRenderPrimitiveFunction fpIm2DRenderPrimitive;
	RwIm2DRenderIndexedPrimitiveFunction fpIm2DRenderIndexedPrimitive;
	RwIm3DRenderLineFunction fpIm3DRenderLine;
	RwIm3DRenderTriangleFunction fpIm3DRenderTriangle;
	RwIm3DRenderPrimitiveFunction fpIm3DRenderPrimitive;
	RwIm3DRenderIndexedPrimitiveFunction fpIm3DRenderIndexedPrimitive;
};
ZOOSTERS_ASSERT_SIZE(RwDevice, 0x38);

// The string functions (RwGlobals::stringFuncs; RW's rwstrxxx macros call through them).
struct RwStringFunctions {
	int (*vecSprintf)(RwChar *buffer, const RwChar *format, ...);
	int (*vecVsprintf)(RwChar *buffer, const RwChar *format, va_list argptr);
	RwChar *(*vecStrcpy)(RwChar *dst, const RwChar *src);
	RwChar *(*vecStrncpy)(RwChar *dst, const RwChar *src, size_t n);
	RwChar *(*vecStrcat)(RwChar *dst, const RwChar *src);
	RwChar *(*vecStrncat)(RwChar *dst, const RwChar *src, size_t n);
	RwChar *(*vecStrrchr)(const RwChar *string, RwInt32 findThis);
	RwChar *(*vecStrchr)(const RwChar *string, RwInt32 findThis);
	RwChar *(*vecStrstr)(const RwChar *string, const RwChar *findThis);
	int (*vecStrcmp)(const RwChar *string1, const RwChar *string2);
	int (*vecStrncmp)(const RwChar *string1, const RwChar *string2, size_t n);
	int (*vecStricmp)(const RwChar *string1, const RwChar *string2);
	size_t (*vecStrlen)(const RwChar *string);
	RwChar *(*vecStrupr)(RwChar *string);
	RwChar *(*vecStrlwr)(RwChar *string);
	RwChar *(*vecStrtok)(RwChar *string, const RwChar *delimit);
	int (*vecSscanf)(const RwChar *buffer, const RwChar *format, ...);
};
ZOOSTERS_ASSERT_SIZE(RwStringFunctions, 0x44);

// The memory functions (RwGlobals::memoryFuncs; also the argument of RwEngineInit).
struct RwMemoryFunctions {
	void *(*rwmalloc)(size_t size, RwUInt32 hint);
	void (*rwfree)(void *mem);
	void *(*rwrealloc)(void *mem, size_t newSize, RwUInt32 hint);
	void *(*rwcalloc)(size_t numObj, size_t sizeObj, RwUInt32 hint);
};

typedef RwBool (*RwStandardFunc)(void *pOut, void *pInOut, RwInt32 nI);
typedef void *(*RwMemoryAllocFn)(RwFreeList *fl, RwUInt32 hint);
typedef RwFreeList *(*RwMemoryFreeFn)(RwFreeList *fl, void *pData);
struct RwMetrics;

struct RwGlobals {
	void *curCamera;  // the camera between RwCameraBeginUpdate and EndUpdate
	void *curWorld;
	RwUInt16 renderFrame;  // camera display count
	RwUInt16 lightFrame;
	RwUInt16 pad[2];
	RwDevice dOpenDevice;
	RwStandardFunc stdFunc[29];  // rwSTANDARDNUMOFSTANDARD
	RwLinkList dirtyFrameList;
	RwStringFunctions stringFuncs;
	RwMemoryFunctions memoryFuncs;
	RwMemoryAllocFn memoryAlloc;  // RwFreeListAlloc
	RwMemoryFreeFn memoryFree;  // RwFreeListFree
	RwMetrics *metrics;
	RwInt32 engineStatus;  // RwEngineStatus
	RwUInt32 resArenaInitSize;
};
ZOOSTERS_ASSERT_OFFSET(RwGlobals, dOpenDevice, 0x10);
ZOOSTERS_ASSERT_OFFSET(RwGlobals, stdFunc, 0x48);
ZOOSTERS_ASSERT_OFFSET(RwGlobals, dirtyFrameList, 0xbc);
ZOOSTERS_ASSERT_OFFSET(RwGlobals, stringFuncs, 0xc4);
ZOOSTERS_ASSERT_OFFSET(RwGlobals, memoryFuncs, 0x108);
ZOOSTERS_ASSERT_OFFSET(RwGlobals, memoryAlloc, 0x118);
ZOOSTERS_ASSERT_SIZE(RwGlobals, 0x12c);

// GLOBAL: 0x62ac18 RwEngineInstance
extern void *RwEngineInstance;          // RW declares it untyped; RwEngineGlobals gives the typed view

static inline RwGlobals *RwEngineGlobals()
{
	return (RwGlobals *)RwEngineInstance;
}

// A plugin's data in the engine instance (offset = RwEngineRegisterPlugin's result).
static inline void *RwEngineGetPluginData(RwInt32 offset)
{
	return (char *)RwEngineInstance + offset;
}

// ---- RW's public macros on the instance, as inline functions (RW's names; the #ifndef lets a not yet converted file keep its macro)
#ifndef RwCameraGetCurrentCamera
static inline RwCamera *RwCameraGetCurrentCamera()
{
	return (RwCamera *)RwEngineGlobals()->curCamera;
}
#endif
#ifndef RwRenderStateSet
static inline RwBool RwRenderStateSet(RwRenderState state, void *value)
{
	return RwEngineGlobals()->dOpenDevice.fpRenderStateSet(state, value);
}
#endif
#ifndef RwRenderStateGet
static inline RwBool RwRenderStateGet(RwRenderState state, void *value)
{
	return RwEngineGlobals()->dOpenDevice.fpRenderStateGet(state, value);
}
#endif
#ifndef RwIm2DGetNearScreenZ
static inline RwReal RwIm2DGetNearScreenZ()
{
	return RwEngineGlobals()->dOpenDevice.zBufferNear;
}
#endif
#ifndef RwIm2DGetFarScreenZ
static inline RwReal RwIm2DGetFarScreenZ()
{
	return RwEngineGlobals()->dOpenDevice.zBufferFar;
}
#endif
#ifndef RwIm2DRenderLine
static inline RwBool RwIm2DRenderLine(RwIm2DVertex *vertices, RwInt32 numVertices, RwInt32 vert1, RwInt32 vert2)
{
	return RwEngineGlobals()->dOpenDevice.fpIm2DRenderLine(vertices, numVertices, vert1, vert2);
}
#endif
#ifndef RwIm2DRenderTriangle
static inline RwBool RwIm2DRenderTriangle(RwIm2DVertex *vertices, RwInt32 numVertices, RwInt32 vert1, RwInt32 vert2, RwInt32 vert3)
{
	return RwEngineGlobals()->dOpenDevice.fpIm2DRenderTriangle(vertices, numVertices, vert1, vert2, vert3);
}
#endif
#ifndef RwIm2DRenderPrimitive
static inline RwBool RwIm2DRenderPrimitive(RwPrimitiveType primType, RwIm2DVertex *vertices, RwInt32 numVertices)
{
	return RwEngineGlobals()->dOpenDevice.fpIm2DRenderPrimitive(primType, vertices, numVertices);
}
#endif
#ifndef RwIm2DRenderIndexedPrimitive
static inline RwBool RwIm2DRenderIndexedPrimitive(RwPrimitiveType primType, RwIm2DVertex *vertices, RwInt32 numVertices,
                                                 RwImVertexIndex *indices, RwInt32 numIndices)
{
	return RwEngineGlobals()->dOpenDevice.fpIm2DRenderIndexedPrimitive(primType, vertices, numVertices, indices, numIndices);
}
#endif
#ifndef RwMalloc
static inline void *RwMalloc(size_t size, RwUInt32 hint)
{
	return RwEngineGlobals()->memoryFuncs.rwmalloc(size, hint);
}
#endif
#ifndef RwFree
static inline void RwFree(void *mem)
{
	RwEngineGlobals()->memoryFuncs.rwfree(mem);
}
#endif
#ifndef RwRealloc
static inline void *RwRealloc(void *mem, size_t newSize, RwUInt32 hint)
{
	return RwEngineGlobals()->memoryFuncs.rwrealloc(mem, newSize, hint);
}
#endif
#ifndef RwCalloc
static inline void *RwCalloc(size_t numObj, size_t sizeObj, RwUInt32 hint)
{
	return RwEngineGlobals()->memoryFuncs.rwcalloc(numObj, sizeObj, hint);
}
#endif
#ifndef RwFreeListAlloc
static inline void *RwFreeListAlloc(RwFreeList *freeList, RwUInt32 hint)
{
	return RwEngineGlobals()->memoryAlloc(freeList, hint);
}
#endif
#ifndef RwFreeListFree
static inline RwFreeList *RwFreeListFree(RwFreeList *freeList, void *data)
{
	return RwEngineGlobals()->memoryFree(freeList, data);
}
#endif
#ifndef rwstrcpy
static inline RwChar *rwstrcpy(RwChar *dst, const RwChar *src)
{
	return RwEngineGlobals()->stringFuncs.vecStrcpy(dst, src);
}
#endif
#ifndef rwstrcat
static inline RwChar *rwstrcat(RwChar *dst, const RwChar *src)
{
	return RwEngineGlobals()->stringFuncs.vecStrcat(dst, src);
}
#endif
#ifndef rwstrcmp
static inline int rwstrcmp(const RwChar *string1, const RwChar *string2)
{
	return RwEngineGlobals()->stringFuncs.vecStrcmp(string1, string2);
}
#endif
#ifndef rwstrncmp
static inline int rwstrncmp(const RwChar *string1, const RwChar *string2, size_t n)
{
	return RwEngineGlobals()->stringFuncs.vecStrncmp(string1, string2, n);
}
#endif
#ifndef rwstricmp
static inline int rwstricmp(const RwChar *string1, const RwChar *string2)
{
	return RwEngineGlobals()->stringFuncs.vecStricmp(string1, string2);
}
#endif
#ifndef rwstrlen
static inline size_t rwstrlen(const RwChar *string)
{
	return RwEngineGlobals()->stringFuncs.vecStrlen(string);
}
#endif


}

#endif
