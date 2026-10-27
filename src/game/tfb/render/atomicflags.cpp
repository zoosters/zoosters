#include "rw/rwcore.h"
#include "rw/rpworld.h"
#include "rw/rpmesh.h"
#include "rw/rpplugins.h"
#include "rw/rwengine.h"
#include "rw/rttoolkits.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "framework/toolkits/animation/skinhelper.h"
#include "framework/toolkits/world/clumphelper.h"
#include "framework/toolkits/world/camerahelper.h"
#include "framework/toolkits/world/lighthelper.h"
#include "tfb/render/tfbatomic.h"
#include "tfb/render/atomicflags.h"
#include "tfb/render/tfbgeometry.h"
#include "framework/toolkits/world/framehelper.h"
#include "tfb/render/atomicflags_views.h"

// byte +2 holds the TFB plugin flag bits
struct AtomicFlags {
	char m_pad00[2];
	unsigned char m_flags;
};
struct AtomicParams {
	bool m_flag;
	float m_from;
	float m_to;
};

__declspec(noinline) RpClump *ClumpSetIsVisible(RpClump *clump, bool flag, float from, float to);
RpAtomic *ClumpSetIsVisibleCB(RpAtomic *atomic, void *data);
__declspec(noinline) void AtomicSetIsVisible(AtomicFlags *a, bool on);
__declspec(noinline) void AtomicSetCanCollide(AtomicFlags *a, bool on);
RpAtomic *SetAtomicFlagsCB(RpAtomic *atomic, int flags);
void AtomicApplyUserDataFlags(RpAtomic *atomic);
void ClumpApplyUserDataFlags(RpClump *clump);   // 0x44eeb0

RpClump *ClumpRemoveFromWorldCB(RpClump *c, void *d);
RpLight *LightRemoveFromWorldCB(RpLight *l, void *d);
RpAtomic *AtomicRemoveFromWorldCB(RpAtomic *a, void *d);
RpWorldSector *WorldSectorRemoveAtomicsCB(RpWorldSector *s, void *d);

// Meshes whose texture name contains "_alpha" sort last.
// GLOBAL: 0x606678 g_alphaTag
extern const char *g_alphaTag;

namespace RWS { namespace MainLoop { namespace Render { RwCamera *GetCamera(); } } }   // 0x462660
#define CAMBUF_SETFB(c, r) RwCameraSetRaster((c), (r))
#define CAMBUF_SETZ(c, r) RwCameraSetZRaster((c), (r))
#define CAMBUF_FB(c) RwCameraGetRaster(c)
#define CAMBUF_Z(c) RwCameraGetZRaster(c)

// 0x44b120
RpAtomic *__stdcall ReadAtomic(RwStream *stream)
{
	RpAtomic *atomic = RpAtomicStreamRead(stream);
	if (atomic)
		AtomicApplyUserDataFlags(atomic);
	return atomic;
}

// 0x44b150
RpClump *__stdcall ReadClump(RwStream *stream)
{
	RpClump *clump = RpClumpStreamRead(stream);
	if (clump)
		ClumpApplyUserDataFlags(clump);
	return clump;
}

// 0x44b330
void *__stdcall ReadPITexDictionary(RwStream *stream)
{
	RwTexDictionary *dict = RtPITexDictionaryStreamRead(stream);
	RwTexDictionarySetCurrent(dict);
	return dict;
}

// 0x44e220
void CClumpPtr_RpClumpDestroy(RpClump **clumpPtr)
{
	RWS::ClumpHelper::Destroy(*clumpPtr);
	*clumpPtr = 0;
}

// 0x44e240
void CLightPtr_RpLightDestroy(RpLight **lightPtr)
{
	RWS::LightHelper::Destroy(*lightPtr);
	*lightPtr = 0;
}

// 0x44e2e0
void AtomicSetIsVisible(AtomicFlags *a, bool on)
{
	int m = a->m_flags;
	if (on)
		m |= 4;
	else
		m &= ~4;
	a->m_flags = m;
}

// 0x44e300
void AtomicSetCanCollide(AtomicFlags *a, bool on)
{
	int m = a->m_flags;
	if (on)
		m |= 1;
	else
		m &= ~1;
	a->m_flags = m;
}

// 0x44eb50
RpAtomic *AtomicRemoveFromWorldCB(RpAtomic *atomic, void *data)
{
	RpWorld *world = RpAtomicGetWorld(atomic);
	if (world)
		RpWorldRemoveAtomic(world, atomic);
	return atomic;
}

// 0x44ec30
RpAtomic *ClumpSetIsVisibleCB(RpAtomic *atomic, void *data)
{
	AtomicParams *p = (AtomicParams *)data;
	if (p->m_flag || p->m_to < 0.0f)
		AtomicSetIsVisible((AtomicFlags *)atomic, p->m_flag);
	if (p->m_from >= p->m_to) {
		TFBPlugin_FD_ClearFlagBits(atomic, 0x200);
		return atomic;
	}
	TFBPlugin_FD_SetFlagBits(atomic, 0x200);
	TFBPlugin_FD_SetRange(atomic, p->m_from, p->m_to);
	return atomic;
}

// 0x44eca0
RpClump *ClumpSetIsVisible(RpClump *clump, bool flag, float from, float to)
{
	AtomicParams p;
	p.m_flag = flag;
	p.m_from = from;
	p.m_to = to;
	return RpClumpForAllAtomics(clump, ClumpSetIsVisibleCB, &p);
}

// 0x44ece0
RpAtomic *ClumpSetCanCollideCB(RpAtomic *a, void *p)
{
	AtomicSetCanCollide((AtomicFlags *)a, p != 0);
	return a;
}

// 0x44ed80
RpAtomic *SetAtomicFlagsCB(RpAtomic *atomic, int flags)
{
	TFBPlugin_FD_SetFlagBits(atomic, flags);
	return atomic;
}

// 0x44eda0
RpClump *ClumpSetAtomicFlags(RpClump *clump, int flags)
{
	return RpClumpForAllAtomics(clump, (RpAtomicCallBack)SetAtomicFlagsCB, (void *)(intptr_t)flags);
}

// 0x44edc0
RpAtomic *ClearAtomicFlagsCB(RpAtomic *atomic, int flags)
{
	TFBPlugin_FD_ClearFlagBits(atomic, flags);
	return atomic;
}

// 0x44ede0
RpClump *ClumpClearAtomicFlags(RpClump *clump, int flags)
{
	return RpClumpForAllAtomics(clump, (RpAtomicCallBack)ClearAtomicFlagsCB, (void *)(intptr_t)flags);
}

// 0x44f1d0
RpClump *ClumpRemoveFromWorldCB(RpClump *clump, void *data)
{
	RpWorld *w = RpClumpGetWorld(clump);
	if (w)
		RpWorldRemoveClump(w, clump);
	return clump;
}

// 0x44f1f0
RpLight *LightRemoveFromWorldCB(RpLight *light, void *data)
{
	RpWorld *w = RpLightGetWorld(light);
	if (w)
		RpWorldRemoveLight(w, light);
	return light;
}

// 0x44dfa0
RpAtomic *DetachAtomicFromClumpCB(RpAtomic *atomic, RpAtomic **out)
{
	RpClumpRemoveAtomic(RpAtomicGetClump(atomic), atomic);
	RpAtomicSetFrame(atomic, 0);
	*out = atomic;
	return 0;
}

// 0x44f190
void *ClumpFindFirstSkin(RpClump *clump)
{
	void *skin = 0;
	struct {
		int (*m_callback)(void *skin, void **out);
		void **m_pData;
	} cbData;
	cbData.m_callback = (int (*)(void *, void **))RWS::SkinHelper::FindFirstSkinCallback;
	cbData.m_pData = &skin;
	RpClumpForAllAtomics(clump, (RpAtomicCallBack)RWS::SkinHelper::ForAllSkinsCallback, &cbData);
	return skin;
}

// 0x44f210
RpWorldSector *WorldSectorRemoveAtomicsCB(RpWorldSector *sector, void *data)
{
	RpWorldSectorForAllAtomics(sector, AtomicRemoveFromWorldCB, data);
	return sector;
}

// 0x44f230
void WorldRemoveAllObjects(RpWorld *world)
{
	if (world) {
		RpWorldForAllClumps(world, ClumpRemoveFromWorldCB, world);
		RpWorldForAllLights(world, LightRemoveFromWorldCB, world);
		RpWorldForAllWorldSectors(world, WorldSectorRemoveAtomicsCB, world);
	}
}

// 0x44e320
int CompareMeshAlphaLast(const RpMesh *a, const RpMesh *b)
{
	bool aAlpha = false;
	bool bAlpha = false;
	if (a && RpMeshGetMaterial(a)) {
		TextureNameBase *tex = (TextureNameView *)RpMaterialGetTexture(RpMeshGetMaterial(a));
		if (tex && strstr(tex->m_name, g_alphaTag))
			aAlpha = true;
	}
	if (b && RpMeshGetMaterial(b)) {
		TextureNameBase *tex = (TextureNameView *)RpMaterialGetTexture(RpMeshGetMaterial(b));
		if (tex && strstr(tex->m_name, g_alphaTag))
			bAlpha = true;
	}
	if (aAlpha == bAlpha)
		return a < b ? -1 : 1;
	return aAlpha ? 1 : -1;
}

// 0x44e8e0
RwCamera *CreateRasterCamera(int width, int height, int depth)
{
	RwCamera *cam = RwCameraCreate();
	CAMBUF_SETFB(cam, RwRasterCreate(width, height, 0, 2));
	CAMBUF_SETZ(cam, RwRasterCreate(width, height, depth, 1));
	RwFrame *frame = RwFrameCreate_thunk();
	RwFrameSetIdentity(frame);
	_rwObjectHasFrameSetFrame(cam, frame);
	RwCameraSetProjection(cam, rwPERSPECTIVE);
	RwCameraSetNearClipPlane(cam, 1.0f);
	RwCameraSetFarClipPlane(cam, 5000.0f);
	if (height) {
		RwV2d vw;
		vw.x = (float)tan(0.5235988f);
		vw.y = vw.x * 0.7f;
		RwCameraSetViewWindow(cam, &vw);
	}
	return cam;
}

namespace RWS {
namespace CameraHelper {

// 0x44e980
RwCamera *CreateSubRasterCamera(RwRect &rect)
{
	RwCamera *main = RWS::MainLoop::Render::GetCamera();
	RwCamera *cam = RwCameraCreate();
	CAMBUF_SETFB(cam, RwRasterCreate(0, 0, 0, 2));
	CAMBUF_SETZ(cam, RwRasterCreate(0, 0, 0, 1));
	RwFrame *frame = RwFrameCreate_thunk();
	RwFrameSetIdentity(frame);
	_rwObjectHasFrameSetFrame(cam, frame);
	RwCameraSetProjection(cam, rwPERSPECTIVE);
	RwCameraSetNearClipPlane(cam, 1.0f);
	RwCameraSetFarClipPlane(cam, 5000.0f);
	RwRasterSubRaster(CAMBUF_FB(cam), CAMBUF_FB(main), &rect);
	RwRasterSubRaster(CAMBUF_Z(cam), CAMBUF_Z(main), &rect);
	RwV2d vw;
	vw.x = (float)tan(0.5235988f);
	vw.y = vw.x * 0.7f;
	RwCameraSetViewWindow(cam, &vw);
	return cam;
}

// 0x44ea40
void SetFieldOfView(float zoom, RwCamera *cam)
{
	struct { RwV2d vw; int pad[3]; int height; RwVideoMode info; } s;   // height: nominal 640x448 screen height
	RwEngineGetVideoModeInfo(&s.info, RwEngineGetCurrentVideoMode());
	s.height = 0x1c0;
	if (s.info.flags & rwVIDEOMODEINTERLACE)
		s.height = 0x380;
	s.vw.x = zoom;
	s.vw.y = (float)s.height * zoom * 0.0015625f;
	RwCameraSetViewWindow(cam, &s.vw);
}

}  // namespace CameraHelper
}  // namespace RWS

// 0x44e3c0
void AtomicApplyUserDataFlags(RpAtomic *atomic)
{
	unsigned flags = TFBPlugin_FD_GetFlags(atomic);
	RpGeometry *geom = RpAtomicGetGeometry(atomic);
	if (geom) {
		if (RpGeometryGetFlags(geom) & rpGEOMETRYPRELIT)
			flags &= ~4u;
		else
			flags |= 4;
	}
	TFBPlugin_FD_SetFlags(atomic, flags);
	RwFrame *frame = RpAtomicGetFrame(atomic);
	if (frame) {
		int n = RwFrameGetUserDataArrayCount(frame);
		for (int i = 0; i < n; i++) {
			RpUserDataArray *arr = RwFrameGetUserDataArray(frame, i);
			const char *name = RpUserDataArrayGetName(arr);
			if (strstr(name, "TRANSFORM_FLAGS") && RpUserDataArrayGetNumElements(arr) > 0 &&
			    RpUserDataArrayGetFormat(arr) == rpINTUSERDATA)
				TFBPlugin_FD_SetTransformFlags(atomic, RpUserDataArrayGetInt(arr, 0));
		}
	}
	if (geom) {
		int n = RpGeometryGetUserDataArrayCount(geom);
		for (int i = 0; i < n; i++) {
			RpUserDataArray *arr = RpGeometryGetUserDataArray(geom, i);
			const char *name = RpUserDataArrayGetName(arr);
			if (strstr(name, "TFB_MESH.TRANSLUCENT") && RpUserDataArrayGetNumElements(arr) > 0 &&
			    RpUserDataArrayGetFormat(arr) == rpINTUSERDATA && (unsigned)RpUserDataArrayGetInt(arr, 0) > 0)
				TFBPlugin_FD_SetFlagBits(atomic, 8);
			if (strstr(name, "TFB_NOZWRITE.NOZWRITE") && RpUserDataArrayGetNumElements(arr) > 0 &&
			    RpUserDataArrayGetFormat(arr) == rpINTUSERDATA && (unsigned)RpUserDataArrayGetInt(arr, 0) > 0)
				TFBPlugin_FD_SetFlagBits(atomic, 0x8000);
		}
		if (!TFBPlugin_E1_TestFlags(geom, 8)) {
			RpMeshHeader *header = RpGeometryGetMeshHeader(geom);
			qsort((char *)header + header->firstMeshOffset + 0x10, header->numMeshes, rwSize_RpMeshBody,
			      (int (*)(const void *, const void *))CompareMeshAlphaLast);
			TFBPlugin_E1_SetFlagBits(geom, 8);
		}
	}
	RpGeometry *g2 = RpAtomicGetGeometry(atomic);
	if (g2) {
		int n = RpGeometryGetNumMaterials(g2);
		for (int i = 0; i < n; i++) {
			RpMaterial *mat = RpGeometryGetMaterial(g2, i);
			if (mat && RpMaterialGetTexture(mat) && !_stricmp(RwTextureGetName(RpMaterialGetTexture(mat)), "ivycenter")) {
				TFBPlugin_FD_SetFlagBits(atomic, 8);
				TFBPlugin_FD_SetFlagBits(atomic, 0x1000);
			}
		}
	}
}
