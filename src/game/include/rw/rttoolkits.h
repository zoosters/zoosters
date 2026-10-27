// RenderWare toolkits besides animation: Rt2d, RtFS (with the Rw file functions that belong to it), box/triangle intersection,
// platform-independent texture dictionaries. All opaque.
#ifndef ZOOSTERS_RW_RTTOOLKITS_H
#define ZOOSTERS_RW_RTTOOLKITS_H

#include "rw/rwcore.h"
extern "C" {

// --- Rt2d: 2D vector graphics (fonts, brushes, a transform stack), drawn through a camera
struct Rt2dBrush;
struct Rt2dFont;
void Rt2dOpen(RwCamera *camera);
void Rt2dClose(void);
RwCamera *Rt2dDeviceGetCamera(void);
RwBool Rt2dDeviceSetCamera(RwCamera *camera);
RwBool Rt2dDeviceSetLayerDepth(RwReal depth);
RwBool Rt2dDeviceSetMetric(RwReal x, RwReal y, RwReal width, RwReal height);
RwBool Rt2dCTMPush(void);
RwBool Rt2dCTMPop(void);
RwBool Rt2dCTMTranslate(RwReal x, RwReal y);
RwBool Rt2dCTMScale(RwReal x, RwReal y);
RwReal *_rt2dCTMGet(void);   // 0x467bf0 (RW 3.6 internal): the current transformation matrix, 16 floats in RwMatrix layout
RwBool Rt2dCTMRotate(RwReal angle);
Rt2dBrush *Rt2dBrushCreate(void);
RwBool Rt2dBrushDestroy(Rt2dBrush *brush);
Rt2dBrush *Rt2dBrushSetRGBA(Rt2dBrush *brush, RwRGBA *colour0, RwRGBA *colour1, RwRGBA *colour2, RwRGBA *colour3);
Rt2dFont *Rt2dFontStreamRead(RwStream *stream);
RwBool Rt2dFontDestroy(Rt2dFont *font);
RwTexDictionary *Rt2dFontTexDictionarySet(RwTexDictionary *texDict); // the dictionary fonts look their textures up in
RwReal Rt2dFontGetHeight(Rt2dFont *font);
RwReal Rt2dFontGetStringWidth(Rt2dFont *font, const RwChar *string, RwReal height);
Rt2dFont *Rt2dFontShow(Rt2dFont *font, const RwChar *string, RwReal height, RwV2d *anchor, Rt2dBrush *brush);

// --- RtFS: file system manager, and the RW file functions that go through it
struct RtFileSystem;
RwBool RtFSManagerOpen(RwInt32 maxNumFileSystems);
void RtFSManagerClose(void);
RwBool RtFSManagerRegister(RtFileSystem *fileSystem);
void RtFSManagerSetDefaultFileSystem(RtFileSystem *fileSystem);
void RtFSManagerSyncAllFiles(void);
RtFileSystem *RtWinFSystemInit(RwInt32 maxNumOpenFiles, RwChar *deviceName, RwChar *fileSystemName);
void *RwFopen(const RwChar *name, const RwChar *access);
RwInt32 RwFclose(void *file);
RwInt32 RwFeof(void *file);
RwBool RwFexist(const RwChar *name);
RwInt32 RwFseek(void *file, long offset, RwInt32 origin);
RwInt32 RwFtell(void *file);

// --- RtIntersection / RtPITexDict
RwBool RtIntersectionBBoxTriangle(const RwBBox *bbox, const RwV3d *vertex0, const RwV3d *vertex1, const RwV3d *vertex2);
RwTexDictionary *RtPITexDictionaryStreamRead(RwStream *stream);

}

#endif
