// D3D8 platform part of RenderWare's core API (the render-state and draw calls the game issues through RW's D3D8 driver).
// The driver takes raw D3D8 values (state ids, handles, vertex buffers), so they stay RwUInt32 / void *, as RW declares them.
#ifndef ZOOSTERS_RW_RWD3D8_H
#define ZOOSTERS_RW_RWD3D8_H

#include "rw/rwcore.h"
#include "rw/d3dtexture8.h"  // slots used: 2 Release, 14 GetLevelDesc, 16 LockRect, 17 UnlockRect
extern "C" {

// The D3D8 device RW created (an IDirect3DDevice8 *), or 0 before the engine is started.
void *RwD3D8GetCurrentD3DDevice(void);
const void *RwD3D8GetCaps(void); // the D3DCAPS8 of the current device
void RwD3D8EngineSetMultiSamplingLevels(RwUInt32 numLevels);
// Tests a bounding box (RwBBox) against the frustum of an RwCamera.
RwBool RwD3D8CameraIsBBoxFullyInsideFrustum(const void *camera, const void *boundingBox);

// State changes (RW caches them and only forwards real changes).
RwBool RwD3D8SetRenderState(RwUInt32 state, RwUInt32 value);
RwBool RwD3D8SetTextureStageState(RwUInt32 stage, RwUInt32 type, RwUInt32 value);
RwBool RwD3D8SetTexture(RwTexture *texture, RwUInt32 stage);
RwBool RwD3D8SetTransform(RwUInt32 state, const void *matrix); // a D3DMATRIX, or 0 for identity
RwBool RwD3D8SetVertexShader(RwUInt32 handle);
RwBool RwD3D8SetPixelShader(RwUInt32 handle);
RwBool RwD3D8SetStreamSource(RwUInt32 streamNumber, void *streamData, RwUInt32 stride);
RwBool RwD3D8SetIndices(void *indexData, RwUInt32 baseVertexIndex);

// Draw calls.
RwBool RwD3D8DrawPrimitive(RwUInt32 primitiveType, RwUInt32 startVertex, RwUInt32 numVertices);
RwBool RwD3D8DrawIndexedPrimitive(RwUInt32 primitiveType, RwUInt32 minIndex, RwUInt32 numVertices, RwUInt32 startIndex,
                                  RwUInt32 numIndices);

// --- D3D8 / D3DX8 that the game calls itself: a few PNG textures (sprites, a font page) are loaded with D3DX and copied into RW
// rasters. Only the interface slots the game uses have parameters; the rest keep d3d8.h's order.
enum { D3DLOCK_READONLY = 0x10 };   // LockRect flags
long __stdcall D3DXCreateTextureFromFileA(void *device, const char *file, D3DTexture8 **tex);   // 0x5274a4

}

#endif
