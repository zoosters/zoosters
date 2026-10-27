// Views of the RenderWare objects the camera code reads field by field (RW 3.6.0.2, RW's member names). include/rw keeps the types opaque:
// cast to a view only inside the camera module.
#ifndef ZOOSTERS_MODULES_TFB_CAMERA_VIEWS_H
#define ZOOSTERS_MODULES_TFB_CAMERA_VIEWS_H

#include "zoosters/base.h"
#include "rw/rwcore.h"
#include "rw/rwviewbase.h"
#include "rw/rpworld.h"
#include "rw/rphanim.h"
#include "rw/rtanim.h"

struct RwRaster;
struct RwTexture;
struct RpAtomic;

// The type / format bytes are what ShadowRender::CreateRasters copies into the sub raster it creates.
struct RwRasterView {
	RwRaster *parent;       // +0x00
	RwUInt8 *pixels;        // +0x04 cpPixels
	RwUInt8 *palette;       // +0x08
	RwInt32 width;          // +0x0c
	RwInt32 height;         // +0x10
	RwInt32 depth;          // +0x14
	RwInt32 stride;         // BINK frames are copied with this pitch
	char pad1c[0x04];       // +0x1c nOffsetX, nOffsetY
	RwUInt8 type;           // +0x20 cType
	RwUInt8 flags;          // +0x21 cFlags
	RwUInt8 privateFlags;   // +0x22
	RwUInt8 format;         // +0x23 cFormat (RwRasterCreate takes it << 8 in its flags)
};
ZOOSTERS_ASSERT_OFFSET(RwRasterView, depth, 0x14);
ZOOSTERS_ASSERT_OFFSET(RwRasterView, stride, 0x18);
ZOOSTERS_ASSERT_OFFSET(RwRasterView, type, 0x20);
ZOOSTERS_ASSERT_OFFSET(RwRasterView, format, 0x23);

// The raster is the first member; filterAddressing is at +0x50 / +0x51 (filter, then addressU | addressV << 4: 2 = linear, 0x33 = clamp / clamp).
struct RwTextureView {
	RwRaster *raster;  // +0
	void *dictionary;  // +0x04
	RwLLLink link;     // +0x08 lInDictionary
	char name[32];     // +0x10
	char mask[32];     // +0x30
	RwUInt8 filter;    // +0x50
	RwUInt8 addressing; // +0x51
};
ZOOSTERS_ASSERT_OFFSET(RwTextureView, filter, 0x50);

static inline RwRasterView *RasterView(RwRaster *raster)
{
	return (RwRasterView *)raster;
}

static inline RwTextureView *TextureView(RwTexture *texture)
{
	return (RwTextureView *)texture;
}

struct RwFrustumPlaneView {
	RwV3d normal;        // +0x00
	RwReal distance;     // +0x0c
	uint8_t closestX;    // +0x10
	uint8_t closestY;    // +0x11
	uint8_t closestZ;    // +0x12
	uint8_t pad13;       // +0x13
};
ZOOSTERS_ASSERT_SIZE(RwFrustumPlaneView, 0x14);

// Names from RW's RwCamera (3.7; the layout agrees with every offset the game uses). Members only copied as words are typed as words so the
// copies stay word moves.
struct RwCameraView {
	RwUInt8 type, subType, flags, privateFlags;   // +0x00 RwObject header
	RwFrame *frame;                      // +0x04 the camera's frame (RwObject::parent)
	RwLLLink inFrame;                    // +0x08
	void *sync;                          // +0x10
	RwInt32 projection;                  // +0x14 (copied)
	void *beginUpdate;                   // +0x18 (copied)
	void *endUpdate;                     // +0x1c (copied)
	RwMatrix viewMatrix;                 // +0x20
	RwRaster *frameBuffer;               // +0x60
	RwRaster *zBuffer;                   // +0x64
	RwV2d viewWindow;                    // +0x68
	RwV2d recipViewWindow;               // +0x70
	RwV2d viewOffset;                    // +0x78
	RwReal nearPlane;                    // +0x80
	RwReal farPlane;                     // +0x84
	RwReal fogPlane;                     // +0x88 (copied)
	RwReal zScale;                       // +0x8c (copied)
	RwReal zShift;                       // +0x90 (copied)
	RwFrustumPlaneView frustumPlanes[6]; // +0x94
};
ZOOSTERS_ASSERT_OFFSET(RwCameraView, frame, 0x04);
ZOOSTERS_ASSERT_OFFSET(RwCameraView, projection, 0x14);
ZOOSTERS_ASSERT_OFFSET(RwCameraView, viewMatrix, 0x20);
ZOOSTERS_ASSERT_OFFSET(RwCameraView, frameBuffer, 0x60);
ZOOSTERS_ASSERT_OFFSET(RwCameraView, viewWindow, 0x68);
ZOOSTERS_ASSERT_OFFSET(RwCameraView, viewOffset, 0x78);
ZOOSTERS_ASSERT_OFFSET(RwCameraView, nearPlane, 0x80);
ZOOSTERS_ASSERT_OFFSET(RwCameraView, zShift, 0x90);
ZOOSTERS_ASSERT_OFFSET(RwCameraView, frustumPlanes, 0x94);

// GLOBAL: 0x618238 g_camBlk
extern RwFrustumPlaneView g_camBlk[6]; // copy of the camera's frustum planes, read by the view-pyramid measurement

static inline RwCameraView *CameraView(RwCamera *camera)
{
	return (RwCameraView *)camera;
}

static inline const RwCameraView *CameraView(const RwCamera *camera)
{
	return (const RwCameraView *)camera;
}

struct RpAtomicView {
	RwObjectHasFrameView object;          // +0x00
	void *repEntry;                       // +0x14
	void *geometry;                       // +0x18
	RwSphereView boundingSphere;          // +0x1c
	RwSphereView worldBoundingSphere;     // +0x2c
	RpClump *clump;                       // +0x3c
};
ZOOSTERS_ASSERT_OFFSET(RpAtomicView, clump, 0x3c);

// Accessors for the partial RtAnim / RpHAnim structs of rw/rtanim.h and rw/rphanim.h.
static inline RwReal AnimationDuration(const RtAnimAnimation *animation)
{
	return animation->duration;
}

static inline RtAnimInterpolator *HierarchyInterpolator(RpHAnimHierarchy *hierarchy)
{
	return hierarchy->currentAnim;
}

static inline RwMatrix *HierarchyMatrices(RpHAnimHierarchy *hierarchy)
{
	return hierarchy->pMatrixArray;
}

static inline RwReal InterpolatorTime(RtAnimInterpolator *interpolator)
{
	return interpolator->currentTime;
}

#endif
