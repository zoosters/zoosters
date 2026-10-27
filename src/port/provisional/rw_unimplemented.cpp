// GENERATED; stubs for: AtomicDefaultRenderCallBack RpAtomicSetStreamAlwaysCallBack RpAtomicSetStreamRightsCallBack
// RpAtomicStreamRead RpDMorphAnimationDestroy RpDMorphAnimationGetNumDMorphTargets RpDMorphAnimationStreamRead
// RpDMorphAtomicAddTime RpDMorphAtomicSetAnimation RpDMorphGeometryGetDMorphTarget RpDMorphGeometryGetNumDMorphTargets
// RpPatchAtomicGetPatchMesh RpSplineDestroy RpSplineStreamRead RtPITexDictionaryStreamRead RwFclose RwFopen RwFseek RwFtell
// RwMatrixQueryRotate RwRGBAToPixel RwResourcesFreeResEntry RwTextureStreamGetSize -- do not edit RenderWare functions src/game
// calls but no shim implements yet (each is a work item for port/shim/): a call logs the name and aborts.
#include <stdio.h>
#include <stdlib.h>
#include "rw/rpplugins.h"
#include "rw/rpworld.h"
#include "rw/rttoolkits.h"
#include "rw/rwcore.h"

#define RW_UNIMPLEMENTED(n) do { fprintf(stderr, "port: unimplemented RenderWare function %s\n", n); abort(); } while (0)

extern "C" {

RpAtomic * AtomicDefaultRenderCallBack(RpAtomic *atomic) { RW_UNIMPLEMENTED("AtomicDefaultRenderCallBack"); return (RpAtomic *)0; }
RwInt32 RpAtomicSetStreamAlwaysCallBack(RwUInt32 pluginId, RwPluginDataChunkAlwaysCallBack alwaysCallBack) { RW_UNIMPLEMENTED("RpAtomicSetStreamAlwaysCallBack"); return (RwInt32)0; }
RwInt32 RpAtomicSetStreamRightsCallBack(RwUInt32 pluginId, RwPluginDataChunkRightsCallBack rightsCallBack) { RW_UNIMPLEMENTED("RpAtomicSetStreamRightsCallBack"); return (RwInt32)0; }
RpAtomic * RpAtomicStreamRead(RwStream *stream) { RW_UNIMPLEMENTED("RpAtomicStreamRead"); return (RpAtomic *)0; }
RpDMorphAnimation * RpDMorphAnimationDestroy(RpDMorphAnimation *animation) { RW_UNIMPLEMENTED("RpDMorphAnimationDestroy"); return (RpDMorphAnimation *)0; }
RwUInt32 RpDMorphAnimationGetNumDMorphTargets(RpDMorphAnimation *animation) { RW_UNIMPLEMENTED("RpDMorphAnimationGetNumDMorphTargets"); return (RwUInt32)0; }
RpDMorphAnimation * RpDMorphAnimationStreamRead(RwStream *stream) { RW_UNIMPLEMENTED("RpDMorphAnimationStreamRead"); return (RpDMorphAnimation *)0; }
RpAtomic * RpDMorphAtomicAddTime(RpAtomic *atomic, RwReal time) { RW_UNIMPLEMENTED("RpDMorphAtomicAddTime"); return (RpAtomic *)0; }
RpAtomic * RpDMorphAtomicSetAnimation(RpAtomic *atomic, RpDMorphAnimation *animation) { RW_UNIMPLEMENTED("RpDMorphAtomicSetAnimation"); return (RpAtomic *)0; }
RpDMorphTarget * RpDMorphGeometryGetDMorphTarget(const RpGeometry *geometry, RwUInt32 index) { RW_UNIMPLEMENTED("RpDMorphGeometryGetDMorphTarget"); return (RpDMorphTarget *)0; }
RwUInt32 RpDMorphGeometryGetNumDMorphTargets(const RpGeometry *geometry) { RW_UNIMPLEMENTED("RpDMorphGeometryGetNumDMorphTargets"); return (RwUInt32)0; }
RpPatchMesh * RpPatchAtomicGetPatchMesh(const RpAtomic *atomic) { RW_UNIMPLEMENTED("RpPatchAtomicGetPatchMesh"); return (RpPatchMesh *)0; }
RwBool RpSplineDestroy(RpSpline *spline) { RW_UNIMPLEMENTED("RpSplineDestroy"); return (RwBool)0; }
RpSpline * RpSplineStreamRead(RwStream *stream) { RW_UNIMPLEMENTED("RpSplineStreamRead"); return (RpSpline *)0; }
RwTexDictionary * RtPITexDictionaryStreamRead(RwStream *stream) { RW_UNIMPLEMENTED("RtPITexDictionaryStreamRead"); return (RwTexDictionary *)0; }
RwInt32 RwFclose(void *file) { RW_UNIMPLEMENTED("RwFclose"); return (RwInt32)0; }
void * RwFopen(const RwChar *name, const RwChar *access) { RW_UNIMPLEMENTED("RwFopen"); return (void *)0; }
RwInt32 RwFseek(void *file, long offset, RwInt32 origin) { RW_UNIMPLEMENTED("RwFseek"); return (RwInt32)0; }
RwInt32 RwFtell(void *file) { RW_UNIMPLEMENTED("RwFtell"); return (RwInt32)0; }
const RwMatrix * RwMatrixQueryRotate(const RwMatrix *matrix, RwV3d *unitAxis, RwReal *angle, RwV3d *center) { RW_UNIMPLEMENTED("RwMatrixQueryRotate"); return (const RwMatrix *)0; }
RwUInt32 RwRGBAToPixel(RwRGBA *colour, RwInt32 rasterFormat) { RW_UNIMPLEMENTED("RwRGBAToPixel"); return (RwUInt32)0; }
RwBool RwResourcesFreeResEntry(RwResEntry *entry) { RW_UNIMPLEMENTED("RwResourcesFreeResEntry"); return (RwBool)0; }
RwUInt32 RwTextureStreamGetSize(const RwTexture *texture) { RW_UNIMPLEMENTED("RwTextureStreamGetSize"); return (RwUInt32)0; }

}
